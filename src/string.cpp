#include <config.h>
#include <ucommon/string.h>
#include <stdarg.h>
#include <ctype.h>
#include <stdio.h>

using namespace UCOMMON_NAMESPACE;

const strsize_t string::npos = (strsize_t)(-1);
const size_t memstring::header = sizeof(cstring);

string::cstring::cstring(strsize_t size)
{
	max = size;
	len = 0;
	fill = 0;
	text[0] = 0;
}

string::cstring::cstring(strsize_t size, char f)
{
	max = size;
	len = 0;
	fill = f;

	if(fill) {
		memset(text, fill, max);
		len = max;
	}
}

void string::cstring::fix(void)
{
	while(fill && len < max)
		text[len++] = fill;
	text[len] = 0;
}

void string::cstring::unfix(void)
{
	while(len && fill) {
		if(text[len - 1] == fill)
			--len;
	}
	text[len] = 0;
}

void string::cstring::clear(strsize_t offset, strsize_t size)
{
	if(!fill || offset >= max)
		return;

	while(size && offset < max) {
		text[offset++] = fill;
		--size;
	}
}

void string::cstring::dec(strsize_t offset)
{
	if(!len)
		return;

	if(offset >= len) {
		text[0] = 0;
		len = 0;
		fix();
		return;
	}

	if(!fill) {
		text[--len] = 0;
		return;
	}

	while(len) {
		if(text[--len] == fill)
			break;
	}
	text[len] = 0;
	fix();
}

void string::cstring::inc(strsize_t offset)
{
	if(!offset)
		++offset;

	if(offset >= len) {
		text[0] = 0;
		len = 0;
		fix();
		return;
	}

	memmove(text, text + offset, len - offset);
	len -= offset;
	fix();
}

void string::cstring::add(const char *str)
{
	strsize_t size = strlen(str);

	if(!size)
		return;

	while(fill && len && text[len - 1] == fill)
		--len;
	
	if(len + size > max)
		size = max - len;

	if(size < 1)
		return;

	memcpy(text + len, str, size);
	len += size;
	fix();
}

void string::cstring::add(char ch)
{
	if(!ch)
		return;

	while(fill && len && text[len - 1] == fill)
		--len;

	if(len == max)
		return;

	text[len++] = ch;	
	fix();
}

void string::cstring::set(strsize_t offset, const char *str, strsize_t size)
{
	if(offset >= max || offset > len)
		return;

	if(offset + size > max)
		size = max - offset;
 
	while(*str && size) {
		text[offset++] = *(str++);
		--size;
	}

	while(size && fill) {
		text[offset++] = fill;
		--size;
	}

	if(offset > len) {
		len = offset;
		text[len] = 0;
	}
}

char string::fill(void)
{
	if(!str)
		return 0;

	return str->fill;
}

strsize_t string::size(void) const
{
    if(!str)
        return 0;

    return str->max;
}

strsize_t string::len(void)
{
	if(!str)
		return 0;

	return str->len;
}

void string::cstring::set(const char *str)
{
	strsize_t size = strlen(str);
	if(size > max)
		size = max;
	
	if(str < text || str > text + len)
		memcpy(text, str, size);
	else if(str != text)
		memmove(text, str, size);
	len = size;
	fix();
}		

string::string()
{
	str = NULL;
}

string::string(const char *s, const char *end)
{
	strsize_t size = 0;

	if(!s)
		s = "";
	else if(!end)
		size = strlen(s);
	else if(end > s)
		size = (strsize_t)(end - s);
	str = create(size);
	str->retain();
	str->set(s);
}		

string::string(const char *s)
{
	strsize_t size = count(s);
	if(!s)
		s = "";
	str = create(size);
	str->retain();
	str->set(s);
}

string::string(const char *s, strsize_t size)
{
	if(!s)
		s = "";
	if(!size)
		size = strlen(s);
	str = create(size);
	str->retain();
	str->set(s);
}

string::string(strsize_t size) 
{
	str = create(size);
	str->retain();
};

string::string(strsize_t size, char fill)
{
    str = create(size, fill);
    str->retain();
};

string::string(strsize_t size, const char *format, ...)
{
	va_list args;
	va_start(args, format);

	str = create(size);
	str->retain();
	vsnprintf(str->text, size + 1, format, args);
	va_end(args);
}

string::string(const string &dup)
{
	str = dup.c_copy();
	if(str)
		str->retain();
}

string::~string()
{
	string::release();
}

string::cstring *string::c_copy(void) const
{
	return str;
}

string string::get(strsize_t offset, strsize_t len) const
{
	if(!str || offset >= str->len)
		return string("");

	if(!len)
		len = str->len - offset;
	return string(len, str->text + offset);
}

string::cstring *string::create(strsize_t size, char fill) const
{
	if(fill)
		return new((size_t)size) cstring(size, fill);
	else
		return new((size_t)size) cstring(size);
}

void string::retain(void)
{
	if(str)
		str->retain();
}

void string::release(void)
{
	if(str)
		str->release();
	str = NULL;
}

char *string::c_mem(void) const
{
	if(!str)
		return NULL;

	return str->text;
}

const char *string::c_str(void) const
{
	if(!str)
		return "";

	return str->text;
}

int string::compare(const char *s) const
{
	const char *mystr = "";

	if(str)
		mystr = str->text;

	if(!s)
		s = "";

	return strcmp(mystr, s);
}

const char *string::last(const char *clist) const
{
	if(!str)
		return NULL;

	return last(str->text, clist);
}

const char *string::first(const char *clist) const
{
	if(!str)
		return NULL;

	return first(str->text, clist);
}

const char *string::begin(void) const
{
	if(!str)
		return NULL;

	return str->text;
}

const char *string::end(void) const
{
	if(!str)
		return NULL;

	return str->text + str->len;
}

const char *string::chr(char ch) const
{
	if(!str)
		return NULL;

	return ::strchr(str->text, ch);
}

const char *string::rchr(char ch) const
{
    if(!str)
        return NULL;

    return ::strrchr(str->text, ch);
}

const char *string::skip(const char *clist, strsize_t offset) const
{
	if(!str || !clist || !*clist || !str->len || offset > str->len)
		return NULL;

	while(offset < str->len) {
		if(!strchr(clist, str->text[offset]))
			return str->text + offset;
		++offset;
	}
	return NULL;
}

const char *string::rskip(const char *clist, strsize_t offset) const
{
	if(!str || !clist || !*clist)
		return NULL;

	if(!str->len)
		return NULL;

	if(offset > str->len)
		offset = str->len;

	while(offset--) {
		if(!strchr(clist, str->text[offset]))
			return str->text + offset;
	}
	return NULL;
}

const char *string::rfind(const char *clist, strsize_t offset) const
{
	if(!str || !clist || !*clist)
		return NULL;

	if(!str->len)
		return str->text;

	if(offset > str->len)
		offset = str->len;

	while(offset--) {
		if(strchr(clist, str->text[offset]))
			return str->text + offset;
	}
	return NULL;
}

void string::chop(const char *clist)
{
	strsize_t offset;

	if(!str)
		return;

	if(!str->len)
		return;

	offset = str->len;
	while(offset) {
		if(!strchr(clist, str->text[offset - 1]))
			break;
		--offset;
	}

	if(!offset) {
		clear();
		return;
	}

	if(offset == str->len)
		return;

	str->len = offset;
	str->fix();
}

void string::strip(const char *clist)
{
	trim(clist);
	chop(clist);
}

void string::trim(const char *clist)
{
	unsigned offset = 0;

	if(!str)
		return;

	while(offset < str->len) {
		if(!strchr(clist, str->text[offset]))
			break;
		++offset;
	}

	if(!offset)
		return;

	if(offset == str->len) {
		clear();
		return;
	}

	memmove(str->text, str->text + offset, str->len - offset);
	str->len -= offset;
	str->fix();
}


const char *string::find(const char *clist, strsize_t offset) const
{
	if(!str || !clist || !*clist || !str->len || offset > str->len)
		return NULL;

	while(offset < str->len) {
		if(strchr(clist, str->text[offset]))
			return str->text + offset;
		++offset;
	}
	return NULL;
}

bool string::unquote(const char *clist)
{
	char *s;

	if(!str)
		return false;

	str->unfix();
	s = unquote(str->text, clist);
	if(!s) {
		str->fix();
		return false;
	}

	set(s);
	return true;
}

void string::upper(void)
{
	if(str)
		upper(str->text);
}

void string::lower(void)
{
	if(str)
		lower(str->text);
}

strsize_t string::offset(const char *s) const
{
	if(!str || !s)
		return npos;

	if(s < str->text || s > str->text + str->max)
		return npos;

	if(s - str->text > str->len)
		return str->len;

	return (strsize_t)(s - str->text);
}

strsize_t string::count(void) const
{
	if(!str)
		return 0;
	return str->len;
}

strsize_t string::ccount(const char *clist) const
{
	if(!str)
		return 0;

	return ccount(str->text, clist);
}

void string::printf(const char *format, ...)
{
	va_list args;
	va_start(args, format);
	if(str) {
		vsnprintf(str->text, str->max + 1, format, args);
		str->len = strlen(str->text);
		str->fix();
	}
	va_end(args);
}

void string::rsplit(const char *s)
{
	if(!str || !s || s <= str->text || s > str->text + str->len)
		return;

	str->set(s);
}

void string::rsplit(strsize_t pos)
{
	if(!str || pos > str->len || !pos)
		return;

	str->set(str->text + pos);
}

void string::split(const char *s)
{
	unsigned pos;

	if(!s || !*s || !str)
		return;

	if(s < str->text || s >= str->text + str->len)
		return;

	pos = s - str->text;
	str->text[pos] = 0;
	str->fix();
}

void string::split(strsize_t pos)
{
	if(!str || pos >= str->len)
		return;

	str->text[pos] = 0;
	str->fix();
}

void string::set(strsize_t offset, const char *s, strsize_t size)
{
	if(!s || !*s || !str)
		return;

	if(!size)
		size = strlen(s);

	str->set(offset, s, size);
}

void string::set(const char *s, char overflow, strsize_t offset, strsize_t size)
{
	size_t len = count(s);

	if(!s || !*s || !str)
		return;
	
	if(offset >= str->max)
		return;

	if(!size || size > str->max - offset)
		size = str->max - offset;

	if(len <= size) {
		set(offset, s, size);
		return;
	}

	set(offset, s, size);

	if(len > size && overflow)
		str->text[offset + size - 1] = overflow;
}

void string::rset(const char *s, char overflow, strsize_t offset, strsize_t size)
{
	size_t len = count(s);
	strsize_t dif;

	if(!s || !*s || !str)
		return;
	
	if(offset >= str->max)
		return;

	if(!size || size > str->max - offset)
		size = str->max - offset;

	dif = len;
	while(dif < size && str->fill) {
		str->text[offset++] = str->fill;
		++dif;
	}

	if(len > size)
		s += len - size;
	set(offset, s, size);
	if(overflow && len > size)
		str->text[offset] = overflow;
}

void string::set(const char *s)
{
	strsize_t len;

	if(!s)
		s = "";

	if(!str) {
		len = strlen(s);
		str = create(len);
		str->retain();
	}

	str->set(s);
}

void string::cut(strsize_t offset, strsize_t size)
{
	if(!str || offset >= str->len)
		return;

	if(!size)
		size = str->len;

	if(offset + size >= str->len) {
		str->len = offset;
		str->fix();
		return;
	}

	memmove(str->text + offset, str->text + offset + size, str->len - offset - size);
	str->len -= size;
	str->fix();
}

bool string::resize(strsize_t size)
{
	char fill;

	if(!size) {
		release();
		str = NULL;
		return true;
	}

	if(str->isCopied() || str->max < size) {
		fill = str->fill;
		str->release();
		str = create(size, fill);
		str->retain();
	}
	return true;
}

void string::clear(strsize_t offset, strsize_t size)
{
	if(!str)
		return;

	if(!size)
		size = str->max;

	str->clear(offset, size);
}

void string::clear(void)
{
	if(str)
		str->set("");
}

void string::cow(strsize_t size)
{
	if(str) {
		if(str->fill)
			size = str->max;
		else
			size += str->len;
	}

	if(!size)
		return;

	if(!str || !str->max || str->isCopied() || size > str->max) {
		cstring *s = create(size);
		s->len = str->len;
		strcpy(s->text, str->text);
		s->retain();
		str->release();
		str = s;
	}
}

void string::add(const char *s)
{
	if(!s || !*s)
		return;

	if(!str) {
		set(s);
		return;
	}

	str->add(s);
}

char string::at(int offset) const
{
	if(!str)
		return 0;

	if(offset >= (int)str->len)
		return 0;

	if(offset > -1)
		return str->text[offset];

	if((strsize_t)(-offset) >= str->len)
		return str->text[0];

	return str->text[(int)(str->len) + offset];
}

string string::operator()(int offset, strsize_t len) const
{
	const char *cp = operator()(offset);
	if(!cp)
		cp = "";

	if(!len)
		len = strlen(cp);

	return string(cp, len);
}

const char *string::operator()(int offset) const
{
	if(!str)
		return NULL;

	if(offset >= (int)str->len)
		return NULL;

	if(offset > -1)
		return str->text + offset;

	if((strsize_t)(-offset) >= str->len)
		return str->text;

	return str->text + str->len + offset;
}



const char string::operator[](int offset) const
{
	if(!str)
		return 0;

	if(offset >= (int)str->len)
		return 0;

	if(offset > -1)
		return str->text[offset];

	if((strsize_t)(-offset) >= str->len)
		return *str->text;

	return str->text[str->len + offset];
}

string::operator strsize_t() const
{
	if(str)
		return str->len;
	return 0;
}

string &string::operator++()
{
	if(str)
		str->inc(1);
	return *this;
}

string &string::operator--()
{
    if(str)
        str->dec(1);
    return *this;
}

string &string::operator<<(::DIR *dp)
{
	dirent *d;

	if(!str || !dp)
		return *this;

	d = readdir(dp);
	if(d)
		set(d->d_name);
	else
		set("");

	return *this;
}	

string &string::operator<<(::FILE *fp)
{
	if(!str || !fp)
		return *this;

	::fgets(str->text, str->max, fp);
	str->len = strlen(str->text);
	if(!str->fill)
		return *this;

	if(str->len && str->text[str->len - 1] == '\n')
		--str->len;

	if(str->len && str->text[str->len - 1] == '\r')
		--str->len;
	str->fix();
	return *this;
}

string &string::operator>>(::FILE *fp)
{
	if(!str || !fp)
		return *this;

	fputs(str->text, fp);
	return *this;
}

string &string::operator-=(strsize_t offset)
{
    if(str)
        str->dec(offset);
    return *this;
}

string &string::operator+=(strsize_t offset)
{
    if(str)
        str->inc(offset);
    return *this;
}

string &string::operator^=(const char *s)
{
	release();
	set(s);
	return *this;
}

string &string::operator=(const char *s)
{
	set(s);
	return *this;
}

string &string::operator^=(const string &s)
{
	release();
	set(s.c_str());
	return *this;
}	

string &string::operator=(const string &s)
{
	if(str == s.str)
		return *this;

	if(s.str)
		s.str->retain();
	
	if(str)
		str->release();

	str = s.str;
	return *this;
}

bool string::full(void) const
{
	if(!str)
		return false;

	if(str->len == str->max && 
	   str->text[str->len - 1] != str->fill)
		return true;
	
	return false;
}

bool string::operator!() const
{
	bool rtn = false;
	if(!str)
		return true;

	str->unfix();
	if(!str->len)
		rtn = true;

	str->fix();
	return rtn;
}

string::operator bool() const
{
	bool rtn = false;

	if(!str)
		return false;

	str->unfix();
	if(str->len)
		rtn = true;
	str->fix();
	return rtn;
}

bool string::operator==(const char *s) const
{
	return (compare(s) == 0);
}

bool string::operator!=(const char *s) const
{
    return (compare(s) != 0);
}

bool string::operator<(const char *s) const
{
	return (compare(s) < 0);
}

bool string::operator<=(const char *s) const
{
    return (compare(s) <= 0);
}

bool string::operator>(const char *s) const
{
    return (compare(s) > 0);
}

bool string::operator>=(const char *s) const
{
    return (compare(s) >= 0);
}

string &string::operator&(const char *s)
{
	add(s);
	return *this;
}

string &string::operator+(const char *s)
{
	if(!s || !*s)
		return *this;

	cow(strlen(s));
	add(s);
	return *this;
}

memstring::memstring(void *mem, strsize_t size, char fill)
{
	str = new((caddr_t)mem) cstring(size, fill);
	str->set("");
}

memstring::~memstring()
{
	str = NULL;
}

memstring *memstring::create(strsize_t size, char fill)
{
	caddr_t mem = (caddr_t)cpr_memalloc(size + sizeof(memstring) + sizeof(cstring));
	return new(mem) memstring(mem + sizeof(memstring), size, fill);
}

memstring *memstring::create(mempager *pager, strsize_t size, char fill)
{
	caddr_t mem = (caddr_t)pager->alloc(size + sizeof(memstring) + sizeof(cstring));
	return new(mem) memstring(mem + sizeof(memstring), size, fill);
}

void memstring::release(void)
{
	str = NULL;
}

bool memstring::resize(strsize_t size)
{
	return false;
}

void memstring::cow(strsize_t adj)
{
}

string::cstring *memstring::c_copy(void) const
{
	cstring *tmp = string::create(str->max, str->fill);
	tmp->set(str->text);
	return tmp;
}
	
tokenstring::tokenstring(const char *cl) :
string()
{
	clist = cl;
	token = NULL;
}

tokenstring::tokenstring(caddr_t mem, const char *cl) :
string()
{
	clist = cl;
	token = trim(mem, cl);
}

const char *tokenstring::get(void)
{
	const char *mem;

	if(!token && !str)
		return NULL;

	if(!token) {
		cow();
		chop(clist);
		token = str->text;
	}
	else if(!*token)
		return NULL;

	while(*token && strchr(clist, *token))
		++token;

	mem = token;	
	while(*token && !strchr(clist, *token))
		++token;

	if(*token)
		*(token++) = 0;

	return mem;
}

const char *tokenstring::get(const char *q)
{
	char *mem;

	if(!token && !str)
		return NULL;

	if(!token) {
		cow();
		chop(clist);
		token = str->text;
	}
	else if(!*token)
		return NULL;

	while(*token && strchr(clist, *token))
		++token;

	mem = token;	
	
	if(strchr(q, *mem)) {
		++token;
		switch(*mem) {
		case '{':
			*mem = '}';
			break;
		case '(':
			*mem = ')';
			break;
		case '[':
			*mem = ']';
			break;
		case '<':
			*mem = '>';
			break;
		}
		while(*token && *token != *mem)
			++token;
		++mem;
	}
	else while(*token && !strchr(clist, *token))
		++token;

	if(*token)
		*(token++) = 0;

	return mem;
}


const char *tokenstring::next(void)
{
	const char *p = token;

	if(!p && str) {
		cow();
		chop(clist);
		p = str->text;
	}
	else if(!p)
		return NULL;

	while(*p && strchr(clist, *p))
		++p;

	return p;
}

tokenstring &tokenstring::operator=(caddr_t s)
{
	token = NULL;
	if(str)
		set(s);
	else
		token = trim(s, clist);
	return *this;
}

tokenstring &tokenstring::operator=(const string &s)
{
	set(s.c_str());
	token = NULL;
	return *this;
}

void string::swap(string &s1, string &s2)
{
	string::cstring *s = s1.str;
	s1.str = s2.str;
	s2.str = s;
}

char *string::dup(const char *cp)
{
	char *mem;

	if(!cp)
		return NULL;

	mem = (char *)malloc(strlen(cp) + 1);
	crit(mem != NULL);
	strcpy(mem, cp);
	return mem;
}	

size_t string::count(const char *cp)
{
	if(!cp)
		return 0;

	return strlen(cp);
}
	
char *string::set(char *str, size_t size, const char *s, size_t len)
{
	if(!str)
		return NULL;

	if(size < 2)
		return str;

	if(!s)
		s = "";

	size_t l = strlen(s);
	if(l >= size)
		l = size - 1;

	if(l > len)
		l = len;

	if(!l) {
		*str = 0;
		return str;
	}

	memmove(str, s, l);
	str[l] = 0;
	return str;
}

char *string::rset(char *str, size_t size, const char *s)
{
	size_t len = count(s);
	if(len > size) 
		s += len - size;
	return set(str, size, s);
}

char *string::set(char *str, size_t size, const char *s)
{
	if(!str)
		return NULL;

	if(size < 2)
		return str;

	if(!s)
		s = "";

	size_t l = strlen(s);

	if(l >= size)
		l = size - 1;

	if(!l) {
		*str = 0;
		return str;
	}

	memmove(str, s, l);
	str[l] = 0;
	return str;
}

char *string::add(char *str, size_t size, const char *s, size_t len)
{
    if(!str)
        return NULL;

    if(!s)
        return str;

    size_t l = strlen(s);
    size_t o = strlen(str);

	if(o >= (size - 1))
		return str;
	set(str + o, size - o, s, l);
	return str;
}

char *string::add(char *str, size_t size, const char *s)
{
	if(!str)
		return NULL;

	if(!s)
		return str;

	size_t o = strlen(str);

	if(o >= (size - 1))
		return str;

	set(str + o, size - o, s);
	return str;
}

char *string::trim(char *str, const char *clist)
{
	if(!str)
		return NULL;

	if(!clist)
		return str;

	while(*str && strchr(clist, *str))
		++str;

	return str;
}

char *string::chop(char *str, const char *clist)
{
	if(!str)
		return NULL;

	if(!clist)
		return str;

	size_t offset = strlen(str);
	while(offset && strchr(clist, str[offset - 1])) {
		*(--str) = 0;
		--offset;
	}
	return str;
}

char *string::strip(char *str, const char *clist)
{
	str = trim(str, clist);
	return chop(str, clist);
}

void string::upper(char *str)
{
	while(str && *str) {
		*str = toupper(*str);
		++str;
	}
}

void string::lower(char *str)
{
    while(str && *str) {
        *str = tolower(*str);
        ++str;
    }
}

unsigned string::ccount(const char *str, const char *clist)
{
	unsigned count = 0;
	while(str && *str) {
		if(strchr(clist, *(str++)))
			++count;
	}
	return count;
}

char *string::skip(char *str, const char *clist)
{
	if(!str || !clist)
		return NULL;

	while(*str && strchr(clist, *str))
		++str;

	if(*str)
		return str;

	return NULL;
}

char *string::rskip(char *str, const char *clist)
{
	size_t len = count(str);

	if(!len || !clist)
		return NULL;

	while(len > 0) {
		if(!strchr(clist, str[--len]))
			return str;
	}
	return NULL;
}

char *string::find(char *str, const char *clist)
{
	if(!str)
		return NULL;

	if(!clist)
		return str;

	while(str && *str) {
		if(strchr(clist, *str))
			return str;
	}
	return NULL;
}

char *string::rfind(char *str, const char *clist)
{
	if(!str)
		return NULL;

	if(!clist)
		return str + strlen(str);

	char *s = str + strlen(str);

	while(s > str) {
		if(strchr(clist, *(--s)))
			return s;
	}
	return NULL;
}

timeout_t string::totimeout(const char *cp, char **ep, bool sec)
{
	char *end, *nend;
	timeout_t base = strtol(cp, &end, 10);
	timeout_t rem = 0;
	unsigned dec = 0;

	if(!cp)
		return 0;

	if(ep)
		*ep = NULL;

	if(!end)
		return base;

	while(end && *end == ':') {
		cp = ++end;
		base *= 60;
		base += strtol(cp, &end, 10);
		sec = true;
	}

	if(end && *end == '.') {
		sec = true;
		rem = strtol(end, &nend, 10);
		while(rem > 1000l)
			rem /= 10;
		while(++dec < 4) {
			rem *= 10;
		}
		end = nend;
	}
	
	switch(*end)
	{
	case 'm':
	case 'M':
		++end;
		if(*end == 's' || *end == 'S') {
			++end;
			break;
		}
		base *= 60000l;
		if(dec)
			base += (rem * 60l);
		break;
	case 's':
	case 'S':
		base *= 1000l;
		if(dec)
			base += rem;
		break;
	case 'h':
		base *= 3600000l;
		if(dec)
			base += (rem * 3600l);
		break;
	default:
		if(sec)
			base *= 1000l;
		if(sec && dec)
			base += rem;
	}
		
	if(ep)
		*ep = end;
	
	return base;
}

char *string::last(char *str, const char *clist)
{
	char *cp, *lp = NULL;

	if(!str)
		return NULL;

	if(!clist)
		return str + strlen(str) - 1;

	while(clist && *clist) {
		cp = strrchr(str, *(clist++));
		if(cp && cp > lp)
			lp = cp;
	}

	return lp;
}

char *string::first(char *str, const char *clist)
{
    char *cp, *fp;

    if(!str)
        return NULL;

	if(!clist)
		return str;

	fp = str + strlen(str);
    while(clist && *clist) {
        cp = strchr(str, *(clist++));
        if(cp && cp < fp)
            fp = cp;
    }

	if(!*fp)
		fp = NULL;
    return fp;
}

int string::compare(const char *s1, const char *s2)
{
	if(!s1)
		s1 = "";
	if(!s2)
		s2 = "";

	return strcmp(s1, s2);
}

int string::compare(const char *s1, const char *s2, size_t s)
{
    if(!s1)
        s1 = "";
    if(!s2)
        s2 = "";

    return strncmp(s1, s2, s);
}

int string::case_compare(const char *s1, const char *s2)
{
	if(!s1)
		s1 = "";

	if(!s2)
		s2 = "";

#ifdef	HAVE_STRICMP
	return stricmp(s1, s2);
#else
	return strcasecmp(s1, s2);
#endif
}

int string::case_compare(const char *s1, const char *s2, size_t s)
{
    if(!s1)
        s1 = "";

    if(!s2)
        s2 = "";

#ifdef  HAVE_STRICMP
    return strnicmp(s1, s2, s);
#else
    return strncasecmp(s1, s2, s);
#endif
}

int32_t string::toint(const char *cp, char **ep)
{
	if(!cp) {
		if(ep)
			*ep = NULL;
		return 0;
	}
	long value = strtol(cp, ep, 10);
	return (int32_t)value;
}	

char *string::unquote(char *str, const char *clist)
{
	size_t len = count(str);
	if(!len || !str)
		return NULL;

	while(clist[0]) {
		if(*str == clist[0] && str[len - 1] == clist[1]) {
			str[len - 1] = 0;
			return ++str;
		}
		clist += 2;
	}
	return NULL;
}

char *string::fill(char *str, size_t size, const char fill)
{
	if(!str)
		return NULL;

	memset(str, fill, size - 1);
	str[size - 1] = 0;
	return str;
}

bool string::tobool(const char *cp, char **ep)
{
	bool rtn = false;

	if(!cp) {
		if(ep)
			*ep = NULL;
		return false;
	}

	if(*cp == '1') {
		rtn = true;
		++cp;
		goto exit;
	}
	else if(*cp == '0') {
		++cp;
		goto exit;
	}

	if(!strnicmp(cp, "true", 4)) {
		cp += 4;
		rtn = true;
		goto exit;
	}
	else if(!strnicmp(cp, "yes", 3)) {
		cp += 3;
		rtn = true;
		goto exit;
	}
	else if(!strnicmp(cp, "false", 5)) {
		cp += 5;
		goto exit;
	}
	else if(!strnicmp(cp, "no", 2)) {
		cp += 2;
		goto exit;
	}

	if(strchr("yYtT", *cp)) {
		++cp;
		rtn = true;
		goto exit;
	}

	if(strchr("nNfF", *cp)) {
		++cp;
		goto exit;
	}

exit:
	if(ep)
		*ep = (char *)cp;

	return rtn;
}

bool string::isinteger(const char *cp)
{
	if(!cp)
		return false;

	while(*cp && isdigit(*cp))
		++cp;

	if(*cp)
		return false;

	return true;
}

bool string::isnumeric(const char *cp)
{
	if(!cp)
		return false;

	while(*cp && strchr("0123456789.+-e", *cp))
		++cp;

	if(*cp)
		return false;

	return true;
}

size_t string::urlencodesize(char *src)
{
	size_t size = 0;

	while(src && *src) {
		if(isalnum(*src) ||  strchr("/.-:;, ", *src))
			++size;
		else
			size += 3;
	}
	return size;
}

size_t string::urlencode(char *dest, size_t limit, const char *src)
{
	static const char *hex = "0123456789abcdef";
	char *ret = dest;
	unsigned char ch;

	assert(dest != NULL && src != NULL && limit > 3);
	
	while(limit-- > 3 && *src) {
		if(*src == ' ') {
			*(dest++) = '+';
			++src;
		}
		else if(isalnum(*src) ||  strchr("/.-:;,", *src))
			*(dest++) = *(src++);
		else {		
			limit -= 2;
			ch = (unsigned char)(*(src++));
			*(dest++) = '%';
			*(dest++) = hex[(ch >> 4)&0xF];
			*(dest++) = hex[ch % 16];
		}
	}		
	*dest = 0;
	return dest - ret;
}

size_t string::urldecode(char *dest, size_t limit, const char *src)
{
	char *ret = dest;
	char hex[3];

	assert(dest != NULL && src != NULL && limit > 1);

	while(limit-- > 1 && *src && !strchr("?&", *src)) {
		if(*src == '%') {
			memset(hex, 0, 3);
			hex[0] = *(++src);
			if(*src)
				hex[1] = *(++src);
			if(*src)
				++src;			
			*(dest++) = (char)strtol(hex, NULL, 16);	
		}
		else if(*src == '+') {
			*(dest++) = ' ';
			++src;
		}
		else
			*(dest++) = *(src++);
	}
	*dest = 0;
	return dest - ret;
}

size_t string::xmlencode(char *out, size_t limit, const char *src)
{
	char *ret = out;

	assert(src != NULL && out != NULL && limit > 0);

	while(src && *src && limit > 6) {
		switch(*src) {
		case '<':
			strcpy(out, "&lt;");
			limit -= 4;
			out += 4;
			break;
		case '>':
			strcpy(out, "&gt;");
			limit -= 4;
			out += 4;
			break;
		case '&':
			strcpy(out, "&amp;");
			limit -= 5;
			out += 5;
			break;
		case '\"':
			strcpy(out, "&quot;");
			limit -= 6;
			out += 6;
			break;
		case '\'':
			strcpy(out, "&apos;");
			limit -= 6;
			out += 6;
			break;
		default:
			--limit;
			*(out++) = *src;
		}
		++src;
	}
	*out = 0;
	return out - ret;
}

size_t string::xmldecode(char *out, size_t limit, const char *src)
{
	char *ret = out;

	assert(src != NULL && out != NULL && limit > 0);

	if(*src == '\'' || *src == '\"')
		++src;
	while(src && limit-- > 1 && !strchr("<\'\">", *src)) {
		if(!strncmp(src, "&amp;", 5)) {
			*(out++) = '&';
			src += 5;
		}
		else if(!strncmp(src, "&lt;", 4)) {
			src += 4;
			*(out++) = '<';
		}
		else if(!strncmp(src, "&gt;", 4)) {
			src += 4;
			*(out++) = '>';
		}
		else if(!strncmp(src, "&quot;", 6)) {
			src += 6;
			*(out++) = '\"';
		}
		else if(!strncmp(src, "&apos;", 6)) {
			src += 6;
			*(out++) = '\'';
		}
		else
			*(out++) = *(src++);
	}
	*out = 0;
	return out - ret;
}

static const unsigned char alphabet[65] =
	"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

size_t string::b64decode(caddr_t out, const char *src, size_t count)
{
	static char *decoder = NULL;
	int i, bits, c;

	unsigned char *dest = (unsigned char *)out;

	if(!decoder) {
		decoder = (char *)malloc(256);
		for (i = 0; i < 256; ++i)
			decoder[i] = 64;
		for (i = 0; i < 64 ; ++i)
			decoder[alphabet[i]] = i;
	}

	bits = 1;

	while(*src) {
		c = (unsigned char)(*(src++));
		if (c == '=') {
			if (bits & 0x40000) {
				if (count < 2) break;
				*(dest++) = (bits >> 10);
				*(dest++) = (bits >> 2) & 0xff;
				break;
			}
			if (bits & 0x1000 && count)
				*(dest++) = (bits >> 4);
			break;
		}
		// skip invalid chars
		if (decoder[c] == 64)
			continue;
		bits = (bits << 6) + decoder[c];
		if (bits & 0x1000000) {
			if (count < 3) break;
			*(dest++) = (bits >> 16);
			*(dest++) = (bits >> 8) & 0xff;
			*(dest++) = (bits & 0xff);
		    	bits = 1;
			count -= 3;
		}
	}
	return (size_t)((caddr_t)dest - out);
}

size_t string::b64encode(char *out, caddr_t src, size_t count)
{
	unsigned bits;
	char *dest = out;

	assert(out != NULL && src != NULL && count > 0);

	while(count >= 3) {
		bits = (((unsigned)src[0])<<16) | (((unsigned)src[1])<<8)
			| ((unsigned)src[2]);
		src += 3;
		count -= 3;
		*(out++) = alphabet[bits >> 18];
	    *(out++) = alphabet[(bits >> 12) & 0x3f];
	    *(out++) = alphabet[(bits >> 6) & 0x3f];
	    *(out++) = alphabet[bits & 0x3f];
	}
	if (count) {
		bits = ((unsigned)src[0])<<16;
		*(out++) = alphabet[bits >> 18];
		if (count == 1) {
			*(out++) = alphabet[(bits >> 12) & 0x3f];
	    		*(out++) = '=';
		}
		else {
			bits |= ((unsigned)src[1])<<8;
			*(out++) = alphabet[(bits >> 12) & 0x3f];
	    		*(out++) = alphabet[(bits >> 6) & 0x3f];
		}
	    	*(out++) = '=';
	}
	*out = 0;
	return out - dest;
}

size_t string::b64len(const char *str)
{
	unsigned count = strlen(str);
	const char *ep = str + count - 1;

	if(!count)
		return 0;

	count /= 4;
	count *= 3;
	if(*ep == '=') {
		--ep;
		--count;
		if(*ep == '=')
			--count;
	}
	return count;
}

