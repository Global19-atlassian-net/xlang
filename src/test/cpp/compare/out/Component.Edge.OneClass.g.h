﻿// WARNING: Please don't edit this file. It was generated by C++/WinRT v2.0.000000.0
#pragma once
#include "winrt/Component.Edge.h"
namespace winrt::Component::Edge::implementation
{
    template <typename D, typename... I>
    struct WINRT_EBO OneClass_base : implements<D, Component::Edge::IOneClass, I...>
    {
        using base_type = OneClass_base;
        using class_type = Component::Edge::OneClass;
        using implements_type = typename OneClass_base::implements_type;
        using implements_type::implements_type;
        
        operator impl::producer_ref<class_type> const() const noexcept
        {
            return { to_abi<default_interface<class_type>>(this) };
        }

        hstring GetRuntimeClassName() const
        {
            return L"Component.Edge.OneClass";
        }
    };
}
namespace winrt::Component::Edge::factory_implementation
{
    template <typename D, typename T, typename... I>
    struct WINRT_EBO OneClassT : implements<D, Windows::Foundation::IActivationFactory, Component::Edge::IOneClassFactory, Component::Edge::IOneClassStatics, I...>
    {
        using instance_type = Component::Edge::OneClass;

        hstring GetRuntimeClassName() const
        {
            return L"Component.Edge.OneClass";
        }
        Component::Edge::OneClass CreateInstance(int32_t a)
        {
            return make<T>(a);
        }
        void StaticMethod(int32_t a)
        {
            return T::StaticMethod(a);
        }
        [[noreturn]] Windows::Foundation::IInspectable ActivateInstance() const
        {
            throw hresult_not_implemented();
        }
    };
}

#if defined(WINRT_FORCE_INCLUDE_ONECLASS_XAML_G_H) || __has_include("Component.Edge.OneClass.xaml.g.h")
#include "Component.Edge.OneClass.xaml.g.h"
#else

namespace winrt::Component::Edge::implementation
{
    template <typename D, typename... I>
    using OneClassT = OneClass_base<D, I...>;
}

#endif
