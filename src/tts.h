
#pragma once
#include <sapi.h>
#include <napi.h>

class Speaker
{
    std::wstring s2ws(const std::string& s)
    {
        int len;
        int slength = (int)s.length() + 1;
        len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
        wchar_t* buf = new wchar_t[len];
        MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
        std::wstring r(buf);
        delete[] buf;
        return r;
    }
public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports);
    static Speaker& get()
    {
        // thread-safe in C++11
        static Speaker s_speaker;
        return s_speaker;
    }
    int speak(const std::string& words);
protected:
    Speaker();
    ~Speaker();
private:
    Speaker(Speaker const&) = delete;
    Speaker(Speaker&&) = delete;
    Speaker& operator=(Speaker const&) = delete;
    Speaker& operator=(Speaker &&) = delete;
    ISpVoice * pVoice_;
};