#pragma once

#include "noncopyable.h"

namespace rs{
  
class ServerBase : noncopyable {
public:
  virtual void start();
  virtual void stop();
};

void ServerBase::start(){

}

void ServerBase::stop(){

}

} // namespace rs