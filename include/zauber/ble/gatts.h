#ifndef ZZBLEGATTS_H
#define ZZBLEGATTS_H

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

    ble_uuid128_t m_uuid;
    ble_gatt_chr_flags m_flags;
    Callback m_callback;

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
            nullptr,
        };
    }
};

template <std::size_t N>
struct Service {
    const std::uint8_t m_type{BLE_GATT_SVC_TYPE_PRIMARY};
    const ble_uuid128_t m_uuid;
    Characteristic m_charas[N];
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

#endif // ZZBLEGATTS_H
