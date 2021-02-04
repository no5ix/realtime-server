#pragma once

namespace rs{

class noncopyable{
public:
  noncopyable(const noncopyable&) = delete;
  void operator=(const noncopyable&) = delete;

protected:
  noncopyable() = default;
  ~noncopyable() = default;
};

} // namespace rs