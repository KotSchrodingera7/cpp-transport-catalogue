#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>
#include <algorithm>

namespace json {

class Node;
// Сохраните объявления Dict и Array без изменения
using Dict = std::map<std::string, Node>;
using Array = std::vector<Node>;

using Value = std::variant<std::nullptr_t, Array, Dict, int, double, std::string, bool>;

// Эта ошибка должна выбрасываться при ошибках парсинга JSON
class ParsingError : public std::runtime_error {
public:
    using runtime_error::runtime_error;
};

using namespace std::literals;

std::ostream& operator<<(std::ostream &out, const Value &value);


class Node {
public:
   /* Реализуйте Node, используя std::variant */

    Node(const Value value) : value_(std::move(value)) {};
    Node() = default;// : value_(std::nullptr_t{}) {};
    Node(int value) : value_(value) {};
    Node(double value) : value_(value) {};
    Node(Array value) : value_(value) {};
    Node(bool value) : value_(value) {};
    Node(std::string value) : value_(value) {};
    Node(Dict value) : value_(value) {};
    Node(std::nullptr_t value) : value_(value) {};
    
    const Value& GetValue() const { return value_; }

template<typename Type>
    bool IsType() const {
        if constexpr (std::is_same_v<Type, double>) {
            return std::holds_alternative<Type>(value_) || std::holds_alternative<int>(value_);
        } 
        return std::holds_alternative<Type>(value_);
    }

template<typename Type> 
    const Type AsType() const {
        if( !IsType<Type>() ) {
            throw std::logic_error("");
        }
        if constexpr (std::is_same_v<Type, double>) {
            if( IsType<int>() ) {
                return static_cast<double>(std::get<int>(value_));
            }
        }
        return std::get<Type>(value_);
    }
    bool IsPureDouble() const {
        return std::holds_alternative<double>(value_);
    }    
private:
    Value value_;
};

class Document {
public:
    explicit Document(Node root);

    const Node& GetRoot() const;

private:
    Node root_;
};

Document Load(std::istream& input);

void Print(const Document& doc, std::ostream& output);

struct PrintContext {
    std::ostream& out;
    int indent_step = 4;
    int indent = 0;

    void PrintIndent() const {
        for (int i = 0; i < indent; ++i) {
            out.put(' ');
        }
    }

    // Возвращает новый контекст вывода с увеличенным смещением
    PrintContext Indented() const {
        return {out, indent_step, indent_step + indent};
    }
};


void PrintValue(std::nullptr_t, const PrintContext& ctx);
void PrintValue(Dict dict, const PrintContext& ctx);
void PrintValue(double number, const PrintContext& ctx);
void PrintValue(int number, const PrintContext& ctx);
void PrintValue(std::string str, const PrintContext& ctx);
void PrintValue(bool value, const PrintContext& ctx);
void PrintValue(Array array, const PrintContext& ctx);

}  // namespace json