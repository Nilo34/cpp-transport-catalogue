#pragma once

#include "json.h"
#include <string>
#include <vector>
#include <memory>

namespace json {

class Builder {
public:
    
    class BaseContext;
    class DictValueContext;
    class DictItemContext;
    class ArrayItemContext;
    
    DictValueContext Key(std::string key);
    BaseContext Value(Node::Value value);
    DictItemContext StartDict();
    ArrayItemContext StartArray();
    BaseContext EndDict();
    BaseContext EndArray();
    
    Node Build();
    
private:
    Node root_;
    std::vector<std::unique_ptr<Node>> nodes_stack_;
    
    void AddNode(Node node);
};

class Builder::BaseContext {
public:
    
    BaseContext(Builder& builder);
    
    DictValueContext Key(std::string key);
    BaseContext Value(Node::Value value);
    DictItemContext StartDict();
    ArrayItemContext StartArray();
    BaseContext EndDict();
    BaseContext EndArray();
    
    Node Build();
    Builder& GetBuilder();
    
protected:
    Builder& builder_;
};

class Builder::DictValueContext : public BaseContext {
public:
    DictValueContext(Builder& builder);
    
    DictValueContext Key(std::string key) = delete;
    DictItemContext Value(Node::Value value);
    BaseContext EndDict() = delete;
    BaseContext EndArray() = delete;
    
    Node Build() = delete;
};
class Builder::DictItemContext : public BaseContext {
public:
    DictItemContext(Builder& builder);
    
    BaseContext Value(Node::Value value) = delete;
    DictItemContext StartDict() = delete;
    ArrayItemContext StartArray() = delete;
    BaseContext EndArray() = delete;
    
    Node Build() = delete;
};
class Builder::ArrayItemContext : public BaseContext {
public:
    ArrayItemContext(Builder& builder);
    
    DictValueContext Key(std::string key) = delete;
    ArrayItemContext Value(Node::Value value);
    BaseContext EndDict() = delete;
    
    Node Build() = delete;
};

} // end namespace json