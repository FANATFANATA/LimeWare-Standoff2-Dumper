#ifndef LIMEWARE_STANDOFF2_DUMPER_ASSEMBLER_H
#define LIMEWARE_STANDOFF2_DUMPER_ASSEMBLER_H

#include <cstdint>
#include <sys/types.h>

namespace Assembler {
    struct adrp {
        uintptr_t address;
        uintptr_t opcode;

        uintptr_t page_base;

        uint16_t rd;
        uint32_t immlo;
        uint32_t immhi;
        uint64_t imm;

        uint64_t page_address;

        explicit adrp(uintptr_t a, uintptr_t o) {
            address = a;
            opcode = o;

            page_base = address & 0xFFFFFFFFFFFFF000;

            rd = opcode & 0x1F;
            immlo = opcode >> 29 & 0x3;
            immhi = opcode >> 5 & 0x7FFFF;
            imm = (immhi << 2 | immlo) << 12;

            page_address = page_base + imm;
        }
    };

    struct ldr {
        uintptr_t address;
        uintptr_t opcode;

        uint16_t rt;
        uint16_t rn;
        uint32_t imm12;
        uint32_t offset;

        explicit ldr(uintptr_t a, uintptr_t o) {
            address = a;
            opcode = o;

            rt = opcode & 0x1F;
            rn = opcode >> 5 & 0x1F;
            imm12 = opcode >> 10 & 0xFFF;
            offset = imm12 << (opcode >> 30 & 0x3);
        }
    };

    struct add {
        uintptr_t address;
        uintptr_t opcode;

        uint16_t rd;
        uint16_t rn;
        uint32_t imm12;
        uint32_t offset;

        explicit add(uintptr_t a, uintptr_t o) {
            address = a;
            opcode = o;

            rd = opcode & 0x1F;
            rn = opcode >> 5 & 0x1F;
            imm12 = opcode >> 10 & 0xFFF;
            offset = imm12;
        }
    };

    struct bl {
        uintptr_t address;
        uintptr_t opcode;

        int64_t imm26;
        int64_t offset;
        uintptr_t target;

        explicit bl(uintptr_t a, uintptr_t o) {
            address = a;
            opcode = o;

            imm26 = opcode & 0x03FFFFFF;
            if (imm26 & 1 << 25) {
                imm26 -= 1 << 26;
            }
            offset = imm26 << 2;

            target = address + offset;
        }
    };

    struct str {
        uintptr_t address;
        uintptr_t opcode;

        uint16_t rt;
        uint16_t rn;
        uint32_t imm12;
        uint32_t offset;

        explicit str(uintptr_t a, uintptr_t o) {
            address = a;
            opcode = o;

            rt = opcode & 0x1F;
            rn = (opcode >> 5) & 0x1F;
            imm12 = (opcode >> 10) & 0xFFF;
            offset = imm12 << (opcode >> 30 & 0x3);
        }
    };

    struct b {
        uintptr_t address;
        uintptr_t opcode;

        int64_t imm26;
        int64_t offset;
        uintptr_t target;

        explicit b(uintptr_t a, uintptr_t o) {
            address = a;
            opcode = o;

            imm26 = opcode & 0x03FFFFFF;
            if (imm26 & 1 << 25) {
                imm26 -= 1 << 26;
            }
            offset = imm26 << 2;

            target = address + offset;
        }
    };

    struct ubfx {
        uintptr_t address;
        uintptr_t opcode;

        bool is_64_bit;
        uint8_t rd;
        uint8_t rn;

        uint8_t immr;
        uint8_t imms;

        uint8_t lsb;
        uint8_t width;

        explicit ubfx(uintptr_t a, uintptr_t o) {
            address = a;
            opcode = o;

            is_64_bit = (opcode >> 31) & 1;

            rd = opcode & 0x1F;
            rn = (opcode >> 5) & 0x1F;

            imms = (opcode >> 10) & 0x3F;
            immr = (opcode >> 16) & 0x3F;

            lsb = immr;
            width = imms - immr + 1;
        }

        [[nodiscard]] bool isValid() const {
            uint32_t masked = opcode & 0xFFC00000;
            bool isUbfm = (masked == 0xD3400000) || (masked == 0x53000000);

            if (!isUbfm) return false;
            uint8_t check_imms = (opcode >> 10) & 0x3F;
            uint8_t check_immr = (opcode >> 16) & 0x3F;

            return check_imms >= check_immr;
        }
    };
};


#endif