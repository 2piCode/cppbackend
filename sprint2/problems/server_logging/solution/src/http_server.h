#pragma once
#include <cstddef>
#include <iostream>
#include <utility>

#include "boost/beast/core/bind_handler.hpp"
#include "boost/beast/core/error.hpp"
#include "boost/beast/core/flat_buffer.hpp"
#include "boost/beast/http/message.hpp"
#include "boost/beast/http/string_body.hpp"
#include "sdk.h"
// boost.beast будет использовать std::string_view вместо boost::string_view
#define BOOST_BEAST_USE_STD_STRING_VIEW

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>

namespace http_server {

namespace net = boost::asio;
using tcp = net::ip::tcp;
namespace beast = boost::beast;
namespace http = beast::http;
namespace sys = boost::system;

inline void ReportError(const beast::error_code ec,
                        const std::string_view what) {
    using namespace std::literals;
    std::cerr << what << ": "sv << ec.message() << std::endl;
}

class SessionBase {
   public:
    SessionBase(const SessionBase&) = delete;
    SessionBase& operator=(const SessionBase&) = delete;

    void Run();

   protected:
    using HttpRequest = http::request<http::string_body>;

    SessionBase(tcp::socket&& socket) : stream_(std::move(socket)) {}
    ~SessionBase() = default;

    template <typename Body, typename Fields>
    void Write(http::response<Body, Fields>&& response) {
        auto safe_response =
            std::make_shared<http::response<Body, Fields>>(std::move(response));
        auto self = GetSharedThis();
        http::async_write(stream_, *safe_response,
                          [safe_response, self](beast::error_code ec,
                                                std::size_t bytes_written) {
                              self->OnWrite(safe_response->need_eof(), ec,
                                            bytes_written);
                          });
    }

    std::string GetClientIp() const {
        return stream_.socket().remote_endpoint().address().to_string();
    }

   private:
    beast::tcp_stream stream_;
    beast::flat_buffer buffer_;
    HttpRequest request_;

    void Read();

    void OnRead(const beast::error_code ec,
                [[maybe_unused]] const std::size_t bytes_read);

    void OnWrite(const bool close, const beast::error_code ec,
                 [[maybe_unused]] const std::size_t bytes_written);

    void Close();

    virtual void HandlerRequest(HttpRequest&& request) = 0;

    virtual std::shared_ptr<SessionBase> GetSharedThis() = 0;
};

template <typename RequestHandler>
class Session : public SessionBase,
                public std::enable_shared_from_this<Session<RequestHandler>> {
   public:
    template <typename Handler>
    Session(tcp::socket&& socket, Handler&& handler)
        : SessionBase(std::move(socket)),
          request_handler_(std::forward<Handler>(handler)) {}

   private:
    RequestHandler request_handler_;

    std::shared_ptr<SessionBase> GetSharedThis() override {
        return this->shared_from_this();
    }

    void HandlerRequest(HttpRequest&& request) override {
        request_handler_(GetClientIp(), std::move(request),
                         [self = this->shared_from_this()](auto&& response) {
                             self->Write(std::move(response));
                         });
    }
};

template <typename RequestHandler>
class Listener : public std::enable_shared_from_this<Listener<RequestHandler>> {
   public:
    template <typename Handler>
    Listener(net::io_context& ioc, const tcp::endpoint& endpoint,
             Handler&& request_handler)
        : ioc_(ioc),
          acceptor_(net::make_strand(ioc)),
          request_handler_(std::forward<Handler>(request_handler)) {
        acceptor_.open(endpoint.protocol());
        acceptor_.set_option(net::socket_base::reuse_address(true));
        acceptor_.bind(endpoint);
        acceptor_.listen(net::socket_base::max_listen_connections);
    }

    void Run() { DoAccept(); }

   private:
    net::io_context& ioc_;
    tcp::acceptor acceptor_;
    RequestHandler request_handler_;

    void DoAccept() {
        acceptor_.async_accept(
            net::make_strand(ioc_),
            beast::bind_front_handler(&Listener::OnAccept,
                                      this->shared_from_this()));
    }

    void OnAccept(const sys::error_code ec, tcp::socket socket) {
        using namespace std::literals;
        if (ec) {
            return ReportError(ec, "accept"sv);
        }

        AsyncRunSession(std::move(socket));

        DoAccept();
    }

    void AsyncRunSession(tcp::socket&& socket) {
        std::make_shared<Session<RequestHandler>>(std::move(socket),
                                                  request_handler_)
            ->Run();
    }
};

template <typename RequestHandler>
inline void ServeHttp(net::io_context& ioc, const tcp::endpoint& endpoint,
                      RequestHandler&& handler) {
    using MyListener = Listener<std::decay_t<RequestHandler>>;

    std::make_shared<MyListener>(ioc, endpoint,
                                 std::forward<RequestHandler>(handler))
        ->Run();
}

}  // namespace http_server
