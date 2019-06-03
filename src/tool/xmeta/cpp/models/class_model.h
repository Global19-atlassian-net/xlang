#pragma once

#include <memory>
#include <string_view>
#include <string>
#include <vector>
#include <variant>
#include "class_or_interface_model.h"
#include "model_types.h"

namespace xlang::xmeta
{
    struct class_modifier
    {
        bool is_sealed = false;
        bool is_static = false;
    };

    struct class_model : class_or_interface_model
    {
        class_model() = delete;

        class_model(std::string_view const& name, 
                size_t decl_line, 
                std::string_view const& assembly_name, 
                std::shared_ptr<namespace_body_model> const& containing_ns_body, 
                class_modifier const& sem, 
                std::string_view const& base_id) :
            class_or_interface_model{ name, decl_line, assembly_name, containing_ns_body },
            m_modifier{ sem },
            m_class_base_ref{ base_id }
        { }
        
        class_model(std::string_view const& name, 
                size_t decl_line, 
                std::string_view const& assembly_name, 
                std::shared_ptr<namespace_body_model> const& containing_ns_body, 
                class_modifier const& sem) :
            class_or_interface_model{ name, decl_line, assembly_name, containing_ns_body },
            m_modifier{ sem },
            m_class_base_ref{ std::nullopt }
        { }

        auto const& get_class_base_ref() const noexcept
        {
            return m_class_base_ref;
        }

        auto const& get_modifier() const noexcept
        {
            return m_modifier;
        }

        bool const& has_synthesized_static_interface(size_t version = 0)
        {
            return m_synthesized_static_interface.size() > version;
        }

        bool const& has_synthesized_factory_interface(size_t version = 0)
        {
            return m_synthesized_factory_interface.size() > version;
        }

        bool const& has_synthesized_instance_interface(size_t version = 0)
        {
            return m_synthesized_instance_interface.size() > version;
        }

        auto const& get_synthesized_static_interface(size_t version = 0) const noexcept
        {
            return m_synthesized_static_interface[version];
        }

        auto const& get_synthesized_factory_interface(size_t version = 0) const noexcept
        {
            return m_synthesized_factory_interface[version];
        }

        auto const& get_synthesized_instance_interface(size_t version = 0) const noexcept
        {
            return m_synthesized_instance_interface[version];
        }

        void add_class_base_ref(std::string_view const& class_base_ref)
        {
            m_class_base_ref = type_ref{ class_base_ref };
        }

        void add_static_interface_ref(std::shared_ptr<interface_model> const& static_interface_ref)
        {
            m_synthesized_static_interface.emplace_back(static_interface_ref);
        }

        void add_factory_interface_ref(std::shared_ptr<interface_model> const& static_interface_ref)
        {
            m_synthesized_factory_interface.emplace_back(static_interface_ref);
        }

        void add_instance_interface_ref(std::shared_ptr<interface_model> const& static_interface_ref)
        {
            m_synthesized_instance_interface.emplace_back(static_interface_ref);
        }

        // TODO: Composition and inheritance is coming in a later update
        //void add_protected_interface_ref(std::shared_ptr<interface_model> const& protected_interface_ref);

        //void add_overrides_interface_ref(std::shared_ptr<interface_model> const& static_interface_ref);

        void validate(xlang_error_manager & error_manager);

        void resolve(symbol_table & symbols, xlang_error_manager & error_manager);

        bool has_circular_inheritance(xlang_error_manager & error_manager);

    private:
        std::optional<type_ref> m_class_base_ref;
        class_modifier m_modifier;
        // TODO: Add type parameters (generic types)

        std::vector<std::shared_ptr<interface_model>> m_synthesized_static_interface;
        std::vector<std::shared_ptr<interface_model>> m_synthesized_factory_interface;
        std::vector<std::shared_ptr<interface_model>> m_synthesized_instance_interface;

        // TODO: Composition and inheritance is coming in a later update
        // std::vector<std::shared_ptr<interface_model>> m_overrides_interfaces;
        // std::vector<std::shared_ptr<interface_model>> m_protected_interfaces;

        std::set<std::shared_ptr<class_or_interface_model>> get_all_bases();

        bool has_circular_inheritance(std::set<std::string> & symbol_set, xlang_error_manager & error_manager);
    };
}