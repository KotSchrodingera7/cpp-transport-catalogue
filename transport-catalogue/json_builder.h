
#pragma once

#include "json.h"

namespace json {
    class Builder {
        class BaseContext;
        class DictValueContext;
        class DictItemContext;
        class ArrayItemContext;

        public:
            enum class State : int8_t {
                WaitKey = 0,
                WaitValue,
                WaitAll
            };

            Builder() : root_node_(), nodes_stack_{&root_node_} {}

            DictValueContext Key(std::string key);
            BaseContext Value(Node::Value value);
            DictItemContext StartDict();
            ArrayItemContext StartArray();

            Builder &EndDict();
            Builder &EndArray();

            Node Build();
        private:
            void CheckState(void);

        private:
            Node root_node_;
            std::vector<Node*> nodes_stack_; 
            std::string last_key_;

            State cur_state_{State::WaitAll};

        private:

            class BaseContext {
                public:
                    BaseContext(Builder& builder) : builder_(builder) {}
                    DictValueContext Key(std::string key) {
                        return builder_.Key(std::move(key));
                    }
                    DictItemContext StartDict() {
                        return builder_.StartDict();
                    }

                    BaseContext Value(Node::Value value) {
                        return builder_.Value(std::move(value));
                    }
                    ArrayItemContext StartArray() {
                        return builder_.StartArray();
                    }
                    Builder &EndArray() {
                        return builder_.EndArray();
                    }
                    Builder &EndDict() {
                        return builder_.EndDict();
                    }
                    Node Build() {
                        return builder_.Build();
                    }
                private:
                    Builder& builder_;
            };

            class DictValueContext : public BaseContext {
                public:
                    DictValueContext(BaseContext base)
                        : BaseContext(base) {}
                    DictValueContext Key(std::string key) = delete;
                    Builder &EndArray() = delete;
                    Builder &EndDict() = delete;
                    Node Build() = delete;
                    DictItemContext Value(Node::Value value) {
                        return BaseContext::Value(value);
                    }
                    

            };

            class DictItemContext : public BaseContext {
                public:
                    DictItemContext(BaseContext base)
                        : BaseContext(base) {}
                    DictItemContext StartDict() = delete;
                    ArrayItemContext StartArray() = delete;
                    Builder &EndArray() = delete;
                    Node Build() = delete;
            };

            class ArrayItemContext : public BaseContext {
                public:
                    ArrayItemContext(BaseContext base)
                        : BaseContext(base) {}

                ArrayItemContext Value(Node::Value value) {
                    return BaseContext::Value(value);
                }
                Builder &EndDict() = delete;
                Node Build() = delete;
                DictValueContext Key(std::string key) = delete;
            };
    };


    class DictItemContext : public Builder {
        public:
            DictItemContext(Builder &builder) : builder_(builder) {} 
            
        private:
            Builder &builder_;
    };
}