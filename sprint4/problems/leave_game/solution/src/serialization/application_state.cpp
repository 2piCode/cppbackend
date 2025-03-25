#include "application_state.h"

#include <fstream>
#include <string>

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

#include "app/application.h"
#include "serialization/application_serialization.h"

void SaveApplicationState(app::Application::Pointer app,
                          std::filesystem::path state_file) {
    std::ofstream ofs(state_file);
    if (!ofs.is_open()) {
        BOOST_LOG_TRIVIAL(info) << "Invalid state file for saving application: "
                                << state_file.string();
        return;
    }

    serialization::ApplicationRepr app_repr(*app);
    boost::archive::text_oarchive oa{ofs};
    oa << app_repr;
    ofs.close();
}

std::optional<app::Application::Pointer> LoadApplicationState(
    std::filesystem::path state_file) {
    std::ifstream ifs(state_file);
    if (!ifs.is_open()) {
        BOOST_LOG_TRIVIAL(info)
            << "Invalid state file for loading application: "
            << state_file.string();
        return std::nullopt;
    }
    serialization::ApplicationRepr app_repr;
    boost::archive::text_iarchive ia{ifs};
    ia >> app_repr;
    return app_repr.Restore();
}
