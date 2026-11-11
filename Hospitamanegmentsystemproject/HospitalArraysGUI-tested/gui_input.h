#pragma once
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <string>
#include <sstream>
using namespace std;

inline bool ParseNumber(const string& text, int& value) {
    stringstream input(text);
    char extra;
    if (!(input >> value)) {
        return false;
    }
    if (input >> extra) {
        return false;
    }
    return true;
}
inline wstring Utf8ToWide(const string& value) {
    if (value.empty()) {
        return L"";
    }
    int length = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, value.c_str(), -1, nullptr, 0);
    if (!length) {
        return L"";
    }
    wchar_t* text = new wchar_t[length];
    if (!MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, value.c_str(), -1, text, length)) {
        delete[] text;
        return L"";
    }
    wstring result = text;
    delete[] text;
    return result;
}
inline string WideToUtf8(const wstring& value) {
    if (value.empty()) {
        return "";
    }
    int length = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, value.c_str(), -1, nullptr, 0,
                                     nullptr, nullptr);
    if (!length) {
        return "";
    }
    char* text = new char[length];
    if (!WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, value.c_str(), -1, text, length,
                             nullptr, nullptr)) {
        delete[] text;
        return "";
    }
    string result = text;
    delete[] text;
    return result;
}
