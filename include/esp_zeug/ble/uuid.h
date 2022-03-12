/*
 * SPDX-FileCopyrightText: 2021-2022 Zauberzeug GmbH
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef ZZ_BLE_UUID_H
#define ZZ_BLE_UUID_H

#include <sdkconfig.h>
#ifndef CONFIG_BT_NIMBLE_ENABLED
#error Bluetooth NimBLE stack must be enabled in sdkconfig
#else

#include <cstdint>
#include <cstdlib>
#include <string_view>

#include <host/ble_uuid.h>

#include "esp_zeug/util.h"

namespace ZZ::Ble::Uuid {

using namespace Util;

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
    result.value = valBuf[1] << 8 | valBuf[0];

    return result;
}

constexpr auto operator"" _uuid32(const char *str, std::size_t len) {
    ble_uuid32_t result{
        BLE_UUID_TYPE_32,
        0,
    };

    uint8_t valBuf[4]{};
    ZZ::Ble::Uuid::parse(std::string_view{str, len}, valBuf, 4);
    result.value = valBuf[3] << 24 | valBuf[2] << 16 | valBuf[1] << 8 | valBuf[0];

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

#endif // ZZ_BLE_UUID_H
