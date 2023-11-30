#pragma once

#include <iostream>
#include <map>
#include <string>
#include <variant>
#include <vector>

namespace json {

class Node;
using Dict = std::map<std::string, Node>;
using Array = std::vector<Node>;

class ParsingError : public std::runtime_error {
public:
    using runtime_error::runtime_error;
};

class Node final
    : private std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string> {
public:
    using variant::variant;
    using Value = variant;
template<typename Type>
bool IsType() const {
    if constexpr (std::is_same_v<Type, double>) {
        return std::holds_alternative<Type>(*this) || std::holds_alternative<int>(*this);
    } 
    return std::holds_alternative<Type>(*this);
}

template<typename Type> 
const Type AsType() const {
    if( !IsType<Type>() ) {
        throw std::logic_error("");
    }
    if constexpr (std::is_same_v<Type, double>) {
        if( IsType<int>() ) {
            return static_cast<double>(std::get<int>(*this));
        }
    }
    return std::get<Type>(*this);
}
    bool IsPureDouble() const {
        return std::holds_alternative<double>(*this);
    }    

    bool operator==(const Node& rhs) const {
        return GetValue() == rhs.GetValue();
    }

    const Value& GetValue() const {
        return *this;
    }

    Value& GetValue() {
        return *this;
    }
};

inline bool operator!=(const Node& lhs, const Node& rhs) {
    return !(lhs == rhs);
}

class Document {
public:
    explicit Document(Node root)
        : root_(std::move(root)) {
    }

    const Node& GetRoot() const {
        return root_;
    }

private:
    Node root_;
};

inline bool operator==(const Document& lhs, const Document& rhs) {
    return lhs.GetRoot() == rhs.GetRoot();
}

inline bool operator!=(const Document& lhs, const Document& rhs) {
    return !(lhs == rhs);
}

Document Load(std::istream& input);

void Print(const Document& doc, std::ostream& output);

}  // namespace json