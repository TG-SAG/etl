#ifndef ETL_BITSET_TEST_CASES_INCLUDED
#define ETL_BITSET_TEST_CASES_INCLUDED

#include "unit_test_framework.h"

#include <bitset>
#include <cstdint>

namespace bitset_test
{
  template <size_t Width, typename TBitset>
  bool matches(const TBitset& data, const std::bitset<Width>& expected)
  {
    for (size_t position = 0; position < Width; ++position)
    {
      if (data.test(position) != expected.test(position))
      {
        return false;
      }
    }

    return data.count() == expected.count() && data.all() == expected.all() &&
           data.any() == expected.any() && data.none() == expected.none();
  }

  template <size_t Width, typename TBitset>
  void check_padding(TBitset& data)
  {
    data.reset();
    CHECK_EQUAL(size_t(0), data.count());
    data.flip();
    CHECK_EQUAL(Width, data.count());
    CHECK(data.all());
    CHECK(data.any());
    CHECK(!data.none());

    data.reset(Width - 1U);
    CHECK_EQUAL(Width - 1U, data.count());
    data.set(Width - 1U);
    data.flip();
    CHECK(data.none());
    CHECK_EQUAL(size_t(0), data.count());

    data.set();
    CHECK_EQUAL(Width, data.count());
    data.flip();
    CHECK_EQUAL(size_t(0), data.count());
    CHECK(data.none());
    CHECK(!data.any());

    data.reset().flip();
    data >>= 1U;
    std::bitset<Width> expected;
    expected.set();
    CHECK((matches<Width>(data, expected >> 1U)));
  }

  template <size_t Width, typename TBitset>
  void check_mutations(TBitset& data)
  {
    std::bitset<Width> expected;
    data.reset();
    for (size_t position = 0; position < Width; ++position)
    {
      data.set(position, position % 3U == 0U);
      expected.set(position, position % 3U == 0U);
    }
    CHECK((matches<Width>(data, expected)));

    for (size_t position = 0; position < Width; ++position)
    {
      data[position] = !data[position];
      expected[position] = !expected[position];
    }
    CHECK((matches<Width>(data, expected)));

    data[Width - 1U] = data[0];
    expected[Width - 1U] = expected[0];
    CHECK((matches<Width>(data, expected)));
    data[Width - 1U].flip();
    expected[Width - 1U].flip();
    CHECK((matches<Width>(data, expected)));

    data.reset();
    data.set(Width - 1U);
    data.from_string("1");
    CHECK((matches<Width>(data, std::bitset<Width>(1ULL))));
    data.from_string("");
    CHECK((matches<Width>(data, std::bitset<Width>())));
  }

  template <size_t Width, typename TBitset>
  void check_shifts(TBitset& data)
  {
    std::bitset<Width> expected;
    for (size_t position = 0; position < Width; ++position)
    {
      expected.set(position, position % 3U == 2U || position == Width - 1U);
    }

    size_t first_bad_left = TBitset::npos;
    size_t first_bad_right = TBitset::npos;
    for (size_t shift = 0; shift <= Width + 1U; ++shift)
    {
      for (size_t position = 0; position < Width; ++position)
      {
        data.set(position, expected[position]);
      }
      data <<= shift;
      if (!matches<Width>(data, expected << shift) && first_bad_left == TBitset::npos)
      {
        first_bad_left = shift;
      }

      data.reset();
      for (size_t position = 0; position < Width; ++position)
      {
        data.set(position, expected[position]);
      }
      data >>= shift;
      if (!matches<Width>(data, expected >> shift) && first_bad_right == TBitset::npos)
      {
        first_bad_right = shift;
      }
    }
    CHECK_EQUAL(TBitset::npos, first_bad_left);
    CHECK_EQUAL(TBitset::npos, first_bad_right);
    data.set();
    data <<= TBitset::npos;
    CHECK(data.none());
    data.set();
    data >>= TBitset::npos;
    CHECK(data.none());
  }

  template <size_t Width, typename TBitset>
  void check_search(TBitset& data)
  {
    bool correct = true;
    for (size_t pattern = 0; pattern < 3U; ++pattern)
    {
      std::bitset<Width> expected;
      data.reset();
      for (size_t position = 0; position < Width; ++position)
      {
        const bool state = pattern == 1U || (pattern == 2U && position % 3U == 2U);
        data.set(position, state);
        expected.set(position, state);
      }

      for (size_t state = 0; state < 2U; ++state)
      {
        for (size_t start = 0; start <= Width + 1U; ++start)
        {
          size_t found = TBitset::npos;
          for (size_t position = start; position < Width; ++position)
          {
            if (expected[position] == bool(state))
            {
              found = position;
              break;
            }
          }
          correct = (data.find_next(bool(state), start) == found) && correct;
          if (start == 0)
          {
            correct = (data.find_first(bool(state)) == found) && correct;
          }
        }
      }
    }
    CHECK(correct);
    CHECK_EQUAL(TBitset::npos, data.find_next(false, TBitset::npos));
    CHECK_EQUAL(TBitset::npos, data.find_next(true, TBitset::npos));
  }

  template <size_t Width, typename TBitset>
  void check_value_shifts(const TBitset& data)
  {
    std::bitset<Width> expected;
    for (size_t position = 0; position < Width; ++position)
    {
      expected.set(position, data.test(position));
    }
    bool correct_left = true;
    bool correct_right = true;
    for (size_t shift = 0; shift <= Width + 1U; ++shift)
    {
      correct_left = matches<Width>(data << shift, expected << shift) && correct_left;
      correct_right = matches<Width>(data >> shift, expected >> shift) && correct_right;
    }
    CHECK(correct_left);
    CHECK(correct_right);
    CHECK((matches<Width>(data << TBitset::npos, std::bitset<Width>())));
    CHECK((matches<Width>(data >> TBitset::npos, std::bitset<Width>())));
  }

  template <size_t Width, typename TOverflow, typename TBitset>
  void check_extract(TBitset& data)
  {
    typedef typename TBitset::element_type element_type;
    const element_type expected_top_mask = static_cast<element_type>(TBitset::All_Set_Element >> (TBitset::allocated_bits() - Width));
    CHECK_EQUAL(expected_top_mask, data.top_mask());
    data.reset();
    for (size_t position = 0; position < Width; ++position)
    {
      data.set(position, position % 3U == 0U || position == Width - 1U);
    }

    bool unsigned_correct = true;
    bool signed_correct = true;
    for (size_t position = 0; position < Width; ++position)
    {
      for (size_t length = 1; length <= 32U && length <= Width - position; ++length)
      {
        uint32_t expected = 0;
        for (size_t offset = 0; offset < length; ++offset)
        {
          if (data.test(position + offset))
          {
            expected |= uint32_t(1) << offset;
          }
        }
        int64_t signed_expected = expected;
        if ((expected & (uint32_t(1) << (length - 1U))) != 0U)
        {
          signed_expected -= int64_t(1) << length;
        }
        unsigned_correct = (data.template extract<uint32_t>(position, length) == expected) && unsigned_correct;
        signed_correct = (data.template extract<int32_t>(position, length) == signed_expected) && signed_correct;
      }
    }
    CHECK(unsigned_correct);
    CHECK(signed_correct);
    CHECK_EQUAL(uint32_t(1), (data.template extract<uint32_t, Width - 1U, 1U>()));
    CHECK_EQUAL(int32_t(-1), (data.template extract<int32_t, Width - 1U, 1U>()));
    if (Width >= 64U)
    {
      data.set();
      CHECK_EQUAL(~uint64_t(0), data.template extract<uint64_t>(Width - 64U, 64U));
      CHECK_EQUAL(int64_t(-1), data.template extract<int64_t>(Width - 63U, 63U));
      CHECK_EQUAL(int64_t(-1), data.template extract<int64_t>(Width - 64U, 64U));
    }
    CHECK_THROW(data.template extract<uint32_t>(Width, 1U), TOverflow);
    CHECK_THROW(data.template extract<uint32_t>(0U, 33U), TOverflow);
    CHECK_THROW(data.template extract<uint32_t>(Width - 1U, 2U), TOverflow);
  }
}

#endif