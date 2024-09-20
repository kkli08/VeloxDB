//
// Created by Damian Li on 2024-08-27.
//

#include "TreeNode.h"

// Constructor
// TreeNode::TreeNode(long long _key, long long _value) : key(_key), value(_value), left(nullptr), right(nullptr), parent(nullptr), color(RED) {}
// TreeNode::TreeNode(long long _key, long long _value, Color c) : key(_key), value(_value), left(nullptr), right(nullptr), parent(nullptr), color(c) {}

// Constructor with default RED color
TreeNode::TreeNode(KeyValueWrapper kv)
    : keyValue(kv), left(nullptr), right(nullptr), parent(nullptr), color(RED) {}

// Constructor with custom color
TreeNode::TreeNode(KeyValueWrapper kv, Color c)
    : keyValue(kv), left(nullptr), right(nullptr), parent(nullptr), color(c) {}

