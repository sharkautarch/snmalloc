#pragma once

#include "../ds_core/ds_core.h"

namespace snmalloc
{
  template<size_t MIN_BITS, typename F>
  void range_to_pow_2_blocks(capptr::Chunk<void> base, size_t length, F f)
  {
    auto end = pointer_offset(base, length);
    base = pointer_align_up(base, bits::one_at_bit(MIN_BITS));
    end = pointer_align_down(end, bits::one_at_bit(MIN_BITS));
    length = pointer_diff(base, end);

    bool first = true;

    // Find the minimum set of maximally aligned blocks in this range.
    // Each block's alignment and size are equal.
    while (length >= sizeof(void*))
    {
      size_t base_align_bits = bits::ctz(address_cast(base));
      size_t length_align_bits = (bits::BITS - 1) - bits::clz(length);
      size_t align_bits = bits::min(base_align_bits, length_align_bits);
      size_t align = bits::one_at_bit(align_bits);

      /*
       * Now that we have found a maximally-aligned block, we can set bounds
       * and be certain that we won't hit representation imprecision.
       */
      f(base, align, first);
      first = false;

      base = pointer_offset(base, align);
      length -= align;
    }
  }

  /**
   * Forward definition to allow multiple template specialisations.
   *
   * This struct is used to recursively compose ranges.
   */
  template<typename... Args>
  struct PipeImpl;

  /**
   * Base case of one range that needs nothing.
   */
  template<typename Only>
  struct PipeImpl<Only>
  {
    using result = Only;
  };

  /**
   * Recursive case of applying a base range as an argument to the
   * next, and then using that as the new base range.
   */
  template<typename First, typename Fun, typename... Rest>
  struct PipeImpl<First, Fun, Rest...>
  {
  public:
    using result =
      typename PipeImpl<typename Fun::template Apply<First>, Rest...>::result;
  };

  /**
   * Nice type so the caller doesn't need to call result explicitly.
   */
  template<typename... Args>
  using Pipe = typename PipeImpl<Args...>::result;
} // namespace snmalloc