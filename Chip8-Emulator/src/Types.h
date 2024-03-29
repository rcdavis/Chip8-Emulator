#pragma once

#include <memory>
#include <cstdint>

using std::uint8_t;
using std::uint16_t;
using std::uint32_t;

template <typename T>
using Scope = std::unique_ptr<T>;

template <typename T>
using Ref = std::shared_ptr<T>;

template <typename T, typename... Args>
Scope<T> CreateScope(Args&&... args) { return std::make_unique<T>(std::forward<Args>(args)...); }

template <typename T, typename... Args>
Ref<T> CreateRef(Args&&... args) { return std::make_shared<T>(std::forward<Args>(args)...); }

template <typename T, typename U>
Ref<T> StaticCastRef(const Ref<U>& r) { return std::static_pointer_cast<T>(r); }

template <typename T, typename U>
Ref<T> DynamicCastRef(const Ref<U>& r) { return std::dynamic_pointer_cast<T>(r); }
