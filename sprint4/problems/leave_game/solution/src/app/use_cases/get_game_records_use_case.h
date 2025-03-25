#pragma once

#include <memory>
#include <string>
#include <vector>

#include <boost/json/object.hpp>

#include "app/use_cases/base.h"
#include "postgres/unit_of_work.h"
#include "utils/logger.h"

enum class GetGameRecordsErrorReason { UnknownToken };

struct GetGameRecordsError : public UseCaseError {
    GetGameRecordsError(std::string code, std::string message,
                        GetGameRecordsErrorReason reason)
        : UseCaseError(code, message), reason(reason) {}

    GetGameRecordsErrorReason reason;
};

struct GameRecord {
    std::string name;
    int score;
    double time_in_game;
};

class GetGameRecordsUseCase {
   public:
    explicit GetGameRecordsUseCase(
        std::shared_ptr<postgres::UnitOfWorkFactory> factory)
        : factory_(factory) {}

    std::vector<GameRecord> GetGameRecords(int start, int max_items) {
        auto unit_of_work = factory_->CreateUnitOfWork();
        auto players = unit_of_work->GetPlayers().Read(start, max_items);
        std::vector<GameRecord> result;

        result.reserve(players.size());
        for (const auto& player : players) {
            GameRecord info;
            info.name = player.name;
            info.score = player.score;
            info.time_in_game = player.time_in_game;
            result.push_back(info);
        }
        return result;
    }

   private:
    std::shared_ptr<postgres::UnitOfWorkFactory> factory_;
};
