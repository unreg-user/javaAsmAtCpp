module;

#include <cstdint>
#include <variant>
#include <array>
#include <bit>

#include "opcodes_mcr.h"
#include "../cstd/macro.h"

export module opcodes;

import simple_cstd;

// https://docs.oracle.com/javase/specs//jvms/se26/html/jvms-4.html

/**
*  ASM: a very small and fast Java bytecode manipulation framework
 Copyright (c) 2000-2011 INRIA, France Telecom
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions
 are met:
 1. Redistributions of source code must retain the above copyright
 notice, this list of conditions and the following disclaimer.
 2. Redistributions in binary form must reproduce the above copyright
 notice, this list of conditions and the following disclaimer in the
 documentation and/or other materials provided with the distribution.
 3. Neither the name of the copyright holders nor the names of its
 contributors may be used to endorse or promote products derived from
 this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 THE POSSIBILITY OF SUCH DAMAGE.
 */
namespace opcodes {
    export constexpr uint32_t magic = 0xCAFEBABE;
    export constexpr uint32_t magic_rev = std::byteswap(magic);
    export constexpr auto magic_array_rev = cstd::convert_to_array<uint8_t>(magic_rev);

    export namespace Acc {
        using Type = uint16_t;

        constexpr Type PUBLIC = 0x0001;
        constexpr Type FINAL = 0x0010;
        constexpr Type SUPER = 0x0020;
        constexpr Type INTERFACE = 0x0200;
        constexpr Type ABSTRACT = 0x0400;
        constexpr Type SYNTHETIC = 0x1000;
        constexpr Type ANNOTATION = 0x2000;
        constexpr Type ENUM = 0x4000;
        constexpr Type MODULE = 0x8000;

        constexpr Type PRIVATE = 0x0002; // class, field, method
        constexpr Type PROTECTED = 0x0004; // class, field, method
        constexpr Type STATIC = 0x0008; // field, method
        constexpr Type SYNCHRONIZED = 0x0020; // method
        constexpr Type OPEN = 0x0020; // module
        constexpr Type TRANSITIVE = 0x0020; // module requires
        constexpr Type VOLATILE = 0x0040; // field
        constexpr Type BRIDGE = 0x0040; // method
        constexpr Type STATIC_PHASE = 0x0040; // module requires
        constexpr Type VARARGS = 0x0080; // method
        constexpr Type TRANSIENT = 0x0080; // field
        constexpr Type NATIVE = 0x0100; // method
        constexpr Type STRICT = 0x0800; // method
        constexpr Type MANDATED = 0x8000; // field, method, parameter, module, module *
        //constexpr Type RECORD = 0x10000; // class
        //constexpr Type DEPRECATED = 0x20000; // class, field, method
    } // namespace Acc

    export enum class NewArrType : std::int8_t {
        T_BOOLEAN = 4,
        T_CHAR = 5,
        T_FLOAT = 6,
        T_DOUBLE = 7,
        T_BYTE = 8,
        T_SHORT = 9,
        T_INT = 10,
        T_LONG = 11
    };

    export enum class LambdaHType : std::int8_t {
        H_GETFIELD = 1,
        H_GETSTATIC = 2,
        H_PUTFIELD = 3,
        H_PUTSTATIC = 4,
        H_INVOKEVIRTUAL = 5,
        H_INVOKESTATIC = 6,
        H_INVOKESPECIAL = 7,
        H_NEWINVOKESPECIAL = 8,
        H_INVOKEINTERFACE = 9
    };

#if 0
    export enum class Opc : std::uint8_t {
        NOP = 0, // visitInsn
        ACONST_NULL = 1, // -
        ICONST_M1 = 2, // -
        ICONST_0 = 3, // -
        ICONST_1 = 4, // -
        ICONST_2 = 5, // -
        ICONST_3 = 6, // -
        ICONST_4 = 7, // -
        ICONST_5 = 8, // -
        LCONST_0 = 9, // -
        LCONST_1 = 10, // -
        FCONST_0 = 11, // -
        FCONST_1 = 12, // -
        FCONST_2 = 13, // -
        DCONST_0 = 14, // -
        DCONST_1 = 15, // -
        BIPUSH = 16, // visitIntInsn
        SIPUSH = 17, // -
        LDC = 18, // visitLdcInsn
        LDC_W = 19,  // visitLdcInsn (2-byte index)
        LDC2_W = 20, // visitLdcInsn (2-byte index, long/double)
        ILOAD = 21, // visitVarInsn
        LLOAD = 22, // -
        FLOAD = 23, // -
        DLOAD = 24, // -ALOAD = 25, // visitVarInsn
        ILOAD_0 = 26, // visitVarInsn (ILOAD, 0)
        ILOAD_1 = 27, // visitVarInsn (ILOAD, 1)
        ILOAD_2 = 28, // visitVarInsn (ILOAD, 2)
        ILOAD_3 = 29, // visitVarInsn (ILOAD, 3)
        LLOAD_0 = 30, // visitVarInsn (LLOAD, 0)
        LLOAD_1 = 31, // visitVarInsn (LLOAD, 1)
        LLOAD_2 = 32, // visitVarInsn (LLOAD, 2)
        LLOAD_3 = 33, // visitVarInsn (LLOAD, 3)
        FLOAD_0 = 34, // visitVarInsn (FLOAD, 0)
        FLOAD_1 = 35, // visitVarInsn (FLOAD, 1)
        FLOAD_2 = 36, // visitVarInsn (FLOAD, 2)
        FLOAD_3 = 37, // visitVarInsn (FLOAD, 3)
        DLOAD_0 = 38, // visitVarInsn (DLOAD, 0)
        DLOAD_1 = 39, // visitVarInsn (DLOAD, 1)
        DLOAD_2 = 40, // visitVarInsn (DLOAD, 2)
        DLOAD_3 = 41, // visitVarInsn (DLOAD, 3)
        ALOAD_0 = 42, // visitVarInsn (ALOAD, 0)
        ALOAD_1 = 43, // visitVarInsn (ALOAD, 1)
        ALOAD_2 = 44, // visitVarInsn (ALOAD, 2)
        ALOAD_3 = 45, // visitVarInsn (ALOAD, 3)
        IALOAD = 46, // visitInsn
        LALOAD = 47, // -
        FALOAD = 48, // -
        DALOAD = 49, // -
        AALOAD = 50, // -
        BALOAD = 51, // -
        CALOAD = 52, // -
        SALOAD = 53, // -
        ISTORE = 54, // visitVarInsn
        LSTORE = 55, // -
        FSTORE = 56, // -
        DSTORE = 57, // -
        ASTORE = 58, // -
        ISTORE_0 = 59, // visitVarInsn (ISTORE, 0)
        ISTORE_1 = 60, // visitVarInsn (ISTORE, 1)
        ISTORE_2 = 61, // visitVarInsn (ISTORE, 2)
        ISTORE_3 = 62, // visitVarInsn (ISTORE, 3)
        LSTORE_0 = 63, // visitVarInsn (LSTORE, 0)
        LSTORE_1 = 64, // visitVarInsn (LSTORE, 1)
        LSTORE_2 = 65, // visitVarInsn (LSTORE, 2)
        LSTORE_3 = 66, // visitVarInsn (LSTORE, 3)
        FSTORE_0 = 67, // visitVarInsn (FSTORE, 0)
        FSTORE_1 = 68, // visitVarInsn (FSTORE, 1)
        FSTORE_2 = 69, // visitVarInsn (FSTORE, 2)
        FSTORE_3 = 70, // visitVarInsn (FSTORE, 3)
        DSTORE_0 = 71, // visitVarInsn (DSTORE, 0)
        DSTORE_1 = 72, // visitVarInsn (DSTORE, 1)
        DSTORE_2 = 73, // visitVarInsn (DSTORE, 2)
        DSTORE_3 = 74, // visitVarInsn (DSTORE, 3)
        ASTORE_0 = 75, // visitVarInsn (ASTORE, 0)
        ASTORE_1 = 76, // visitVarInsn (ASTORE, 1)
        ASTORE_2 = 77, // visitVarInsn (ASTORE, 2)
        ASTORE_3 = 78, // visitVarInsn (ASTORE, 3)
        IASTORE = 79, // visitInsn
        LASTORE = 80, // -
        FASTORE = 81, // -
        DASTORE = 82, // -
        AASTORE = 83, // -
        BASTORE = 84, // -
        CASTORE = 85, // -
        SASTORE = 86, // -
        POP = 87, // -
        POP2 = 88, // -
        DUP = 89, // -
        DUP_X1 = 90, // -
        DUP_X2 = 91, // -
        DUP2 = 92, // -
        DUP2_X1 = 93, // -
        DUP2_X2 = 94, // -
        SWAP = 95, // -
        IADD = 96, // -
        LADD = 97, // -
        FADD = 98, // -
        DADD = 99, // -
        ISUB = 100, // -
        LSUB = 101, // -
        FSUB = 102, // -
        DSUB = 103, // -
        IMUL = 104, // -
        LMUL = 105, // -
        FMUL = 106, // -
        DMUL = 107, // -
        IDIV = 108, // -
        LDIV = 109, // -
        FDIV = 110, // -
        DDIV = 111, // -
        IREM = 112, // -
        LREM = 113, // -
        FREM = 114, // -
        DREM = 115, // -
        INEG = 116, // -
        LNEG = 117, // -
        FNEG = 118, // -
        DNEG = 119, // -
        ISHL = 120, // -
        LSHL = 121, // -
        ISHR = 122, // -
        LSHR = 123, // -
        IUSHR = 124, // -
        LUSHR = 125, // -
        IAND = 126, // -
        LAND = 127, // -
        IOR = 128, // -
        LOR = 129, // -
        IXOR = 130, // -
        LXOR = 131, // -
        IINC = 132, // visitIincInsn
        I2L = 133, // visitInsn
        I2F = 134, // -
        I2D = 135, // -
        L2I = 136, // -
        L2F = 137, // -
        L2D = 138, // -
        F2I = 139, // -
        F2L = 140, // -
        F2D = 141, // -
        D2I = 142, // -
        D2L = 143, // -
        D2F = 144, // -
        I2B = 145, // -
        I2C = 146, // -
        I2S = 147, // -
        LCMP = 148, // -
        FCMPL = 149, // -
        FCMPG = 150, // -
        DCMPL = 151, // -
        DCMPG = 152, // -
        IFEQ = 153, // visitJumpInsn
        IFNE = 154, // -
        IFLT = 155, // -
        IFGE = 156, // -
        IFGT = 157, // -
        IFLE = 158, // -
        IF_ICMPEQ = 159, // -
        IF_ICMPNE = 160, // -
        IF_ICMPLT = 161, // -
        IF_ICMPGE = 162, // -
        IF_ICMPGT = 163, // -
        IF_ICMPLE = 164, // -
        IF_ACMPEQ = 165, // -
        IF_ACMPNE = 166, // -
        GOTO = 167, // -
        JSR = 168, // -
        RET = 169, // visitVarInsn
        TABLESWITCH = 170, // visitTableSwitchInsn
        LOOKUPSWITCH = 171, // visitLookupSwitch
        IRETURN = 172, // visitInsn
        LRETURN = 173, // -
        FRETURN = 174, // -
        DRETURN = 175, // -
        ARETURN = 176, // -
        RETURN = 177, // -
        GETSTATIC = 178, // visitFieldInsn
        PUTSTATIC = 179, // -
        GETFIELD = 180, // -
        PUTFIELD = 181, // -
        INVOKEVIRTUAL = 182, // visitMethodInsn
        INVOKESPECIAL = 183, // -
        INVOKESTATIC = 184, // -
        INVOKEINTERFACE = 185, // -
        INVOKEDYNAMIC = 186, // visitInvokeDynamicInsn
        NEW = 187, // visitTypeInsn
        NEWARRAY = 188, // visitIntInsn
        ANEWARRAY = 189, // visitTypeInsn
        ARRAYLENGTH = 190, // visitInsn
        ATHROW = 191, // -
        CHECKCAST = 192, // visitTypeInsn
        INSTANCEOF = 193, // -
        MONITORENTER = 194, // visitInsn
        MONITOREXIT = 195, // -
        WIDE = 196,
        MULTIANEWARRAY = 197, // visitMultiANewArrayInsn
        IFNULL = 198, // visitJumpInsn
        IFNONNULL = 199, // -
        GOTO_W = 200, // visitJumpInsn (4-byte branch offset)
        JSR_W = 201, // visitJumpInsn (4-byte branch offset, deprecated)
        BREAKPOINT = 202, // reserved for debuggers (0xca)
        // 203..253 — unassigned / invalid in class files
        IMPDEP1 = 254, // reserved for VM implementation (0xfe)
        IMPDEP2 = 255  // reserved for VM implementation (0xff)
    };
#endif

        export enum class Opc : std::uint8_t {
        NOP = 0,
        ACONST_NULL = 1,
        ICONST_M1 = 2,
        ICONST_0 = 3,
        ICONST_1 = 4,
        ICONST_2 = 5,
        ICONST_3 = 6,
        ICONST_4 = 7,
        ICONST_5 = 8,
        LCONST_0 = 9,
        LCONST_1 = 10,
        FCONST_0 = 11,
        FCONST_1 = 12,
        FCONST_2 = 13,
        DCONST_0 = 14,
        DCONST_1 = 15,
        BIPUSH = 16,
        SIPUSH = 17,
        LDC = 18,
        LDC_W = 19,
        LDC2_W = 20,
        ILOAD = 21,
        LLOAD = 22,
        FLOAD = 23,
        DLOAD = 24,
        ILOAD_0 = 26,
        ILOAD_1 = 27,
        ILOAD_2 = 28,
        ILOAD_3 = 29,
        LLOAD_0 = 30,
        LLOAD_1 = 31,
        LLOAD_2 = 32,
        LLOAD_3 = 33,
        FLOAD_0 = 34,
        FLOAD_1 = 35,
        FLOAD_2 = 36,
        FLOAD_3 = 37,
        DLOAD_0 = 38,
        DLOAD_1 = 39,
        DLOAD_2 = 40,
        DLOAD_3 = 41,
        ALOAD_0 = 42,
        ALOAD_1 = 43,
        ALOAD_2 = 44,
        ALOAD_3 = 45,
        IALOAD = 46,
        LALOAD = 47,
        FALOAD = 48,
        DALOAD = 49,
        AALOAD = 50,
        BALOAD = 51,
        CALOAD = 52,
        SALOAD = 53,
        ISTORE = 54,
        LSTORE = 55,
        FSTORE = 56,
        DSTORE = 57,
        ASTORE = 58,
        ISTORE_0 = 59,
        ISTORE_1 = 60,
        ISTORE_2 = 61,
        ISTORE_3 = 62,
        LSTORE_0 = 63,
        LSTORE_1 = 64,
        LSTORE_2 = 65,
        LSTORE_3 = 66,
        FSTORE_0 = 67,
        FSTORE_1 = 68,
        FSTORE_2 = 69,
        FSTORE_3 = 70,
        DSTORE_0 = 71,
        DSTORE_1 = 72,
        DSTORE_2 = 73,
        DSTORE_3 = 74,
        ASTORE_0 = 75,
        ASTORE_1 = 76,
        ASTORE_2 = 77,
        ASTORE_3 = 78,
        IASTORE = 79,
        LASTORE = 80,
        FASTORE = 81,
        DASTORE = 82,
        AASTORE = 83,
        BASTORE = 84,
        CASTORE = 85,
        SASTORE = 86,
        POP = 87,
        POP2 = 88,
        DUP = 89,
        DUP_X1 = 90,
        DUP_X2 = 91,
        DUP2 = 92,
        DUP2_X1 = 93,
        DUP2_X2 = 94,
        SWAP = 95,
        IADD = 96,
        LADD = 97,
        FADD = 98,
        DADD = 99,
        ISUB = 100,
        LSUB = 101,
        FSUB = 102,
        DSUB = 103,
        IMUL = 104,
        LMUL = 105,
        FMUL = 106,
        DMUL = 107,
        IDIV = 108,
        LDIV = 109,
        FDIV = 110,
        DDIV = 111,
        IREM = 112,
        LREM = 113,
        FREM = 114,
        DREM = 115,
        INEG = 116,
        LNEG = 117,
        FNEG = 118,
        DNEG = 119,
        ISHL = 120,
        LSHL = 121,
        ISHR = 122,
        LSHR = 123,
        IUSHR = 124,
        LUSHR = 125,
        IAND = 126,
        LAND = 127,
        IOR = 128,
        LOR = 129,
        IXOR = 130,
        LXOR = 131,
        IINC = 132,
        I2L = 133,
        I2F = 134,
        I2D = 135,
        L2I = 136,
        L2F = 137,
        L2D = 138,
        F2I = 139,
        F2L = 140,
        F2D = 141,
        D2I = 142,
        D2L = 143,
        D2F = 144,
        I2B = 145,
        I2C = 146,
        I2S = 147,
        LCMP = 148,
        FCMPL = 149,
        FCMPG = 150,
        DCMPL = 151,
        DCMPG = 152,
        IFEQ = 153,
        IFNE = 154,
        IFLT = 155,
        IFGE = 156,
        IFGT = 157,
        IFLE = 158,
        IF_ICMPEQ = 159,
        IF_ICMPNE = 160,
        IF_ICMPLT = 161,
        IF_ICMPGE = 162,
        IF_ICMPGT = 163,
        IF_ICMPLE = 164,
        IF_ACMPEQ = 165,
        IF_ACMPNE = 166,
        GOTO = 167,
        // JSR = 168,
        // RET = 169,
        TABLESWITCH = 170,
        LOOKUPSWITCH = 171,
        IRETURN = 172,
        LRETURN = 173,
        FRETURN = 174,
        DRETURN = 175,
        ARETURN = 176,
        RETURN = 177,
        GETSTATIC = 178,
        PUTSTATIC = 179,
        GETFIELD = 180,
        PUTFIELD = 181,
        INVOKEVIRTUAL = 182,
        INVOKESPECIAL = 183,
        INVOKESTATIC = 184,
        INVOKEINTERFACE = 185,
        INVOKEDYNAMIC = 186,
        NEW = 187,
        NEWARRAY = 188,
        ANEWARRAY = 189,
        ARRAYLENGTH = 190,
        ATHROW = 191,
        CHECKCAST = 192,
        INSTANCEOF = 193,
        MONITORENTER = 194,
        MONITOREXIT = 195,
        WIDE = 196,
        MULTIANEWARRAY = 197,
        IFNULL = 198,
        IFNONNULL = 199,
        GOTO_W = 200,
        // JSR_W = 201,
        BREAKPOINT = 202,
        // IMPDEP1 = 254,
        // IMPDEP2 = 255
    };

    export namespace Ver {
        using Type = std::uint32_t;
        constexpr Type V_PREVIEW = 0xFFFF0000;

        enum class v : Type {
            V1_1 = 3U << 16 | 45U,
            V1_2 = 0U << 16 | 46U,
            V1_3 = 0U << 16 | 47U,
            V1_4 = 0U << 16 | 48U,
            V1_5 = 0U << 16 | 49U,
            V1_6 = 0U << 16 | 50U,
            V1_7 = 0U << 16 | 51U,
            V1_8 = 0U << 16 | 52U,
            V9 = 0U << 16 | 53U,
            V10 = 0U << 16 | 54U,
            V11 = 0U << 16 | 55U,
            V12 = 0U << 16 | 56U,
            V13 = 0U << 16 | 57U,
            V14 = 0U << 16 | 58U,
            V15 = 0U << 16 | 59U,
            V16 = 0U << 16 | 60U,
            V17 = 0U << 16 | 61U,
            V18 = 0U << 16 | 62U,
            V19 = 0U << 16 | 63U,
            V20 = 0U << 16 | 64U,
            V21 = 0U << 16 | 65U,
            V22 = 0U << 16 | 66U,
            V23 = 0U << 16 | 67U,
            V24 = 0U << 16 | 68U,
            V25 = 0U << 16 | 69U,
            V26 = 0U << 16 | 70U,
        };
    } // namespace Ver

    export using TagIdEnumType = uint8_t;

    export enum class TagIdEnum : TagIdEnumType {
        CONSTANT_Integer = 3,
        CONSTANT_Float = 4,
        CONSTANT_Long = 5,
        CONSTANT_Double = 6,
        CONSTANT_Class = 7,
        CONSTANT_String = 8,
        CONSTANT_MethodHandle = 15,
        CONSTANT_MethodType = 16,
        CONSTANT_Dynamic = 17,
        CONSTANT_Utf8 = 1,
        CONSTANT_Fieldref = 9,
        CONSTANT_Methodref = 10,
        CONSTANT_InterfaceMethodref = 11,
        CONSTANT_NameAndType = 12,
        CONSTANT_InvokeDynamic = 18,
        CONSTANT_Module = 19,
        CONSTANT_Package = 20
    };

    export TagIdEnumType as_number(TagIdEnum instance) {
        return static_cast<TagIdEnumType>(instance);
    }

    export TagIdEnum as_enum(TagIdEnumType number) {
        return static_cast<TagIdEnum>(number);
    }

#if 0
    enum class StackFrameTypes {
        TOP = Frame.ITEM_TOP,
        INTEGER = Frame.ITEM_INTEGER,
        FLOAT = Frame.ITEM_FLOAT,
        DOUBLE = Frame.ITEM_DOUBLE,
        LONG = Frame.ITEM_LONG,
        NULL_ = Frame.ITEM_NULL,
        UNINITIALIZED_THIS = Frame.ITEM_UNINITIALIZED_THIS,
    };
#endif
} // namespace opcodes
