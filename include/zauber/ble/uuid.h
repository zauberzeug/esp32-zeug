#ifndef ZZBLEUUID_H
#define ZZBLEUUID_H

#include <sdkconfig.h>
#ifndef CONFIG_BT_NIMBLE_ENABLED
#error Bluetooth NimBLE stack must be enabled in sdkconfig
#else

#include <cstdint>
#include <cstdlib>
#include <string_view>

#include <host/ble_uuid.h>

namespace ZZ::Ble::Uuid {

constexpr auto isHex(char c) -> std::uint8_t {
    if (c >= '0' && c <= '9') {
        return true;
    } else if (c >= 'a' && c <= 'f') {
        return true;
    } else if (c >= 'A' && c <= 'F') {
        return true;
    }

    return false;
}

constexpr auto hexVal(char c) -> std::uint8_t {
    if (c >= '0' && c <= '9') {
        return c - '0';
    } else if (c >= 'a' && c <= 'f') {
        return 0xA + (c - 'a');
    } else if (c >= 'A' && c <= 'F') {
        return 0xA + (c - 'A');
    }

    /* UB from hereon out */
    return 0xFF;
}

constexpr auto charPairToByte(char high, char low) -> std::uint8_t {
    return (hexVal(high) << 4) | (hexVal(low) << 0);
}

constexpr auto parse(const std::string_view &sv,
                     std::uint8_t *dst, std::size_t dstLen) -> void {
    std::size_t idx{0};
    char high{};
    bool haveHigh{false};

    for (char c : sv) {
        if (!isHex(c)) {
            continue;
        }

        if (haveHigh) {
            dst[(dstLen - 1) - idx++] = charPairToByte(high, c);
            haveHigh = false;

            if (idx == dstLen) {
                break;
            }
        } else {
            high = c;
            haveHigh = true;
        }
    }
}

} // namespace ZZ::Ble::Uuid

constexpr auto operator"" _uuid16(const char *str, std::size_t len) {
    ble_uuid16_t result{
        BLE_UUID_TYPE_16,
        0,
    };

    uint8_t valBuf[2]{};
    ZZ::Ble::Uuid::parse(std::string_view{str, len}, valBuf, 2);
    result.value = valBuf[0] << 8 | valBuf[1];

    return result;
}

constexpr auto operator"" _uuid32(const char *str, std::size_t len) {
    ble_uuid32_t result{
        BLE_UUID_TYPE_32,
        0,
    };

    uint8_t valBuf[4]{};
    ZZ::Ble::Uuid::parse(std::string_view{str, len}, valBuf, 4);
    result.value = valBuf[0] << 24 | valBuf[1] << 16 | valBuf[2] << 8 | valBuf[3];

    return result;
}

constexpr auto operator"" _uuid128(const char *str, std::size_t len) {
    ble_uuid128_t result{
        BLE_UUID_TYPE_128,
        0,
    };

    ZZ::Ble::Uuid::parse(std::string_view{str, len}, result.value, 16);

    return result;
}

#endif

#endif // ZZBLEUUID_H
