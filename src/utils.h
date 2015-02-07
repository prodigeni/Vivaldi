#ifndef IL_UTILS_H
#define IL_UTILS_H

#include <vector>
#include <boost/utility/string_ref.hpp>

namespace il {

inline boost::string_ref ltrim(boost::string_ref str)
{
  auto last = std::find_if_not(begin(str), end(str), isspace);
  str.remove_prefix(last - begin(str));
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
  vector_ref(I first, I last) : m_data{&*first}, m_sz( last - first ) { }

  vector_ref remove_prefix(int prefix) const
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

}

#endif
