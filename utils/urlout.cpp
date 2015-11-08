#include <ucommon/secure.h>
#include <sys/stat.h>

using namespace ucommon;

static shell::flagopt helpflag('h',"--help",    _TEXT("display this list"));
static shell::flagopt althelp('?', NULL, NULL);
static shell::flagopt reqcert('C', NULL, _TEXT("requires certificate"));
static shell::flagopt verified('V', NULL, _TEXT("requires verification"));
static shell::numericopt port('p', "--port", _TEXT("port to use"), "port", 0);

int main(int argc, char **argv)
{
    shell::bind("urlout");
    shell args(argc, argv);

    if(is(helpflag) || is(althelp) || !args() || args() > 1) {
        printf("%s\n", _TEXT("Usage: urlout [options] url-path"));
        printf("\n%s\n", _TEXT("Options:"));
        shell::help();
        printf("\n%s\n", _TEXT("Report bugs to dyfet@gnu.org"));
        return 0;
    }

    bool url_secure = false;
    const char *url = args[0];
    const char *proto = "80";
    secure::client_t ctx = NULL;
    String svc;
    
    if(eq(url, "https://", 8)) {
        url_secure = true;
        url += 8;
    }
    else if(eq(url, "http://", 7)) {
        url += 7;
    }
    else
        url_secure = true;

    string_t host = url;
    string_t path = strdup(url);

    if(url_secure && secure::init()) {
        proto = "443";
        ctx = secure::client();
        if(ctx->err() != secure::OK) 
            shell::errexit(2, _TEXT("%s: no certificates found"), argv[0]);
    }

    if(is(port))
        svc = str(*port);
    else
        svc = proto;

    const char *find = path.find("/");
    if(find)
        path.rsplit(find);
    else
        path = "/";

    host.split(host.find("/"));

    sstream web(ctx);
    web.open(host, svc);

    if(!web.is_open())
        shell::errexit(1, _TEXT("%s: failed to access"), url); 

    if(is(verified) && !web.is_verified())
        shell::errexit(8, _TEXT("%s: unverified host"), url);

    if(is(reqcert) && !web.is_certificate())
        shell::errexit(9, _TEXT("%s: no certificate"), url);

    web << "GET /\r\n\r\n";
    web.flush();

    std::cout << web.rdbuf();
}
