
#include <Windows.h>
#include "Encoding.h"
using namespace std;

void Encoding::GBK2Unicode(const std::string &src, std::wstring &dest) {
    auto ptr = src.c_str();

	int size = MultiByteToWideChar(CP_ACP, 0, ptr,src.size(), NULL, NULL);
	dest.resize(size);
	int len = MultiByteToWideChar(CP_ACP, 0, ptr, src.size(), (LPWSTR)dest.c_str(), size);

}

std::wstring
Encoding::GBK2Unicode(const std::string &src) {
	std::wstring ret;
	GBK2Unicode(src, ret);
	return ret;
}

void Encoding::Unicode2GBK(const std::wstring &src, std::string &dest) {

	PCWCH ptr = src.c_str();
	int size = WideCharToMultiByte(CP_THREAD_ACP, 0, ptr, src.size(), NULL, 0, NULL, NULL);
	dest.resize(size);
	int len = WideCharToMultiByte(CP_THREAD_ACP, 0, ptr, src.size(), (LPSTR)dest.c_str(), size, NULL, NULL);
}

void 
Encoding::Unicode2Utf8(const std::wstring& src,std::string &dest)
{

	LPCWCH ptr = src.c_str();
	int size = WideCharToMultiByte(CP_UTF8, 0, ptr,src.size() , NULL, 0, NULL, NULL);
	dest.resize(size);
	int len = WideCharToMultiByte(CP_UTF8, 0, ptr, src.size(), (char*)dest.c_str(), size, NULL, NULL);

}

std::string
Encoding::Unicode2Utf8(const std::wstring& src)
{
	std::string ret;
	Unicode2Utf8(src, ret);
	return ret;
}

void 
Encoding::GBK2Utf8(const std::string& src, std::string &dest) {
	std::wstring unicode;
	GBK2Unicode(src, unicode);
	Unicode2Utf8(unicode, dest);
}

std::string
Encoding::GBK2Utf8(const std::string& src) {
	std::string ret;
	GBK2Utf8(src, ret);
	return ret;
}

void
Encoding::Utf82Unicode(const std::string &src, std::wstring &dest) {

	LPCCH ptr = src.c_str();
	int size = MultiByteToWideChar(CP_UTF8, 0, ptr, src.size(), NULL, NULL);
	dest.resize(size);
	int len = MultiByteToWideChar(CP_UTF8, 0, ptr, src.size(), (LPWSTR)dest.c_str(), size);

}

std::wstring
Encoding::Utf82Unicode(const std::string &src) {
	std::wstring ret;
	Utf82Unicode(src, ret);
	return ret;
}

void
Encoding::Utf82GBK(const std::string& src, std::string &dest) {
	std::wstring unicode;
	Utf82Unicode(src, unicode);
	Unicode2GBK(unicode, dest);
}

std::string
Encoding::Utf82GBK(const std::string& src) {
	std::string ret;
	Utf82GBK(src, ret);
	return ret;
}

std::string 
Encoding::Unicode2GBK(const std::wstring &src) {
	std::string ret;
	Unicode2GBK(src, ret);
	return ret;
}

unsigned char ToHex(unsigned char x) {
    return  x > 9 ? x + 55 : x + 48;
}

unsigned char FromHex(unsigned char x) {
    unsigned char y;
    if (x >= 'A' && x <= 'Z') y = x - 'A' + 10;
    else if (x >= 'a' && x <= 'z') y = x - 'a' + 10;
    else if (x >= '0' && x <= '9') y = x - '0';
    else return 0;
    return y;
}

string Encoding::UrlDecode(const string& str)
{
    string dst = "";
    size_t length = str.length();
    for (size_t i = 0; i < length; i++) {
        if (str[i] == '+') {
            dst += ' ';
        } else if (i + 2 < length && str[i] == '%') {
            unsigned char high = FromHex((unsigned char)str[++i]);
            unsigned char low = FromHex((unsigned char)str[++i]);
            dst += high*16 + low;
        } else {
            dst += str[i];
        }
    }
    return dst;
}

string Encoding::UrlEncode(const string& str)
{
    string dst = "";
    size_t length = str.length();
    for(size_t i = 0; i < length; i++) {
        if(isalnum((unsigned char)str[i]) ||
            (str[i] == '-') ||
            (str[i] == '_') ||
            (str[i] == '.') ||
            (str[i] == '~')) {
            dst += str[i];
        } else if(str[i] == ' ') {
            dst += "+";
        } else {
            dst += '%';
            dst += ToHex((unsigned char)str[i] >> 4);
            dst += ToHex((unsigned char)str[i] % 16);
        }
    }
    return dst;
}
