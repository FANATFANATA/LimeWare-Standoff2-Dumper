//
// Created by alexandr on 27.04.2026.
//

#ifndef LIMEWARE_STANDOFF2_DUMPER_MEMORY_H
#define LIMEWARE_STANDOFF2_DUMPER_MEMORY_H


#include <string>
#include <functional>

class Memory {
public:
    Memory() = default;
    ~Memory() = default;

    struct ModuleInfo {
        ModuleInfo() = default;
        ~ModuleInfo() = default;

        std::string path;
        uintptr_t base = 0;
        uintptr_t end = 0;
        uintptr_t size = 0;

        [[nodiscard]] bool isValid() const { return base && end && end - base > 0 && !path.empty(); }
        [[nodiscard]] std::string getPath() const { return path; }
        [[nodiscard]] uintptr_t getBase() const { return base; }
        [[nodiscard]] uintptr_t getEnd() const { return end; }
        [[nodiscard]] uintptr_t getSize() const { return size; }

        [[nodiscard]] std::vector<uintptr_t> findPattern(const std::string &hex, bool need_first_only = true) const;
    };

    virtual bool initialize();

private:
    ModuleInfo m_libunity = {};
    ModuleInfo getModuleInfo(const char *name, const std::function<void(ModuleInfo)> &callback = {});

    uintptr_t findAssembliesStart(ModuleInfo module);
    uintptr_t findAssembliesEnd(ModuleInfo module);

    uintptr_t findAssemblyOpen(ModuleInfo module);
    uintptr_t findAssemblyName(ModuleInfo module);
    uintptr_t findAssemblyGetImage(ModuleInfo module);

    uintptr_t findImageTypeCount(ModuleInfo module);
    uintptr_t findGetAssemblyTypeHandle(ModuleInfo module);
    uintptr_t findGetTypeFromHandle(ModuleInfo module);
    uintptr_t findImageGetName(ModuleInfo module);

    uintptr_t findClassGetName(ModuleInfo module);
    uintptr_t findClassNamespace(ModuleInfo module);
    uintptr_t findClassGetParent(ModuleInfo module);
    uintptr_t findClassGetFields(ModuleInfo module);
    uintptr_t findClassGetProperties(ModuleInfo module);
    uintptr_t findClassGetMethods(ModuleInfo module);
    uintptr_t findClassGetType(ModuleInfo module);
    uintptr_t findClassGetEnumBasetype(ModuleInfo module);
    uintptr_t findClassGetFlags(ModuleInfo module);
    uintptr_t findClassGetInterfaces(ModuleInfo module);
    uintptr_t findClassGetDeclaringType(ModuleInfo module);
    uintptr_t findClassGetIsGeneric(ModuleInfo module);

    uintptr_t findFieldGetName(ModuleInfo module);
    uintptr_t findFieldGetOffset(ModuleInfo module);
    uintptr_t findFieldGetType(ModuleInfo module);
    uintptr_t findFieldGetDefaultValue(ModuleInfo module);

    uintptr_t findTypeGetType(ModuleInfo module);
    uintptr_t findTypeAttrs(ModuleInfo module);
    uintptr_t findTypeAttrsShift(ModuleInfo module);
    uintptr_t findTypeGetClassFromType(ModuleInfo module);
    uintptr_t findTypeGetData(ModuleInfo module);
    uintptr_t findTypeIsByref(ModuleInfo module);
    uintptr_t findTypeGetName(ModuleInfo module);

    uintptr_t findPropertyName(ModuleInfo module);
    uintptr_t findPropertyGetMethod(ModuleInfo module);
    uintptr_t findPropertySetMethod(ModuleInfo module);

    uintptr_t findMethodName(ModuleInfo module);
    uintptr_t findMethodReturnType(ModuleInfo module);
    uintptr_t findMethodGetFlags(ModuleInfo module);
    uintptr_t findMethodParametersCount(ModuleInfo module);
    uintptr_t findMethodGetParameter(ModuleInfo module);
    uintptr_t findMethodPointer(ModuleInfo module);
    uintptr_t findMethodVirtualPointer(ModuleInfo module);
    uintptr_t findMethodSlot(ModuleInfo module);
    uintptr_t findMethodGetIsGeneric(ModuleInfo module);
    uintptr_t findMethodGetParameterName(ModuleInfo module);

public:

    struct {
        uintptr_t assemblies_start = 0x987C038;
        uintptr_t assemblies_end = 0x987C040;

        //assembly
        uintptr_t assembly_open = 0x4FF4AA8;
        uintptr_t assembly_name = 0x30;
        uintptr_t assembly_get_image = 0x4FF4AA0;

        // image
        uintptr_t image_type_count = 0x4;
        uintptr_t image_get_assembly_type_handle = 0x87ACE50;
        uintptr_t handle_get_type_from_handle = 0x87AD300;
        uintptr_t image_name = 0x30;

        // class
        uintptr_t class_get_name = 0x877D674;
        uintptr_t class_namespace = 0x88;
        uintptr_t class_get_parent = 0x877D768;
        uintptr_t class_get_fields = 0x877D240;
        uintptr_t class_get_properties = 0x877D770;
        uintptr_t class_get_methods = 0x877D480;
        uintptr_t class_type = 0x10;
        uintptr_t class_get_enum_basetype = 0x877D15C;
        uintptr_t class_get_flags = 0x4FF4124;
        uintptr_t class_get_interfaces = 0x877D3B8;
        uintptr_t class_get_declaring_type = 0x877FE2C;
        uintptr_t class_get_is_generic = 0x877DCF0;

        // field
        uintptr_t field_get_name = 0x500EB20;
        uintptr_t field_get_offset = 0x500EB40;
        uintptr_t field_get_type = 0x500F29C;
        uintptr_t field_get_default_value = 0x87AC80C;

        // type
        uintptr_t type_get_type = 0x87BCB34;
        uintptr_t type_attrs = 0x10;
        uintptr_t type_attrs_shift = 0x2;
        uintptr_t type_get_class_from_type = 0x877CBCC;
        uintptr_t type_get_data = 0x87BD280;
        uintptr_t type_is_byref = 0x10;
        uintptr_t type_get_name = 0x87BD284;

        // property
        uintptr_t property_name = 0x10;
        uintptr_t property_get_method = 0x18;
        uintptr_t property_set_method = 0x28;

        // method
        uintptr_t method_name = 0x30;
        uintptr_t method_return_type = 0x40;
        uintptr_t method_get_flags = 0x5D9429C;
        uintptr_t method_parameters_count = 0x14;
        uintptr_t method_get_parameter = 0x87AE794;
        uintptr_t method_pointer = 0x18;
        uintptr_t method_virtual_pointer = 0x0;
        uintptr_t method_slot = 0x5A;
        uintptr_t method_get_is_generic = 0x4FF432C;
        uintptr_t method_get_parameter_name = 0x87AE7B4;
    } offsets;

    [[nodiscard]] uintptr_t getUnityBase() const { return m_libunity.getBase(); }
    [[nodiscard]] uintptr_t getUnityEnd() const { return m_libunity.getEnd(); }
    [[nodiscard]] uintptr_t getUnitySize() const { return m_libunity.getSize(); }

    [[nodiscard]] std::string dumpHex(uintptr_t address, size_t size);
    void dumpHexToFile(uintptr_t address, size_t size, const std::string& filePath);
    [[nodiscard]] bool isPtrValid(uintptr_t address) const;
};



#endif //LIMEWARE_STANDOFF2_DUMPER_MEMORY_H