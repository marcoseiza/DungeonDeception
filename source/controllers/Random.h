#ifndef CONTROLLERS_RANDOM_H_
#define CONTROLLERS_RANDOM_H_

#include <random>

class Random {
 public:
  /** Initialize the random class with a random device seed. */
  static void Init() { engine.seed(std::random_device()()); }

  /**
   * Returns a random number between [0, 1].
   * @return A random number.
   */
  static float Float() {
    return (float)dist(engine) / (float)std::numeric_limits<uint32_t>::max();
  }

 private:
  static std::mt19937 engine;
  static std::uniform_int_distribution<std::mt19937::result_type> dist;
};

#endif  // CONTROLLERS_RANDOM_H_