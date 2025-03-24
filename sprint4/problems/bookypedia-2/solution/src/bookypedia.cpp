#include "bookypedia.h"

#include <iostream>

#include "app/unit_of_work_factory_impl.h"
#include "menu/menu.h"
#include "ui/view.h"

namespace bookypedia {

using namespace std::literals;

Application::Application(const AppConfig& config)
    : use_cases_{std::make_unique<app::UnitOfWorkFactoryImpl>(
          pqxx::connection(config.db_url))} {}

void Application::Run() {
    menu::Menu menu{std::cin, std::cout};

    menu.AddAction("Help"s, {}, "Show instructions"s, [&menu](std::istream&) {
        menu.ShowInstructions();
        return true;
    });
    menu.AddAction("Exit"s, {}, "Exit program"s,
                   [&menu](std::istream&) { return false; });
    ui::View view{menu, use_cases_, std::cin, std::cout};
    menu.Run();
}

}  // namespace bookypedia
