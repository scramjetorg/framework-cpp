#ifndef CRTP_H
#define CRTP_H

// The Curiously Recurring Template Pattern
template <typename T, template <typename> class crtpPhantomType>
struct crtp {
  T& derived() { return static_cast<T&>(*this); }
  T const& derived() const { return static_cast<T const&>(*this); }

 private:
  crtp() {}
  friend crtpPhantomType<T>;
};

#endif  // CRTP_H
