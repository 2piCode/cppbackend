#pragma once

#include <random>
#include <string>

#include "model/model.h"
#include "model/tagged.h"

class PlayerTokens {
   public:
    PlayerTokens() = default;
    PlayerTokens(const PlayerTokens&) = delete;
    PlayerTokens& operator=(const PlayerTokens&) = delete;
    std::string GetToken(const model::Dog* dog) const;
    std::string GetToken(const model::Dog::Id& id) const;

   private:
};
