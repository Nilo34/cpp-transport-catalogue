#include "json_builder.h"

#include <utility>
#include <variant>


namespace json {

Builder::DictValueContext Builder::Key(std::string key) {
    
    if (nodes_stack_.empty()) {
        throw std::logic_error("it is not possible to create a key");
    }
    
    auto key_ptr = std::make_unique<Node>(key);
    
    if (nodes_stack_.back()->IsDict()) {
        nodes_stack_.emplace_back(std::move(key_ptr));
    }
    
    return DictValueContext(*this);
}

Builder::BaseContext Builder::Value(Node::Value value) {
    AddNode(Node(value));
    return BaseContext(*this);
}

Builder::DictItemContext Builder::StartDict() {
    nodes_stack_.emplace_back(std::move(std::make_unique<Node>(Dict())));
    return DictItemContext(*this);
}

Builder::ArrayItemContext Builder::StartArray() {
    nodes_stack_.emplace_back(std::move(std::make_unique<Node>(Array())));
    return ArrayItemContext(*this);
}

Builder::BaseContext Builder::EndDict() {
    
    if (nodes_stack_.empty()) {
        throw std::logic_error("the object has not been opened");
    }
    
    Node node = *nodes_stack_.back();
    
    if (!node.IsDict()) {
        throw std::logic_error("the current object is not an dictionary");
    }
    
    nodes_stack_.pop_back();
    AddNode(std::move(node));
    
    return BaseContext(*this);
}

Builder::BaseContext Builder::EndArray() {
    
    if (nodes_stack_.empty()) {
        throw std::logic_error("the object has not been opened");
    }
    
    Node node = *nodes_stack_.back();
    
    if (!node.IsArray()) {
        throw std::logic_error("the current object is not an array");
    }
    
    nodes_stack_.pop_back();
    AddNode(std::move(node));
    
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
            Dict& dict = nodes_stack_.back()->AsDict();
            dict.emplace(std::move(str), std::move(node));
        }
        
        return ;
        
    } else if (nodes_stack_.back()->IsArray()) {
        Array& arr = nodes_stack_.back()->AsArray();
        arr.emplace_back(std::move(node));
        
        return ;
        
    } else if ((!nodes_stack_.back()->IsString()) && (!nodes_stack_.back()->IsArray())) {
        throw std::logic_error("it is not possible to create an object");
    }
    
}


Builder::BaseContext::BaseContext(Builder& builder)
: builder_(builder)
{    
}
Builder::DictValueContext Builder::BaseContext::Key(std::string key) {
    return builder_.Key(std::move(key));
}
Builder::BaseContext Builder::BaseContext::Value(Node::Value value) {
    return builder_.Value(std::move(value));
}
Builder::DictItemContext Builder::BaseContext::StartDict() {
    return builder_.StartDict();
}
Builder::ArrayItemContext Builder::BaseContext::StartArray() {
    return builder_.StartArray();
}
Builder::BaseContext Builder::BaseContext::EndDict() {
    return builder_.EndDict();
}
Builder::BaseContext Builder::BaseContext::EndArray() {
    return builder_.EndArray();
}

Node Builder::BaseContext::Build() {
    return builder_.Build();
}
Builder& Builder::BaseContext::GetBuilder() {
    return builder_;
}

Builder::DictValueContext::DictValueContext(Builder& builder)
: BaseContext(builder)
{    
}
Builder::DictItemContext Builder::DictValueContext::Value(Node::Value value) {
    return BaseContext::Value(std::move(value)).GetBuilder();
}


Builder::DictItemContext::DictItemContext(Builder& builder)
: BaseContext(builder)
{    
}


Builder::ArrayItemContext::ArrayItemContext(Builder& builder)
: BaseContext(builder)
{    
}
Builder::ArrayItemContext Builder::ArrayItemContext::Value(Node::Value value) {
    return BaseContext::Value(std::move(value)).GetBuilder();
}

} // end namespace json