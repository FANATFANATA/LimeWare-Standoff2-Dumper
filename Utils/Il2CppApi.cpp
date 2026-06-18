//
// Created by alexandr on 27.04.2026.
//

#include "Il2CppApi.h"

#include "LimeWare.h"

bool Il2CppApi::initialize() {
    return true;
}

Il2Cpp::Assembly* Il2CppApi::openAssembly(const char* name) {
    static Il2Cpp::Assembly *(*il2cpp_domain_assembly_open)(const char*) = reinterpret_cast<decltype(il2cpp_domain_assembly_open)>(
        g.memory->getUnityBase() + g.memory->offsets.assembly_open);
    return il2cpp_domain_assembly_open(name);
}

std::vector<Il2Cpp::Assembly*> Il2CppApi::getAssemblies() {
    std::vector<Il2Cpp::Assembly*> result;
    const uintptr_t base = g.memory->getUnityBase();
    const uintptr_t addr_start = base + g.memory->offsets.assemblies_start;
    const uintptr_t addr_end = base + g.memory->offsets.assemblies_end;

    const uintptr_t vector_start = *reinterpret_cast<uintptr_t*>(addr_start);
    const uintptr_t vector_end = *reinterpret_cast<uintptr_t *>(addr_end);

    if (!vector_start || !vector_end) {
        g.log->error("Could not read assembly boundaries.");
        return result;
    }

    for (uintptr_t current = vector_start; current < vector_end; current += sizeof(uintptr_t)) {
        const uintptr_t assembly = *reinterpret_cast<uintptr_t *>(current);
        if (assembly) {
            result.push_back(reinterpret_cast<Il2Cpp::Assembly*>(assembly));
        }
    }
    return result;
}

const char *Il2CppApi::getAssemblyName(Il2Cpp::Assembly *assembly) {
        return *reinterpret_cast<const char **>(reinterpret_cast<uintptr_t>(assembly) + g.memory->offsets.assembly_name);
}

Il2Cpp::Image *Il2CppApi::getImage(Il2Cpp::Assembly *assembly) {
        static Il2Cpp::Image *(*il2cpp_assembly_get_image)(Il2Cpp::Assembly *) = reinterpret_cast<decltype(il2cpp_assembly_get_image)>(
        g.memory->getUnityBase() + g.memory->offsets.assembly_get_image);
    return il2cpp_assembly_get_image(assembly);
}

int Il2CppApi::getTypeCount(Il2Cpp::Image *image) {
	return *reinterpret_cast<int*>(reinterpret_cast<uintptr_t>(image) + g.memory->offsets.image_type_count);
}

std::vector<Il2Cpp::Class*> Il2CppApi::getClasses(Il2Cpp::Image *image) {
	std::vector<Il2Cpp::Class*> result;
    static void *(*get_assembly_type_handle)(Il2Cpp::Image*, int index) = reinterpret_cast<decltype(get_assembly_type_handle)>(
        g.memory->getUnityBase() + g.memory->offsets.image_get_assembly_type_handle);

    static Il2Cpp::Class *(*get_type_from_handle)(void*) = reinterpret_cast<decltype(get_type_from_handle)>(
        g.memory->getUnityBase() + g.memory->offsets.handle_get_type_from_handle);

    int typeCount = getTypeCount(image);
    for (int i = 0; i < typeCount; i++) {
        void *type_handle = get_assembly_type_handle(image, i);
        if (type_handle) {
            Il2Cpp::Class *type = get_type_from_handle(type_handle);
            if (type) {
                result.push_back(type);
            }
        }
    }
    return result;
}

const char *Il2CppApi::getImageName(Il2Cpp::Image *image) {
	return *reinterpret_cast<const char**>(reinterpret_cast<uintptr_t>(image) + g.memory->offsets.image_name);
}

Il2Cpp::Class *Il2CppApi::getClassFromName(const char *assembly, const char *name) {
    Il2Cpp::Assembly *a = openAssembly(assembly);
    if (g.memory->isPtrValid(reinterpret_cast<uintptr_t>(a))) {
        Il2Cpp::Image *i = getImage(a);
        if (g.memory->isPtrValid(reinterpret_cast<uintptr_t>(i))) {
            for (Il2Cpp::Class *klass : getClasses(i)) {
                if (!strcmp(g.il2cpp->getClassName(klass), name)) {
                    return klass;
                }
            }
        }
    }
    return nullptr;
}

const char *Il2CppApi::getClassName(Il2Cpp::Class *klass) {
	static const char *(*il2cpp_class_get_name)(Il2Cpp::Class *) = reinterpret_cast<decltype(il2cpp_class_get_name)>(
        g.memory->getUnityBase() + g.memory->offsets.class_get_name);
    return il2cpp_class_get_name(klass);
}

const char *Il2CppApi::getClassNamespace(Il2Cpp::Class *klass) {
	return *reinterpret_cast<const char**>(reinterpret_cast<uintptr_t>(klass) + g.memory->offsets.class_namespace);
}

Il2Cpp::Class *Il2CppApi::getParent(Il2Cpp::Class *klass) {
	static Il2Cpp::Class *(*il2cpp_class_get_parent)(Il2Cpp::Class *) = reinterpret_cast<decltype(il2cpp_class_get_parent)>(
        g.memory->getUnityBase() + g.memory->offsets.class_get_parent);
    return il2cpp_class_get_parent(klass);
}

std::vector<Il2Cpp::Field*> Il2CppApi::getFields(Il2Cpp::Class *klass) {
	static Il2Cpp::Field *(*il2cpp_get_fields)(Il2Cpp::Class*, void*) = reinterpret_cast<decltype(il2cpp_get_fields)>(
        g.memory->getUnityBase() + g.memory->offsets.class_get_fields);

    void *iterator = nullptr;
    std::vector<Il2Cpp::Field*> fields;

    while (Il2Cpp::Field* field = il2cpp_get_fields(klass, &iterator)) {
        fields.push_back(field);
    }

    return fields;
}

std::vector<Il2Cpp::Property*> Il2CppApi::getProperties(Il2Cpp::Class *klass) {
	static Il2Cpp::Field *(*il2cpp_get_properties)(Il2Cpp::Class*, void*) = reinterpret_cast<decltype(il2cpp_get_properties)>(
        g.memory->getUnityBase() + g.memory->offsets.class_get_properties);

    void *iterator = nullptr;
    std::vector<Il2Cpp::Property*> properties;

    while (Il2Cpp::Property* property = il2cpp_get_properties(klass, &iterator)) {
        properties.push_back(property);
    }

    return properties;
}

std::vector<Il2Cpp::Method*> Il2CppApi::getMethods(Il2Cpp::Class *klass) {
	static Il2Cpp::Method *(*il2cpp_get_class_methods)(Il2Cpp::Class*, void*) = reinterpret_cast<decltype(il2cpp_get_class_methods)>(
        g.memory->getUnityBase() + g.memory->offsets.class_get_methods);

    void *iterator = nullptr;
    std::vector<Il2Cpp::Method*> methods;

    while (Il2Cpp::Method* method = il2cpp_get_class_methods(klass, &iterator)) {
        methods.push_back(method);
    }

    return methods;
}

Il2Cpp::Type *Il2CppApi::getThisType(Il2Cpp::Class *klass) {
	return reinterpret_cast<Il2Cpp::Type*>(reinterpret_cast<uintptr_t>(klass) + g.memory->offsets.class_type);
}

bool Il2CppApi::isClassEnum(Il2Cpp::Class *klass) {
	static Il2Cpp::Type *(*il2cpp_class_get_enum_basetype)(Il2Cpp::Class*) = reinterpret_cast<decltype(il2cpp_class_get_enum_basetype)>(
	    g.memory->getUnityBase() + g.memory->offsets.class_get_enum_basetype);
    return il2cpp_class_get_enum_basetype(klass) != nullptr;
}

uint64_t Il2CppApi::getClassFlags(Il2Cpp::Class *klass) {
	static int64_t (*il2cpp_class_get_flags)(Il2Cpp::Class*) = reinterpret_cast<decltype(il2cpp_class_get_flags)>(
        g.memory->getUnityBase() + g.memory->offsets.class_get_flags);
    return il2cpp_class_get_flags(klass);
}

std::vector<Il2Cpp::Class*> Il2CppApi::getInterfaces(Il2Cpp::Class *klass) {
	static Il2Cpp::Class *(*il2cpp_get_class_interfaces)(Il2Cpp::Class*, void*) = reinterpret_cast<decltype(il2cpp_get_class_interfaces)>(
        g.memory->getUnityBase() + g.memory->offsets.class_get_interfaces);

    void *iterator = nullptr;
    std::vector<Il2Cpp::Class*> interfaces;

    while (Il2Cpp::Class* interface = il2cpp_get_class_interfaces(klass, &iterator)) {
        interfaces.push_back(interface);
    }

    return interfaces;
}

Il2Cpp::Class *Il2CppApi::getDeclaringClass(Il2Cpp::Class *klass) {
	static Il2Cpp::Class *(*il2cpp_class_get_declaring_type)(Il2Cpp::Class*) = reinterpret_cast<decltype(il2cpp_class_get_declaring_type)>(
        g.memory->getUnityBase() + g.memory->offsets.class_get_declaring_type);
    return il2cpp_class_get_declaring_type(klass);
}

bool Il2CppApi::getIsGeneric(Il2Cpp::Class *klass) {
	static uint8_t (*il2cpp_class_is_generic)(Il2Cpp::Class*) = reinterpret_cast<decltype(il2cpp_class_is_generic)>(
        g.memory->getUnityBase() + g.memory->offsets.class_get_is_generic);
    return il2cpp_class_is_generic(klass) == 1;
}

const char *Il2CppApi::getFieldName(Il2Cpp::Field *field) {
	static const char *(*il2cpp_field_get_name)(Il2Cpp::Field*) = reinterpret_cast<decltype(il2cpp_field_get_name)>(
        g.memory->getUnityBase() + g.memory->offsets.field_get_name);
    return il2cpp_field_get_name(field);
}

int32_t Il2CppApi::getFieldOffset(Il2Cpp::Field *field) {
	static int32_t (*il2cpp_field_get_offset)(Il2Cpp::Field*) = reinterpret_cast<decltype(il2cpp_field_get_offset)>(
        g.memory->getUnityBase() + g.memory->offsets.field_get_offset);
    return il2cpp_field_get_offset(field);
}

Il2Cpp::Type *Il2CppApi::getFieldType(Il2Cpp::Field *field) {
    static Il2Cpp::Type *(*il2cpp_field_get_type)(Il2Cpp::Field*) = reinterpret_cast<decltype(il2cpp_field_get_type)>(
        g.memory->getUnityBase() + g.memory->offsets.field_get_type);
    return il2cpp_field_get_type(field);
}

void *Il2CppApi::getFieldDefaultValue(Il2Cpp::Field *field, Il2Cpp::Type **type) {
	static void *(*il2cpp_get_field_default_value)(Il2Cpp::Field*, Il2Cpp::Type**) = reinterpret_cast<decltype(il2cpp_get_field_default_value)>(
        g.memory->getUnityBase() + g.memory->offsets.field_get_default_value);

    return il2cpp_get_field_default_value(field, type);
}


int8_t Il2CppApi::getTypeIndex(Il2Cpp::Type *type) {
	static int8_t (*il2cpp_type_get_type)(Il2Cpp::Type*) = reinterpret_cast<decltype(il2cpp_type_get_type)>(
        g.memory->getUnityBase() + g.memory->offsets.type_get_type);
    return il2cpp_type_get_type(type);
}

int16_t Il2CppApi::getAttrs(Il2Cpp::Type *type) {
	return *reinterpret_cast<int16_t*>(reinterpret_cast<uintptr_t>(type) + g.memory->offsets.type_attrs) >> g.memory->offsets.type_attrs_shift;
}

Il2Cpp::Class *Il2CppApi::getClassFromType(Il2Cpp::Type *type) {
	static Il2Cpp::Class *(*il2cpp_class_from_il2cpp_type)(Il2Cpp::Type*, char) = reinterpret_cast<decltype(il2cpp_class_from_il2cpp_type)>(
        g.memory->getUnityBase() + g.memory->offsets.type_get_class_from_type);
    return il2cpp_class_from_il2cpp_type(type, 1);
}

Il2Cpp::ArrayType *Il2CppApi::getArrayType(Il2Cpp::Type *type) {
	static void *(*il2cpp_type_get_data)(Il2Cpp::Type*) = reinterpret_cast<decltype(il2cpp_type_get_data)>(
        g.memory->getUnityBase() + g.memory->offsets.type_get_data);

    return il2cpp_type_get_data(type);
}

Il2Cpp::Type *Il2CppApi::getRootType(Il2Cpp::Type *type) {
	static void *(*il2cpp_type_get_data)(Il2Cpp::Type*) = reinterpret_cast<decltype(il2cpp_type_get_data)>(
        g.memory->getUnityBase() + g.memory->offsets.type_get_data);
    return il2cpp_type_get_data(type);
}

bool Il2CppApi::getIsByref(Il2Cpp::Type *type) {
	return getTypeName(type).find("&") != std::string::npos;
}

std::string Il2CppApi::getTypeName(Il2Cpp::Type *type, bool simple) {
	static std::string (*il2cpp_type_get_name)(Il2Cpp::Type*, uint8_t) = reinterpret_cast<decltype(il2cpp_type_get_name)>(
        g.memory->getUnityBase() + g.memory->offsets.type_get_name);

    std::string fullName = il2cpp_type_get_name(type, 0);

    if (!simple) {
        return fullName;
    }

    std::string simplified;
    std::string current;

    for (size_t i = 0; i < fullName.length(); ++i) {
        char c = fullName[i];

        if (c == '<' || c == '>' || c == ',') {
            size_t lastDot = current.find_last_of('.');
            if (lastDot != std::string::npos) {
                simplified += current.substr(lastDot + 1);
            } else {
                simplified += current;
            }

            simplified += c;
            if (c == ',') simplified += ' ';
            current.clear();

            if (c == ',' && i + 1 < fullName.length() && fullName[i+1] == ' ') i++;
        } else {
            current += c;
        }
    }

    if (!current.empty()) {
        size_t lastDot = current.find_last_of('.');
        simplified += (lastDot != std::string::npos) ? current.substr(lastDot + 1) : current;
    }

    return simplified;
}

Il2Cpp::Type *Il2CppApi::getTypeFromArrayType(Il2Cpp::ArrayType *array_type) {
	return *reinterpret_cast<Il2Cpp::Type**>(reinterpret_cast<uintptr_t>(array_type) + 8);
}

const char *Il2CppApi::getPropertyName(Il2Cpp::Property *property) {
	return *reinterpret_cast<const char **>(reinterpret_cast<uintptr_t>(property) + g.memory->offsets.property_name);
}

Il2Cpp::Method *Il2CppApi::getPropertySetter(Il2Cpp::Property *property) {
	return *reinterpret_cast<Il2Cpp::Method**>(reinterpret_cast<uintptr_t>(property) + g.memory->offsets.property_set_method);
}

Il2Cpp::Method *Il2CppApi::getPropertyGetter(Il2Cpp::Property *property) {
	return *reinterpret_cast<Il2Cpp::Method**>(reinterpret_cast<uintptr_t>(property) + g.memory->offsets.property_get_method);
}

const char *Il2CppApi::getMethodName(Il2Cpp::Method *method) {
	return *reinterpret_cast<const char**>(reinterpret_cast<uintptr_t>(method) + g.memory->offsets.method_name);
}

Il2Cpp::Type *Il2CppApi::getMethodReturnType(Il2Cpp::Method *method) {
	return *reinterpret_cast<Il2Cpp::Type**>(reinterpret_cast<uintptr_t>(method) + g.memory->offsets.method_return_type);
}

int16_t Il2CppApi::getMethodFlags(Il2Cpp::Method *method) {
	static int16_t (*il2cpp_method_get_flags)(Il2Cpp::Method *method) = reinterpret_cast<decltype(il2cpp_method_get_flags)>(
	    g.memory->getUnityBase() + g.memory->offsets.method_get_flags);
    return il2cpp_method_get_flags(method);
}

int8_t Il2CppApi::getParameterCount(Il2Cpp::Method *method) {
	return *reinterpret_cast<uint8_t*>(reinterpret_cast<uintptr_t>(method) + g.memory->offsets.method_parameters_count);
}

Il2Cpp::Type *Il2CppApi::getParameter(Il2Cpp::Method *method, size_t index) {
	static Il2Cpp::Type *(*il2cpp_method_get_parameter)(Il2Cpp::Method *, size_t index) = reinterpret_cast<decltype(il2cpp_method_get_parameter)>(
	    g.memory->getUnityBase() + g.memory->offsets.method_get_parameter);
    return il2cpp_method_get_parameter(method, index);
}

uintptr_t Il2CppApi::getMethodRealVirtualAddress(Il2Cpp::Method *method) {
	return *reinterpret_cast<uintptr_t*>(reinterpret_cast<uintptr_t>(method) + g.memory->offsets.method_pointer);
}

uintptr_t Il2CppApi::getMethodVirtualAddress(Il2Cpp::Method *method) {
	return *reinterpret_cast<uintptr_t*>(reinterpret_cast<uintptr_t>(method) + g.memory->offsets.method_virtual_pointer);
}

uint16_t Il2CppApi::getMethodSlot(Il2Cpp::Method *method) {
	return *reinterpret_cast<uint16_t*>(reinterpret_cast<uintptr_t>(method) + g.memory->offsets.method_slot);
}

uint8_t Il2CppApi::getMethodIsGeneric(Il2Cpp::Method *method) {
    static uint8_t (*il2cpp_method_is_generic)(Il2Cpp::Method *method) = reinterpret_cast<decltype(il2cpp_method_is_generic)>(
        g.memory->getUnityBase() + g.memory->offsets.method_get_is_generic);
    return il2cpp_method_is_generic(method);
}

const char *Il2CppApi::getParameterName(Il2Cpp::Method *method, size_t index) {
	static const char *(*il2cpp_get_parameter_name)(Il2Cpp::Method*, size_t) = reinterpret_cast<decltype(il2cpp_get_parameter_name)>(
        g.memory->getUnityBase() + g.memory->offsets.method_get_parameter_name);

    return il2cpp_get_parameter_name(method, index);
}