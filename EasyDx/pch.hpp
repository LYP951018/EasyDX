#pragma once
//qualifier applied to function type has no meaning; ignored 1> unique_ptr(pointer _Ptr,
#pragma warning(disable : 4180)

#include <cstdint>
#include <cstddef>
#include <type_traits>
#include <utility>
#include <functional>
#include <memory>
#include <vector>
#include <algorithm>
#include <gsl/span>
#include <range/v3/all.hpp>

#include "Common.hpp"