#include "zauber/nvs-cache.h"

#include <cassert>
#include <map>

#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE
#include <esp_log.h>

namespace ZZ {

static const char *TAG{"NvsCache"};
const nvs_handle_t NvsCache::NULL_HANDLE{0};

NvsCache::NvsCache(const std::string_view &nspace)
    : m_handle{NULL_HANDLE}, m_nspace{nspace} {
}

NvsCache::~NvsCache() {
    if (m_handle) {
        nvs_close(m_handle);
    }
}

auto NvsCache::init(nvs_open_mode_t mode) -> esp_err_t {
    assert(m_handle == NULL_HANDLE);
    return nvs_open(m_nspace.c_str(), mode, &m_handle);
}

auto NvsCache::swap(NvsCache &o) -> void {
    m_cacheMap.swap(o.m_cacheMap);
    m_nspace.swap(o.m_nspace);
    std::swap(m_handle, o.m_handle);
}

#define RET_IF_ERR(exp, errVar) do { errVar = exp; if (errVar != ESP_OK) { return errVar; } } while(false)

static auto queryInternal(nvs_handle_t handle, const char *key, NvsCache::Value &out, ZZ::NvsType::Type type) -> esp_err_t {
    esp_err_t ec;

    switch (type) {
    case ZZ::NvsType::String: {
        size_t length;
        RET_IF_ERR(nvs_get_str(handle, key, nullptr, &length), ec);

        std::string str{};
        str.resize(length);
        RET_IF_ERR(nvs_get_str(handle, key, &str[0], &length), ec);

        out = std::move(str);
        break;
    }
    case ZZ::NvsType::Int16: {
        int16_t i16;
        RET_IF_ERR(nvs_get_i16(handle, key, &i16), ec);

        out = i16;
        break;
    }
    case ZZ::NvsType::Invalid:
        assert(!"Invalid type not allowed in query");
    }

    return ESP_OK;
}

static auto storeInternal(nvs_handle_t handle, const char *key, const NvsCache::Value &value) -> esp_err_t {
    esp_err_t ec;

    switch (value.index()) {
    case ZZ::NvsType::String:
        RET_IF_ERR(nvs_set_str(handle, key, std::get<std::string>(value).c_str()), ec);
        break;

    case ZZ::NvsType::Int16:
        RET_IF_ERR(nvs_set_i16(handle, key, std::get<int16_t>(value)), ec);
        break;

    case ZZ::NvsType::Invalid:
        assert(!"Invalid type not allowed in store");
    }

    return ESP_OK;
}

auto NvsCache::getTyped(const char *key, ZZ::NvsType::Type type) -> const Value & {
    assert(m_handle != NULL_HANDLE);

    const CacheMap::const_iterator iter{m_cacheMap.find(key)};

    if (iter == m_cacheMap.cend()) {
        ESP_LOGI(TAG, "miss: %s", key);
        /* Cache miss */
        Value val{};
        esp_err_t ec{queryInternal(m_handle, key, val, type)};

        if (ec != ESP_OK) {
            ESP_LOGI(TAG, "Error reading NVS: %s", esp_err_to_name(ec));
        }

        auto insPair{m_cacheMap.emplace(key, std::move(val))};
        return insPair.first->second;
    } else {
        ESP_LOGI(TAG, "hit: %s", key);
        return iter->second;
    }
}

auto NvsCache::set(const char *key, Value &&value) -> void {
    assert(m_handle != NULL_HANDLE);

    storeInternal(m_handle, key, value);
    m_cacheMap.emplace(key, value);
}

auto NvsCache::commit() const -> void {
    assert(m_handle != NULL_HANDLE);

    nvs_commit(m_handle);
}

} // namespace ZZ
