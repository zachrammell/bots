/*
 * Useful mix-ins, free functions, and template nonsense
 */

#pragma once
#include <type_traits>
#include <filesystem>

template <typename T>
class non_copyable
{
public:
  non_copyable(non_copyable const&) = delete;
  T& operator= (T const&) = delete;
protected:
  non_copyable() = default;
  ~non_copyable() = default; // Protected non-virtual destructor
};

template<bool Condition, typename ValType>
struct conditional_value;

template<typename ValType>
struct conditional_value<true, ValType>
{
  ValType& value;

  conditional_value(ValType&& A, ValType&&)
    : value{ A }
  {}
};

template<typename ValType>
struct conditional_value<false, ValType>
{
  ValType& value;

  conditional_value(ValType&&, ValType&& B)
    : value{ B }
  {}
};

template<typename ValType>
struct conditional_value<true, std::initializer_list<ValType>>
{
  std::initializer_list<ValType>& value;

  conditional_value(std::initializer_list<ValType>&& A, std::initializer_list<ValType>&&)
    : value{ A }
  {}
};

template<typename ValType>
struct conditional_value<false, std::initializer_list<ValType>>
{
  std::initializer_list<ValType>& value;

  conditional_value(std::initializer_list<ValType>&&, std::initializer_list<ValType>&& B)
    : value{ B }
  {}
};

template<typename Enum>
constexpr auto to_integral(Enum e) -> std::underlying_type_t<Enum>
{
  return static_cast<std::underlying_type_t<Enum>>(e);
}

template<typename Target, typename Source>
constexpr auto brute_cast(Source const& s) -> std::enable_if_t<sizeof(Target) == sizeof(Source), Target>
{
  union { Target t; Source s; } u;
  u.s = s;
  return u.t;
}

namespace detail
{

template <typename Member, typename Class>
struct offset_of
{
  union U
  {
    char c;
    Member m; // instance of type of member
    Class object;
    constexpr U() : c(0) {} // Avoid calling any constructors by making c the active member
  };
  static constexpr U u = {};

  static constexpr std::ptrdiff_t offset(Member (Class::* member))
  {
    // The single member is positioned at the same place as the class,
    // so the distance between the start of the class and the in-class member's address is the offset.
    return (std::addressof(u.object.*member) - std::addressof(u.m))
            * sizeof(Member);
  }
};

}

template <typename T1, typename T2>
constexpr std::ptrdiff_t offset_of(T1 T2::* member)
{
  return detail::offset_of<T1, T2>::offset(member);
}

std::size_t number_of_files_in_directory(std::filesystem::path const& path);
