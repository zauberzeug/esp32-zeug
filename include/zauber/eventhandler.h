#ifndef EVENTHANDLER_H
#define EVENTHANDLER_H

#include <functional>

#include <esp_err.h>
#include <esp_event.h>

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

#endif // EVENTHANDLER_H
