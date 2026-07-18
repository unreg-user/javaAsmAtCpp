#pragma once

#define DEREF_WITH_TYPE(type, that) derefVisit([](type& deref) {return deref.that;})
#define DEREF(that) DEREF_WITH_TYPE(auto, that)
#define DEREF_WITH_TYPE_CONST(type, that) derefVisit([](const type& deref) {return deref.that;})
#define DEREF_CONST(that) DEREF_WITH_TYPE_CONST(auto, that)

// custom

#define DEREF_ABST_INSN(that) DEREF_WITH_TYPE(AbstractInsnNode, that)
#define DEREF_ABST_INSN_CONST(that) DEREF_WITH_TYPE_CONST(AbstractInsnNode, that)