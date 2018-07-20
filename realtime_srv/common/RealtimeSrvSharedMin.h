#pragma once


#ifdef __linux__
#define IS_LINUX
#else
#ifdef _WIN32
#define IS_WIN
#else
#define IS_MAC
#endif
#endif


#include <functional>
#include <stdint.h>
#include <memory>

#include <cstring>
#include <cmath>

#include <vector>
#include <unordered_map>
#include <map>
#include <string>
#include <list>
#include <queue>
#include <deque>
#include <set>
#include <unordered_set>
#include <cassert>

#include "realtime_srv/common/noncopyable.h"


namespace realtime_srv
{

using std::shared_ptr;
using std::unique_ptr;
using std::weak_ptr;
using std::vector;
using std::queue;
using std::list;
using std::deque;
using std::unordered_map;
using std::map;
using std::string;
using std::set;
using std::unordered_set;
using std::function;

using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;

}
