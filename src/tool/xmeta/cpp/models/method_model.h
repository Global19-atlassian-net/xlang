#pragma once

#include <optional>
#include <string_view>
#include <vector>

#include "base_model.h"
#include "formal_parameter_model.h"
#include "model_ref.h"
#include "model_types.h"

namespace xlang::xmeta
{
    struct method_semantics
    {
        bool is_protected = false;
        bool is_static = false;
    };

    struct method_model : base_model
    {
        method_model() = delete;
        method_model(std::string_view const& id, size_t decl_line, 
                std::string_view const& assembly_name, 
                std::optional<type_ref>&& return_type, 
                std::vector<formal_parameter_model>&& formal_params,
                method_semantics const& sem) :
            base_model{ id, decl_line, assembly_name },
            m_formal_parameters{ std::move(formal_params) },
            m_return_type{ std::move(return_type) },
            m_semantic{ sem },
            m_implemented_method_ref{ "" }
        { }

        method_model(std::string_view const& id, size_t decl_line,
                std::string_view const& assembly_name,
                std::optional<type_ref>&& return_type) :
            base_model{ id, decl_line, assembly_name },
            m_return_type{ std::move(return_type) },
            m_implemented_method_ref{ "" }
        { }


        method_model(std::string_view const& id, 
                size_t decl_line, 
                std::string_view const& assembly_name, 
                std::optional<type_ref>&& return_type, 
                std::vector<formal_parameter_model>&& formal_params,
                std::string_view const& overridden_method_ref) :
            base_model{ id, decl_line, assembly_name },
            m_formal_parameters{ std::move(formal_params) },
            m_return_type{ std::move(return_type) },
            m_implemented_method_ref{ std::string(overridden_method_ref) }
        { }

        auto const& get_formal_parameters() const noexcept
        {
            return m_formal_parameters;
        }

        auto const& get_overridden_method_ref() const noexcept
        {
            return m_implemented_method_ref;
        }

        auto const& get_return_type() const noexcept
        {
            return m_return_type;
        }

        auto const& get_semantic() const noexcept
        {
            return m_semantic;
        }

        void set_overridden_method_ref(std::shared_ptr<method_model> const& ref) noexcept
        {
            m_implemented_method_ref.resolve(ref);
        }

        void add_formal_parameter(formal_parameter_model&& formal_param)
        {
            m_formal_parameters.emplace_back(std::move(formal_param));
        }

        void resolve(std::map<std::string, class_type_semantics> const& symbols, std::string fully_qualified_id)
        {
            if (m_return_type)
            {
                if (!m_return_type->get_semantic().is_resolved())
                {
                    // TODO: Once we have using directives, we will need to go through many fully_qualified_ids here
                    std::string ref_name = m_return_type->get_semantic().get_ref_name();
                    std::string symbol = ref_name.find(".") != std::string::npos ? ref_name : fully_qualified_id + "." + ref_name;
                    auto iter = symbols.find(symbol);
                    if (iter == symbols.end())
                    {
                        // TODO: Reccord the unresolved type and continue
                    }
                    else
                    {
                        m_return_type->set_semantic(iter->second);
                    }
                }
            }
            for (formal_parameter_model & param : m_formal_parameters)
            {
                param.resolve(symbols, fully_qualified_id);
            }
        }

    private:
        method_semantics m_semantic;
        std::optional<type_ref> m_return_type;
        std::vector<formal_parameter_model> m_formal_parameters;
        model_ref<std::shared_ptr<method_model>> m_implemented_method_ref;
        // TODO: Add type parameters (generic types)
    };
}
