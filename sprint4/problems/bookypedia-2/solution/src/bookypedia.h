#pragma once
#include <pqxx/pqxx>

#include "app/use_cases_impl.h"

namespace bookypedia {

struct AppConfig {
    std::string db_url;
};

class Application {
   public:
    explicit Application(const AppConfig& config);

    void Run();

   private:
    app::UseCasesImpl use_cases_;
};

}  // namespace bookypedia
