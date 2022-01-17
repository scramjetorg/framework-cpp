#ifndef TEST_DATA_H
#define TEST_DATA_H

#include <string>
#include <vector>

#include "../testClass.hpp"

namespace test_utils {

template <typename inputType, typename outputType = inputType>
struct TestData {
  using input = inputType;
  using output = outputType;

  std::vector<inputType> testSequence() {
    throw std::invalid_argument("Unknown input sequence for given type");
  }
  std::function<outputType(inputType)> modifyData() {
    throw std::invalid_argument(
        "Unknown transform each function for given type");
  }
  std::vector<inputType> modifiedTestSequence() {
    throw std::invalid_argument("Unknown expected result for given type");
  }
  std::function<outputType(inputType)> filterData() {
    throw std::invalid_argument(
        "Unknown transform filter function for given type");
  }
  std::vector<inputType> filteredTestSequence() {
    throw std::invalid_argument("Unknown expected result for given type");
  }
};

template <>
struct TestData<int> {
  using input = int;
  using output = int;

  std::vector<int> testSequence() { return std::vector<int>{1, 2, 1, 3, 2, 4}; }
  std::function<int(int&)> modifyData() {
    return [](int& chunk) -> int { return chunk + 1; };
  }
  std::vector<int> modifiedTestSequence() {
    return std::vector<int>{2, 3, 2, 4, 3, 5};
  }
  std::function<bool(int&)> filterData() {
    return [](int& chunk) -> bool { return static_cast<bool>(chunk % 2); };
  }
  std::vector<int> filteredTestSequence() { return std::vector<int>{2, 2, 4}; }
};

template <>
struct TestData<std::string> {
  using input = std::string;
  using output = std::string;

  std::vector<std::string> testSequence() {
    return std::vector<std::string>{"napis 1", "napis 2", "napis 3",
                                    "napis 4", "napis 5", "napis 6"};
  }
  std::function<std::string(std::string)> modifyData() {
    return [](std::string chunk) -> std::string { return chunk.append("0"); };
  }
  std::vector<std::string> modifiedTestSequence() {
    return std::vector<std::string>{"napis 10", "napis 20", "napis 30",
                                    "napis 40", "napis 50", "napis 60"};
  }
  std::function<bool(std::string&)> filterData() {
    return [](std::string& chunk) -> bool {
      return chunk.back() == '2' || chunk.back() == '4' || chunk.back() == '6';
    };
  }
  std::vector<std::string> filteredTestSequence() {
    return std::vector<std::string>{"napis 20", "napis 40", "napis 60"};
  }
};

template <>
struct TestData<TestClass, std::string> {
  using input = TestClass;
  using output = std::string;

  std::vector<TestClass> testSequence() {
    return std::vector<TestClass>{
        TestClass("TestClass1"), TestClass("TestClass2"),
        TestClass("TestClass3"), TestClass("TestClass4"),
        TestClass("TestClass5"), TestClass("TestClass6")};
  }
  std::function<std::string(const TestClass&)> modifyData() {
    return [](const TestClass& chunk) -> std::string {
      auto name = chunk.name_;
      return name.append("0");
    };
  }
  std::vector<std::string> modifiedTestSequence() {
    return std::vector<std::string>{"TestClass10", "TestClass20",
                                    "TestClass30", "TestClass40",
                                    "TestClass50", "TestClass60"};
  }
  std::function<bool(const TestClass&)> filterData() {
    return [](const TestClass& chunk) -> bool {
      return chunk.name_.back() == '2' || chunk.name_.back() == '4' ||
             chunk.name_.back() == '6';
    };
  }
  std::vector<std::string> filteredTestSequence() {
    return std::vector<std::string>{"TestClass20", "TestClass40",
                                    "TestClass60"};
  }
};

template <>
struct TestData<TestClass*> {
  using input = TestClass*;
  using output = TestClass*;

  TestData()
      : v_test_class({new TestClass("TestClass1"), new TestClass("TestClass2"),
                      new TestClass("TestClass3"), new TestClass("TestClass4"),
                      new TestClass("TestClass5"),
                      new TestClass("TestClass6")}) {}
  ~TestData() {
    for (auto&& i : v_test_class) {
      delete i;
    };
  }

  std::vector<TestClass*> testSequence() {
    return std::vector<TestClass*>(v_test_class);
  }
  std::function<TestClass*(TestClass*)> modifyData() {
    return [](TestClass* chunk) -> TestClass* {
      chunk->name_.append("0");
      return chunk;
    };
  }
  std::vector<TestClass*> modifiedTestSequence() {
    return std::vector<TestClass*>(v_test_class);
  }
  std::function<bool(const TestClass*)> filterData() {
    return [](const TestClass* chunk) -> bool {
      return chunk->name_.back() == '2' || chunk->name_.back() == '4' ||
             chunk->name_.back() == '6';
    };
  }
  std::vector<TestClass*> filteredTestSequence() {
    std::vector<TestClass*> vec;
    vec.push_back(v_test_class[1]);
    vec.push_back(v_test_class[3]);
    vec.push_back(v_test_class[5]);
    return vec;
  }

 private:
  std::vector<TestClass*> v_test_class;
};

}  // namespace test_utils

#endif  // TEST_DATA_H
