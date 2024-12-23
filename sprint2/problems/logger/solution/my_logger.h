#pragma once

#include <chrono>
#include <fstream>
#include <iomanip>
#include <mutex>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <thread>

using namespace std::literals;

#define LOG(...) Logger::GetInstance().Log(__VA_ARGS__)

class Logger {
    auto GetTime() const {
        if (manual_ts_) {
            return *manual_ts_;
        }

        return std::chrono::system_clock::now();
    }

    auto GetTimeStamp() const {
        const auto now = GetTime();
        const auto t_c = std::chrono::system_clock::to_time_t(now);
        return std::put_time(std::localtime(&t_c), "%F %T");
    }

    // Для имени файла возьмите дату с форматом "%Y_%m_%d"
    std::string GetFileTimeStamp() const {
        const auto now = GetTime();
        const auto t_c = std::chrono::system_clock::to_time_t(now);
        std::ostringstream os;
        os << std::put_time(std::localtime(&t_c), "%Y_%m_%d");
        return os.str();
    }

    Logger() = default;
    Logger(const Logger&) = delete;

   public:
    static Logger& GetInstance() {
        static Logger obj;
        return obj;
    }

    // Выведите в поток все аргументы.
    template <class... Ts>
    void Log(const Ts&... args) {
        std::lock_guard<std::mutex> lock(m_);
        std::ofstream log_file(
            "/var/log/sample_log_" + GetFileTimeStamp() + ".log",
            std::ios::app);
        log_file << GetTimeStamp() << ": ";
        (log_file << ... << args) << std::endl;
    }

    // Установите manual_ts_. Учтите, что эта операция может выполняться
    // параллельно с выводом в поток, вам нужно предусмотреть
    // синхронизацию.
    void SetTimestamp(std::chrono::system_clock::time_point ts) {
        std::lock_guard<std::mutex> lock(m_);
        manual_ts_ = ts;
    }

   private:
    std::mutex m_;
    std::optional<std::chrono::system_clock::time_point> manual_ts_;
};
