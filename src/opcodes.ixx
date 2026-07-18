module;

#include <cstdint>
#include "macro.h"

export module opcodes;

export namespace opcodes {
    namespace Acc {
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
    }

    enum class NewArrType : std::int8_t {
        T_BOOLEAN = 4,
        T_CHAR = 5,
        T_FLOAT = 6,
        T_DOUBLE = 7,
        T_BYTE = 8,
        T_SHORT = 9,
        T_INT = 10,
        T_LONG = 11
    };

    enum class LambdaHType : std::int8_t {
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

    enum class Opc : std::uint8_t {
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

    namespace Ver {
        using Type = std::uint32_t;
        constexpr Type V_PREVIEW = 0xFFFF0000;

        enum class v : Type {
            V1_1 = 3 << 16 | 45,
            V1_2 = 0 << 16 | 46,
            V1_3 = 0 << 16 | 47,
            V1_4 = 0 << 16 | 48,
            V1_5 = 0 << 16 | 49,
            V1_6 = 0 << 16 | 50,
            V1_7 = 0 << 16 | 51,
            V1_8 = 0 << 16 | 52,
            V9 = 0 << 16 | 53,
            V10 = 0 << 16 | 54,
            V11 = 0 << 16 | 55,
            V12 = 0 << 16 | 56,
            V13 = 0 << 16 | 57,
            V14 = 0 << 16 | 58,
            V15 = 0 << 16 | 59,
            V16 = 0 << 16 | 60,
            V17 = 0 << 16 | 61,
            V18 = 0 << 16 | 62,
            V19 = 0 << 16 | 63,
            V20 = 0 << 16 | 64,
            V21 = 0 << 16 | 65,
            V22 = 0 << 16 | 66,
            V23 = 0 << 16 | 67,
            V24 = 0 << 16 | 68,
            V25 = 0 << 16 | 69,
            V26 = 0 << 16 | 70,
        };
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

    using VerType = Ver::Type;
    using AccType = Acc::Type;
}
