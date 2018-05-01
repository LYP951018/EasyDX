#pragma once
//qualifier applied to function type has no meaning; ignored 1> unique_ptr(pointer _Ptr,
#pragma warning(disable : 4180)

#define NOMINMAX

#include <cstdint>
#include <cstddef>
#include <type_traits>
#include <utility>
#include <functional>
#include <memory>
#include <tuple>
#include <vector>
#include <algorithm>
#include <gsl/gsl>
#include <optional>
#include <string_view>
#include <string>
#include <chrono>
#include <filesystem>
#include <wrl/client.h>
#include <variant>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <string>
#include <array>
#include "DXDef.hpp"
#include "Detection.hpp"
#include "EasyDx.Common/Common.hpp"
#include "Concepts.hpp"
