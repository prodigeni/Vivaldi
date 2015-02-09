#ifndef VV_UTILS_H
#define VV_UTILS_H

#include <vector>
#include <boost/utility/string_ref.hpp>

namespace vv {

inline boost::string_ref ltrim(boost::string_ref str)
{
  auto last = std::find_if_not(begin(str), end(str), isspace);
  str.remove_prefix(static_cast<size_t>(last - begin(str)));
  return str;
}

inline boost::string_ref remove_prefix(boost::string_ref str, size_t prefix)
{
  str.remove_prefix(prefix);
  return str;
}

template <typename T>
class vector_ref {
public:
  using iterator = const T*;

  vector_ref(const std::vector<T>& vec)
    : m_data {vec.data()},
      m_sz   {vec.size()}
  { }

  template <typename I>
  vector_ref(I first, I last)
  : m_data {&*first},
    m_sz   ( static_cast<size_t>(last - first) )
  { }

  vector_ref remove_prefix(long prefix) const
  {
    return {m_data + prefix, m_data + m_sz};
  }

  const T& front() const { return *m_data; }
  const T& back() const { return m_data[m_sz - 1]; }

  size_t size() const { return m_sz; }

  const T& operator[](int idx) { return m_data[idx]; }

  iterator begin() const { return m_data; }
  iterator end() const { return m_data + m_sz; }

private:
  const T* m_data;
  size_t m_sz;
};

inline bool isnamechar(char c)
{
  return !ispunct(c) || c == '_';
}

template <typename T>
vector_ref<T> ltrim(vector_ref<T> vec, const T& item)
{
  auto last = find_if(begin(vec), end(vec),
                      [&](const auto& i) { return i != item; });
  return vec.remove_prefix(last - begin(vec));
}

}

#endif
