//
// Created by alexandr on 27.04.2026.
//

#include "Memory.h"

#include <cinttypes>
#include <elf.h>
#include <fcntl.h>
#include <libgen.h>
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <sys/mman.h>

#include "LimeWare.h"
#include "Assembler.h"

static uintptr_t safeFront(const std::vector<uintptr_t>& v) {
    return v.empty() ? UINTPTR_MAX : v.front();
}
static uintptr_t safeBack(const std::vector<uintptr_t>& v) {
    return v.empty() ? UINTPTR_MAX : v.back();
}

std::vector<uintptr_t> Memory::ModuleInfo::findPattern(const std::string& pattern, bool need_first_only) const {
    std::vector<uint8_t> bytes;
    std::string mask;

    std::istringstream iss(pattern);
    std::string token;
    while (iss >> token) {
        if (token == "?") {
            bytes.push_back(0);
            mask.push_back('?');
        } else if (token.size() == 2) {
            auto is_hex = [](char c) {
                return std::isxdigit(static_cast<unsigned char>(c));
            };
            if (is_hex(token[0]) && is_hex(token[1])) {
                uint8_t byte = static_cast<uint8_t>(std::strtol(token.c_str(), nullptr, 16));
                bytes.push_back(byte);
                mask.push_back('x');
            } else {
                return {};
            }
        } else {
            return {};
        }
    }

    size_t scan_size = mask.size();
    if (scan_size == 0 || scan_size > size) {
        return {};
    }

    std::vector<uintptr_t> results;
    const uint8_t* base_ptr = reinterpret_cast<const uint8_t*>(base);
    const size_t max_offset = size - scan_size;

    for (size_t offset = 0; offset <= max_offset; ++offset) {
        bool found = true;
        for (size_t j = 0; j < scan_size; ++j) {
            if (mask[j] == 'x') {
                if (base_ptr[offset + j] != bytes[j]) {
                    found = false;
                    break;
                }
            }
        }
        if (found) {
            results.push_back(offset + 0x4000);
            if (need_first_only) {
                break;
            }
        }
    }

    return results;
}

bool Memory::initialize() {

    m_libunity = getModuleInfo("libunity.so");

    getModuleInfo("libunity.so", [&](const ModuleInfo &module) {
        if (module.size < 100000000) {
            return;
        }

        int current_step = 0;
        const int total_steps = 44;

        auto log_progress = [&](const char* name, uintptr_t address) {
            current_step++;
            int percent = (current_step * 100) / total_steps;

            std::string bar = "";
            int bar_width = 10;
            int progress = (percent * bar_width) / 100;
            for (int i = 0; i < bar_width; i++) {
                if (i < progress) bar += "=";
                else if (i == progress) bar += ">";
                else bar += ".";
            }

            g().log->info("[%s] %3d%% | Finished: %s (0x%lX)", bar.c_str(), percent, name, (unsigned long)address);
        };

        // --- Блок Assemblies ---
        if (offsets.assemblies_start == UINTPTR_MAX) offsets.assemblies_start = findAssembliesStart(module); log_progress("assemblies_start", offsets.assemblies_start);
        if (offsets.assemblies_end == UINTPTR_MAX) offsets.assemblies_end = findAssembliesEnd(module); log_progress("assemblies_end", offsets.assemblies_end);

        // --- Блок Assembly ---
        if (offsets.assembly_open == UINTPTR_MAX) offsets.assembly_open = findAssemblyOpen(module); log_progress("assembly_open", offsets.assembly_open);
        if (offsets.assembly_name == UINTPTR_MAX) offsets.assembly_name = findAssemblyName(module); log_progress("assembly_name", offsets.assembly_name);
        if (offsets.assembly_get_image == UINTPTR_MAX) offsets.assembly_get_image = findAssemblyGetImage(module); log_progress("assembly_get_image", offsets.assembly_get_image);

        // --- Блок Image & Handle ---
        if (offsets.image_type_count == UINTPTR_MAX) offsets.image_type_count = findImageTypeCount(module); log_progress("image_type_count", offsets.image_type_count);
        if (offsets.image_get_assembly_type_handle == UINTPTR_MAX) offsets.image_get_assembly_type_handle = findGetAssemblyTypeHandle(module); log_progress("image_get_assembly_type_handle", offsets.image_get_assembly_type_handle);
        if (offsets.handle_get_type_from_handle == UINTPTR_MAX) offsets.handle_get_type_from_handle = findGetTypeFromHandle(module); log_progress("handle_get_type_from_handle", offsets.handle_get_type_from_handle);
        if (offsets.image_name == UINTPTR_MAX) offsets.image_name = findImageName(module); log_progress("image_name", offsets.image_name);

        // --- Блок Class ---
        if (offsets.class_get_name == UINTPTR_MAX) offsets.class_get_name = findClassGetName(module); log_progress("class_get_name", offsets.class_get_name);
        if (offsets.class_namespace == UINTPTR_MAX) offsets.class_namespace = findClassNamespace(module); log_progress("class_namespace", offsets.class_namespace);
        if (offsets.class_get_parent == UINTPTR_MAX) offsets.class_get_parent = findClassGetParent(module); log_progress("class_get_parent", offsets.class_get_parent);
        if (offsets.class_get_fields == UINTPTR_MAX) offsets.class_get_fields = findClassGetFields(module); log_progress("class_get_fields", offsets.class_get_fields);
        if (offsets.class_get_properties == UINTPTR_MAX) offsets.class_get_properties = findClassGetProperties(module); log_progress("class_get_properties", offsets.class_get_properties);
        if (offsets.class_get_methods == UINTPTR_MAX) offsets.class_get_methods = findClassGetMethods(module); log_progress("class_get_methods", offsets.class_get_methods);
        if (offsets.class_type == UINTPTR_MAX) offsets.class_type = findClassType(module); log_progress("class_type", offsets.class_type);
        if (offsets.class_get_enum_basetype == UINTPTR_MAX) offsets.class_get_enum_basetype = findClassGetEnumBasetype(module); log_progress("class_get_enum_basetype", offsets.class_get_enum_basetype);
        if (offsets.class_get_flags == UINTPTR_MAX) offsets.class_get_flags = findClassGetFlags(module); log_progress("class_get_flags", offsets.class_get_flags);
        if (offsets.class_get_interfaces == UINTPTR_MAX) offsets.class_get_interfaces = findClassGetInterfaces(module); log_progress("class_get_interfaces", offsets.class_get_interfaces);
        if (offsets.class_get_declaring_type == UINTPTR_MAX) offsets.class_get_declaring_type = findClassGetDeclaringType(module); log_progress("class_get_declaring_type", offsets.class_get_declaring_type);
        if (offsets.class_get_is_generic == UINTPTR_MAX) offsets.class_get_is_generic = findClassGetIsGeneric(module); log_progress("class_get_is_generic", offsets.class_get_is_generic);

        // --- Блок Field ---
        if (offsets.field_get_name == UINTPTR_MAX) offsets.field_get_name = findFieldGetName(module); log_progress("field_get_name", offsets.field_get_name);
        if (offsets.field_get_offset == UINTPTR_MAX) offsets.field_get_offset = findFieldGetOffset(module); log_progress("field_get_offset", offsets.field_get_offset);
        if (offsets.field_get_type == UINTPTR_MAX) offsets.field_get_type = findFieldGetType(module); log_progress("field_get_type", offsets.field_get_type);
        if (offsets.field_get_default_value == UINTPTR_MAX) offsets.field_get_default_value = findFieldGetDefaultValue(module); log_progress("field_get_default_value", offsets.field_get_default_value);

        // --- Блок Type ---
        if (offsets.type_get_type == UINTPTR_MAX) offsets.type_get_type = findTypeGetType(module); log_progress("type_get_type", offsets.type_get_type);
        if (offsets.type_attrs == UINTPTR_MAX) offsets.type_attrs = findTypeAttrs(module); log_progress("type_attrs", offsets.type_attrs);
        if (offsets.type_attrs_shift == UINTPTR_MAX) offsets.type_attrs_shift = findTypeAttrsShift(module); log_progress("type_attrs_shift", offsets.type_attrs_shift);
        if (offsets.type_get_class_from_type == UINTPTR_MAX) offsets.type_get_class_from_type = findTypeGetClassFromType(module); log_progress("type_get_class_from_type", offsets.type_get_class_from_type);
        if (offsets.type_get_data == UINTPTR_MAX) offsets.type_get_data = findTypeGetData(module); log_progress("type_get_data", offsets.type_get_data);
        if (offsets.type_get_name == UINTPTR_MAX) offsets.type_get_name = findTypeGetName(module); log_progress("type_get_name", offsets.type_get_name);

        // --- Блок Property ---
        if (offsets.property_name == UINTPTR_MAX) offsets.property_name = findPropertyName(module); log_progress("property_name", offsets.property_name);
        if (offsets.property_get_method == UINTPTR_MAX) offsets.property_get_method = findPropertyGetMethod(module); log_progress("property_get_method", offsets.property_get_method);
        if (offsets.property_set_method == UINTPTR_MAX) offsets.property_set_method = findPropertySetMethod(module); log_progress("property_set_method", offsets.property_set_method);

        // --- Блок Method ---
        if (offsets.method_name == UINTPTR_MAX) offsets.method_name = findMethodName(module); log_progress("method_name", offsets.method_name);
        if (offsets.method_return_type == UINTPTR_MAX) offsets.method_return_type = findMethodReturnType(module); log_progress("method_return_type", offsets.method_return_type);
        if (offsets.method_get_flags == UINTPTR_MAX) offsets.method_get_flags = findMethodGetFlags(module); log_progress("method_get_flags", offsets.method_get_flags);
        if (offsets.method_parameters_count == UINTPTR_MAX) offsets.method_parameters_count = findMethodParametersCount(module); log_progress("method_parameters_count", offsets.method_parameters_count);
        if (offsets.method_get_parameter == UINTPTR_MAX) offsets.method_get_parameter = findMethodGetParameter(module); log_progress("method_get_parameter", offsets.method_get_parameter);
        if (offsets.method_pointer == UINTPTR_MAX) offsets.method_pointer = findMethodPointer(module); log_progress("method_pointer", offsets.method_pointer);
        if (offsets.method_virtual_pointer == UINTPTR_MAX) offsets.method_virtual_pointer = findMethodVirtualPointer(module); log_progress("method_virtual_pointer", offsets.method_virtual_pointer);
        if (offsets.method_slot == UINTPTR_MAX) offsets.method_slot = findMethodSlot(module); log_progress("method_slot", offsets.method_slot);
        if (offsets.method_get_is_generic == UINTPTR_MAX) offsets.method_get_is_generic = findMethodGetIsGeneric(module); log_progress("method_get_is_generic", offsets.method_get_is_generic);
        if (offsets.method_get_parameter_name == UINTPTR_MAX) offsets.method_get_parameter_name = findMethodGetParameterName(module); log_progress("method_get_parameter_name", offsets.method_get_parameter_name);

        g().log->info("=== OFFSETS ===");
        g().log->info("Assemblies start: %p", offsets.assemblies_start);
        g().log->info("Assemblies end: %p", offsets.assemblies_end);
        g().log->info("Assembly->Open(): %p", offsets.assembly_open);
        g().log->info("Assembly->name: %p", offsets.assembly_name);
        g().log->info("Assembly->getImage(): %p", offsets.assembly_get_image);
        g().log->info("Image->type_count: %p", offsets.image_type_count);
        g().log->info("Image->getAssemblyTypeHandle(): %p", offsets.image_get_assembly_type_handle);
        g().log->info("Handle->getTypeFromHandle(): %p", offsets.handle_get_type_from_handle);
        g().log->info("Image->name: %p", offsets.image_name);
        g().log->info("Class->getName(): %p", offsets.class_get_name);
        g().log->info("Class->namespace: %p", offsets.class_namespace);
        g().log->info("Class->getParent(): %p", offsets.class_get_parent);
        g().log->info("Class->getFields(): %p", offsets.class_get_fields);
        g().log->info("Class->getProperties(): %p", offsets.class_get_properties);
        g().log->info("Class->getMethods(): %p", offsets.class_get_methods);
        g().log->info("Class->type: %p", offsets.class_type);
        g().log->info("Class->getEnumBasetype(): %p", offsets.class_get_enum_basetype);
        g().log->info("Class->getFlags(): %p", offsets.class_get_flags);
        g().log->info("Class->getInterfaces(): %p", offsets.class_get_interfaces);
        g().log->info("Class->getDeclaringType(): %p", offsets.class_get_declaring_type);
        g().log->info("Class->isGeneric(): %p", offsets.class_get_is_generic);
        g().log->info("Field->getName(): %p", offsets.field_get_name);
        g().log->info("Field->getOffset(): %p", offsets.field_get_offset);
        g().log->info("Field->getType(): %p", offsets.field_get_type);
        g().log->info("Field->getDefaultValue(): %p", offsets.field_get_default_value);
        g().log->info("Type->getType(): %p", offsets.type_get_type);
        g().log->info("Type->attrs: %p", offsets.type_attrs);
        g().log->info("Type->attrs >> shift: %p", offsets.type_attrs_shift);
        g().log->info("Type->getClassFromType():%p", offsets.type_get_class_from_type);
        g().log->info("Type->getData(): %p", offsets.type_get_data);
        g().log->info("Type->getName(): %p", offsets.type_get_name);
        g().log->info("Property->name: %p", offsets.property_name);
        g().log->info("Property->get_method: %p", offsets.property_get_method);
        g().log->info("Property->set_method: %p", offsets.property_set_method);
        g().log->info("Method->name: %p", offsets.method_name);
        g().log->info("Method->return_type: %p", offsets.method_return_type);
        g().log->info("Method->getFlags(): %p", offsets.method_get_flags);
        g().log->info("Method->parameters_count: %p", offsets.method_parameters_count);
        g().log->info("Method->getParameter(): %p", offsets.method_get_parameter);
        g().log->info("Method->pointer: %p", offsets.method_pointer);
        g().log->info("Method->virtual_pointer: %p", offsets.method_virtual_pointer);
        g().log->info("Method->slot: %p", offsets.method_slot);
        g().log->info("Method->isGeneric(): %p", offsets.method_get_is_generic);
        g().log->info("Method->getParameterName(): %p", offsets.method_get_parameter_name);
    });

    return m_libunity.isValid();
}

Memory::ModuleInfo Memory::getModuleInfo(const char *name, const std::function<void(ModuleInfo)> &callback) {
    ModuleInfo info = {};
    FILE *f = fopen("/proc/self/maps", "rt");
    if (!f) {
        return info;
    }

    char line[512];
    while (fgets(line, sizeof line, f)) {
        uintptr_t tmp_base, tmp_end;
        char perms[128], path[256];
        if (sscanf(line, "%" PRIXPTR "-%" PRIXPTR " %4s %*s %*s %*s %s", &tmp_base, &tmp_end, perms, path) > 0) {
            if (!strcmp(basename(path), name)) {
                int permissions = (perms[0] == 'r' ? (1 << 1) : 0) |
                                  (perms[1] == 'w' ? (1 << 2) : 0) |
                                  (perms[2] == 'x' ? (1 << 3) : 0) |
                                  (perms[3] == 'p' ? (1 << 4) : 0);

                if ((permissions & (1 << 1)) && (permissions & (1 << 4))) {
                    auto *elfHeader = reinterpret_cast<Elf64_Ehdr*>(tmp_base);
                    if (elfHeader->e_ident[EI_MAG0] == ELFMAG0 && elfHeader->e_ident[EI_MAG1] == ELFMAG1 &&
                        elfHeader->e_ident[EI_MAG2] == ELFMAG2 && elfHeader->e_ident[EI_MAG3] == ELFMAG3) {
                        info.path = path;
                        info.base = tmp_base;
                        info.end = tmp_end;
                        info.size = tmp_end - tmp_base;
                        if (callback) {
                            callback(info);
                        }
                    }
                }
            }
        }
    }

    fclose(f);
    return info;
}

std::string Memory::dumpHex(uintptr_t address, size_t size) {
    std::stringstream ss;
    const unsigned char* p = reinterpret_cast<const unsigned char*>(address);

    for (size_t i = 0; i < size; i += 16) {
        ss << std::setw(8) << std::setfill('0') << std::hex << (address + i) << ":  ";

        for (size_t j = 0; j < 16; ++j) {
            if (i + j < size) {
                ss << std::setw(2) << std::setfill('0') << std::hex
                   << static_cast<int>(p[i + j]) << " ";
            } else {
                ss << "   ";
            }
        }

        ss << "  ";

        for (size_t j = 0; j < 16; ++j) {
            if (i + j < size) {
                unsigned char ch = p[i + j];
                if (ch >= 32 && ch <= 126) {
                    ss << ch;
                } else {
                    ss << '.';
                }
            }
        }
        ss << "\n";
    }

    return ss.str();
}

void Memory::dumpHexToFile(uintptr_t address, size_t size, const std::string& filePath) {
    std::ofstream outFile(filePath);

    if (!outFile.is_open()) {
        return;
    }

    const unsigned char* p = reinterpret_cast<const unsigned char*>(address);

    for (size_t i = 0; i < size; i += 8) {
        outFile << std::dec << std::setw(8) << std::setfill('0') << i << ":  ";

        for (size_t j = 0; j < 8; ++j) {
            if (i + j < size) {
                outFile << std::setw(2) << std::setfill('0') << std::hex
                        << static_cast<int>(p[i + j]) << " ";
            } else {
                outFile << "   ";
            }
        }

        outFile << "  ";
        for (size_t j = 0; j < 8; ++j) {
            if (i + j < size) {
                unsigned char ch = p[i + j];
                if (ch >= 32 && ch <= 126) {
                    outFile << ch;
                } else {
                    outFile << '.';
                }
            }
        }
        outFile << "\n";
    }

    outFile.close();
}

bool Memory::isPtrValid(uintptr_t address) const {
    if ((address & 0xFFFFFFFFFFFFFF) <= 0x1000000000 || !address) {
        return false;
    }
    static int fd = open("/dev/random", O_WRONLY);
    if (fd != -1) {
        if (write(fd, reinterpret_cast<void *>(address), 1) == -1) {
            if (errno == EFAULT) {
                return false;
            }
        }
    }

    unsigned char vec;
    long pagesize = sysconf(_SC_PAGESIZE);
    uintptr_t page_addr = address & ~(pagesize - 1);

    if (mincore(reinterpret_cast<void *>(page_addr), pagesize, &vec) != 0) {
        return false;
    }

    return true;
}

uintptr_t Memory::findAssembliesStart(ModuleInfo module) {
    auto pattern_result = module.findPattern("? ? ? ? ? ? ? 91 ? ? ? A9 BF 02 16 EB");
    if (pattern_result.empty()) return UINTPTR_MAX;
    uintptr_t adrp_address = g().memory->getUnityBase() + pattern_result.front();
    uintptr_t add_address = adrp_address + 0x4;

    Assembler::adrp adrp = Assembler::adrp(adrp_address, *reinterpret_cast<uint32_t*>(adrp_address));
    Assembler::add add = Assembler::add(add_address, *reinterpret_cast<uint32_t*>(add_address));

    return adrp.page_address + add.offset - g().memory->getUnityBase();
}

uintptr_t Memory::findAssembliesEnd(ModuleInfo module) {
    return offsets.assemblies_start + 0x8;
}

uintptr_t Memory::findAssemblyOpen(ModuleInfo module) {
    auto pattern_result = module.findPattern("? ? ? F8 ? ? ? A9 ? ? ? A9 F3 03 00 AA ? ? ? 95 ? ? ? ? ? ? ? 91", false);
    if (pattern_result.empty()) return UINTPTR_MAX;
    return pattern_result.back();
}

uintptr_t Memory::findAssemblyName(ModuleInfo module) {
    auto pattern_result = module.findPattern("? ? ? F8 ? ? ? A9 ? ? ? A9 F3 03 00 "
                                             "AA ? ? ? 95 ? ? ? ? ? ? ? 91");
    if (pattern_result.empty()) return UINTPTR_MAX;
    uintptr_t function_address = g().memory->getUnityBase() + pattern_result.front();
    uintptr_t ldr_address = function_address + 0x3C;
    Assembler::ldr ldr = Assembler::ldr(ldr_address, *reinterpret_cast<uint32_t*>(ldr_address));
    return ldr.offset;
}


uintptr_t Memory::findAssemblyGetImage(ModuleInfo module) {
    auto pattern_result = module.findPattern("? ? ? 97 ? ? ? 97 E8 03 13 AA", false);
    if (pattern_result.empty()) return UINTPTR_MAX;
    uintptr_t bl_address = g().memory->getUnityBase() + pattern_result.back();
    Assembler::bl bl = Assembler::bl(bl_address, *reinterpret_cast<uint32_t*>(bl_address));
    return bl.target - g().memory->getUnityBase();
}

uintptr_t Memory::findImageTypeCount(ModuleInfo module) {
    auto pattern_result = module.findPattern("? ? ? D1 ? ? ? F9 ? ? ? A9 ? ? ? A9 "
                                             "F3 03 00 AA ? ? ? F9 F4 03 02 AA");
    if (pattern_result.empty()) return UINTPTR_MAX;
    uintptr_t function_address = g().memory->getUnityBase() + pattern_result.front();

    uintptr_t ldr_address = function_address + 0x58;
    Assembler::ldr ldr = Assembler::ldr(ldr_address, *reinterpret_cast<uint32_t*>(ldr_address));

    return ldr.offset;
}


uintptr_t Memory::findGetAssemblyTypeHandle(ModuleInfo module) {
    auto pattern_result = module.findPattern("? ? ? F9 ? ? ? B9 08 01 01 0B");
    if (pattern_result.empty()) return UINTPTR_MAX;
    return pattern_result.front();
}

uintptr_t Memory::findGetTypeFromHandle(ModuleInfo module) {
    auto pattern_result = module.findPattern("? ? ? 97 F9 03 00 AA ? ? ? F9 E1 03 18 AA");
    if (pattern_result.empty()) return UINTPTR_MAX;
    uintptr_t bl_address = g().memory->getUnityBase() + pattern_result.front();
    Assembler::bl bl = Assembler::bl(bl_address, *reinterpret_cast<uint32_t*>(bl_address));
    return bl.target - g().memory->getUnityBase();
}

uintptr_t Memory::findImageName(ModuleInfo module) {
    auto pattern_result = module.findPattern("? ? ? F8 ? ? ? A9 ? ? ? A9 F3 03 00 "
                                             "AA ? ? ? 95 ? ? ? ? ? ? ? 91", false);
    if (pattern_result.empty()) return UINTPTR_MAX;
    uintptr_t function_address = g().memory->getUnityBase() + pattern_result.back();
    uintptr_t ldr_address = function_address + 0xA4;
    Assembler::ldr ldr = Assembler::ldr(ldr_address, *reinterpret_cast<uint32_t*>(ldr_address));
    return ldr.offset;
}


uintptr_t Memory::findClassGetName(ModuleInfo module) {
    auto pattern_result = module.findPattern("? ? ? D1 ? ? ? A9 ? ? ? A9 F3 03 01 AA "
                                             "F4 03 00 AA ? ? ? A9 ? ? ? ? ? ? ? 91");
    if (pattern_result.empty()) return UINTPTR_MAX;
    uintptr_t function_address = g().memory->getUnityBase() + pattern_result.front();
    uintptr_t bl_address = function_address + 0x34;
    Assembler::bl bl = Assembler::bl(bl_address, *reinterpret_cast<uint32_t*>(bl_address));
    return bl.target - g().memory->getUnityBase();
}

uintptr_t Memory::findClassNamespace(ModuleInfo module) {
    Il2Cpp::Class *object = g().il2cpp->getClassFromName("mscorlib.dll", "Object");
    if (g().memory->isPtrValid(reinterpret_cast<uintptr_t>(object))) {
        for (uintptr_t offset = 0; offset < 512; offset += 8) {
            const char *namespaze = *reinterpret_cast<const char**>(reinterpret_cast<uintptr_t>(object) + offset);
            if (g().memory->isPtrValid(reinterpret_cast<uintptr_t>(namespaze))) {
                if (!strcmp(namespaze, "System")) {
                    return offset;
                }
            }
        }
    }
    return UINTPTR_MAX;
}

uintptr_t Memory::findClassGetParent(ModuleInfo module) {
    auto pattern_result = module.findPattern("? ? ? D1 ? ? ? F9 ? ? ? A9 ? ? ? A9 "
                                             "? ? ? A9 ? ? ? A9 F3 03 00 AA ? ? ? 97");
    if (pattern_result.empty()) return UINTPTR_MAX;
    uintptr_t function_address = g().memory->getUnityBase() + pattern_result.front();
    uintptr_t bl_address = function_address + 0x24;
    Assembler::bl bl = Assembler::bl(bl_address, *reinterpret_cast<uint32_t*>(bl_address));
    return bl.target - g().memory->getUnityBase();
}

uintptr_t Memory::findClassGetFields(ModuleInfo module) {
    auto pattern_result = module.findPattern("? ? ? 97 F4 03 00 AA ? ? ? B5 ? ? ? 17");
    if (pattern_result.empty()) return UINTPTR_MAX;
    uintptr_t bl_address = g().memory->getUnityBase() + pattern_result.front();
    Assembler::bl bl = Assembler::bl(bl_address, *reinterpret_cast<uint32_t*>(bl_address));
    return bl.target - g().memory->getUnityBase();
}

uintptr_t Memory::findClassGetProperties(ModuleInfo module) {
    auto pattern_result = module.findPattern("? ? ? 94 F7 03 00 AA ? ? ? B5 ? ? ? A9", false);
    if (pattern_result.empty()) return UINTPTR_MAX;
    uintptr_t bl_address = g().memory->getUnityBase() + pattern_result.back();
    Assembler::bl bl = Assembler::bl(bl_address, *reinterpret_cast<uint32_t*>(bl_address));
    return bl.target - g().memory->getUnityBase();
}

uintptr_t Memory::findClassGetMethods(ModuleInfo module) {
    auto pattern_result = module.findPattern("? ? ? 94 ? ? ? B4 F6 03 00 AA ? ? ? 79");
    if (pattern_result.empty()) return UINTPTR_MAX;
    uintptr_t bl_address = g().memory->getUnityBase() + pattern_result.front();
    Assembler::bl bl = Assembler::bl(bl_address, *reinterpret_cast<uint32_t*>(bl_address));
    return bl.target - g().memory->getUnityBase();
}

uintptr_t Memory::findClassType(ModuleInfo module) {
    auto pattern_result = module.findPattern("? ? ? 97 ? ? ? 39 F8 03 1F AA");
    if (pattern_result.empty()) return UINTPTR_MAX;
    uintptr_t bl_address = g().memory->getUnityBase() + pattern_result.front();
    Assembler::bl bl = Assembler::bl(bl_address, *reinterpret_cast<uint32_t*>(bl_address));

    uintptr_t function_address = bl.target;
    uintptr_t add_address = function_address + 0x4;
    Assembler::add add = Assembler::add(add_address, *reinterpret_cast<uint32_t*>(add_address));

    return add.offset;
}

uintptr_t Memory::findClassGetEnumBasetype(ModuleInfo module) {
    auto pattern_result = module.findPattern("? ? ? 97 ? ? ? 39 F8 03 1F AA");
    if (pattern_result.empty()) return UINTPTR_MAX;
    uintptr_t bl_address = g().memory->getUnityBase() + pattern_result.front();
    Assembler::bl bl = Assembler::bl(bl_address, *reinterpret_cast<uint32_t*>(bl_address));
    return bl.target - g().memory->getUnityBase();
}

uintptr_t Memory::findClassGetFlags(ModuleInfo module) {
    auto pattern_result = module.findPattern("? ? ? B9 C0 03 5F D6 ? ? ? 39 ? ? ? 53 C0 03 5F D6 ? ? ? 39 ? ? ? 37 ? ? ? 39");
    if (pattern_result.empty()) return UINTPTR_MAX;
    return pattern_result.front();
}

uintptr_t Memory::findClassGetInterfaces(ModuleInfo module) {
    auto pattern_result = module.findPattern("? ? ? 94 ? ? ? F9 ? ? ? B4 ? ? ? 91 ? ? ? "
                                             "91 ? ? ? 91 ? ? ? 94");
    if (pattern_result.empty()) return UINTPTR_MAX;
    uintptr_t bl_address = g().memory->getUnityBase() + pattern_result.front();
    Assembler::bl bl = Assembler::bl(bl_address, *reinterpret_cast<uint32_t*>(bl_address));
    return bl.target - g().memory->getUnityBase();
}

uintptr_t Memory::findClassGetDeclaringType(ModuleInfo module) {
    auto pattern_result = module.findPattern("? ? ? F9 C0 03 5F D6 ? ? ? F8 ? ? ? A9 ? ? ? 79", false);
    if (pattern_result.empty()) return UINTPTR_MAX;
    return pattern_result.back();
}

uintptr_t Memory::findClassGetIsGeneric(ModuleInfo module) {
    auto pattern_result = module.findPattern("? ? ? F8 ? ? ? A9 ? ? ? A9 ? ? ? A9 ? ? ? F9 F3 03 01 2A ? ? ? 52");
    if (pattern_result.empty()) return UINTPTR_MAX;
    uintptr_t function_address = g().memory->getUnityBase() + pattern_result.front();
    uintptr_t bl_address = function_address + 0x3C;
    Assembler::bl bl = Assembler::bl(bl_address, *reinterpret_cast<uint32_t*>(bl_address));
    return bl.target - g().memory->getUnityBase();
}

uintptr_t Memory::findFieldGetName(ModuleInfo module) {
    auto pattern_result = module.findPattern("? ? ? 97 F5 03 00 AA E8 03 00 91");
    if (pattern_result.empty()) return UINTPTR_MAX;
    uintptr_t bl_address = g().memory->getUnityBase() + pattern_result.front();
    Assembler::bl bl = Assembler::bl(bl_address, *reinterpret_cast<uint32_t*>(bl_address));
    return bl.target - g().memory->getUnityBase();
}

uintptr_t Memory::findFieldGetOffset(ModuleInfo module) {
    auto pattern_result = module.findPattern("? ? ? 94 FA 03 00 AA F8 03 1D 2A");
    if (pattern_result.empty()) return UINTPTR_MAX;
    uintptr_t bl_address = g().memory->getUnityBase() + pattern_result.front();
    Assembler::bl bl = Assembler::bl(bl_address, *reinterpret_cast<uint32_t*>(bl_address));
    return bl.target - g().memory->getUnityBase();
}

uintptr_t Memory::findFieldGetType(ModuleInfo module) {
    auto pattern_result = module.findPattern("? ? ? 97 ? ? ? 97 ? ? ? B1", false);
    if (pattern_result.empty()) return UINTPTR_MAX;
    uintptr_t bl_address = g().memory->getUnityBase() + pattern_result.back();
    Assembler::bl bl = Assembler::bl(bl_address, *reinterpret_cast<uint32_t*>(bl_address));
    return bl.target - g().memory->getUnityBase();
}

uintptr_t Memory::findFieldGetDefaultValue(ModuleInfo module) {
    auto pattern_result = module.findPattern("? ? ? D1 ? ? ? F9 ? ? ? A9 ? ? ? A9 ? ? ? F9 F4 03 00 AA F3 03 01 AA ? ? ? F9");
    if (pattern_result.empty()) return UINTPTR_MAX;
    return pattern_result.front();
}

uintptr_t Memory::findTypeGetType(ModuleInfo module) {
    auto pattern_result = module.findPattern("? ? ? 94 ? ? ? 71 ? ? ? B9 ? ? ? 54 ? ? ? 52");
    if (pattern_result.empty()) return UINTPTR_MAX;
    uintptr_t bl_address = g().memory->getUnityBase() + pattern_result.front();
    Assembler::bl bl = Assembler::bl(bl_address, *reinterpret_cast<uint32_t*>(bl_address));
    return bl.target - g().memory->getUnityBase();
}

uintptr_t Memory::findTypeAttrs(ModuleInfo module) {
    auto pattern_result = module.findPattern("? ? ? D1 ? ? ? F9 ? ? ? A9 ? ? "
                                             "? A9 ? ? ? F9 F4 03 00 AA F3 03 01 "
                                             "AA E0 03 08 AA");
    if (pattern_result.empty()) return UINTPTR_MAX;
    uintptr_t first_function_address = g().memory->getUnityBase() + pattern_result.front();
    uintptr_t first_bl_address = first_function_address + 0xAC;
    Assembler::bl first_bl = Assembler::bl(first_bl_address, *reinterpret_cast<uint32_t*>(first_bl_address));

    uintptr_t second_function_address = first_bl.target;
    uintptr_t second_bl_address = second_function_address + 0x60;
    Assembler::bl second_bl = Assembler::bl(second_bl_address, *reinterpret_cast<uint32_t*>(second_bl_address));

    uintptr_t third_function_address = second_bl.target;
    uintptr_t third_bl_address = third_function_address + 0xC4;
    Assembler::bl third_bl = Assembler::bl(third_bl_address, *reinterpret_cast<uint32_t*>(third_bl_address));

    uintptr_t function_address = third_bl.target;
    uintptr_t root_bl_address = function_address + 0x60;
    Assembler::bl bl = Assembler::bl(root_bl_address, *reinterpret_cast<uint32_t*>(root_bl_address));

    Assembler::b b = Assembler::b(bl.target, *reinterpret_cast<uint32_t*>(bl.target));
    Assembler::b root_b = Assembler::b(b.target, *reinterpret_cast<uint32_t*>(b.target));

    uintptr_t root_function_address = root_b.target;
    uintptr_t root_ldr_address = root_function_address + 0x4;

    Assembler::ldr ldr = Assembler::ldr(root_ldr_address, *reinterpret_cast<uint32_t*>(root_ldr_address));
    return ldr.offset;
}

uintptr_t Memory::findTypeAttrsShift(ModuleInfo module) {
    auto pattern_result = module.findPattern("? ? ? D1 ? ? ? F9 ? ? ? A9 ? ? "
                                             "? A9 ? ? ? F9 F4 03 00 AA F3 03 01 "
                                             "AA E0 03 08 AA");
    if (pattern_result.empty()) return UINTPTR_MAX;
    uintptr_t first_function_address = g().memory->getUnityBase() + pattern_result.front();
    uintptr_t first_bl_address = first_function_address + 0xAC;
    Assembler::bl first_bl = Assembler::bl(first_bl_address, *reinterpret_cast<uint32_t*>(first_bl_address));

    uintptr_t second_function_address = first_bl.target;
    uintptr_t second_bl_address = second_function_address + 0x60;
    Assembler::bl second_bl = Assembler::bl(second_bl_address, *reinterpret_cast<uint32_t*>(second_bl_address));

    uintptr_t third_function_address = second_bl.target;
    uintptr_t third_bl_address = third_function_address + 0xC4;
    Assembler::bl third_bl = Assembler::bl(third_bl_address, *reinterpret_cast<uint32_t*>(third_bl_address));

    uintptr_t function_address = third_bl.target;
    uintptr_t root_bl_address = function_address + 0x60;
    Assembler::bl bl = Assembler::bl(root_bl_address, *reinterpret_cast<uint32_t*>(root_bl_address));

    Assembler::b b = Assembler::b(bl.target, *reinterpret_cast<uint32_t*>(bl.target));
    Assembler::b root_b = Assembler::b(b.target, *reinterpret_cast<uint32_t*>(b.target));

    uintptr_t root_function_address = root_b.target;
    uintptr_t root_ubfx_address = root_function_address + 0x8;

    Assembler::ubfx ubfx = Assembler::ubfx(root_ubfx_address, *reinterpret_cast<uint32_t*>(root_ubfx_address));
    return ubfx.isValid() ? ubfx.lsb : 0;
}


uintptr_t Memory::findTypeGetClassFromType(ModuleInfo module) {
    auto pattern_result = module.findPattern("? ? ? A9 ? ? ? A9 ? ? ? 39 F3 03 00 AA F4 03 01 2A E0 03 15 2A");
    if (pattern_result.empty()) return UINTPTR_MAX;
    return pattern_result.front();
}

uintptr_t Memory::findTypeGetData(ModuleInfo module) {
    uintptr_t class_from_type = findTypeGetClassFromType(module);
    if (class_from_type == UINTPTR_MAX) return UINTPTR_MAX;
    uintptr_t function_address = g().memory->getUnityBase() + class_from_type;
    uintptr_t bl_address = function_address + 0xC8;
    Assembler::bl bl = Assembler::bl(bl_address, *reinterpret_cast<uint32_t*>(bl_address));
    return bl.target - g().memory->getUnityBase();
}

uintptr_t Memory::findTypeGetName(ModuleInfo module) {
    auto pattern_result = module.findPattern("? ? ? 94 ? ? ? 39 ? ? ? A9 ? ? ? D3 ? ? ? "
                                             "72 E1 02 89 9A");
    if (pattern_result.empty()) return UINTPTR_MAX;
    uintptr_t bl_address = g().memory->getUnityBase() + pattern_result.front();
    Assembler::bl bl = Assembler::bl(bl_address, *reinterpret_cast<uint32_t*>(bl_address));
    return bl.target - g().memory->getUnityBase();
}

uintptr_t Memory::findPropertyName(ModuleInfo module) {
    Il2Cpp::Class *object = g().il2cpp->getClassFromName("UnityEngine.CoreModule.dll", "Object");
    if (g().memory->isPtrValid(reinterpret_cast<uintptr_t>(object))) {
        Il2Cpp::Property *property = g().il2cpp->getProperties(object).at(0);
        if (g().memory->isPtrValid(reinterpret_cast<uintptr_t>(property))) {
            for (uintptr_t offset = 0; offset < 64; offset += 8) {
                const char *name = *reinterpret_cast<const char **>(reinterpret_cast<uintptr_t>(property) + offset);
                if (g().memory->isPtrValid(reinterpret_cast<uintptr_t>(name))) {
                    if (!strcmp(name, "name")) {
                        return offset;
                    }
                }
            }
        }
    }
    return UINTPTR_MAX;
}

uintptr_t Memory::findPropertyGetMethod(ModuleInfo module) {
    Il2Cpp::Class *object = g().il2cpp->getClassFromName("UnityEngine.CoreModule.dll", "Object");
    if (g().memory->isPtrValid(reinterpret_cast<uintptr_t>(object))) {
        Il2Cpp::Property *property = g().il2cpp->getProperties(object).at(0);
        if (g().memory->isPtrValid(reinterpret_cast<uintptr_t>(property))) {
            for (uintptr_t offset = 0; offset < 128; offset += 8) {
                Il2Cpp::Method *method = *reinterpret_cast<Il2Cpp::Method **>(reinterpret_cast<uintptr_t>(property) + offset);
                if (g().memory->isPtrValid(reinterpret_cast<uintptr_t>(method))) {
                    for (uintptr_t method_offset = 0; method_offset < 128; method_offset += 8) {
                        const char *name = *reinterpret_cast<const char **>(reinterpret_cast<uintptr_t>(method) + method_offset);
                        if (g().memory->isPtrValid(reinterpret_cast<uintptr_t>(name))) {
                            if (!strcmp(name, "get_name")) {
                                return offset;
                            }
                        }
                    }
                }
            }
        }
    }
    return UINTPTR_MAX;
}

uintptr_t Memory::findPropertySetMethod(ModuleInfo module) {
    Il2Cpp::Class *object = g().il2cpp->getClassFromName("UnityEngine.CoreModule.dll", "Object");
    if (g().memory->isPtrValid(reinterpret_cast<uintptr_t>(object))) {
        Il2Cpp::Property *property = g().il2cpp->getProperties(object).at(0);
        if (g().memory->isPtrValid(reinterpret_cast<uintptr_t>(property))) {
            for (uintptr_t offset = 0; offset < 128; offset += 8) {
                if (offset == offsets.property_get_method) {
                    continue;
                }
                Il2Cpp::Method *method = *reinterpret_cast<Il2Cpp::Method **>(reinterpret_cast<uintptr_t>(property) + offset);
                if (g().memory->isPtrValid(reinterpret_cast<uintptr_t>(method))) {
                    for (uintptr_t method_offset = 0; method_offset < 128; method_offset += 8) {
                        const char *name = *reinterpret_cast<const char **>(reinterpret_cast<uintptr_t>(method) + method_offset);
                        if (g().memory->isPtrValid(reinterpret_cast<uintptr_t>(name))) {
                            if (!strcmp(name, "set_name")) {
                                return offset;
                            }
                        }
                    }
                }
            }
        }
    }
    return UINTPTR_MAX;
}

uintptr_t Memory::findMethodName(ModuleInfo module) {
    Il2Cpp::Class *object = g().il2cpp->getClassFromName("UnityEngine.CoreModule.dll", "Object");
    if (g().memory->isPtrValid(reinterpret_cast<uintptr_t>(object))) {
        Il2Cpp::Property *property = g().il2cpp->getProperties(object).at(0);
        if (g().memory->isPtrValid(reinterpret_cast<uintptr_t>(property))) {
            for (uintptr_t offset = 0; offset < 128; offset += 8) {
                Il2Cpp::Method *method = *reinterpret_cast<Il2Cpp::Method **>(reinterpret_cast<uintptr_t>(property) + offset);
                if (g().memory->isPtrValid(reinterpret_cast<uintptr_t>(method))) {
                    for (uintptr_t method_offset = 0; method_offset < 128; method_offset += 8) {
                        const char *name = *reinterpret_cast<const char **>(reinterpret_cast<uintptr_t>(method) + method_offset);
                        if (g().memory->isPtrValid(reinterpret_cast<uintptr_t>(name))) {
                            if (!strcmp(name, "set_name")) {
                                return method_offset;
                            }
                        }
                    }
                }
            }
        }
    }
    return UINTPTR_MAX;
}

uintptr_t Memory::findMethodReturnType(ModuleInfo module) {
    Il2Cpp::Class *klass = g().il2cpp->getClassFromName("mscorlib.dll", "RuntimeType");
    if (g().memory->isPtrValid(reinterpret_cast<uintptr_t>(klass))) {
        for (Il2Cpp::Method *method : g().il2cpp->getMethods(klass)) {
            if (!strcmp(g().il2cpp->getMethodName(method), "GetEvents_native")) {
                for (uintptr_t offset = 0; offset < 128; offset += 8) {
                    Il2Cpp::Type *t = *reinterpret_cast<Il2Cpp::Type **>(reinterpret_cast<uintptr_t>(method) + offset);
                    if (g().memory->isPtrValid(reinterpret_cast<uintptr_t>(t))) {
                        if (g().il2cpp->getTypeIndex(t) == 0x18) {
                            return offset;
                        }
                    }
                }
            }
        }
    }
    return UINTPTR_MAX;
}

uintptr_t Memory::findMethodGetFlags(ModuleInfo module) {
    Il2Cpp::Class *klass = g().il2cpp->getClassFromName("mscorlib.dll", "MonoMethodInfo");
    if (g().memory->isPtrValid(reinterpret_cast<uintptr_t>(klass))) {
        for (Il2Cpp::Method *method : g().il2cpp->getMethods(klass)) {
            if (!strcmp(g().il2cpp->getMethodName(method), "get_method_attributes")) {
                for (uintptr_t offset1 = 0; offset1 < 128; offset1 += 8) {
                    uintptr_t va = *reinterpret_cast<uintptr_t *>(reinterpret_cast<uintptr_t>(method) + offset1);
                    if (g().memory->isPtrValid(va)) {
                        for (uintptr_t offset2 = 0; offset2 < 128; offset2 += 8) {
                            if (offset1 == offset2) {
                                continue;
                            }

                            uintptr_t rva = *reinterpret_cast<uintptr_t *>(reinterpret_cast<uintptr_t>(method) + offset2);
                            if (va == rva) {
                                return va - g().memory->getUnityBase();
                            }
                        }
                    }
                }
            }
        }
    }
    return UINTPTR_MAX;
}

uintptr_t Memory::findMethodParametersCount(ModuleInfo module) {
    Il2Cpp::Class *klass = g().il2cpp->getClassFromName("UnityEngine.CoreModule.dll", "Object");
    if (g().memory->isPtrValid(reinterpret_cast<uintptr_t>(klass))) {
        for (Il2Cpp::Method *method : g().il2cpp->getMethods(klass)) {
            if (!strcmp(g().il2cpp->getMethodName(method), "Internal_InstantiateSingleWithParent_Injected")) {
                for (uintptr_t offset = 0; offset < 128; offset += 1) {
                    int8_t parameters_count = *reinterpret_cast<int8_t *>(reinterpret_cast<uintptr_t>(method) + offset);
                    if (parameters_count == 4) {
                        return offset;
                    }
                }
            }
        }
    }
    return UINTPTR_MAX;
}

uintptr_t Memory::findMethodGetParameter(ModuleInfo module) {
    auto pattern_result = module.findPattern("? ? ? 39 1F 01 01 6B ? ? ? 54 ? ? ? F9 ? ? ? F8");
    if (pattern_result.empty()) return UINTPTR_MAX;
    return pattern_result.front();
}

uintptr_t Memory::findMethodPointer(ModuleInfo module) {
    Il2Cpp::Class *klass = g().il2cpp->getClassFromName("mscorlib.dll", "MonoMethodInfo");
    if (g().memory->isPtrValid(reinterpret_cast<uintptr_t>(klass))) {
        Il2Cpp::Method *method = g().il2cpp->getMethods(klass).at(0);
        for (uintptr_t offset1 = 0; offset1 < 128; offset1 += 8) {
            uintptr_t va = *reinterpret_cast<uintptr_t *>(reinterpret_cast<uintptr_t>(method) + offset1);
            if (g().memory->isPtrValid(va)) {
                for (uintptr_t offset2 = 0; offset2 < 128; offset2 += 8) {
                    if (offset1 == offset2) {
                        continue;
                    }

                    uintptr_t rva = *reinterpret_cast<uintptr_t *>(reinterpret_cast<uintptr_t>(method) + offset2);
                    if (va == rva) {
                        return offset2;
                    }
                }
            }
        }
    }
    return UINTPTR_MAX;
}

uintptr_t Memory::findMethodVirtualPointer(ModuleInfo module) {
    Il2Cpp::Class *klass = g().il2cpp->getClassFromName("mscorlib.dll", "MonoMethodInfo");
    if (g().memory->isPtrValid(reinterpret_cast<uintptr_t>(klass))) {
        Il2Cpp::Method *method = g().il2cpp->getMethods(klass).at(0);
        for (uintptr_t offset1 = 0; offset1 < 128; offset1 += 8) {
            uintptr_t va = *reinterpret_cast<uintptr_t *>(reinterpret_cast<uintptr_t>(method) + offset1);
            if (g().memory->isPtrValid(va)) {
                for (uintptr_t offset2 = 0; offset2 < 128; offset2 += 8) {
                    if (offset1 == offset2) {
                        continue;
                    }

                    uintptr_t rva = *reinterpret_cast<uintptr_t *>(reinterpret_cast<uintptr_t>(method) + offset2);
                    if (va == rva) {
                        return offset1;
                    }
                }
            }
        }
    }
    return UINTPTR_MAX;
}

uintptr_t Memory::findMethodSlot(ModuleInfo module) {
    Il2Cpp::Class *klass = g().il2cpp->getClassFromName("UnityEngine.CoreModule.dll", "Object");
    if (g().memory->isPtrValid(reinterpret_cast<uintptr_t>(klass))) {
        for (Il2Cpp::Method *method : g().il2cpp->getMethods(klass)) {
            if (!strcmp(g().il2cpp->getMethodName(method), "ToString")) {
                for (uintptr_t offset = 0; offset < 128; offset += 2) {
                    uint16_t slot = *reinterpret_cast<int8_t *>(reinterpret_cast<uintptr_t>(method) + offset);
                    if (slot == 3) {
                        return offset;
                    }
                }
            }
        }
    }
    return UINTPTR_MAX;
}

uintptr_t Memory::findMethodGetIsGeneric(ModuleInfo module) {
    auto pattern_result = module.findPattern("? ? ? F8 ? ? ? A9 ? ? ? A9 ? ? ? A9 ? ? "
                                             "? A9 ? ? ? A9 F5 03 01 AA ? ? ? 91");
    if (pattern_result.empty()) return UINTPTR_MAX;
    uintptr_t function_address = g().memory->getUnityBase() + pattern_result.front();
    uintptr_t bl_address = function_address + 0xB8;
    Assembler::bl bl = Assembler::bl(bl_address, *reinterpret_cast<uint32_t*>(bl_address));
    return bl.target - g().memory->getUnityBase();
}

uintptr_t Memory::findMethodGetParameterName(ModuleInfo module) {
    auto pattern_result = module.findPattern("? ? ? 94 ? ? ? F9 ? ? ? F8 ? ? ? 91 ? ? ? "
                                             "F9 ? ? ? 39");
    if (pattern_result.empty()) return UINTPTR_MAX;
    uintptr_t bl_address = g().memory->getUnityBase() + pattern_result.front();
    Assembler::bl bl = Assembler::bl(bl_address, *reinterpret_cast<uint32_t*>(bl_address));
    return bl.target - g().memory->getUnityBase();
}