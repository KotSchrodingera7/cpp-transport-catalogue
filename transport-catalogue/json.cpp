#include "json.h"

using namespace std;

namespace json {

// namespace {


using Number = std::variant<int, double>;
Node LoadNode(istream& input);

Node LoadArray(istream& input) {
    Array result;
    char c;
    for (; input >> c && c != ']';) {
        if (c != ',') {
            input.putback(c);
        }
        result.push_back(LoadNode(input));
    }
    if( c != ']') {
    // if ( !result.size() ) {
        throw json::ParsingError("");
    }
    return Node(move(result));
}

Node LoadInt(istream& input) {
    int result = 0;
    while (isdigit(input.peek())) {
        result *= 10;
        result += input.get() - '0';
    }
    return Node(result);
}

Number LoadNumber(std::istream& input) {
    using namespace std::literals;

    std::string parsed_num;

    // Считывает в parsed_num очередной символ из input
    auto read_char = [&parsed_num, &input] {
        parsed_num += static_cast<char>(input.get());
        if (!input) {
            throw ParsingError("Failed to read number from stream"s);
        }
    };

    // Считывает одну или более цифр в parsed_num из input
    auto read_digits = [&input, read_char] {
        if (!std::isdigit(input.peek())) {
            throw ParsingError("A digit is expected"s);
        }
        while (std::isdigit(input.peek())) {
            read_char();
        }
    };

    if (input.peek() == '-') {
        read_char();
    }
    // Парсим целую часть числа
    if (input.peek() == '0') {
        read_char();
        // После 0 в JSON не могут идти другие цифры
    } else {
        read_digits();
    }

    bool is_int = true;
    // Парсим дробную часть числа
    if (input.peek() == '.') {
        read_char();
        read_digits();
        is_int = false;
    }

    // Парсим экспоненциальную часть числа
    if (int ch = input.peek(); ch == 'e' || ch == 'E') {
        read_char();
        if (ch = input.peek(); ch == '+' || ch == '-') {
            read_char();
        }
        read_digits();
        is_int = false;
    }

    try {
        if (is_int) {
            // Сначала пробуем преобразовать строку в int
            try {
                return std::stoi(parsed_num);
            } catch (...) {
                // В случае неудачи, например, при переполнении,
                // код ниже попробует преобразовать строку в double
            }
        }
        return std::stod(parsed_num);
    } catch (...) {
        throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
    }
}

// Считывает содержимое строкового литерала JSON-документа
// Функцию следует использовать после считывания открывающего символа ":
std::string LoadString(std::istream& input) {
    using namespace std::literals;
    
    auto it = std::istreambuf_iterator<char>(input);
    auto end = std::istreambuf_iterator<char>();
    std::string s;
    while (true) {
        if (it == end) {
            // Поток закончился до того, как встретили закрывающую кавычку?
            throw ParsingError("String parsing error");
        }
        const char ch = *it;
        if (ch == '"') {
            // Встретили закрывающую кавычку
            ++it;
            break;
        } else if (ch == '\\') {
            // Встретили начало escape-последовательности
            ++it;
            if (it == end) {
                // Поток завершился сразу после символа обратной косой черты
                throw ParsingError("String parsing error");
            }
            const char escaped_char = *(it);
            // Обрабатываем одну из последовательностей: \\, \n, \t, \r, \"
            switch (escaped_char) {
                case 'n':
                    s.push_back('\n');
                    break;
                case 't':
                    s.push_back('\t');
                    break;
                case 'r':
                    s.push_back('\r');
                    break;
                case '"':
                    s.push_back('"');
                    break;
                case '\\':
                    s.push_back('\\');
                    break;
                default:
                    // Встретили неизвестную escape-последовательность
                    throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
            }
        } else if (ch == '\n' || ch == '\r') {
            // Строковый литерал внутри- JSON не может прерываться символами \r или \n
            throw ParsingError("Unexpected end of line"s);
        } else {
            // Просто считываем очередной символ и помещаем его в результирующую строку
            s.push_back(ch);
        }
        ++it;
    }

    return s;
}

Node LoadDict(istream& input) {
    Dict result;

    char c;
    for (; input >> c && c != '}';) {
        if (c == ',') {
            input >> c;
        }

        string key = LoadString(input);
        input >> c;
        result.insert({move(key), LoadNode(input)});
    }

    if( c != '}') {
    // if ( !result.size() ) {
        throw json::ParsingError("");
    }

    return Node(move(result));
}

Node LoadNode(istream& input) {
    char c;
    input >> c;

    if( c == ']' || c == '}') {
        throw json::ParsingError("");
    }
    if (c == '[') {
        return LoadArray(input);
    } else if (c == '{') {
        return LoadDict(input);
    } else if (c == '"') {
        return Node(move(LoadString(input)));
    } else if( c == 'n') {
        input.putback(c);
        std::string result(4, ' ');

        for(int i = 0; i < static_cast<int>(result.size()); ++i) {
            input >> result[i];
        }

        if( result == "null") {
            return Node(std::nullptr_t{});
        }

        throw json::ParsingError("");
    } else if( c == 't') {
        input.putback(c);
        std::string result(4, ' ');

        for(int i = 0; i < static_cast<int>(result.size()); ++i) {
            input >> result[i];
        }
        if( result == "true") {
            return Node(true);
        }

        throw json::ParsingError("");
        
    } if( c == 'f') {
        input.putback(c);
        std::string result(5, ' ');

        for(int i = 0; i < static_cast<int>(result.size()); ++i) {
            input >> result[i];
        }
        if( result == "false") {
            return Node(false);
        }

        throw json::ParsingError("");
    } else {
        input.putback(c);

        try {
            auto number = LoadNumber(input); 
            if( std::holds_alternative<double>(number) ) {
                return Node(std::get<double>(number));
            }
            return Node(std::get<int>(number));
        } catch (...) {
            
        }
        throw json::ParsingError("");
    }
}
// }  // namespace

Document::Document(Node root)
    : root_(move(root)) {
}

const Node& Document::GetRoot() const {
    return root_;
}

Document Load(istream& input) {
    return Document{LoadNode(input)};
}

void PrintNode(const Node& node, const PrintContext& ctx) {

    std::visit(
        [&ctx](const auto& value){ PrintValue(value, ctx); },
        node.GetValue());
} 

void PrintValue(std::nullptr_t, const PrintContext& ctx) {
    ctx.out << "null"sv;
}

void PrintValue(Dict dict, const PrintContext& ctx) {

    ctx.out << "{\n";
    bool start = true;
    
    for(const auto &[name, node] : dict) {
        if( start ) {
            ctx.Indented().PrintIndent();
            ctx.out << "\"" << name << "\": ";
            PrintNode(node, ctx.Indented());
            start = false;
        } else {
            ctx.out << ",\n";
            ctx.Indented().PrintIndent();
            ctx.out << "\"" << name << "\": "; 
            PrintNode(node, ctx.Indented());
        }
    }
    ctx.out << "\n";
    ctx.PrintIndent();
    ctx.out << "}";
}

void PrintValue(double number, const PrintContext& ctx) {
    ctx.out << (double)number/1.0;
}

void PrintValue(int number, const PrintContext& ctx) {
    ctx.out << number;
}

void PrintValue(std::string str, const PrintContext& ctx) {
    std::string str_ = str;

    std::vector<char> escape_symbls_ = {'\\', '\n', '\r', '"'};
    
    for(const char &symbol : escape_symbls_) {
        auto pos = str_.find(symbol);
        while( pos != std::string::npos ) {
            if( symbol == '\n') {
                str_[pos] = 'n';
            } else if( symbol == '\r') {
                str_[pos] = 'r';
            }
            str_.insert(pos, 1, '\\');
            pos = str_.find(symbol, pos + 2);
        }
    }
    ctx.out << "\"" << str_ << "\"";
}

void PrintValue(bool value, const PrintContext& ctx) {
    std::string bool_str = value ? "true" : "false";
    ctx.out << bool_str;
}

void PrintValue(Array array, const PrintContext& ctx) {

    ctx.out << "[\n";
    bool start = true;
    ctx.Indented().PrintIndent();
    for(Node node_ : array) {
        if( start ) {
            PrintNode(node_, ctx.Indented());
            start = false;
        } else {
            ctx.out << ",\n"; 
            ctx.Indented().PrintIndent();
            PrintNode(node_, ctx.Indented());
        }
    }
    ctx.out << "\n";
    ctx.PrintIndent();
    ctx.out << "]";
}




void Print(const Document& doc, std::ostream& output) {

    PrintNode(doc.GetRoot().GetValue(), {output, 4, 0});
}

}  // namespace json// output << doc.GetRoot().GetValue();