//
// Created by alexandr on 27.04.2026.
//

#ifndef LIMEWARE_STANDOFF2_DUMPER_IL2CPPAPI_H
#define LIMEWARE_STANDOFF2_DUMPER_IL2CPPAPI_H

#include <cstdint>
#include <vector>

namespace Il2Cpp {
    typedef void Domain;
    typedef void Assembly;
    typedef void Image;
    typedef void Class;
    typedef void Field;
    typedef void Property;
    typedef void Method;
    typedef void Type;
    typedef void ArrayType;
    typedef void GenericMethod;
    typedef void GenericClass;
}


class Il2CppApi {
public:
    Il2CppApi() = default;
    ~Il2CppApi() = default;

    bool initialize();

    // assembly
    Il2Cpp::Assembly* openAssembly(const char* name);
    std::vector<Il2Cpp::Assembly*> getAssemblies();
    const char *getAssemblyName(Il2Cpp::Assembly *assembly);
    Il2Cpp::Image *getImage(Il2Cpp::Assembly *assembly);

    // image
    int getTypeCount(Il2Cpp::Image *image);
    std::vector<Il2Cpp::Class*> getClasses(Il2Cpp::Image *image);
    const char *getImageName(Il2Cpp::Image *image);

    // class
    Il2Cpp::Class *getClassFromName(const char *assembly, const  char *name);
    const char *getClassName(Il2Cpp::Class *klass);
    const char *getClassNamespace(Il2Cpp::Class *klass);
    Il2Cpp::Class *getParent(Il2Cpp::Class *klass);
    std::vector<Il2Cpp::Field*> getFields(Il2Cpp::Class *klass);
    std::vector<Il2Cpp::Property*> getProperties(Il2Cpp::Class *klass);
    std::vector<Il2Cpp::Method*> getMethods(Il2Cpp::Class *klass);
    Il2Cpp::Type *getThisType(Il2Cpp::Class *klass);
    bool isClassEnum(Il2Cpp::Class *klass);
    uint64_t getClassFlags(Il2Cpp::Class *klass);
    std::vector<Il2Cpp::Class*> getInterfaces(Il2Cpp::Class *klass);
    Il2Cpp::Class *getDeclaringClass(Il2Cpp::Class *klass);
    bool getIsGeneric(Il2Cpp::Class *klass);

    // field
    const char *getFieldName(Il2Cpp::Field *field);
    int32_t getFieldOffset(Il2Cpp::Field *field);
    Il2Cpp::Type *getFieldType(Il2Cpp::Field *field);
    void *getFieldDefaultValue(Il2Cpp::Field *field, Il2Cpp::Type **type);

    // type
    int8_t getTypeIndex(Il2Cpp::Type *type);
    int16_t getAttrs(Il2Cpp::Type *type);
    Il2Cpp::Class *getClassFromType(Il2Cpp::Type *type);
    Il2Cpp::ArrayType *getArrayType(Il2Cpp::Type *type);
    Il2Cpp::Type *getRootType(Il2Cpp::Type *type);
    bool getIsByref(Il2Cpp::Type *type);
    std::string getTypeName(Il2Cpp::Type *type, bool simple = true);

    // array type
    Il2Cpp::Type *getTypeFromArrayType(Il2Cpp::ArrayType *array_type);

    // property
    const char *getPropertyName(Il2Cpp::Property *property);
    Il2Cpp::Method *getPropertySetter(Il2Cpp::Property *property);
    Il2Cpp::Method *getPropertyGetter(Il2Cpp::Property *property);

    // method
    const char *getMethodName(Il2Cpp::Method *method);
    Il2Cpp::Type *getMethodReturnType(Il2Cpp::Method *method);
    int16_t getMethodFlags(Il2Cpp::Method *method);
    int8_t getParameterCount(Il2Cpp::Method *method);
    Il2Cpp::Type *getParameter(Il2Cpp::Method *method, size_t index);
    uintptr_t getMethodRealVirtualAddress(Il2Cpp::Method *method);
    uintptr_t getMethodVirtualAddress(Il2Cpp::Method *method);
    uint16_t getMethodSlot(Il2Cpp::Method *method);
    uint8_t getMethodIsGeneric(Il2Cpp::Method *method);
    const char *getParameterName(Il2Cpp::Method *method, size_t index);
};


#endif //LIMEWARE_STANDOFF2_DUMPER_IL2CPPAPI_H