#ifndef __VIREN2D_HELPERS_ENUM_H__
#define __VIREN2D_HELPERS_ENUM_H__

#include <type_traits>

// Provide bitwise operators for type-safe enums (must be in global scope)

//DONE [x] add documentation
//DONE [x] add C++ test (tests/xxx_test.cpp)
/** Bitwise OR */
template<typename Enum>
Enum operator |(Enum lhs, Enum rhs) {
  static_assert(std::is_enum<Enum>::value, "Template parameter is not an enum type");
  using underlying = typename std::underlying_type<Enum>::type;
  return static_cast<Enum> (
    static_cast<underlying>(lhs) | static_cast<underlying>(rhs)
  );
}


//DONE [x] add documentation
//DONE [x] add C++ test (tests/xxx_test.cpp)
/** Bitwise OR assignment*/
template<typename Enum>
Enum& operator |=(Enum& lhs, Enum rhs) {
  static_assert(std::is_enum<Enum>::value, "Template parameter is not an enum type");
  using underlying = typename std::underlying_type<Enum>::type;
  lhs = static_cast<Enum> (
    static_cast<underlying>(lhs) | static_cast<underlying>(rhs)
  );
  return lhs;
}


//DONE [x] add documentation
//DONE [x] add C++ test (tests/xxx_test.cpp)
/** Bitwise AND */
template<typename Enum>
Enum operator &(Enum lhs, Enum rhs) {
  static_assert(std::is_enum<Enum>::value, "Template parameter is not an enum type");
  using underlying = typename std::underlying_type<Enum>::type;
  return static_cast<Enum> (
    static_cast<underlying>(lhs) & static_cast<underlying>(rhs)
  );
}


//DONE [x] add documentation
//DONE [x] add C++ test (tests/xxx_test.cpp)
/** Bitwise AND assignment */
template<typename Enum>
Enum& operator &=(Enum& lhs, Enum rhs) {
  static_assert(std::is_enum<Enum>::value, "Template parameter is not an enum type");
  using underlying = typename std::underlying_type<Enum>::type;
  lhs = static_cast<Enum> (
    static_cast<underlying>(lhs) & static_cast<underlying>(rhs)
  );
  return lhs;
}


//DONE [x] add documentation
//DONE [x] add C++ test (tests/xxx_test.cpp)
/** Bitwise XOR */
template<typename Enum>
Enum operator ^(Enum lhs, Enum rhs) {
  static_assert(std::is_enum<Enum>::value, "Template parameter is not an enum type");
  using underlying = typename std::underlying_type<Enum>::type;
  return static_cast<Enum> (
    static_cast<underlying>(lhs) ^ static_cast<underlying>(rhs)
  );
}


//DONE [x] add documentation
//DONE [x] add C++ test (tests/xxx_test.cpp)
/** Bitwise XOR assignment */
template<typename Enum>
Enum& operator ^=(Enum& lhs, Enum rhs) {
  static_assert(std::is_enum<Enum>::value, "Template parameter is not an enum type");
  using underlying = typename std::underlying_type<Enum>::type;
  lhs = static_cast<Enum> (
    static_cast<underlying>(lhs) ^
    static_cast<underlying>(rhs)
  );
  return lhs;
}


//DONE [x] add documentation
//DONE [x] add C++ test (tests/xxx_test.cpp)
/** Bitwise NOT */
template<typename Enum>
Enum operator ~(Enum rhs) {
  static_assert(std::is_enum<Enum>::value, "Template parameter is not an enum type");
  using underlying = typename std::underlying_type<Enum>::type;
  return static_cast<Enum> (~static_cast<underlying>(rhs));
}



namespace viren2d {
//DONE [x] add documentation
//DONE [x] add C++ test (tests/xxx_test.cpp)
/** Check if the given flag is set. */
template<typename Enum>
bool IsFlagSet(Enum value, Enum flag) {
  static_assert(std::is_enum<Enum>::value, "Template parameter is not an enum type");
  using underlying = typename std::underlying_type<Enum>::type;
  return static_cast<underlying>(value & flag) != 0;
}


/**
 * @brief Iterator listing all enum values.
 *
 * Only works for continuous enum definitions, i.e.
 * there cannot be "gaps" between enum values.
 *
 * Taken from https://stackoverflow.com/a/31836401
 */
template <typename C, C beginval, C endval>
class ContinuousEnumIterator {
 private:
  typedef typename std::underlying_type<C>::type val_t;
  int val;

 public:
  ContinuousEnumIterator(const C & f)
    : val(static_cast<val_t>(f))
  {}

  ContinuousEnumIterator()
    : val(static_cast<val_t>(beginval))
  {}

  ContinuousEnumIterator operator++() {
    ++val;
    return *this;
  }

  C operator*() {
    return static_cast<C>(val);
  }

  ContinuousEnumIterator begin() const {
    return *this;  // default ctor is good
  }

  ContinuousEnumIterator end() const {
      static const ContinuousEnumIterator enditer = ++ContinuousEnumIterator(endval);  // cache it
      return enditer;
  }

  bool operator!=(const ContinuousEnumIterator& i) {
    return val != i.val;
  }
};

}  // namespace viren2d

#endif  // __VIREN2D_HELPERS_ENUM_H__

