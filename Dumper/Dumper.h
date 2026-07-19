#ifndef LIMEWARE_STANDOFF2_DUMPER_DUMPER_H
#define LIMEWARE_STANDOFF2_DUMPER_DUMPER_H

#include "Il2CppApi.h"

#include <vector>


class Dumper {

    std::vector<Il2Cpp::Assembly*> m_vecAssemblies;
    std::vector<Il2Cpp::Image*> m_vecImages;
    std::vector<Il2Cpp::Class*> m_vecClasses;

    std::string getTypeIndexStr(Il2Cpp::Type *type);

    std::string getFieldAccessStr(Il2Cpp::Field *field);
    std::string getFieldModificatorStr(Il2Cpp::Field *field);

    std::string getMethodAccessStr(Il2Cpp::Method *method);
    std::string getMethodModificatorStr(Il2Cpp::Method *method);

    std::string getClassAccessStr(Il2Cpp::Class *klass);
    std::string getClassModificatorStr(Il2Cpp::Class *klass);

    std::string dumpField(Il2Cpp::Field *field, bool is_enum, bool is_struct);
    std::string dumpProperty(Il2Cpp::Property *property);
    std::string dumpMethod(Il2Cpp::Method *method);
    std::string dumpClass(Il2Cpp::Class *klass);

public:
    Dumper() = default;
    ~Dumper() = default;

    bool initialize();
    void dump(const char *path);
};


#endif