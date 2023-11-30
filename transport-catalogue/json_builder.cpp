#include "json_builder.h"

namespace json {

Builder::DictValueContext Builder::Key(std::string key) {
    if( cur_state_ != State::WaitKey ) {
        throw std::logic_error("Not dictionary");
    }
    
    last_key_ = std::move(key);
    cur_state_ = State::WaitValue;

    return BaseContext(*this);
}



Builder::BaseContext Builder::Value(Node::Value value) {
    if( cur_state_ == State::WaitKey ) {
        throw std::logic_error("Waiting key");
    }

    if( cur_state_ == State::WaitAll && std::holds_alternative<nullptr_t>(root_node_.GetValue()) ) {
        if(std::holds_alternative<int>(value)) {
            root_node_ = std::get<int>(value);
        } else if(std::holds_alternative<bool>(value)) {
            root_node_ = std::get<bool>(value);
        } else if(std::holds_alternative<double>(value)) {
            root_node_ = std::get<double>(value);
        } else if(std::holds_alternative<std::string>(value)) {
            root_node_ = std::get<std::string>(value);
        } else if(std::holds_alternative<Array>(value)) {
            root_node_ = std::get<Array>(value);
        } else if(std::holds_alternative<Dict>(value)) {
            root_node_ = std::get<Dict>(value);
    } } else if( cur_state_ == State::WaitAll ) {
        auto &node_ = nodes_stack_.back()->GetValue();
        if( std::holds_alternative<Array>(node_) ) {
            if(std::holds_alternative<int>(value)) {
                std::get<Array>(node_).push_back(std::get<int>(value));
            } else if(std::holds_alternative<bool>(value)) {
                std::get<Array>(node_).push_back(std::get<bool>(value));
            } else if(std::holds_alternative<double>(value))  {
                std::get<Array>(node_).push_back(std::get<double>(value));
            } else if(std::holds_alternative<std::string>(value)) {
                std::get<Array>(node_).push_back(std::get<std::string>(value));
            } else if(std::holds_alternative<Array>(value)) {
                std::get<Array>(node_).push_back(std::get<Array>(value));
            } else if(std::holds_alternative<Dict>(value)) {
                std::get<Array>(node_).push_back(std::get<Dict>(value));
            }

        } else {
            throw std::logic_error("Added value with state of wait all");
        }
    } else if( cur_state_ == State::WaitValue ) {
        auto &node_ = nodes_stack_.back()->GetValue();

        if( !std::holds_alternative<Dict>(node_) ) {
            throw std::logic_error("Set value to dict");
        }
        if(std::holds_alternative<int>(value)) {
            std::get<Dict>(node_)[last_key_] = std::get<int>(value);
        } else if(std::holds_alternative<bool>(value)) {
            std::get<Dict>(node_)[last_key_] = std::get<bool>(value);
        } else if(std::holds_alternative<double>(value))  {
            std::get<Dict>(node_)[last_key_] = std::get<double>(value);
        } else if(std::holds_alternative<std::string>(value)) {
            std::get<Dict>(node_)[last_key_] = std::get<std::string>(value);
        } else if(std::holds_alternative<Array>(value)) {
            std::get<Dict>(node_)[last_key_] = std::get<Array>(value);
        } else if(std::holds_alternative<Dict>(value)) {
            std::get<Dict>(node_)[last_key_] = std::get<Dict>(value);
        } else if(std::holds_alternative<nullptr_t>(value)) {
            std::get<Dict>(node_)[last_key_] = std::get<nullptr_t>(value);
        }
        cur_state_ = State::WaitKey;
    }
    return BaseContext{*this};
}

Builder::DictItemContext Builder::StartDict() {

    if( nodes_stack_.size() == 0 || cur_state_ == State::WaitKey) {
        throw std::logic_error("Not start dict");
    }

    auto &node_ = nodes_stack_.back()->GetValue();

    if( std::holds_alternative<nullptr_t>(node_) ) {
        node_ = Dict{};
    } else if( std::holds_alternative<Array>(node_) ) {
        Node &node_new = std::get<Array>(node_).emplace_back(Dict{});
        nodes_stack_.push_back(&node_new);
    } else if (cur_state_ == State::WaitValue ) {
        std::get<Dict>(node_)[last_key_] = Dict{};
        nodes_stack_.push_back(&std::get<Dict>(node_)[last_key_]);
    } else {
        throw std::logic_error("Logic error in StartDict");
    }

    cur_state_ = State::WaitKey;
    return BaseContext(*this);
}

Builder &Builder::EndDict() {

    auto &node_ = nodes_stack_.back()->GetValue();

    if( !std::holds_alternative<Dict>(node_) ) {
        // Print(json::Document{root_node_}, std::cout);
        throw std::logic_error("Logic error in EndDict");
    }

    nodes_stack_.pop_back();
    CheckState();
    return *this;
}


Builder::ArrayItemContext Builder::StartArray() {
    if( nodes_stack_.size() == 0 || cur_state_ == State::WaitKey) {
        throw std::logic_error("Not start dict");
    }

    auto &node_ = nodes_stack_.back()->GetValue();

    if( std::holds_alternative<nullptr_t>(node_) ) {
        node_ = Array{};
    } else if( std::holds_alternative<Array>(node_) ) {
        Node &node_new = std::get<Array>(node_).emplace_back(Array{});
        nodes_stack_.push_back(&node_new);
    } else if (cur_state_ == State::WaitValue ) {
        std::get<Dict>(node_)[last_key_] = Array{};
        nodes_stack_.push_back(&std::get<Dict>(node_)[last_key_]);
    } else {
        throw std::logic_error("Logic error in StartDict");
    }

    cur_state_ = State::WaitAll;
    return BaseContext(*this);
}

Builder &Builder::EndArray() {
    auto &node_ = nodes_stack_.back()->GetValue();

    if( !std::holds_alternative<Array>(node_) ) {
        throw std::logic_error("Logic error in EndArray");
    }

    nodes_stack_.pop_back();
    CheckState();
    return *this;
}

void Builder::CheckState(void) {
    if( nodes_stack_.size() > 0 ) {
        auto &node_ = nodes_stack_.back()->GetValue();

        if( std::holds_alternative<Dict>(node_) ) {
            cur_state_ = State::WaitKey;
        }   

        if( std::holds_alternative<Array>(node_) ) {
            cur_state_ = State::WaitAll;
        }
    }
}

Node Builder::Build() {
    if( std::holds_alternative<nullptr_t>(root_node_.GetValue()) ) {
        throw std::logic_error("Nothing build");
    }
    return root_node_;
}
}