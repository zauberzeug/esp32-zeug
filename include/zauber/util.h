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

template <std::size_t Size>
class TextBuffer {
    std::array<char, Size> m_buf;
    std::size_t m_len;

public:
    constexpr TextBuffer() : m_len{0} {
        static_assert(Size >= 1, "TextBuffer must have space for null terminator");
        m_buf[0] = '\0';
    }

    constexpr TextBuffer(const std::string_view &sv) {
        std::size_t cpyCount = std::min(sv.size(), Size - 1);
        std::memcpy(m_buf.data(), sv.data(), cpyCount);
        m_buf[cpyCount] = '\0';

        m_len = cpyCount;
    }

    auto data() const -> const char * {
        return m_buf.data();
    }

    auto length() const -> std::size_t {
        return m_len;
    }

    operator std::string_view() {
        return std::string_view{data(), length()};
    }

    operator std::string() {
        return std::string{data(), length()};
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
