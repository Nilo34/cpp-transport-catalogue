#include "json.h"

/*
 * Место для вашей JSON-библиотеки
 */

#include <cassert>

using namespace std;

namespace json {

// -------------- Node ---------------------

Node::Node(std::nullptr_t)
    : Node()
{
}
Node::Node(int value)
    : as_value_(value)
{
}
Node::Node(double value)
    : as_value_(value)
{
}
Node::Node(string value)
    : as_value_(move(value))
{
}
Node::Node(bool value)
    : as_value_(value)
{
}
Node::Node(Array array)
    : as_value_(move(array))
{
}
Node::Node(Dict map)
    : as_value_(move(map))
{
}


bool Node::IsInt() const {
    return holds_alternative<int>(as_value_);
}
bool Node::IsPureDouble() const { //Возвращает true, если в Node хранится double.
    return holds_alternative<double>(as_value_);
}
bool Node::IsDouble() const { //Возвращает true, если в Node хранится int либо double.
    return (IsInt() || IsPureDouble());
}
bool Node::IsBool() const {
    return holds_alternative<bool>(as_value_);
}
bool Node::IsString() const {
    return holds_alternative<string>(as_value_);
}
bool Node::IsNull() const {
    return holds_alternative<nullptr_t>(as_value_);
}
bool Node::IsArray() const {
    return holds_alternative<Array>(as_value_);
}
bool Node::IsMap() const {
    return holds_alternative<Dict>(as_value_);
}


int Node::AsInt() const {
    if (IsInt()) {
        return get<int>(as_value_);
    }
    throw logic_error("is not int"s);
}
bool Node::AsBool() const {
    if (IsBool()) {
        return get<bool>(as_value_);
    }
    throw logic_error("is not bool"s);
}
double Node::AsDouble() const {
    if (IsDouble()) {
        if (IsPureDouble()) {
            return get<double>(as_value_);
        }
        return get<int>(as_value_);
    }
    throw logic_error("is not double"s);
}
const string& Node::AsString() const {
    if (IsString()) {
        //std::cout << get<string>(as_value_) << std::endl;
        return get<string>(as_value_);
    }
    throw logic_error("is not string"s);
}
const Array& Node::AsArray() const {
    if (IsArray()) {
        return get<Array>(as_value_);
    }
    throw logic_error("is not array"s);
}
const Dict& Node::AsMap() const {
    if (IsMap()) {
        return get<Dict>(as_value_);
    }
    throw logic_error("is not map"s);
}

const Node::Value& Node::GetValue() const {
    return as_value_;
}


bool operator==(const Node& lhs, const Node& rhs) {
    return lhs.GetValue() == rhs.GetValue();
}
bool operator!=(const Node& lhs, const Node& rhs) {
    return !(lhs == rhs);
}

// -------------- LoadNode ---------------------

namespace Loading {

using Number = std::variant<int, double>;

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

bool Loadbool(std::istream& input) {
    std::string str;
    
    while (std::isalpha(input.peek())) {
        str.push_back(static_cast<char>(input.get()));
    }
    
    if (str == "true"s) {
        return true;
    } else if (str == "false"s) {
        return false;
    }
    
    throw ParsingError("Failed to convert "s + str + " to bool"s);
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
                    s.push_back('\"');
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

std::nullptr_t LoadNull(std::istream& input) {
    std::string str;
    
    while (std::isalpha(input.peek())) {
        str.push_back(static_cast<char>(input.get()));
    }
    
    if (str == "null"s) {
        return nullptr;
    }
    
    throw ParsingError("Failed to convert "s + str + " to null"s);
}

Node LoadArray(std::istream& input) {
    Array result;

    for (char c; input >> c && c != ']';) {
        if (c != ',') {
            input.putback(c);
        }
        result.push_back(LoadNode(input));
    }
    
    if (!input) {
        throw ParsingError("Incorrect array input"s);
    }
    
    return Node(move(result));
}


Node LoadDict(std::istream& input) {
    Dict result;

    for (char c; input >> c && c != '}';) {
        if (c == ',') {
            input >> c;
        }

        string key = LoadString(input);
        input >> c;
        result.insert({move(key), LoadNode(input)});
    }
    
    if (!input) {
        throw ParsingError("Incorrect array input"s);
    }

    return Node(move(result));
}

Node LoadNode(std::istream& input) {
    char c;
    if (!(input >> c)) {
        throw ParsingError("Incorrect input"s);
    }
    
    if (c == '[') {
        return LoadArray(input);
    } else if (c == '{') {
        return LoadDict(input);
    } else if (c == '"') {
        return LoadString(input);
    } else if ((c == 't') || (c == 'f')) {
        input.putback(c);
        return Loadbool(input);
    } else if (c == 'n') {
        input.putback(c);
        return LoadNull(input);
    } else {
        input.putback(c);
        auto result = LoadNumber(input);
        if (holds_alternative<double>(result)) {
            return get<double>(result);
        }
        return get<int>(result);
    }
}

}  // namespace Loading

// -------------- Document ---------------------

Document::Document(Node root)
    : root_(move(root)) {
}

const Node& Document::GetRoot() const {
    return root_;
}

bool operator==(const Document& lhs, const Document& rhs) {
    return lhs.GetRoot() == rhs.GetRoot();
}
bool operator!=(const Document& lhs, const Document& rhs){
    return !(lhs == rhs);
}

Document Load(std::istream& input) {
    return Document{Loading::LoadNode(input)};
}

// -------------- Print ---------------------

void PrintContext::PrintIndent() const {
    for (int i = 0; i < indent; ++i) {
        out.put(' ');
    }
}

// Возвращает новый контекст вывода с увеличенным смещением
PrintContext PrintContext::Indented() const {
    return {out, indent_step, indent_step + indent};
}

void PrintString(const std::string& value, std::ostream& out) {
    using namespace std::literals;
    
    out << "\""sv;
    //std::cout << "\""sv;
    
    for (const char ch : value) {
        switch (ch) {
            case '\n':
                out << R"(\n)"sv;
                //std::cout << R"(\n)"sv;
                break;
            case '\t':
                out << "\t"sv;
                //std::cout << "\t"sv;
                break;
            case '\r':
                out << R"(\r)"sv;
                //std::cout << R"(\r)"sv;
                break;
            case '\"':
                out << R"(\")"sv;
                //std::cout << R"(\")"sv;
                break;
            case '\\':
                out << R"(\\)"sv;
                //std::cout << R"(\\)"sv;
                break;
            default:
                out << ch;
                //std::cout << ch;
                break;
        }
    }
    
    out << "\""sv;
    //std::cout << "\""sv << std::endl;
}

// Перегрузка функции PrintValue для вывода значений null
void PrintValue(std::nullptr_t, const PrintContext& ctx) {
    ctx.out << "null"sv;
}

// Перегрузка функции PrintValue для вывода значений string
void PrintValue(const std::string& value, const PrintContext& ctx) {
    PrintString(value, ctx.out);
}

// Перегрузка функции PrintValue для вывода значений bool
void PrintValue(const bool value, const PrintContext& ctx) {
    ctx.out << std::boolalpha << value;
}

// Перегрузка функции PrintValue для вывода значений Array
void PrintValue(const Array& values, const PrintContext& ctx) {
    ctx.out << "["sv << std::endl;
    
    bool need_a_comma = false;
    json::PrintContext ctx_indented = ctx.Indented();
    
    for (const Node& node : values) {
        if (need_a_comma) {
            ctx.out << ", "sv << std::endl;
        } else {
            need_a_comma = true;
        }
        
        ctx_indented.PrintIndent();
        PrintNode(node, ctx_indented);
    }
    
    ctx.out << std::endl;
    ctx.PrintIndent();
    ctx.out << "]"sv;
}

// Перегрузка функции PrintValue для вывода значений Dict
void PrintValue(const Dict& values, const PrintContext& ctx) {
    ctx.out << "{"sv << std::endl;
    
    bool need_a_comma = false;
    json::PrintContext ctx_indented = ctx.Indented();
    
    for (const auto& [key, node] : values) {
        if (need_a_comma) {
            ctx.out << ", "sv << std::endl;
        } else {
            need_a_comma = true;
        }
        
        ctx_indented.PrintIndent();
        PrintString(key, ctx.out);
        ctx.out << ": "sv;
        PrintNode(node, ctx_indented);
    }
    
    ctx.out << std::endl;
    ctx.PrintIndent();
    ctx.out << "}"sv;
}

void PrintNode(const Node& node, const PrintContext& ctx) {
    std::visit(
        [&ctx](const auto& value){ PrintValue(value, ctx); },
        node.GetValue());
}

void Print(const Document& doc, std::ostream& output) {
    PrintNode(doc.GetRoot(), PrintContext{output});
}

}  // namespace json