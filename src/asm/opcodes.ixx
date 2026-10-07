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

namespace opcodes {
    export constexpr uint32_t magic = 0xCAFEBABE;
    export constexpr uint32_t magic_rev = std::byteswap(magic);
    export constexpr auto magic_array_rev = cstd::convert_to_array<uint8_t>(magic_rev);

    export namespace java_ts {
        using Int = uint32_t;
        using Long = uint64_t;
        using Float = Int;
        using Double = Long;

        using IntDT = Void;
        using LongDT = Void;
        using FloatDT = float;
        using DoubleDT = double;

        DU_UNION_VIA_AL_DT(Int);
        DU_UNION_VIA_AL_DT(Float);
        DU_UNION_VIA_AL_DT(Double);
        DU_UNION_VIA_AL_DT(Long);
    }

    export namespace Acc {
        namespace Class {
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
        } // namespace Class

        using Type = uint32_t;

        constexpr Type PUBLIC = 0x0001; // class, field, method
        constexpr Type PRIVATE = 0x0002; // class, field, method
        constexpr Type PROTECTED = 0x0004; // class, field, method
        constexpr Type STATIC = 0x0008; // field, method
        constexpr Type FINAL = 0x0010; // class, field, method, parameter
        constexpr Type SUPER = 0x0020; // class
        constexpr Type SYNCHRONIZED = 0x0020; // method
        constexpr Type OPEN = 0x0020; // module
        constexpr Type TRANSITIVE = 0x0020; // module requires
        constexpr Type VOLATILE = 0x0040; // field
        constexpr Type BRIDGE = 0x0040; // method
        constexpr Type STATIC_PHASE = 0x0040; // module requires
        constexpr Type VARARGS = 0x0080; // method
        constexpr Type TRANSIENT = 0x0080; // field
        constexpr Type NATIVE = 0x0100; // method
        constexpr Type INTERFACE = 0x0200; // class
        constexpr Type ABSTRACT = 0x0400; // class, method
        constexpr Type STRICT = 0x0800; // method
        constexpr Type SYNTHETIC = 0x1000; // class, field, method, parameter, module *
        constexpr Type ANNOTATION = 0x2000; // class
        constexpr Type ENUM = 0x4000; // class(?) field inner
        constexpr Type MANDATED = 0x8000; // field, method, parameter, module, module *
        constexpr Type MODULE = 0x8000; // class
        constexpr Type RECORD = 0x10000; // class
        constexpr Type DEPRECATED = 0x20000; // class, field, method
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
        ILOAD = 21, // visitVarInsn
        LLOAD = 22, // -
        FLOAD = 23, // -
        DLOAD = 24, // -
        ALOAD = 25, // -
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
        MULTIANEWARRAY = 197, // visitMultiANewArrayInsn
        IFNULL = 198, // visitJumpInsn
        IFNONNULL = 199 // -
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

    export using TagIdType = uint8_t;

    export enum class TagIdEnum : TagIdType {
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

    export TagIdType as_number(TagIdEnum instance) {
        return static_cast<TagIdType>(instance);
    }

    export TagIdEnum as_enum(TagIdType number) {
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
    /*export {
        using VerType = Ver::Type;
        using AccType = Acc::Type;
    }*/
} // namespace opcodes
