#ifndef THREAD_LIST_H
#define THREAD_LIST_H

#include <list>
#include <mutex>

template <typename T>
class ThreadList {
 public:
  using const_iterator = typename std::list<T>::const_iterator;
  using iterator = typename std::list<T>::iterator;

  auto front() {
    std::lock_guard<std::mutex> lg(m);
    return l.front();
  }

  auto back() {
    std::lock_guard<std::mutex> lg(m);
    return l.back();
  }

  const_iterator begin() const {
    std::lock_guard<std::mutex> lg(m);
    return l.begin();
  }

  const_iterator end() const {
    std::lock_guard<std::mutex> lg(m);
    return l.end();
  }

  bool empty() const {
    std::lock_guard<std::mutex> lg(m);
    return l.empty();
  }
  auto size() const {
    std::lock_guard<std::mutex> lg(m);
    return l.size();
  }

  auto take_front() {
    std::lock_guard<std::mutex> lg(m);
    auto front = std::move(l.front());
    l.pop_front();
    return front;
  }
  auto take_back() {
    std::lock_guard<std::mutex> lg(m);
    auto back = std::move(l.back());
    l.pop_back();
    return back;
  }

  void clear() {
    std::lock_guard<std::mutex> lg(m);
    l.clear();
  }

  auto insert(const_iterator& pos, const T& value) {
    std::lock_guard<std::mutex> lg(m);
    return l.insert(pos, value);
  }

  auto insert(const_iterator& pos, T&& value) {
    std::lock_guard<std::mutex> lg(m);
    return l.insert(pos, std::move(value));
  }

  template <typename... Args>
  auto emplace(const_iterator& pos, Args&&... args) {
    std::lock_guard<std::mutex> lg(m);
    return l.emplace(pos, std::move(args...));
  }

  template <typename... Args>
  auto emplace(const_iterator pos, Args&&... args) {
    std::lock_guard<std::mutex> lg(m);
    return l.emplace(pos, std::move(args...));
  }

  auto erase(const_iterator pos) {
    std::lock_guard<std::mutex> lg(m);
    return l.erase(pos);
  };

  void push_back(const T& value) {
    std::lock_guard<std::mutex> lg(m);
    l.push_back(value);
  }

  void push_back(T&& value) {
    std::lock_guard<std::mutex> lg(m);
    l.push_back(std::move(value));
  }

  template <typename... Args>
  auto& emplace_back(Args&&... args) {
    std::lock_guard<std::mutex> lg(m);
    return l.emplace_back(std::move(args...));
  }

  void pop_back() {
    std::lock_guard<std::mutex> lg(m);
    l.pop_back();
  }

  void push_front(const T& value) {
    std::lock_guard<std::mutex> lg(m);
    l.push_front(value);
  }

  void push_front(T&& value) {
    std::lock_guard<std::mutex> lg(m);
    l.push_front(std::move(value));
  }

  template <typename... Args>
  auto& emplace_front(Args&&... args) {
    std::lock_guard<std::mutex> lg(m);
    return l.emplace_front(std::move(args...));
  }

  void pop_front() {
    std::lock_guard<std::mutex> lg(m);
    l.pop_front();
  }

 private:
  mutable std::mutex m;
  std::list<T> l;
};

#endif  // THREAD_LIST_H