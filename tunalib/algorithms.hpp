#pragma once

#include "initializer_list.h"

#include <stdlib.h>

// Get rid of any macros that someone already may have defined.
#undef min
#undef max
#undef clamp
#undef bit
#undef abs
#undef labs

namespace Tuna
{
  namespace internal
  {
    template <typename T, typename... Args>
    constexpr inline __forceinline __flatten auto min_v(arg_type<T> cur_val, arg_type<T> arg, arg_type<Args> ... args)
    {
      if constexpr(sizeof...(args) == 0)
      {
        return (cur_val < arg) ? cur_val : arg;
      }
      else
      {
        return min_v((cur_val < arg) ? cur_val : arg, args...);
      }
    }

    template <typename T, typename... Args>
    constexpr inline __forceinline __flatten auto max_v(arg_type<T> cur_val, arg_type<T> arg, arg_type<Args> ... args)
    {
      if constexpr(sizeof...(args) == 0)
      {
        return (cur_val >= arg) ? cur_val : arg;
      }
      else
      {
        return max_v((cur_val >= arg) ? cur_val : arg, args...);
      }
    }
  }
  
  template <typename T, typename... Args>
  constexpr inline __forceinline __flatten auto min(arg_type<T> arg, arg_type<Args> ... args)
  {
    return internal::min_v(arg, args...);
  }

  template <typename T, typename... Args>
  constexpr inline __forceinline __flatten auto max(arg_type<T> arg, arg_type<Args> ... args)
  {
    return internal::max_v(arg, args...);
  }

  template <typename T>
  constexpr inline __forceinline __flatten T abs(arg_type<T> arg)
  {
    if constexpr(type_trait<T>::is_unsigned)
    {
      return arg;
    }
    else
    {
      return (arg < T(0)) ? -arg : arg;
    }
  }

  template <typename T>
  constexpr inline __forceinline __flatten typename type_trait<T>::unsigned_type uabs(arg_type<T> arg)
  {
    if constexpr(type_trait<T>::is_unsigned)
    {
      return arg;
    }
    else
    {
      return (arg < T(0)) ? -arg : arg;
    }
  }

  template <typename T>
  constexpr inline __forceinline __flatten auto clamp(arg_type<T> val, arg_type<T> _min, arg_type<T> _max)
  {
    return min(max(val, _min), _max);
  }

  template <typename T, typename R = typename type_trait<typename type_trait<T>::unsigned_type>::smaller_type>
  constexpr inline __forceinline __flatten R hi(arg_type<T> value)
  {
    constexpr uint8 shift = (sizeof(R) * 8);

    return type_trait<T>::as_unsigned(value) >> shift;
  }

  template <typename T, typename R = typename type_trait<typename type_trait<T>::unsigned_type>::smaller_type>
  constexpr inline __forceinline __flatten R lo(arg_type<T> value)
  {
    return type_trait<T>::as_unsigned(value);
  }

  template <typename T, typename U>
  constexpr inline __forceinline __flatten bool is_within(arg_type<T> value, arg_type<U> min, arg_type<U> max)
  {
    return value >= min && value <= max;
  }

  // Simple static-cast like routine. Also makes extracting sub-types easier from encapsulations like 'flash'.
  template <typename T = void, typename U>
  constexpr inline __forceinline __flatten auto as(arg_type<U> value);

  namespace _internal
  {
    template <typename T>
    constexpr inline __forceinline __flatten bool is_pow2(arg_type<T> value)
    {
      return (value != T{ 0 }) && !(value & (value - T{1}));
    }

    template <typename T>
    inline uint8 log2_rt(T value)
    {
      // Runtime variant.
      // TODO https://graphics.stanford.edu/~seander/bithacks.html
      // TODO support uint64, maybe? Presently we don't. would take more memory.
      static constexpr const T b[] = { 0x2, 0xC, 0xF0, T(0xFF00), T(0xFFFF0000) };
      static constexpr const uint8 S[] = { 1, 2, 4, 8, 16 };

      uint8 result = 0;
      for (int8 i = sizeof(T); i >= 0; --i)
      {
        __assume(i >= 0 && i <= sizeof(T));
        if (value & b[i])
        {
          const uint8 S_val = S[i];
          value >>= S_val;
          result |= S_val;
        }
      }

      return result;
    }

    template <typename T>
    constexpr inline uint8 log2(arg_type<T> value, uint8 r = 0, bool pow2 = true)
    {
      using unsigned_t = typename type_trait<T>::unsigned_type;

      if (__builtin_constant_p(value))
      {
        // Compile-time variant that can be expressed as constexpr. We don't want recursion otherwise.
        const unsigned_t uvalue = unsigned_t{ value };
        if (uvalue > 1)
        {
          const unsigned_t new_value = uvalue >> 1_u8;
          return _internal::log2<unsigned_t>(new_value, r + 1, pow2 && is_pow2(new_value) && is_pow2(uvalue));
        }
        else
        {
          if (r == 0 && value == 0)
          {
            return 0;
          }
          return pow2 ? (r) : (r + 1);
        }
      }
      else
      {
        // Runtime variant.
        return log2_rt(unsigned_t{ value });
      }
    }

    constexpr_test(log2, 10_u8);
    constexpr_test(is_pow2, 10_u8);
  }

  namespace constant
  {
    template <uint64 v> constexpr const uint8 log2 = _internal::log2(v);
    template <uint64 v> constexpr const bool is_pow2 = _internal::is_pow2(v);
  }

  template <typename T>
  constexpr inline uint8 log2(arg_type<T> value)
  {
    return _internal::log2(value);
  }

  template <typename T>
  constexpr inline __forceinline __flatten bool is_pow2(arg_type<T> value)
  {
    return _internal::is_pow2(value);
  }

  template <typename T>
  constexpr inline __forceinline __flatten __const remove_reference<T> bit (uint8 _bit)
  {
    return remove_reference<T>(1 << _bit);
  }

  template <typename T, typename... BITS>
  constexpr inline __forceinline __flatten __const remove_reference<T> bits (BITS... _bits)
  {
    remove_reference<T> value = 0;

    T _unused[] = { (value |= bit<T>(_bits))... };

    return value;
  }
}
