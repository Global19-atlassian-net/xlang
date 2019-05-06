#pragma once

#include <string_view>

#include "base_model.h"
#include "method_model.h"
#include "model_types.h"

namespace xlang::xmeta
{
    struct property_semantics
    {
        bool is_protected = false;
        bool is_static = false;
    };

    struct property_model : base_model
    {
        property_model() = delete;
        property_model(std::string_view const& id, 
                size_t decl_line, 
                std::string_view const& assembly_name, 
                property_semantics const& sem, 
                type_ref&& type,
                std::shared_ptr<method_model> const& get_method, 
                std::shared_ptr<method_model> const& set_method) :
            base_model{ id, decl_line, assembly_name },
            m_semantic{ sem },
            m_type{ std::move(type) },
            m_get_method{ get_method },
            m_set_method{ set_method }
        { }

        property_model(std::string_view const& id,
                size_t decl_line,
                std::string_view const& assembly_name,
                type_ref&& type) :
            base_model{ id, decl_line, assembly_name },
            m_type{ std::move(type) }
        { }

        auto const& get_semantic() const noexcept
        {
            return m_semantic;
        }

        auto const& get_type() const noexcept
        {
            return m_type;
        }

        auto const& get_get_method() const noexcept
        {
            return m_get_method;
        }

        auto const& get_set_method() const noexcept
        {
            return m_set_method;
        }

        void set_get_method(std::shared_ptr<method_model> const& m)
        {
            m_get_method = m;
        }

        void set_set_method(std::shared_ptr<method_model> const& m)
        {
            m_set_method = m;
        }

        void resolve(std::map<std::string, class_type_semantics> symbols, std::string fully_qualified_id)
        {
            if (!m_type.get_semantic().is_resolved())
            {
                /* Events should not have been resolved. If it was, it means it was not a
                class type and not a delegate type */
                std::string ref_name = m_type.get_semantic().get_ref_name();
                std::string symbol = ref_name.find(".") != std::string::npos
                    ? ref_name : fully_qualified_id + "." + ref_name;
                auto iter = symbols.find(symbol);
                if (iter == symbols.end())
                {
                    // TODO: Record the unresolved type and continue once we have a good error story for reporting errors in models
                }
                else
                {
                    m_type.set_semantic(iter->second);
                }
            }
        }

    private:
        property_semantics m_semantic;
        type_ref m_type;
        bool m_is_array;
        std::shared_ptr<method_model> m_get_method;
        std::shared_ptr<method_model> m_set_method;
    };
}
