#include <ucommon/secure.h>
#include <sys/stat.h>

using namespace ucommon;

static shell::flagopt helpflag('h',"--help",    _TEXT("display this list"));
static shell::flagopt althelp('?', NULL, NULL);
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

    char *path = strdup(url);
    char *host = strdup(url);

    if(url_secure && secure::init()) {
        proto = "443";
        ctx = secure::client();
    }

    if(is(port))
        svc = str(*port);
    else
        svc = proto;

    char *sep = strchr(path, '/');
    if(sep)
        path = sep;
    else
        path = (char *)"/";

    sep = strchr(host, '/');
    if(sep)
        *sep = 0;

    sstream web(ctx);
    web.open(host, *svc);

    web << "GET /\r\n\r\n";
    web.flush();

    std::cout << web.rdbuf();
}
