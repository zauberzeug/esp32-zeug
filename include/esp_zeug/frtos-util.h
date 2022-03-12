/*
 * SPDX-FileCopyrightText: 2021-2022 Zauberzeug GmbH
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef ZZ_FRTOS_UTIL_H
#define ZZ_FRTOS_UTIL_H

#include <array>
#include <cassert>
#include <cstring>
#include <functional>
#include <string_view>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <esp_log.h>

#include "util.h"

namespace ZZ::FrtosUtil {
namespace Core {
enum Id : BaseType_t {
    Any = tskNO_AFFINITY,
    PRO = 0,
    APP = 1,
};

static auto idToStr(BaseType_t id) -> const char * {
    switch (id) {
    case Core::PRO:
        return "PRO";
    case Core::APP:
        return "APP";
    default:
        return "UNKNOWN";
    }
}
} // namespace Core

template <std::size_t StackSize = 256 * 16>
class Task {
    using Entrypoint = std::function<void()>;
    static const UBaseType_t DEFAULT_PRIORITY{5};

    const ZZ::Util::TextBuffer<16> m_name;
    const TickType_t m_loopDelay;
    const Core::Id m_coreId;
    const Entrypoint m_entrypoint;

    TaskHandle_t m_task{nullptr};

    /* FreeRTOS internal */
    StaticTask_t m_taskBuffer;
    StackType_t m_stack[StackSize];

    static auto nativeFunction(void *data) -> void {
        auto &self{*static_cast<Task *>(data)};
        auto id{xPortGetCoreID()};

        ESP_LOGI("esp_zeug/FrtosUtil", "Task [%.*s] executing on core [%s]", self.m_name.length(), self.m_name.data(), Core::idToStr(id));

        while (true) {
            self.m_entrypoint();
            delayMiliSeconds(self.m_loopDelay);
        }
    }

public:
    Task(const std::string_view &name, Entrypoint entrypoint)
        : m_name{name}, m_loopDelay{0}, m_coreId{Core::Any}, m_entrypoint{entrypoint} {
    }

    Task(const std::string_view &name, TickType_t loopDelayMs, Entrypoint entrypoint)
        : m_name{name}, m_loopDelay{loopDelayMs}, m_coreId{Core::Any}, m_entrypoint{entrypoint} {
    }

    Task(const std::string_view &name, Core::Id coreId, Entrypoint entrypoint)
        : m_name{name}, m_loopDelay{0}, m_coreId{coreId}, m_entrypoint{entrypoint} {
    }

    Task(const std::string_view &name, TickType_t loopDelayMs, Core::Id coreId, Entrypoint entrypoint)
        : m_name{name}, m_loopDelay{loopDelayMs}, m_coreId{coreId}, m_entrypoint{entrypoint} {
    }

    auto run() -> void {
        assert(m_task == nullptr);
        m_task = xTaskCreateStaticPinnedToCore(nativeFunction,
                                               m_name.data(),
                                               StackSize,
                                               this,
                                               DEFAULT_PRIORITY,
                                               m_stack,
                                               &m_taskBuffer,
                                               m_coreId);
    }

    auto halt() -> void {
        assert(m_task != nullptr);
        vTaskDelete(m_task);
    }

    static auto haltCurrent() -> void {
        vTaskDelete(nullptr);
    }

    static auto delayMiliSeconds(TickType_t ms) -> void {
        vTaskDelay(ms / portTICK_PERIOD_MS);
    }

    static auto delaySeconds(float seconds) -> void {
        delayMiliSeconds(seconds * 1000);
    }
};
} // namespace ZZ::FrtosUtil

#endif // ZZ_FRTOS_UTIL_H
