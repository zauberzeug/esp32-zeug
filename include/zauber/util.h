#ifndef ZZ_UTIL_H
#define ZZ_UTIL_H

#include <array>
#include <cstdarg>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <string>
#include <string_view>

namespace ZZ::Util {

template <size_t maxLen>
class FixString {
    std::array<char, maxLen> chars;
    std::size_t length{0};

public:
    FixString(const std::string_view &sv) {
        std::size_t cpyCount = std::min(sv.size(), maxLen - 1);
        std::memcpy(&chars[0], sv.data(), cpyCount);
        chars[cpyCount] = '\0';

        length = cpyCount;
    }

    auto view() const -> std::string_view {
        return std::string_view{chars.data(), length};
    }

    auto c_str() const -> const char * {
        return chars.data();
    }
};

template <std::size_t Size>
class TextBuffer {
    std::array<char, Size> m_buf;
    std::size_t m_len;

public:
    TextBuffer() : m_len{0} {
        static_assert(Size >= 1, "TextBuffer must have space for null terminator");
        m_buf[0] = '\0';
    }

    auto ptr() const -> const char * {
        return m_buf.data();
    }

    auto len() const -> std::size_t {
        return m_len;
    }

    operator std::string_view() {
        return std::string_view{ptr(), len()};
    }

    operator std::string() {
        return std::string{ptr(), len()};
    }

    auto printf(const char *format, ...) -> void {
        std::va_list args;

        va_start(args, format);
        m_len = std::vsnprintf(m_buf.data(), Size, format, args);
        va_end(args);
    }
};

} // namespace ZZ::Util

#endif // ZZ_UTIL_H