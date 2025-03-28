#pragma once
#include <cassert>
#include <chrono>
#include <functional>
#include <memory>
#include <utility>

namespace loot_gen {

class LootGenerator {
   public:
    using RandomGenerator = std::function<double()>;
    using TimeInterval = std::chrono::milliseconds;
    using Pointer = std::shared_ptr<LootGenerator>;

    /*
     * base_interval - базовый отрезок времени > 0
     * probability - вероятность появления трофея в течение базового интервала
     * времени random_generator - генератор псевдослучайных чисел в диапазоне от
     * [0 до 1]
     */
    LootGenerator(TimeInterval base_interval, double probability,
                  TimeInterval time_without_loot,
                  RandomGenerator random_gen = DefaultGenerator)
        : base_interval_{base_interval},
          probability_{probability},
          time_without_loot_(time_without_loot),
          random_generator_{std::move(random_gen)} {
        assert(base_interval_.count() > 0);
        assert(0 <= probability_ && probability_ <= 1);
    }

    /*
     * Возвращает количество трофеев, которые должны появиться на карте спустя
     * заданный промежуток времени.
     * Количество трофеев, появляющихся на карте не превышает количество
     * мародёров.
     *
     * time_delta - отрезок времени, прошедший с момента предыдущего вызова
     * Generate loot_count - количество трофеев на карте до вызова Generate
     * looter_count - количество мародёров на карте
     */
    unsigned Generate(TimeInterval time_delta, unsigned loot_count,
                      unsigned looter_count);

    TimeInterval GetBaseInterval() const { return base_interval_; }
    double GetProbability() const { return probability_; }
    TimeInterval GetTimeWithoutLoot() const { return time_without_loot_; }

   private:
    static double DefaultGenerator() noexcept { return 1.0; };
    TimeInterval base_interval_;
    double probability_;
    TimeInterval time_without_loot_{};
    RandomGenerator random_generator_;
};

}  // namespace loot_gen
