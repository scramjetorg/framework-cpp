#include <atomic>
#include <future>
#include <iostream>

#include "helpers/Logger/logger.hpp"
#include "ifca/ifca.hpp"
#include "ifca/transform/map.hpp"

int main(int /*argc*/, char* /*argv*/[]) {
  std::atomic<float> sum = 0;
  auto sumValues = [&sum](int value) {
    sum = sum.load() + value;
    return value;
  };
  auto ifca = ifca::Ifca().addTransform(ifca::map(sumValues));

  auto inputReader = std::async(std::launch::async, [&]() {
    int c;
    float line = 0;
    while (std::cin >> c) {
      LOG_DEBUG() << "cin value: " << c;
      ifca.write(c);
      line += 1;
      std::cout << "Write line: " << line << "\r";
    }
    LOG_INFO() << "EoF reached";
    ifca.end();
  });

  auto outputReader = std::async(std::launch::async, [&]() {
      while (true) ifca.read().get();
  });
  outputReader.wait();
  std::cout << "Calculated sum: " << sum << std::endl;
  return 0;
}