﻿// WARNING: Please don't edit this file. It was generated by C++/WinRT v2.0.000000.0
#pragma once
#include "winrt/base.h"
namespace winrt::Component
{
    struct INotExclusive;
}
namespace winrt::Component::Fast
{
    struct SlowClass;
    struct FastClass;
}
namespace winrt::Component::Result
{
    struct IClassStatics;
    struct Class;
}
namespace winrt::impl
{
    template <> struct category<Component::Result::IClassStatics>
    {
        using type = interface_category;
    };
    template <> struct category<Component::Result::Class>
    {
        using type = class_category;
    };
    template <> struct name<Component::Result::IClassStatics>
    {
        static constexpr auto & value{ L"Component.Result.IClassStatics" };
    };
    template <> struct name<Component::Result::Class>
    {
        static constexpr auto & value{ L"Component.Result.Class" };
    };
    template <> struct guid_storage<Component::Result::IClassStatics>
    {
        static constexpr guid value{ 0xD062B990,0x72D1,0x5E23,{ 0xAF,0x55,0xC5,0xAB,0x71,0xA4,0x6F,0xBD } };
    };
    template <> struct abi<Component::Result::IClassStatics>
    {
        struct type : inspectable_abi
        {
            virtual int32_t WINRT_CALL GetInt32(int32_t*) noexcept = 0;
            virtual int32_t WINRT_CALL GetString(void**) noexcept = 0;
            virtual int32_t WINRT_CALL GetFastClass(void**) noexcept = 0;
            virtual int32_t WINRT_CALL GetSlowClass(void**) noexcept = 0;
            virtual int32_t WINRT_CALL GetInterface(void**) noexcept = 0;
        };
    };
    template <typename D>
    struct consume_Component_Result_IClassStatics
    {
        int32_t GetInt32() const;
        hstring GetString() const;
        Component::Fast::FastClass GetFastClass() const;
        Component::Fast::SlowClass GetSlowClass() const;
        Component::INotExclusive GetInterface() const;
    };
    template <> struct consume<Component::Result::IClassStatics>
    {
        template <typename D> using type = consume_Component_Result_IClassStatics<D>;
    };
}
