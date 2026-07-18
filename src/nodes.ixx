module;

#include <memory>
#include <string>
#include <vector>

export module nodes;

import opcodes;

export namespace nodes {
    namespace traits {
        template <typename Method, typename Field, typename RecordComponent>
        struct ClassNT {
            using MethodNType = Method;
            using FieldNType = Field;
            using RecordComponentNType = RecordComponent;
        };
    }

    namespace full {
        struct MethodN {
            std::vector<std::string> exceptions;
            std::string name, desc, signature, superName, sourceFile;
            opcodes::AccType access;
        };

        struct ClassN : traits::ClassNT<MethodN, void, void> {
            std::vector<std::string> interfaces;
            std::string name, signature, superName, sourceFile;
            opcodes::AccType access;
            std::vector<MethodNType> fields;
        };
    }
}
