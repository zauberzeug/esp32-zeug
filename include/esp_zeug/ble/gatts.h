/*
 * SPDX-FileCopyrightText: 2021-2022 Zauberzeug GmbH
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef ZZ_BLE_GATTS_H
#define ZZ_BLE_GATTS_H

#include <sdkconfig.h>
#ifndef CONFIG_BT_NIMBLE_ENABLED
#error Bluetooth NimBLE stack must be enabled in sdkconfig
#else

#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>

#include <host/ble_gatt.h>
#include <host/ble_uuid.h>

namespace ZZ::Ble::Gatts {

struct Characteristic {
    using Callback = std::function<int(std::uint16_t, std::uint16_t, ble_gatt_access_ctxt *)>;

    ble_uuid_any_t m_uuid;
    ble_gatt_chr_flags m_flags;
    std::uint16_t *m_valueHandle;
    Callback m_callback;

    Characteristic() : m_flags{0}, m_callback{} {}

    Characteristic(const ble_uuid16_t &uuid, ble_gatt_chr_flags flags, Callback callback)
        : m_flags{flags}, m_valueHandle{nullptr}, m_callback{callback} {
        m_uuid.u16 = uuid;
    }

    Characteristic(const ble_uuid32_t &uuid, ble_gatt_chr_flags flags, Callback callback)
        : m_flags{flags}, m_valueHandle{nullptr}, m_callback{callback} {
        m_uuid.u32 = uuid;
    }

    Characteristic(const ble_uuid128_t &uuid, ble_gatt_chr_flags flags, Callback callback)
        : m_flags{flags}, m_valueHandle{nullptr}, m_callback{callback} {
        m_uuid.u128 = uuid;
    }

    Characteristic(const ble_uuid16_t &uuid, ble_gatt_chr_flags flags, std::uint16_t *valueHandle, Callback callback)
        : m_flags{flags}, m_valueHandle{valueHandle}, m_callback{callback} {
        m_uuid.u16 = uuid;
    }

    Characteristic(const ble_uuid32_t &uuid, ble_gatt_chr_flags flags, std::uint16_t *valueHandle, Callback callback)
        : m_flags{flags}, m_valueHandle{valueHandle}, m_callback{callback} {
        m_uuid.u32 = uuid;
    }

    Characteristic(const ble_uuid128_t &uuid, ble_gatt_chr_flags flags, std::uint16_t *valueHandle, Callback callback)
        : m_flags{flags}, m_valueHandle{valueHandle}, m_callback{callback} {
        m_uuid.u128 = uuid;
    }

    static int nativeCb(std::uint16_t conHandle,
                        std::uint16_t attrHandle,
                        ble_gatt_access_ctxt *ctx,
                        void *arg) {
        auto &self = *static_cast<Characteristic *>(arg);
        return self.m_callback(conHandle, attrHandle, ctx);
    }

    constexpr auto def() const {
        return ble_gatt_chr_def{
            &m_uuid.u,
            nativeCb,
            const_cast<Characteristic *>(this),
            nullptr,
            m_flags,
            0,
            m_valueHandle,
            nullptr,
        };
    }
};

template <std::size_t N>
struct Service {
    const std::uint8_t m_type{BLE_GATT_SVC_TYPE_PRIMARY};
    const ble_uuid128_t m_uuid;
    std::array<Characteristic, N> m_charas;
    std::array<ble_gatt_chr_def, N + 1> m_charaDefs;

    constexpr Service(const ble_uuid128_t &uuid, const Characteristic (&list)[N])
        : m_uuid{uuid} {
        for (std::size_t idx = 0; idx < N; ++idx) {
            /* We have to capture each Characteristic first because,
             * coming from an initializer list, it is treated as a floating rvalue */
            m_charas[idx] = list[idx];
            m_charaDefs[idx] = m_charas[idx].def();
        }

        m_charaDefs[N] = ble_gatt_chr_def{};
    }

    constexpr auto def() const {
        return ble_gatt_svc_def{
            m_type,
            &m_uuid.u,
            nullptr,
            m_charaDefs.data(),
        };
    }
};

} // namespace ZZ::Ble::Gatts

#endif

#endif // ZZ_BLE_GATTS_H
