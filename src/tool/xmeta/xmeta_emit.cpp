#include "xmeta_emit.h"
#include <winrt/base.h>
#include <locale>
#include <codecvt>
#include <string>
#include <comutil.h>
#include <iostream>
#include <codecvt>
#include <locale>

using namespace winrt;
using namespace xlang::meta::reader;
using namespace xlang::meta::writer;
using namespace xlang::xmeta;

// TODO: TypeRefEmitter
// method impl
// class/interface impl

namespace
{
    std::wstring s2ws(const std::string& as)
    {
        return std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t>{}.from_bytes(as);
    }

    // Useful helpers deal with meta::reader types, so use them primarily and translate to/from cor tokens for IMetaDataEmit and friends.
    mdModule to_token(Module module)
    {
        return module.index() + 1 | mdtModule;
    }

    mdTypeRef to_token(TypeRef type_ref)
    {
        return type_ref.index() + 1 | mdtTypeRef;
    }

    mdAssembly to_token(Assembly assembly)
    {
        return assembly.index() + 1 | mdtAssembly;
    }

    mdAssemblyRef to_token(AssemblyRef assembly_ref)
    {
        return assembly_ref.index() + 1 | mdtAssemblyRef;
    }

    Module to_Module(mdModule token)
    {
        XLANG_ASSERT(TypeFromToken(token) == mdtModule);
        return { nullptr, token - 1 };
    }

    TypeRef to_TypeRef(mdTypeRef token)
    {
        XLANG_ASSERT(TypeFromToken(token) == mdtTypeRef);
        return { nullptr, token - 1 };
    }

    TypeDef to_TypeDef(mdTypeDef token)
    {
        XLANG_ASSERT(TypeFromToken(token) == mdtTypeDef);
        return { nullptr, token - 1 };
    }

    Assembly to_Assembly(mdAssembly token)
    {
        XLANG_ASSERT(TypeFromToken(token) == mdtAssembly);
        return { nullptr, token - 1 };
    }

    AssemblyRef to_AssemblyRef(mdAssemblyRef token)
    {
        XLANG_ASSERT(TypeFromToken(token) == mdtAssemblyRef);
        return { nullptr, token - 1 };
    }

    ElementType to_ElementType(enum_semantics arg)
    {
        switch (arg)
        {
        case enum_semantics::Int8:
            return ElementType::I1;
        case enum_semantics::UInt8:
            return ElementType::U1;
        case enum_semantics::Int16:
            return ElementType::I2;
        case enum_semantics::UInt16:
            return ElementType::U2;
        case enum_semantics::Int32:
            return ElementType::I4;
        case enum_semantics::UInt32:
            return ElementType::U4;
        case enum_semantics::Int64:
            return ElementType::I8;
        case enum_semantics::UInt64:
            return ElementType::U8;
        default:
            XLANG_ASSERT(false);
            return ElementType::Void;
        }
    }

    std::string remove_extension(const std::string& filename) {
        size_t lastdot = filename.find_last_of(".");
        if (lastdot == std::string::npos) return filename;
        return filename.substr(0, lastdot);
    }

}


namespace xlang::xmeta
{
    void xmeta_emit::initialize() 
    {
        // Getting the meta data dispenser
        check_hresult(CoInitialize(NULL));
    
        // Windows only for now
        check_hresult(CoCreateInstance(CLSID_CorMetaDataDispenser,
            nullptr,
            CLSCTX_INPROC,
            IID_IMetaDataDispenser,
            m_metadata_dispenser.put_void()));

        {
            variant_t version{ L"WindowsRuntime 1.4" };
            check_hresult(m_metadata_dispenser->SetOption(MetaDataRuntimeVersion, &version.GetVARIANT()));
        }


        check_hresult(m_metadata_dispenser->DefineScope(
                CLSID_CorMetaDataRuntime,
                0,
                IID_IMetaDataAssemblyEmit,
                reinterpret_cast<IUnknown**>(m_metadata_assembly_emitter.put_void())));


        check_hresult(m_metadata_assembly_emitter->QueryInterface(IID_IMetaDataEmit2, m_metadata_emitter.put_void()));
        check_hresult(m_metadata_emitter->QueryInterface(IID_IMetaDataImport, m_metadata_import.put_void()));
       // Defining the mscorlib assemblyref
        define_assembly();
        define_common_reference_assembly();

        mdModule token_current_module;
        check_hresult(m_metadata_import->GetModuleFromScope(&token_current_module));
        m_module = to_Module(token_current_module);
    }

    void xmeta_emit::uninitialize()
    {
        CoUninitialize();
    }

    // This doesn't really output in a PE format
    void xmeta_emit::save_to_file() const
    {
        m_metadata_emitter->Save((s2ws(remove_extension(m_assembly_name)) + L".xmeta").c_str(), 0);
    }

	std::vector<uint8_t> xmeta_emit::save_to_memory() const
    {
        DWORD save_size;
        m_metadata_emitter->GetSaveSize(CorSaveSize::cssAccurate, &save_size);
		std::vector<uint8_t> metadata;
        metadata.resize(save_size);
        check_hresult(m_metadata_emitter->SaveToMemory(metadata.data(), save_size));
		return metadata;
    }

    void xmeta_emit::define_assembly()
    {
        constexpr DWORD sha1_hash_algo{ 0x8004 };
        check_hresult(m_metadata_assembly_emitter->DefineAssembly(
            nullptr,
            0,
            sha1_hash_algo,
            s2ws(m_assembly_name).c_str(),
            &(s_genericMetadata),
            afContentType_WindowsRuntime,
            &token_assembly));
        check_hresult(m_metadata_emitter->SetModuleProps(s2ws(m_assembly_name).c_str()));
    }

    void xmeta_emit::define_common_reference_assembly()
    {
        // Hard-coded the mscorlib strong name.
        constexpr BYTE s_mscorlibStrongNameBlob[] = { 0xb7, 0x7a, 0x5c, 0x56, 0x19, 0x34, 0xe0, 0x89 };
        check_hresult(m_metadata_assembly_emitter->DefineAssemblyRef(
            s_mscorlibStrongNameBlob,
            sizeof(s_mscorlibStrongNameBlob),
            L"mscorlib",
            &s_genericMetadata,
            nullptr,
            0,
            afContentType_Default,
            &token_mscorlib));

        check_hresult(m_metadata_emitter->DefineTypeRefByName(token_mscorlib, L"System.Enum", &token_enum));
        check_hresult(m_metadata_emitter->DefineTypeRefByName(token_mscorlib, L"System.ValueType", &token_value_type));
        check_hresult(m_metadata_emitter->DefineTypeRefByName(token_mscorlib, L"System.MulticastDelegate", &token_delegate));
    }

    mdTypeDef xmeta_emit::define_type_def(std::string const& name, DWORD const& type_flag, mdToken token_extend, mdToken token_implements[])
    {
        auto wname = s2ws(name);
        mdTypeDef token_typedef;
        check_hresult(m_metadata_emitter->DefineTypeDef(
            wname.c_str(),
            type_flag,
            token_extend, 
            token_implements,
            &token_typedef));

        mdTypeRef token_typeref;

        check_hresult(m_metadata_emitter->DefineTypeRefByName(to_token(m_module), wname.c_str(), &token_typeref));

        auto result = type_references.insert(std::make_pair(name, to_TypeRef(token_typeref)));
        if (!result.second)
        {
            throw_invalid("Encountered duplicate TypeRef: " + name);
        }

        return token_typedef;
    }


    void xmeta_emit::listen_namespace_model(std::shared_ptr<namespace_model> const& model) {};
    
    void xmeta_emit::listen_class_model(std::shared_ptr<class_model> const& model) 
    {
        //mdTypeDef token_class_type_def = mdTokenNil;
        //mdTypeRef token_local_type_ref = mdTokenNil;

        //std::string class_name = model->get_id();
        //DWORD type_flag = runtimeclass_type_flag;
        //// MIDL3 will disable certain flags depending on these conditions
        ////if (pRuntimeClass->IsComposable())
        ////{
        ////    // Turn off the sealed flag
        ////    type_flag = dwTypeFlag & (~tdSealed);
        ////}

        //////  Empty runtime classes should appear with the abstract flag.
        ////if (pRuntimeClass->IsEmpty())
        ////{
        ////    type_flag = dwTypeFlag | tdAbstract;
        ////}

        //mdTypeDef implements[] = { mdTokenNil };

        //auto class_type_def = define_type_def(
        //    class_name,
        //    type_flag,
        //    mdTypeRefNil, // Extends (Going to be null until we find out if it is base class)
        //    implements // Implements (Going to be null until we find out if it is base class));
        //);

        //// TODO: Class base and interface implements

        //for (auto const& val : model->get_methods())
        //{
        //    define_method(val, token_class_type_def);
        //}
        //for (auto const& val : model->get_properties())
        //{
        //    define_property(val, token_class_type_def);
        //}
        //for (auto const& val : model->get_events())
        //{
        //    define_event(val, token_class_type_def);
        //}
    }

    void xmeta_emit::define_method(std::shared_ptr<method_model> const& model, mdTypeDef const& token_def) 
    {
        //std::wstring method_name = s2ws(model->get_id());

        //DWORD method_flag;
        //mdMethodDef token_method_def;
        ////m_metadata_emitter->DefineMethod(
        ////    token_def,
        ////    method_name.c_str(),
        ////    ,
        ////    ,
        ////    miRuntime,
        ////    &token_method_def);

        ///* Define return value */
        //mdParamDef token_return; // To be used for attributes later
        //define_return(model->get_return_type(), token_method_def, &token_return);

        ///* Define formal parameters */
        //int index = 1;
        //for (auto const& val : model->get_formal_parameters())
        //{
        //    define_parameters(val, token_method_def, index);
        //    index++;
        //}
    }

    void xmeta_emit::define_property(std::shared_ptr<property_model> const& model, mdTypeDef const& token_def) 
    {
        //std::wstring property_name = s2ws(model->get_id());
        //
        //mdMethodDef token_get_method = mdTokenNil;
        //mdMethodDef token_set_method = mdTokenNil;
        //DWORD property_flag = 0;
        //DWORD c_plus_type_flag;
        //std::shared_ptr<method_model> get_method_model = model->get_get_method();
        //if (get_method_model != nullptr) //TODO: This case is not suppose to happen
        //{
        //    std::wstring get_method_name = s2ws(get_method_model->get_id());
        //    PCCOR_SIGNATURE pv_sig_blob = NULL;
        //    ULONG cb_sig_blob = 0;
        //    DWORD impl_flag = 0;
        //    check_hresult(m_metadata_emitter->DefineMethod(
        //        token_def,
        //        get_method_name.c_str(),
        //        property_flag,
        //        pv_sig_blob,
        //        cb_sig_blob,
        //        miRuntime,
        //        impl_flag,
        //        &token_get_method));
        //}

        //std::shared_ptr<method_model> set_method_model = model->get_set_method();
        //if (set_method_model != nullptr)
        //{
        //    std::wstring set_method_name = s2ws(set_method_model->get_id());
        //    PCCOR_SIGNATURE pv_sig_blob = NULL;
        //    ULONG cb_sig_blob = 0;
        //    DWORD impl_flag = 0;
        //    check_hresult(m_metadata_emitter->DefineMethod(
        //        token_def,
        //        set_method_name.c_str(),
        //        property_flag,
        //        pv_sig_blob,
        //        cb_sig_blob,
        //        miRuntime,
        //        impl_flag,
        //        &token_set_method));
        //}

        //mdProperty token_property;
        ////m_metadata_emitter->DefineProperty(
        ////    token_class,
        ////    property_name.c_str(),
        ////    property_flag,
        ////    , // pvSig
        ////    , // cbsig
        ////    , // c_plus_type_flag
        ////    nullptr,
        ////    0,
        ////    token_set_method,
        ////    token_get_method,
        ////    mdTokenNil,
        ////    &token_property);
    }

    void xmeta_emit::define_event(std::shared_ptr<event_model> const& model, mdTypeDef const& token_def)
    {
        //std::wstring event_name = s2ws(model->get_id());

        //mdMethodDef token_add_method = mdTokenNil;
        //mdMethodDef token_remove_method = mdTokenNil;
        //DWORD event_flag = 0;
        //DWORD c_plus_type_flag;
        //std::shared_ptr<method_model> add_method_model = model->get_add_method();
        //if (add_method_model != nullptr) 
        //{
        //    std::wstring get_method_name = s2ws(add_method_model->get_id());
        //    PCCOR_SIGNATURE pv_sig_blob = NULL;
        //    ULONG cb_sig_blob = 0;
        //    DWORD impl_flag = 0;
        //    check_hresult(m_metadata_emitter->DefineMethod(
        //        token_def,
        //        get_method_name.c_str(),
        //        event_flag,
        //        pv_sig_blob,
        //        cb_sig_blob,
        //        miRuntime,
        //        impl_flag,
        //        &token_add_method));

        //}
        //std::shared_ptr<method_model> remove_method_model = model->get_remove_method();
        //if (remove_method_model != nullptr)
        //{
        //    std::wstring set_method_name = s2ws(remove_method_model->get_id());
        //    PCCOR_SIGNATURE pv_sig_blob = NULL;
        //    ULONG cb_sig_blob = 0;
        //    DWORD impl_flag = 0;
        //    check_hresult(m_metadata_emitter->DefineMethod(
        //        token_def,
        //        set_method_name.c_str(),
        //        event_flag,
        //        pv_sig_blob,
        //        cb_sig_blob,
        //        miRuntime,
        //        impl_flag,
        //        &token_remove_method));
        //}

        //mdProperty token_event;
        ////m_metadata_emitter->DefineEvent(
        ////    token_class_def,
        ////    event_name.c_str(),
        ////    property_flag,
        ////    , // pvSig
        ////    , // cbsig
        ////    , // c_plus_type_flag
        ////    nullptr,
        ////    0,
        ////    token_add_method,
        ////    token_remove_method,
        ////    mdTokenNil,
        ////    &token_event);
    }
    
    void xmeta_emit::listen_struct_model(std::shared_ptr<struct_model> const& model) 
    {
        //mdTypeDef implements[] = { mdTokenNil };
        //auto struct_type_def = define_type_def(model->get_id(), struct_type_flag, token_value_type, implements);

        //for (std::pair<type_ref, std::string> const& field : model->get_fields())
        //{
        //    mdFieldDef token_field;
        //    //m_metadata_emitter->DefineField(
        //    //    token_struct_type_def, 
        //    //    s2ws(field.second).c_str(), 
        //    //    fdPublic, 
        //    //    ,
        //    //    ,
        //    //    ELEMENT_TYPE_END,
        //    //    nullptr,
        //    //    0,
        //    //    &token_field);

        //}
    }
    
    void xmeta_emit::listen_interface_model(std::shared_ptr<interface_model> const& model) 
    {
        //DWORD type_flag = interface_type_flag;
        ////if (pInterface->HasExclusiveToAttribute())
        ////{
        ////    // Mark this type as NotPublic.
        ////    type_flag &= ~tdVisibilityMask;
        ////    type_flag |= tdNotPublic;
        ////}

        //mdTypeDef implements[] = { mdTokenNil };
        //auto token_interface_type_def = define_type_def(
        //    model->get_id(),
        //    type_flag,
        //    mdTypeRefNil, // Extends (Going to be null until we find out if it is base class)
        //    implements // Implements (Going to be null until we find out if it is base class)
        //);

        //for (auto const& val : model->get_methods())
        //{
        //    define_method(val, token_interface_type_def);
        //}
        //for (auto const& val : model->get_properties())
        //{
        //    define_property(val, token_interface_type_def);
        //}
        //for (auto const& val : model->get_events())
        //{
        //    define_event(val, token_interface_type_def);
        //}
    }

    void xmeta_emit::listen_enum_model(std::shared_ptr<enum_model> const& model) 
    {
        auto const& type_name = model->get_id();
        static constexpr DWORD enum_type_flag = tdPublic | tdSealed | tdClass | tdAutoLayout | tdWindowsRuntime;
        mdTypeDef implements[] = { mdTokenNil };
        auto token_enum_type_def = define_type_def(type_name, enum_type_flag, token_enum, implements);
        
        static constexpr DWORD enum_value_flag = fdRTSpecialName | fdSpecialName | fdPrivate;
        ElementType const underlying_type = to_ElementType(model->get_type());
        signature_blob value_signature;
        value_signature.add_signature(FieldSig{ TypeSig{underlying_type} });
        mdFieldDef field_token;
        check_hresult(m_metadata_emitter->DefineField(token_enum_type_def,
            L"value__",
            enum_value_flag,
            value_signature.data(),
            value_signature.size(),
            ELEMENT_TYPE_END,
            nullptr,
            0,
            &field_token));

        auto iter = type_references.find(type_name);
        if (iter == type_references.end())
        {
            throw_invalid("Failed to find TypeRef for: " + type_name);
        }
        TypeRef const& enum_type_ref = iter->second;

        static constexpr DWORD enumerator_flag = fdHasDefault | fdLiteral | fdStatic | fdPublic;
        signature_blob enumerator_signature;
        enumerator_signature.add_signature(FieldSig{ TypeSig{enum_type_ref.coded_index<TypeDefOrRef>()} });

        for (enum_member const& enum_member : model->get_members())
        {
            call(enum_member.get_resolved_value(), [&](auto const& val)
                {
                    using val_type = std::decay_t<decltype(val)>;
                    static_assert(std::is_integral_v<val_type>);
                    auto const& name = s2ws(enum_member.get_id());
                    check_hresult(m_metadata_emitter->DefineField(token_enum_type_def,
                        name.c_str(),
                        enumerator_flag,
                        enumerator_signature.data(),
                        enumerator_signature.size(),
                        static_cast<DWORD>(underlying_type),
                        &val,
                        static_cast<ULONG>(sizeof(val_type)),
                        &field_token));
                });
        }
    }
    
    void xmeta_emit::listen_delegate_model(delegate_model const& model) 
    {
        //mdTypeDef implements[] = { mdTokenNil };
        //auto token_delegate_type_def = define_type_def(model.get_id(), delegate_type_flag, token_delegate, implements);

        ///* Define return value */
        //mdParamDef token_return; // To be used for attributes later
        //define_return(model.get_return_type(), token_delegate_type_def, &token_return);

        ///* Define formal parameters */
        //int index = 1;
        //for (auto const& val : model.get_formal_parameters())
        //{
        //    define_parameters(val, token_delegate_type_def, index);
        //    index++;
        //}
    }

    void xmeta_emit::define_return(std::optional<type_ref> const& retun_type, mdTypeDef const& type_def, mdParamDef *token_return)
    {
        /* Define return value */
        std::string return_name = type_semantics_to_string(retun_type->get_semantic());
        check_hresult(m_metadata_emitter->DefineParam(
            type_def,
            0,  // Index Zero represents the return value.
            s2ws(return_name).c_str(),
            0,  // return tyes have no flag set
            (DWORD)-1,  // Ignore dwCPlusTypeFlag
            nullptr,    // No constant value
            0,
            token_return));
    }

    void xmeta_emit::define_parameters(formal_parameter_model const& model, mdMethodDef const& token_method_def, int parameter_index)
    {
        std::wstring param_name = s2ws(model.get_id());

        DWORD param_flags = 0;
        if (parameter_index != 0)
        {
            if (model.get_semantic() == parameter_semantics::in)
            {
                param_flags |= pdIn;
            }
            if (model.get_semantic() == parameter_semantics::out)
            {
                param_flags |= pdOut;
            }
        }
        throw EXCEPTION_ACCESS_VIOLATION;
        mdParamDef token_param_def; //To be used for attributes later
        check_hresult(m_metadata_emitter->DefineParam(
            token_method_def,
            0, 
            param_name.c_str(),
            param_flags,
            (DWORD) - 1,
            nullptr,
            0,
            &token_param_def));
    }
    

    inline std::string type_semantics_to_string(model_ref<type_semantics> const& semantic_type)
    {
        type_semantics const& ts = semantic_type.get_resolved_target();
        if (std::holds_alternative<std::shared_ptr<class_model>>(ts))
        {
            return std::get<std::shared_ptr<class_model>>(ts)->get_id();
        }
        if (std::holds_alternative<std::shared_ptr<enum_model>>(ts))
        {
            return std::get<std::shared_ptr<enum_model>>(ts)->get_id();
        }
        if (std::holds_alternative<std::shared_ptr<interface_model>>(ts))
        {
            return std::get<std::shared_ptr<interface_model>>(ts)->get_id();
        }
        if (std::holds_alternative<std::shared_ptr<struct_model>>(ts))
        {
            return std::get<std::shared_ptr<struct_model>>(ts)->get_id();
        }
        if (std::holds_alternative<simple_type>(ts))
        {
            switch (std::get<simple_type>(ts))
            {
            case simple_type::Boolean:
                return std::string("Boolean");
            case simple_type::String:
                return std::string("String");
            case simple_type::Int8:
                return std::string("Int8");
            case simple_type::Int16:
                return std::string("Int16");
            case simple_type::Int32:
                return std::string("Int32");
            case simple_type::Int64:
                return std::string("Int64");
            case simple_type::UInt8:
                return std::string("Uint8");
            case simple_type::UInt16:
                return std::string("Uint16");
			case simple_type::UInt32:
				return std::string("Uint32");
			case simple_type::UInt64:
				return std::string("Uint64");
			case simple_type::Char16:
                return std::string("Char16");
            case simple_type::Guid:
                return std::string("Guid");
            case simple_type::Single:
                return std::string("Single");
            case simple_type::Double:
                return std::string("Double");
            }
        }
        if (std::holds_alternative<object_type>(ts))
        {
            return std::string("Object");
        }
        assert(false);
    }

}


