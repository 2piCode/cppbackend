#pragma once

#include "app/game/game.h"

struct ListMapResult {
    app::Game::Maps maps;
};

class ListMapUseCase {
   public:
    explicit ListMapUseCase(app::Game::Pointer game) : game_(game) {}

    app::Game::Maps GetMaps() const { return game_->GetMaps(); }

   private:
    app::Game::Pointer game_;
};
