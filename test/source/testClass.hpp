#ifndef TEST_CLASS_H
#define TEST_CLASS_H

#include <ostream>
#include <string>

namespace test_utils {

/**
 * @brief Simple test class allowing to count implicit copy and moves of object.
 *
 */
struct TestClass {
  TestClass(std::string name = "") : name_(name) {}
  TestClass(const TestClass& o)
      : name_(o.name_),
        copy_count_(o.copy_count_ + 1),
        move_count_(o.move_count_) {}
  TestClass& operator=(const TestClass& o) {
    name_ = o.name_;
    copy_count_ = o.copy_count_ + 1;
    move_count_ = o.move_count_;
    return *this;
  }
  bool operator==(const std::string& s) const { return name_ == s; }
  bool operator==(const TestClass& tc) const { return name_ == tc.name_; }

  TestClass(TestClass&& o)
      : name_(std::move(o.name_)),
        copy_count_(o.copy_count_),
        move_count_(o.move_count_ + 1) {}
  TestClass& operator=(TestClass&& o) {
    name_ = o.name_;
    copy_count_ = o.copy_count_;
    move_count_ = o.move_count_ + 1;
    return *this;
  }
  ~TestClass() {}

  friend std::ostream& operator<<(std::ostream& os, const TestClass& tc);

  std::string name_;
  int copy_count_;
  int move_count_;
};

std::ostream& operator<<(std::ostream& os, const TestClass& tc) {
  os << tc.name_ << " copied: " << tc.copy_count_
     << " moved: " << tc.move_count_ << "\n";
  return os;
}

}  // namespace test_utils

#endif  // TEST_CLASS_H
