/*
 * SPDX-FileCopyrightText: 2021-2022 Zauberzeug GmbH
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef ZZ_EVENTHANDLER_H
#define ZZ_EVENTHANDLER_H

#include <functional>

#include <esp_err.h>
#include <esp_event.h>

#include "esp_zeug/util.h"

namespace ZZ {

class EventHandler {
public:
    using Callback = std::function<void(int32_t, void *)>;

    EventHandler(esp_event_base_t eventBase, Callback cb)
        : m_eventBase{eventBase}, m_eventId{ESP_EVENT_ANY_ID}, m_cb{cb} {}

    EventHandler(esp_event_base_t eventBase, int32_t eventId, Callback cb)
        : m_eventBase{eventBase}, m_eventId{eventId}, m_cb{cb} {}

    auto registerMainLoop() -> esp_err_t {
        return esp_event_handler_instance_register(m_eventBase,
                                                   m_eventId,
                                                   nativeHandler,
                                                   this, &m_instance);
    }

    auto postMainLoop(int32_t eventId,
                      const Util::ByteBufferView &data = Util::ByteBufferView{},
                      TickType_t ticksToWait = portMAX_DELAY) -> esp_err_t {
        /* esp_event_post taking void* instead of const void* for event_data
         * has to be a defect */
        return esp_event_post(m_eventBase, eventId,
                              const_cast<void *>(reinterpret_cast<const void *>(data.data())),
                              data.size(), ticksToWait);
    }

private:
    esp_event_base_t m_eventBase;
    int32_t m_eventId;
    esp_event_handler_instance_t m_instance{nullptr};
    Callback m_cb;

    static auto nativeHandler(void *ctx, esp_event_base_t,
                              int32_t eventId, void *eventData) -> void {
        auto &self = *static_cast<EventHandler *>(ctx);
        self.m_cb(eventId, eventData);
    }
};

} // namespace ZZ

#endif // ZZ_EVENTHANDLER_H
