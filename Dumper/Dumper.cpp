//
// Created by alexandr on 25.04.2026.
//

#include "Dumper.h"

#include "LimeWare.h"

#include <cinttypes>
#include <fstream>
#include <filesystem>

bool Dumper::initialize() {
    m_vecAssemblies = g.il2cpp->getAssemblies();
    for (Il2Cpp::Assembly *assembly : m_vecAssemblies) {
        if (g.memory->isPtrValid(reinterpret_cast<uintptr_t>(assembly))) {
            Il2Cpp::Image *image = g.il2cpp->getImage(assembly);
            if (g.memory->isPtrValid(reinterpret_cast<uintptr_t>(image))) {
                m_vecImages.push_back(image);
                for (Il2Cpp::Class* klass : g.il2cpp->getClasses(image)) {
                    if (g.memory->isPtrValid(reinterpret_cast<uintptr_t>(klass))) {
                        m_vecClasses.push_back(klass);
                    }
                }
            }
        }
    }

    g.log->info("Assemblies found: %d", m_vecAssemblies.size());
    g.log->info("Images found: %d", m_vecImages.size());
    g.log->info("Classes found: %d", m_vecClasses.size());

    return !m_vecAssemblies.empty() && !m_vecImages.empty() && !m_vecClasses.empty();
}

void Dumper::dump(const char *path) {
    std::filesystem::path directory(path);

    if (!std::filesystem::exists(directory)) {
        g.log->error("Directory does not exist: %s.", directory.c_str());
        return;
    }

    std::filesystem::path dump_file(directory / "dump.cs");
    std::ofstream dump(dump_file.c_str());

    for (int i = 0; i < m_vecImages.size(); i++) {
        dump << "// Image " << i << ": " << g.il2cpp->getImageName(m_vecImages[i]) << std::endl;
    }

    dump << std::endl;


    for (Il2Cpp::Class *klass : m_vecClasses) {
        dump << dumpClass(klass) << std::endl;
    }
}

std::string Dumper::getTypeIndexStr(Il2Cpp::Type *type) {
    int8_t type_index = g.il2cpp->getTypeIndex(type);
    switch (type_index) {
        case 0x00: break;
        case 0x01: return "void";
        case 0x02: return "bool";
        case 0x03: return "char";
        case 0x04: return "byte";
        case 0x05: return "ubyte";
        case 0x06: return "short";
        case 0x07: return "ushort";
        case 0x08: return "int";
        case 0x09: return "uint";
        case 0x0a: return "long";
        case 0x0b: return "ulong";
        case 0x0c: return "float";
        case 0x0d: return "double";
        case 0x0e: return "string";
        case 0x0f: return "void*";
        case 0x18: return "IntPtr";
        case 0x19: return "UIntPtr";
        case 0x1c: return "object";
        case 0x1e:
        case 0x13: return "T";
        case 0x11:
        case 0x12: {
            Il2Cpp::Class *klass = g.il2cpp->getClassFromType(type);
            if (g.memory->isPtrValid(reinterpret_cast<uintptr_t>(klass))) {
                if (g.il2cpp->getIsByref(type)) {
                    int16_t attrs = g.il2cpp->getAttrs(type);
                    if (attrs & 0x2 && !(attrs & 0x1)) {
                        return "out " + std::string(g.il2cpp->getClassName(klass));
                    }
                    if (!(attrs & 0x2) && attrs & 0x1) {
                        return "in " + std::string(g.il2cpp->getClassName(klass));
                    }
                    return "ref " + std::string(g.il2cpp->getClassName(klass));
                }
                return g.il2cpp->getClassName(klass);
            }
            return "object";
        }
        case 0x14: {
            Il2Cpp::ArrayType *array_type = g.il2cpp->getArrayType(type);
            if (g.memory->isPtrValid(reinterpret_cast<uintptr_t>(array_type))) {
                Il2Cpp::Type *root_type = g.il2cpp->getTypeFromArrayType(array_type);
                if (g.memory->isPtrValid(reinterpret_cast<uintptr_t>(root_type))) {
                    return getTypeIndexStr(root_type) + "[]";
                }
            }
            return "object[]";
        }
            break;
        case 0x1D: {
            Il2Cpp::ArrayType *array_type = g.il2cpp->getArrayType(type);
            if (g.memory->isPtrValid(reinterpret_cast<uintptr_t>(array_type))) {
                return getTypeIndexStr(array_type) + "[]";
            }
            return "object[]";
        }
        case 0x15: return g.il2cpp->getTypeName(type);
        default:
            break;
    }
    return "object";
}

std::string Dumper::getFieldAccessStr(Il2Cpp::Field *field) {
    Il2Cpp::Type *type = g.il2cpp->getFieldType(field);
    if (g.memory->isPtrValid(reinterpret_cast<uintptr_t>(type))) {
        int access = g.il2cpp->getAttrs(type) & 0x7;
        switch (access) {
            case 0x1: return "private";
            case 0x2:
            case 0x3: return "internal";
            case 0x4: return "protected";
            case 0x5: return "protected internal";
            case 0x6: return "public";
            default: break;
        }
    }
    return "";
}

std::string Dumper::getFieldModificatorStr(Il2Cpp::Field *field) {
    std::string result;
    Il2Cpp::Type *type = g.il2cpp->getFieldType(field);
    if (g.memory->isPtrValid(reinterpret_cast<uintptr_t>(type))) {
        int16_t modificator = g.il2cpp->getAttrs(type);
        if (modificator & 0x40) {
            result += "const ";
        } else {
            if (modificator & 0x10) {
                result += "static ";
            }
            if (modificator & 0x20) {
                result += "readonly ";
            }
        }
        if (!result.empty()) result.pop_back();

        return result;
    }
    return "";
}

std::string Dumper::getMethodAccessStr(Il2Cpp::Method *method) {
    int16_t access = g.il2cpp->getMethodFlags(method) & 0x7;
    switch (access) {
        case 0x1:return "private";
        case 0x2:
        case 0x3: return "internal";
        case 0x4: return "protected";
        case 0x5: return "protected internal";
        case 0x6: return "public";
        default: break;
    }
    return "";
}

std::string Dumper::getMethodModificatorStr(Il2Cpp::Method *method) {
    std::string result;
    int16_t modificator = g.il2cpp->getMethodFlags(method);

    result += modificator & 0x10 ? "static " : "";
    if (modificator & 0x400) {
        result += !(modificator & 0x100) ? "abstract " : "abstract override ";
    } else if (modificator & 0x20) {
        result += (modificator & 0x100) == 0x100 ? "sealed override " : "";
    } else if (modificator & 0x40) {
        result += (modificator & 0x100) == 0x100 ? "virtual " : "override ";
    }
    result += modificator & 0x2000 ? "extern " : "";

    if (!result.empty()) result.pop_back();

    return result;
}

std::string Dumper::getClassAccessStr(Il2Cpp::Class *klass) {
    int64_t access = g.il2cpp->getClassFlags(klass) & 0x7;
    switch (access) {
        case 0x1:
        case 0x2: return "public";
        case 0x0:
        case 0x5:
        case 0x6: return "internal";
        case 0x3: return "private";
        case 0x4: return "protected";
        case 0x7: return "protected internal";
        default: break;
    }
    return "";
}

std::string Dumper::getClassModificatorStr(Il2Cpp::Class *klass) {
    std::string result;
    int64_t flags = g.il2cpp->getClassFlags(klass);
    Il2Cpp::Type *this_type = g.il2cpp->getThisType(klass);
    if (g.memory->isPtrValid(reinterpret_cast<uintptr_t>(this_type))) {
        bool is_struct = g.il2cpp->getTypeIndex(this_type) == 0x11;
        bool is_enum = g.il2cpp->isClassEnum(klass) == 1;
        if (flags & 0x80 && flags & 0x100) {
            result += "static ";
        } else if (!(flags & 0x20) && flags & 0x80) {
            result += "abstract ";
        } else if (!is_struct && !is_enum && flags & 0x100) {
            result += "sealed ";
        }
        if (flags & 0x20) {
            result += "interface ";
        } else if (is_enum) {
            result += "enum ";
        } else if (is_struct) {
            result += "struct ";
        } else {
            result += "class ";
        }
    }

    if (!result.empty()) result.pop_back();

    return result;
}

std::string Dumper::dumpField(Il2Cpp::Field *field, bool is_enum, bool is_struct) {
    std::string result;
    Il2Cpp::Type *type = g.il2cpp->getFieldType(field);
    if (g.memory->isPtrValid(reinterpret_cast<uintptr_t>(type))) {
        result += getFieldAccessStr(field) + " ";
        result += !getFieldModificatorStr(field).empty() ? getFieldModificatorStr(field) + " " : "";
        result += getTypeIndexStr(type) + " ";
        result += std::string(g.il2cpp->getFieldName(field));

        if (g.il2cpp->getAttrs(type) & 0x40) {
            result += " = ";
            Il2Cpp::Type *default_type = nullptr;
            void *default_value = g.il2cpp->getFieldDefaultValue(field, &default_type);
            if (g.memory->isPtrValid(reinterpret_cast<uintptr_t>(default_value)) && g.memory->isPtrValid(reinterpret_cast<uintptr_t>(default_type))) {
                std::stringstream ss;

                switch (g.il2cpp->getTypeIndex(default_type)) {
                    case 0x2:
                        ss << *static_cast<bool*>(default_value);
                        break;
                    case 0x5:
                        ss << *static_cast<uint8_t*>(default_value);
                        break;
                    case 0x4:
                        ss << *static_cast<int8_t*>(default_value);
                        break;
                    case 0x6:
                        ss << *static_cast<int16_t*>(default_value);
                        break;
                    case 0x7:
                    case 0x3:
                        ss << *static_cast<uint16_t*>(default_value);
                        break;
                    case 0x8: {
                        const char* p = static_cast<const char*>(default_value);
                        uint32_t val = 0;
                        unsigned char r = static_cast<unsigned char>(*p++);

                        if ((r & 0x80) == 0) val = r;
                        else if ((r & 0xC0) == 0x80) { val = (r & ~0x80) << 8; val |= static_cast<unsigned char>(*p++); }
                        else if ((r & 0xE0) == 0xC0) {
                            val = (r & ~0xC0) << 24;
                            val |= (static_cast<uint32_t>(static_cast<unsigned char>(*p++)) << 16);
                            val |= (static_cast<uint32_t>(static_cast<unsigned char>(*p++)) << 8);
                            val |= static_cast<unsigned char>(*p++);
                        }
                        else if (r == 0xF0) { val = *reinterpret_cast<const uint32_t*>(p); }
                        else if (r == 0xFE) val = 0xFFFFFFFF - 1;
                        else if (r == 0xFF) val = 0xFFFFFFFF;

                        if (val == 0xFFFFFFFF) ss << INT32_MIN;
                        else {
                            if (val & 1) ss << -(int32_t)((val >> 1) + 1);
                            else ss << (int32_t)(val >> 1);
                        }

                        break;
                    }

                    case 0x9: {
                        const char* p = static_cast<const char*>(default_value);
                        uint32_t val = 0;
                        unsigned char r = static_cast<unsigned char>(*p++);

                        if ((r & 0x80) == 0) val = r;
                        else if ((r & 0xC0) == 0x80) { val = (r & ~0x80) << 8; val |= static_cast<unsigned char>(*p++); }
                        else if ((r & 0xE0) == 0xC0) {
                            val = (r & ~0xC0) << 24;
                            val |= (static_cast<uint32_t>(static_cast<unsigned char>(*p++)) << 16);
                            val |= (static_cast<uint32_t>(static_cast<unsigned char>(*p++)) << 8);
                            val |= static_cast<unsigned char>(*p++);
                        }
                        else if (r == 0xF0) { val = *reinterpret_cast<const uint32_t*>(p); }
                        else if (r == 0xFE) val = 0xFFFFFFFF - 1;
                        else if (r == 0xFF) val = 0xFFFFFFFF;

                        ss << val;
                        break;
                    }
                    case 0x19:
                    case 0xB:
                        ss << *static_cast<uint64_t*>(default_value);
                        break;
                    case 0x18:
                    case 0xA:
                        ss << *static_cast<int64_t*>(default_value);
                        break;
                    case 0xC:
                        ss << *static_cast<float*>(default_value);
                        break;
                    case 0xD:
                        ss << *static_cast<double*>(default_value);
                        break;
                    case 0xE: {
                        const char* s_ptr = static_cast<const char*>(default_value);
                        size_t start = 0;
                        while (s_ptr[start] != '\0' && (static_cast<unsigned char>(s_ptr[start]) <= 31 || static_cast<unsigned char>(s_ptr[start]) >= 127)) {
                            start++;
                        }

                        size_t end = start;
                        while (s_ptr[end] != '\0' && static_cast<unsigned char>(s_ptr[end]) > 31 && static_cast<unsigned char>(s_ptr[end]) < 127) {
                            end++;
                        }

                        size_t len = end - start;

                        ss << "\"" << std::string_view(s_ptr + start, len) << "\"";
                    }
                        break;
                    default:
                        break;
                }

                result += ss.str();
            }
        }
        result += ";";

        int32_t offset = g.il2cpp->getFieldOffset(field);
        if (is_enum) {
            if (result.find("const") == std::string::npos) {
                offset -= offset > 0x10 ? 0x10 : 0x0;
                result += " // " + (std::stringstream() << "0x" << std::hex << std::uppercase << offset).str();
            }
        } else if (is_struct) {
            if (result.find("const") == std::string::npos && result.find("static") == std::string::npos) {
                offset -= offset > 0x10 ? 0x10 : 0x0;
            }
            result += " // " + (std::stringstream() << "0x" << std::hex << std::uppercase << offset).str();
        } else {
            result += " // " + (std::stringstream() << "0x" << std::hex << std::uppercase << offset).str();
        }

    }
    return result;
}

std::string Dumper::dumpProperty(Il2Cpp::Property *property) {
    std::string result;
    Il2Cpp::Method *getter = g.il2cpp->getPropertyGetter(property);
    if (g.memory->isPtrValid(reinterpret_cast<uintptr_t>(getter))) {
        result += !getMethodAccessStr(getter).empty() ? getMethodAccessStr(getter) + " " : "";
        Il2Cpp::Type *return_type = g.il2cpp->getMethodReturnType(getter);
        if (g.memory->isPtrValid(reinterpret_cast<uintptr_t>(return_type))) {
            result += getTypeIndexStr(return_type) + " ";
            result += g.il2cpp->getPropertyName(property);
            result += " { get; ";
            if (g.memory->isPtrValid(reinterpret_cast<uintptr_t>(g.il2cpp->getPropertySetter(property)))) {
                result += "set; }";
            } else {
                result += "}";
            }
        }
    }
    return result;
}

std::string Dumper::dumpMethod(Il2Cpp::Method *method) {
    std::string result;

    uintptr_t va = g.il2cpp->getMethodVirtualAddress(method) - g.memory->getUnityBase();
    uintptr_t rva = g.il2cpp->getMethodRealVirtualAddress(method) - g.memory->getUnityBase();

    result += "// RVA: " + (std::stringstream() << "0x" << std::hex << std::uppercase << rva).str() + " Offset: " +
        (std::stringstream() << "0x" << std::hex << std::uppercase << rva).str() + " VA: " +
            (std::stringstream() << "0x" << std::hex << std::uppercase << va).str() +
                (g.il2cpp->getMethodSlot(method) != 65535 ? " Slot: " + std::to_string(g.il2cpp->getMethodSlot(method)) : "") + "\n\t";

    result += getMethodAccessStr(method) + " ";
    result += !getMethodModificatorStr(method).empty() ? getMethodModificatorStr(method) + " " : "";

    Il2Cpp::Type *return_type = g.il2cpp->getMethodReturnType(method);
    if (g.memory->isPtrValid(reinterpret_cast<uintptr_t>(return_type))) {
        result += getTypeIndexStr(return_type) + " ";
    }

    result += std::string(g.il2cpp->getMethodName(method)) + "(";
    for (int parameter_index = 0; parameter_index < g.il2cpp->getParameterCount(method); parameter_index++) {
        Il2Cpp::Type *parameter = g.il2cpp->getParameter(method, parameter_index);
        if (g.memory->isPtrValid(reinterpret_cast<uintptr_t>(parameter))) {
            result += getTypeIndexStr(parameter) + " ";
            result += g.il2cpp->getParameterName(method, parameter_index);
            if (parameter_index != g.il2cpp->getParameterCount(method) - 1) {
                 result += ", ";
            }
        }
    }

    result += ") { }";
    return result;
}

std::string Dumper::dumpClass(Il2Cpp::Class *klass) {
    std::string result;
    result += "// Namespace: " + std::string(g.il2cpp->getClassNamespace(klass)) + "\n";
    Il2Cpp::Type *this_type = g.il2cpp->getThisType(klass);
    if (g.memory->isPtrValid(reinterpret_cast<uintptr_t>(this_type))) {
        result += g.il2cpp->getClassFlags(klass) & 0x2000 ? "[Serializable]\n" : "";
        result += getClassAccessStr(klass) + " ";
        result += getClassModificatorStr(klass) + " ";

        Il2Cpp::Class *declaring = g.il2cpp->getDeclaringClass(klass);
        if (g.memory->isPtrValid(reinterpret_cast<uintptr_t>(declaring))) {
            result += std::string(g.il2cpp->getClassName(declaring)) + ".";
        }

        result += g.il2cpp->getIsGeneric(klass) && g.memory->isPtrValid(reinterpret_cast<uintptr_t>(g.il2cpp->getThisType(klass))) ?
            g.il2cpp->getTypeName(g.il2cpp->getThisType(klass)) : g.il2cpp->getClassName(klass);

        if (!g.il2cpp->isClassEnum(klass) && g.il2cpp->getTypeIndex(this_type) != 0x11) {
            Il2Cpp::Class *parent = g.il2cpp->getParent(klass);
            if (g.memory->isPtrValid(reinterpret_cast<uintptr_t>(parent))) {
                if (!strcmp(g.il2cpp->getClassName(parent), "Object") && !strcmp(g.il2cpp->getClassNamespace(parent), "System")) {

                } else {
                    Il2Cpp::Type *parent_type = g.il2cpp->getThisType(parent);
                    if (g.memory->isPtrValid(reinterpret_cast<uintptr_t>(parent_type))) {
                        result += " : " + g.il2cpp->getTypeName(parent_type) + (g.il2cpp->getInterfaces(klass).size() > 0 ? ", " : "");
                    }
                }
            }

            if (g.il2cpp->getInterfaces(klass).size() > 0 && result.find(" : ") == std::string::npos) {
                result += " : ";
            }

            for (int interface_index = 0; interface_index < g.il2cpp->getInterfaces(klass).size(); interface_index++) {
                Il2Cpp::Class *interface = g.il2cpp->getInterfaces(klass).at(interface_index);
                if (g.memory->isPtrValid(reinterpret_cast<uintptr_t>(interface))) {
                    Il2Cpp::Type *interface_type = g.il2cpp->getThisType(interface);
                    if (g.memory->isPtrValid(reinterpret_cast<uintptr_t>(interface_type))) {
                        if (interface_index != 0) {
                            result += ", ";
                        }
                        result += g.il2cpp->getTypeName(interface_type);
                    }
                }
            }

            if (g.il2cpp->getInterfaces(klass).size() > 1) {
                result.pop_back();
                result.pop_back();
            }
        }

        result += "\n{";
        result += !g.il2cpp->getFields(klass).empty() > 0 ? "\n\t// Fields\n" : "";

        for (Il2Cpp::Field *field : g.il2cpp->getFields(klass)) {
            result += "\t" + dumpField(field, g.il2cpp->isClassEnum(klass), g.il2cpp->getTypeIndex(this_type) == 0x11) + "\n";
        }

        if (!g.il2cpp->isClassEnum(klass)) {
            result += !g.il2cpp->getProperties(klass).empty() ? "\n\t// Properties\n" : "";

            for (Il2Cpp::Property *property : g.il2cpp->getProperties(klass)) {
                result += "\t" + dumpProperty(property) + "\n";
            }

            result += !g.il2cpp->getMethods(klass).empty() > 0 ? "\n\t// Methods\n\n" : "";

            for (int method_index = 0; method_index < g.il2cpp->getMethods(klass).size(); method_index++) {
                Il2Cpp::Method *method = g.il2cpp->getMethods(klass).at(method_index);
                if (g.memory->isPtrValid(reinterpret_cast<uintptr_t>(method))) {
                    result += "\t" + dumpMethod(method) + "\n";
                    if (method_index != g.il2cpp->getMethods(klass).size() - 1) {
                        result += "\n";
                    }
                }
            }
        }

        result +="}\n";
    }

    return result;
}