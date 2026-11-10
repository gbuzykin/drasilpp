#pragma once

#include "util/type_id.h"

#include <uxs/common.h>

#include <concepts>
#include <memory>
#include <utility>

#define DRASILPP_IMPLEMENT_DYNAMIC_CAST(...) \
 protected: \
    UXS_FORCE_INLINE const void* dynamicCast(util::type_id id) const override { \
        using namespace drasilpp; \
        using ThisTy = std::remove_cv_t<std::remove_pointer_t<decltype(this)>>; \
        return detail::dynamicCastImpl<ThisTy, AsIface<ThisTy> __VA_OPT__(, ) __VA_ARGS__>{}(this, id); \
    } \
    UXS_FORCE_INLINE void* dynamicCast(util::type_id id) { \
        return const_cast<void*>(std::as_const(*this).dynamicCast(id)); \
    } \
\
 private: \
    static_assert(true)

namespace drasilpp {

struct RttiBase {
    virtual ~RttiBase() = default;
    virtual const void* dynamicCast(util::type_id id) const = 0;
    UXS_FORCE_INLINE void* dynamicCast(util::type_id id) {
        return const_cast<void*>(std::as_const(*this).dynamicCast(id));
    }
};

template<typename Ty>
struct AsIface {
    using type = Ty;
};

template<typename TyTo, typename TyFrom>
    requires std::convertible_to<TyFrom*, TyTo*>
TyTo* upCast(TyFrom* ptr) {
    return ptr;
}

template<typename TyTo, typename TyFrom>
    requires std::convertible_to<TyFrom*, TyTo*>
std::shared_ptr<TyTo> upCast(const std::shared_ptr<TyFrom>& ptr) {
    return {ptr};
}

template<typename TyTo, typename TyFrom>
    requires std::convertible_to<TyFrom*, TyTo*>
std::shared_ptr<TyTo> upCast(std::shared_ptr<TyFrom>&& ptr) {
    return {std::move(ptr)};
}

template<typename TyTo, typename TyFrom>
TyTo* dynamicCast(TyFrom* ptr) {
    if constexpr (std::is_convertible_v<TyFrom*, TyTo*>) {
        return ptr;
    } else {
        return ptr ? static_cast<TyTo*>(ptr->dynamicCast(util::type_traits<TyTo>::id())) : nullptr;
    }
}

template<typename TyTo, typename TyFrom>
std::shared_ptr<TyTo> dynamicCast(const std::shared_ptr<TyFrom>& ptr) {
    if (auto* result = dynamicCast<TyTo>(ptr.get())) { return {ptr, result}; }
    return nullptr;
}

template<typename TyTo, typename TyFrom>
std::shared_ptr<TyTo> dynamicCast(std::shared_ptr<TyFrom>&& ptr) {
    if (auto* result = dynamicCast<TyTo>(ptr.get())) { return {std::move(ptr), result}; }
    return nullptr;
}

template<typename TyTo, typename TyFrom>
bool isKindOf(TyFrom* ptr) {
    if constexpr (std::is_convertible_v<TyFrom*, TyTo*>) {
        return true;
    } else {
        return ptr && ptr->dynamicCast(util::type_traits<TyTo>::id());
    }
}

template<typename TyTo, typename TyFrom>
bool isKindOf(const std::shared_ptr<TyFrom>& ptr) {
    return isKindOf<TyTo>(ptr.get());
}

namespace detail {
template<typename, typename...>
struct dynamicCastImpl {};
template<typename Ty>
struct dynamicCastImpl<Ty> {
    const void* operator()(const Ty*, util::type_id) const { return nullptr; }
};
template<typename Ty, typename TyTo, typename... Tail>
struct dynamicCastImpl<Ty, AsIface<TyTo>, Tail...> {
    const void* operator()(const Ty* ptr, util::type_id id) const {
        if (id == util::type_traits<TyTo>::id()) { return static_cast<const TyTo*>(ptr); }
        return dynamicCastImpl<Ty, Tail...>{}(ptr, id);
    }
};
template<typename Ty, typename TyTo, typename... Tail>
struct dynamicCastImpl<Ty, TyTo, Tail...> {
    const void* operator()(const Ty* ptr, util::type_id id) const {
        if (const void* result = dynamicCastImpl<Ty, Tail...>{}(ptr, id)) { return result; }
        return ptr->TyTo::dynamicCast(id);
    }
};
}  // namespace detail

}  // namespace drasilpp
