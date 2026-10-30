#ifndef ETL_VECTOR_TEST_CASES_INCLUDED
#define ETL_VECTOR_TEST_CASES_INCLUDED

#include "unit_test_framework.h"

#include <algorithm>
#include <vector>

namespace vector_test
{
  template <typename TVector>
  void check_equal(const TVector& data, const std::vector<typename TVector::value_type>& expected)
  {
    CHECK_EQUAL(expected.size(), data.size());
    if (expected.size() != data.size())
    {
      return;
    }
    CHECK_EQUAL(data.capacity() - expected.size(), data.available());
    CHECK_EQUAL(expected.empty(), data.empty());
    CHECK_EQUAL(expected.size() == data.capacity(), data.full());
    CHECK(std::equal(expected.begin(), expected.end(), data.begin()));
    CHECK(std::equal(expected.rbegin(), expected.rend(), data.rbegin()));
    CHECK(data.cbegin() == data.begin());
    CHECK(data.cend() == data.end());
    if (!expected.empty())
    {
      CHECK(data.front() == expected.front());
      CHECK(data.back() == expected.back());
      CHECK(data.at(expected.size() - 1U) == expected.back());
    }
  }

  template <typename TVector>
  void check_insert_erase(TVector& data, const typename TVector::value_type* values)
  {
    typedef typename TVector::difference_type difference_type;
    typedef typename TVector::value_type value_type;
    typename TVector::pointer const original_buffer = data.data();
    for (size_t length = 0; length <= data.capacity(); ++length)
    {
      for (size_t position = 0; position <= length; ++position)
      {
        if (length != data.capacity())
        {
          data.assign(values, values + length);
          std::vector<value_type> expected(values, values + length);
          expected.insert(expected.begin() + difference_type(position), values[0]);
          CHECK(data.insert(data.begin() + difference_type(position), values[0]) == data.begin() + difference_type(position));
          check_equal(data, expected);
        }

        for (size_t count = 0; count <= data.capacity() - length; ++count)
        {
          data.assign(values, values + length);
          std::vector<value_type> expected(values, values + length);
          expected.insert(expected.begin() + difference_type(position), count, values[0]);
          data.insert(data.begin() + difference_type(position), count, values[0]);
          check_equal(data, expected);

          data.assign(values, values + length);
          expected.assign(values, values + length);
          expected.insert(expected.begin() + difference_type(position), values, values + count);
          data.insert(data.begin() + difference_type(position), values, values + count);
          check_equal(data, expected);
        }

        for (size_t finish = position; finish <= length; ++finish)
        {
          data.assign(values, values + length);
          std::vector<value_type> expected(values, values + length);
          expected.erase(expected.begin() + difference_type(position), expected.begin() + difference_type(finish));
          CHECK(data.erase(data.begin() + difference_type(position), data.begin() + difference_type(finish)) ==
                data.begin() + difference_type(position));
          check_equal(data, expected);
        }
      }
    }
    CHECK(data.data() == original_buffer);
  }
}

#endif