#pragma once
#include "boost/asio/dispatch.hpp"
#include "boost/smart_ptr/shared_ptr.hpp"
#include "clock.h"
#ifdef _WIN32
#include <sdkddkver.h>
#endif

#include <boost/asio/bind_executor.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/post.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/strand.hpp>
#include <memory>

#include "hotdog.h"
#include "result.h"

namespace net = boost::asio;
using namespace std::chrono;

// Функция-обработчик операции приготовления хот-дога
using HotDogHandler = std::function<void(Result<HotDog> hot_dog)>;

class ThreadChecker {
   public:
    explicit ThreadChecker(std::atomic_int& counter) : counter_{counter} {}

    ThreadChecker(const ThreadChecker&) = delete;
    ThreadChecker& operator=(const ThreadChecker&) = delete;

    ~ThreadChecker() {
        // assert выстрелит, если между вызовом конструктора и деструктора
        // значение expected_counter_ изменится
        assert(expected_counter_ == counter_);
    }

   private:
    std::atomic_int& counter_;
    int expected_counter_ = ++counter_;
};

class Order : public std::enable_shared_from_this<Order> {
   public:
    Order(net::io_context& io, int id, Store& store, GasCooker& gas_cooker,
          HotDogHandler handler)
        : io_(io),
          id_(id),
          gas_cooker_(gas_cooker),
          bread_(store.GetBread()),
          sausage_(store.GetSausage()),
          handler_(std::move(handler)) {}

    void Execute() {
        net::post(strand_,
                  [self = shared_from_this()]() { self->BakeBread(); });

        net::post(strand_,
                  [self = shared_from_this()]() { self->FrySausage(); });
    }

   private:
    net::io_context& io_;
    net::strand<net::io_context::executor_type> strand_{net::make_strand(io_)};
    int id_;
    GasCooker& gas_cooker_;
    net::steady_timer bread_timer_{io_};
    net::steady_timer sausage_timer_{io_};
    std::shared_ptr<Bread> bread_;
    std::shared_ptr<Sausage> sausage_;
    HotDogHandler handler_;
    bool is_ready_ = false;
    std::atomic_int counter_{0};

    void FrySausage() {
        sausage_->StartFry(gas_cooker_, [self = shared_from_this()]() {
            self->sausage_timer_.expires_after(
                HotDog::MIN_SAUSAGE_COOK_DURATION);
            self->sausage_timer_.async_wait([self](sys::error_code ec) {
                self->sausage_->StopFry();
                self->AssembleHotDog();
            });
        });
    }

    void BakeBread() {
        bread_->StartBake(gas_cooker_, [self = shared_from_this()]() {
            self->bread_timer_.expires_after(HotDog::MIN_BREAD_COOK_DURATION);
            self->bread_timer_.async_wait([self](sys::error_code ec) {
                net::dispatch(self->strand_, [self, ec] {
                    self->bread_->StopBaking();
                    self->OnBaked();
                });
            });
        });
    }

    void OnFried() {
        ThreadChecker checker{counter_};
        net::post(strand_,
                  [self = shared_from_this()]() { self->AssembleHotDog(); });
    }

    void OnBaked() {
        ThreadChecker checker{counter_};
        net::post(strand_,
                  [self = shared_from_this()]() { self->AssembleHotDog(); });
    }

    void AssembleHotDog() {
        if (is_ready_) {
            return;
        }
        if (bread_->IsCooked() && sausage_->IsCooked()) {
            handler_(Result<HotDog>(HotDog{id_, sausage_, bread_}));
            is_ready_ = true;
        }
    }
};

// Класс "Кафетерий". Готовит хот-доги
class Cafeteria {
   public:
    explicit Cafeteria(net::io_context& io) : io_{io} {}

    // Асинхронно готовит хот-дог и вызывает handler, как только хот-дог будет
    // готов. Этот метод может быть вызван из произвольного потока
    void OrderHotDog(HotDogHandler handler) {
        // TODO: Реализуйте метод самостоятельно
        // При необходимости реализуйте дополнительные классы
        net::dispatch(strand_, [this, handler]() {
            const int id = this->next_order_id_++;
            std::make_shared<Order>(io_, id, store_, *gas_cooker_,
                                    std::move(handler))
                ->Execute();
        });
    }

   private:
    net::io_context& io_;
    net::strand<net::io_context::executor_type> strand_{net::make_strand(io_)};
    int next_order_id_ = 0;
    // Используется для создания ингредиентов хот-дога
    Store store_;
    // Газовая плита. По условию задачи в кафетерии есть только одна газовая
    // плита на 8 горелок Используйте её для приготовления ингредиентов
    // хот-дога. Плита создаётся с помощью make_shared, так как GasCooker
    // унаследован от enable_shared_from_this.
    std::shared_ptr<GasCooker> gas_cooker_ = std::make_shared<GasCooker>(io_);
};
