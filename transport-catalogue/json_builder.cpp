#include "json_builder.h"

#include <utility>
#include <variant>


namespace json {

void Builder::AddNode(Node node) {
    
    if (nodes_stack_.empty()) {
        
        if (!root_.IsNull()) {
            throw std::logic_error("the object has already been created");
        }
        
        root_ = std::move(node);
        return ;
        
    } else if (nodes_stack_.back()->IsString()) {
        std::string str = nodes_stack_.back()->AsString();
        nodes_stack_.pop_back();
        
        if (nodes_stack_.back()->IsDict()) {
            Dict dict = nodes_stack_.back()->AsDict();
            dict.emplace(std::move(str), node);
            nodes_stack_.pop_back();
            
            auto dict_ptr = std::make_unique<Node>(dict);
            nodes_stack_.emplace_back(std::move(dict_ptr));
        }
        
        return ;
        
    } else if (nodes_stack_.back()->IsArray()) {
        Array arr = nodes_stack_.back()->AsArray();
        arr.emplace_back(node);
        nodes_stack_.pop_back();
        
        auto array_ptr = std::make_unique<Node>(arr);
        nodes_stack_.emplace_back(std::move(array_ptr));
        
        return ;
        
    } else if ((!nodes_stack_.back()->IsString()) && (!nodes_stack_.back()->IsArray())) {
        throw std::logic_error("it is not possible to create an object");
    }
    
}

DictValueContext Builder::Key(std::string key) {
    
    if (nodes_stack_.empty()) {
        throw std::logic_error("it is not possible to create a key");
    }
    
    auto key_ptr = std::make_unique<Node>(key);
    
    if (nodes_stack_.back()->IsDict()) {
        nodes_stack_.emplace_back(std::move(key_ptr));
    }
    
    return DictValueContext(*this);
}

BaseContext Builder::Value(Node::Value value) {
    AddNode(Node(value));
    return BaseContext(*this);
}

DictItemContext Builder::StartDict() {
    nodes_stack_.emplace_back(std::move(std::make_unique<Node>(Dict())));
    return DictItemContext(*this);
}

ArrayItemContext Builder::StartArray() {
    nodes_stack_.emplace_back(std::move(std::make_unique<Node>(Array())));
    return ArrayItemContext(*this);
}

BaseContext Builder::EndDict() {
    
    if (nodes_stack_.empty()) {
        throw std::logic_error("the object has not been opened");
    }
    
    Node node = *nodes_stack_.back();
    
    if (!node.IsDict()) {
        throw std::logic_error("the current object is not an dictionary");
    }
    
    nodes_stack_.pop_back();
    AddNode(node);
    
    return BaseContext(*this);
}

BaseContext Builder::EndArray() {
    
    if (nodes_stack_.empty()) {
        throw std::logic_error("the object has not been opened");
    }
    
    Node node = *nodes_stack_.back();
    
    if (!node.IsArray()) {
        throw std::logic_error("the current object is not an array");
    }
    
    nodes_stack_.pop_back();
    AddNode(node);
    
    return BaseContext(*this);
}

Node Builder::Build() {
    
    if (root_.IsNull()) {
        throw std::logic_error("json is empty");
    }
    
    if (!nodes_stack_.empty()) {
        throw std::logic_error("incomplete arrays and dictionaries");
    }
    
    return root_;
}


BaseContext::BaseContext(Builder& builder)
: builder_(builder)
{    
}
DictValueContext BaseContext::Key(std::string key) {
    return builder_.Key(key);
}
BaseContext BaseContext::Value(Node::Value value) {
    return builder_.Value(value);
}
DictItemContext BaseContext::StartDict() {
    return builder_.StartDict();
}
ArrayItemContext BaseContext::StartArray() {
    return builder_.StartArray();
}
BaseContext BaseContext::EndDict() {
    return builder_.EndDict();
}
BaseContext BaseContext::EndArray() {
    return builder_.EndArray();
}

Node BaseContext::Build() {
    return builder_.Build();
}
Builder& BaseContext::GetBuilder() {
    return builder_;
}

DictValueContext::DictValueContext(Builder& builder)
: BaseContext(builder)
{    
}
DictItemContext DictValueContext::Value(Node::Value value) {
    return BaseContext::Value(value).GetBuilder();
}


DictItemContext::DictItemContext(Builder& builder)
: BaseContext(builder)
{    
}


ArrayItemContext::ArrayItemContext(Builder& builder)
: BaseContext(builder)
{    
}
ArrayItemContext ArrayItemContext::Value(Node::Value value) {
    return BaseContext::Value(value).GetBuilder();;
}





} // end namespace json