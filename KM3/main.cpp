#include <atomic>
#include <future>
#include <iostream>

#include "helpers/Logger/logger.hpp"
#include "ifca/exceptions.hpp"
#include "ifca/ifca.hpp"
#include "ifca/transform/map.hpp"

int main(int /*argc*/, char* /*argv*/[]) {
  std::atomic<size_t> sum = 0;
  size_t linesRead = 0;
  auto sumValues = [&sum](int value) {
    sum = sum.load() + value;
    return value;
  };
  auto ifca = ifca::Ifca(20).addTransform(ifca::map(sumValues));

  auto inputReader = std::async(std::launch::async, [&]() {
    int c;
    size_t line = 0;
    try {
      while (std::cin >> c) {
        ifca.write(c).wait();
        line += 1;
        if (line % 1000 == 0) std::cout << "Written line: " << line << "\r";
        if (std::cin.fail()) {
          LOG_ERROR() << "Fail bit reached";
          break;
        }
      }
      LOG_INFO() << "EoF reached";
    } catch (const std::exception& exc) {
      LOG_ERROR() << "Write error catched: " << exc.what();
    } catch (...) {
      LOG_ERROR() << "Write unknown error catched";
    }
    LOG_INFO() << "Written lines: " << line << "\n";
    ifca.end();
    return line;
  });

  auto outputReader = std::async(std::launch::async, [&]() {
    size_t readLines = 0;
    while (true) {
      try {
        ifca.read().get();
      } catch (ReadEnd&) {
        LOG_INFO() << "Read End catched";
        break;
      } catch (...) {
        LOG_ERROR() << "Read unknown error catched";
        break;
      }
      ++readLines;
    };
    LOG_INFO() << "Lines read: " << readLines;
  });
  linesRead = inputReader.get();
  outputReader.wait();
  std::cout << "Calculated sum: " << sum << "\n"
            << "Values read: " << linesRead << "\n"
            << "Average: " << sum / static_cast<float>(linesRead) << "\n";

  return 0;
}