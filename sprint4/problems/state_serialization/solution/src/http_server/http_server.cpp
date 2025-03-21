#include "http_server.h"

#include <stdexcept>

namespace http_server {
void SessionBase::Run() {
    net::dispatch(
        stream_.get_executor(),
        beast::bind_front_handler(&SessionBase::Read, GetSharedThis()));
}

void SessionBase::Read() {
    using namespace std::literals;
    request_ = {};
    stream_.expires_after(30s);
    http::async_read(
        stream_, buffer_, request_,
        beast::bind_front_handler(&SessionBase::OnRead, GetSharedThis()));
}

void SessionBase::OnRead(const beast::error_code ec,
                         [[maybe_unused]] const std::size_t bytes_read) {
    using namespace std::literals;
    if (ec == http::error::end_of_stream) {
        return Close();
    }

    if (ec) {
        return ReportError(ec, "read"sv);
    }

    HandlerRequest(std::move(request_));
}

void SessionBase::OnWrite(const bool close, const beast::error_code ec,
                          [[maybe_unused]] const std::size_t bytes_written) {
    using namespace std::literals;

    if (ec) {
        return ReportError(ec, "write"sv);
    }

    if (close) {
        return Close();
    }

    Read();
}

void SessionBase::Close() {
    beast::error_code ec;
    stream_.socket().shutdown(tcp::socket::shutdown_send, ec);
    if (ec) {
        throw std::runtime_error("Incorrect close session");
    }
}

}  // namespace http_server
