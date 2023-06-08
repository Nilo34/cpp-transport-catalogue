#pragma once
/*
 * Место для вашей JSON-библиотеки
 */

#include <iostream>
#include <sstream>
#include <map>
#include <string>
#include <vector>
#include <variant>

namespace json {

class Node;
// Сохраните объявления Dict и Array без изменения
using Dict = std::map<std::string, Node>;
using Array = std::vector<Node>;

// Эта ошибка должна выбрасываться при ошибках парсинга JSON
class ParsingError : public std::runtime_error {
public:
    using runtime_error::runtime_error;
};

class Node {
public:
   /* Реализуйте Node, используя std::variant */
    using Value = std::variant<std::nullptr_t, int, double, std::string, bool, Array, Dict>;
    
    Node() = default;
    Node(std::nullptr_t);
    
    template <typename Value>
    Node(Value value) 
    : as_value_(std::move(value))
    {
    }
    
    bool IsInt() const;
    bool IsPureDouble() const; //Возвращает true, если в Node хранится double.
    bool IsDouble() const; //Возвращает true, если в Node хранится int либо double.
    bool IsBool() const;
    bool IsString() const;
    bool IsNull() const;
    bool IsArray() const;
    bool IsMap() const;
    
    int AsInt() const;
    bool AsBool() const;
    // AsDouble() возвращает значение типа double, если внутри хранится double либо int. 
    // В последнем случае возвращается приведённое в double значение
    double AsDouble() const;
    const std::string& AsString() const;
    const Array& AsArray() const;
    const Dict& AsMap() const;
    
    const Value& GetValue() const;

private:
    Value as_value_;
};

bool operator==(const Node& lhs, const Node& rhs);
bool operator!=(const Node& lhs, const Node& rhs);

namespace Loading {
using Number = std::variant<int, double>;

Number LoadNumber(std::istream& input);
bool Loadbool(std::istream& input);
std::string LoadString(std::istream& input);
std::nullptr_t LoadNull(std::istream& input);
Node LoadArray(std::istream& input);
Node LoadDict(std::istream& input);
Node LoadNode(std::istream& input);

}  // namespace Loading

class Document {
public:
    Document() = default;
    explicit Document(Node root);
    
    const Node& GetRoot() const;

private:
    Node root_;
};

bool operator==(const Document& lhs, const Document& rhs);
bool operator!=(const Document& lhs, const Document& rhs);

Document Load(std::istream& input);

// Контекст вывода, хранит ссылку на поток вывода и текущий отсуп
struct PrintContext {
    std::ostream& out;
    int indent_step = 4;
    int indent = 0;

    void PrintIndent() const;

    // Возвращает новый контекст вывода с увеличенным смещением
    PrintContext Indented() const;
};

void PrintString(const std::string& value, std::ostream& out);

void PrintNode(const Node& node, const PrintContext& ctx);

// Шаблон, подходящий для вывода double и int
template <typename Value>
void PrintValue(const Value& value, const PrintContext& ctx) {
    ctx.out << value;
}
void PrintValue(std::nullptr_t, const PrintContext& ctx);
void PrintValue(const std::string& value, const PrintContext& ctx);
void PrintValue(const bool value, const PrintContext& ctx);
void PrintValue(const Array& values, const PrintContext& ctx);
void PrintValue(const Dict& values, const PrintContext& ctx);

void Print(const Document& doc, std::ostream& output);

}  // namespace json