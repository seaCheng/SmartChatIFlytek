#pragma once
#include <string>

class Encoding {
public:
    static void Unicode2GBK(const std::wstring &src, std::string &dest);
    static std::string Unicode2GBK(const std::wstring &src);
    static void GBK2Unicode(const std::string &src, std::wstring &dest);
    static std::wstring GBK2Unicode(const std::string &src);
    static void Unicode2Utf8(const std::wstring& src, std::string &dest);
    static std::string Unicode2Utf8(const std::wstring& src);
    static void GBK2Utf8(const std::string& src, std::string &dest);
    static std::string GBK2Utf8(const std::string& src);
    static void Utf82GBK(const std::string &src, std::string &dest);
    static std::string Utf82GBK(const std::string &src);
    static void Utf82Unicode(const std::string &src, std::wstring &dest);
    static std::wstring Utf82Unicode(const std::string &src);

    static std::string UrlDecode(const std::string& str);
    static std::string UrlEncode(const std::string& str);

};
