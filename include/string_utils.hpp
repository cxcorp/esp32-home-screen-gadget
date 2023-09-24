#pragma once
#include <string>
#include <sstream>
#include <iomanip>

namespace StringUtils
{
    static const char *ws = " \t\n\r\f\v";

    // trim from end of string (right)
    inline std::string &rtrim(std::string &s, const char *t = ws)
    {
        s.erase(s.find_last_not_of(t) + 1);
        return s;
    }

    // trim from beginning of string (left)
    inline std::string &ltrim(std::string &s, const char *t = ws)
    {
        s.erase(0, s.find_first_not_of(t));
        return s;
    }

    // trim from both ends of string (right then left)
    inline std::string &trim(std::string &s, const char *t = ws)
    {
        return ltrim(rtrim(s, t), t);
    }

    std::string percentEncode(const std::string &input)
    {
        std::ostringstream encoded;
        encoded << std::uppercase << std::hex << std::setfill('0');

        for (char c : input)
        {
            if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~')
            {
                encoded << c;
            }
            else
            {
                encoded << '%' << std::setw(2) << static_cast<int>(static_cast<unsigned char>(c));
            }
        }

        return encoded.str();
    }
};