#ifndef ZZ_NVSCACHE_H
#define ZZ_NVSCACHE_H

#include <cstdint>
#include <map>
#include <string>
#include <variant>

#include <esp_err.h>
#include <nvs_flash.h>

namespace ZZ {

namespace NvsType {
enum Type {
    Invalid = 0,
    String,
    Int16,
};

template <typename T>
struct TypeInfo {
    using CppType = T;
};

template <>
struct TypeInfo<std::string> {
    const Type valueType{String};
};

template <>
struct TypeInfo<int16_t> {
    const Type valueType{Int16};
};
} // namespace NvsType

class NvsCache {
public:
    using InvalidType = std::monostate;
    using Value = std::variant<InvalidType, std::string, int16_t>;

    NvsCache(const std::string_view &nspace);
    ~NvsCache();

    auto init(nvs_open_mode_t mode) -> esp_err_t;
    auto swap(NvsCache &o) -> void;

    const auto getTyped(const char *key, ZZ::NvsType::Type type) -> const Value &;

    template <typename T>
    auto get(const char *key) -> const T & {
        constexpr ZZ::NvsType::TypeInfo<T> info{};
        return std::get<info.valueType>(getTyped(key, info.valueType));
    }

    template <typename T>
    auto getWithDefault(const char *key, const T &def) -> const T & {
        constexpr NvsType::TypeInfo<T> info{};
        const Value &entry{getTyped(key, info.valueType)};

        if (entry.index() == info.valueType) {
            return std::get<T>(entry);
        } else {
            return def;
        }
    }

    template <typename T>
    auto getWithDefault(const char *key, const T &&def) -> const T & {
        static_assert(std::is_lvalue_reference<decltype(def)>::value, "Default value must outlive this call (no rvalues)!");

        static const T neverReturned;
        return neverReturned;
    }

    auto getStrWithDefault(const char *key, const char *def) -> const char * {
        const Value &entry{getTyped(key, NvsType::String)};

        if (entry.index() == NvsType::String) {
            return std::get<std::string>(entry).c_str();
        } else {
            return def;
        }
    }

    auto set(const char *key, Value &&value) -> void;
    auto commit() const -> void;

private:
    static const nvs_handle_t NULL_HANDLE;

    nvs_handle_t m_handle;
    std::string m_nspace;
    using CacheMap = std::map<std::string, Value>;
    CacheMap m_cacheMap;
};

} // namespace ZZ

#endif // ZZ_NVSCACHE_H
