#include "TreeNode.h"

// Constructor that takes key and value and assigns them to keyValue (default RED color)
template<typename K, typename V>
TreeNode::TreeNode(K key, V value)
    : keyValue(KeyValueWrapper(key, value)), left(nullptr), right(nullptr), parent(nullptr), color(RED) {}

// Constructor that takes key, value, and color
template<typename K, typename V>
TreeNode::TreeNode(K key, V value, Color c)
    : keyValue(KeyValueWrapper(key, value)), left(nullptr), right(nullptr), parent(nullptr), color(c) {}

// Destructor
TreeNode::~TreeNode() {
    // Clean-up if necessary
}
