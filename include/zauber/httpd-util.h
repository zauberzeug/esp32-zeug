#ifndef ZZ_HTTPDUTIL_H
#define ZZ_HTTPDUTIL_H

#include <cassert>
#include <cstddef>
#include <cstdint>

#include <functional>
#include <string>
#include <string_view>
#include <vector>

#include <esp_err.h>
#include <esp_http_client.h>
#include <esp_http_server.h>

namespace ZZ::HttpdUtil {
/* Server helpers */

using ByteBufferView = std::basic_string_view<std::byte>;

/* Wrapper class results in identical code being generated: https://godbolt.org/z/MYYxfTTfv */
struct IncomingRequest {
    httpd_req_t *m_req;

    IncomingRequest(httpd_req_t *wrappedHandle)
        : m_req{wrappedHandle} {
    }

    operator httpd_req_t *() {
        return m_req;
    }

    auto setHeaderField(const char *field, const char *value) -> esp_err_t {
        return httpd_resp_set_hdr(m_req, field, value);
    }

    auto setHeaderField(const std::string &field, const std::string &value) -> esp_err_t {
        return httpd_resp_set_hdr(m_req, field.c_str(), value.c_str());
    }

    auto sendWholeBuffer(const ByteBufferView &buffer) -> esp_err_t {
        return httpd_resp_send(m_req, reinterpret_cast<const char *>(buffer.data()), buffer.size());
    }

    auto sendBufferChunk(const ByteBufferView &buffer) -> esp_err_t {
        return httpd_resp_send_chunk(m_req, reinterpret_cast<const char *>(buffer.data()), buffer.size());
    }

    auto sendBufferEnd() -> esp_err_t {
        return httpd_resp_send_chunk(m_req, nullptr, 0);
    }

    auto sendTextResponse(const std::string_view &response) -> esp_err_t {
        return httpd_resp_send(m_req, response.data(), response.size());
    }
};

// XXX proper error code propagation
struct ResponseType {
    const std::string m_type;
    const std::string m_hdrField;
    const std::string m_hdrValue;

    auto apply(IncomingRequest &req) const -> void {
        httpd_resp_set_type(req, m_type.c_str());

        if (!m_hdrField.empty()) {
            req.setHeaderField(m_hdrField.c_str(), m_hdrValue.c_str());
        }
    }
};

struct PlainTextType : ResponseType {
    PlainTextType() : ResponseType{"text/plain", "", ""} {}
};

struct HtmlTextType : ResponseType {
    HtmlTextType() : ResponseType{"text/html", "", ""} {}
};

struct JsonType : ResponseType {
    JsonType() : ResponseType{"application/json", "", ""} {}
};

struct JpegResponseType : ResponseType {
    JpegResponseType(const std::string &filename = "image.jpg")
        : ResponseType{"image/jpeg", "Content-Disposition", std::string{"inline; filename=" + filename}} {}
};

struct IconResponseType : ResponseType {
    IconResponseType()
        : ResponseType{"image/x-icon", "Content-Disposition", "inline; filename=favicon.ico"} {}
};

struct BinaryResponseType : ResponseType {
    BinaryResponseType(const std::string &filename = "data.bin")
        : ResponseType{"application/octet-stream", "Content-Disposition", "inline; filename=" + filename} {}
};

extern PlainTextType plainTextType;
extern HtmlTextType htmlTextType;
extern JsonType jsonType;

template<http_method T>
class EndpointHandler {
    using Callback = std::function<esp_err_t(IncomingRequest &req)>;

    const std::string m_endpoint;
    const ResponseType m_type;
    const Callback m_callback;
    httpd_uri_t m_nativeHandler;

    static auto invoke(httpd_req_t *req) -> esp_err_t {
        auto &self = *static_cast<const EndpointHandler*>(req->user_ctx);
        IncomingRequest wrappedReq{req};

        self.m_type.apply(wrappedReq);
        return self.m_callback(wrappedReq);
    }

public:
    EndpointHandler(const std::string &endpoint, Callback callback)
        : EndpointHandler{endpoint, plainTextType, callback} {}

    EndpointHandler(const std::string &endpoint, const ResponseType &type, Callback callback)
        : m_endpoint{endpoint}, m_type{type}, m_callback{callback} {
        // Ideally we'd like to use a tighter static init here, but until C++20 we don't have
        // C99-style designated initializers at our disposal
        m_nativeHandler.uri = m_endpoint.c_str();
        m_nativeHandler.method = T;
        m_nativeHandler.handler = invoke;
        m_nativeHandler.user_ctx = this;
    }

    /* Calling this more than once results in undefined behavior */
    auto registerWithServer(httpd_handle_t server) const -> esp_err_t {
        return httpd_register_uri_handler(server, &m_nativeHandler);
    }
};

using GetHandler = EndpointHandler<HTTP_GET>;
using PostHandler = EndpointHandler<HTTP_POST>;

/* Client helpers */
struct OutgoingRequest {
    using EventCallback = std::function<esp_err_t(esp_http_client_event_handle_t)>;

    esp_http_client_handle_t m_handle;
    const EventCallback m_callback;

    static auto nativeCallback(esp_http_client_event_handle_t event) -> esp_err_t {
        OutgoingRequest &self{*reinterpret_cast<OutgoingRequest *>(event->user_data)};

        return self.m_callback(event);
    }

    OutgoingRequest(esp_http_client_config_t config, EventCallback callback)
        : m_callback{callback} {
        config.user_data = this;
        config.event_handler = nativeCallback;
        m_handle = esp_http_client_init(&config);
    }

    auto perform() -> esp_err_t {
        return esp_http_client_perform(m_handle);
    }

    ~OutgoingRequest() {
        esp_http_client_cleanup(m_handle);
    }
};
} // namespace ZZ::HttpdUtil

#endif // ZZ_HTTPDUTIL_H
