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

template <typename T>
class vector_ref {
public:
  using iterator = const T*;

  const static size_t npos{std::numeric_limits<size_t>::max()};

  vector_ref(const std::vector<T>& vec)
    : m_data {vec.data()},
      m_sz   {vec.size()}
  { }

  template <typename I>
  vector_ref(I first, I last)
  : m_data {&*first},
    m_sz   ( static_cast<size_t>(last - first) )
  { }

  vector_ref subvec(size_t front, size_t back = npos) const
  {
    return {m_data + front, m_data + (back == npos ? m_sz : back)};
  }

  vector_ref shifted_by(long offset) const
  {
    return {m_data + offset, m_data + m_sz};
  }

  const T& front() const { return *m_data; }
  const T& back() const { return m_data[m_sz - 1]; }

  size_t size() const { return m_sz; }

  const T& operator[](int idx) { return m_data[idx]; }

  iterator begin() const { return m_data; }
  iterator end() const { return m_data + m_sz; }

  const T* data() const { return m_data; }

private:
  const T* m_data;
  size_t m_sz;
};

inline bool isnamechar(char c)
{
  return !isspace(c) && (!ispunct(c) || c == '_');
}

template <typename T>
vector_ref<T> ltrim(vector_ref<T> vec, const T& item)
{
  auto last = find_if(begin(vec), end(vec),
                      [&](const auto& i) { return i != item; });
  return vec.subvec(static_cast<size_t>(last - begin(vec)));
}

template <typename T, typename F>
vector_ref<T> ltrim_if(vector_ref<T> vec, const F& pred)
{
  auto last = find_if_not(begin(vec), end(vec), pred);
  return vec.subvec(static_cast<size_t>(last - begin(vec)));
}

}

#endif
