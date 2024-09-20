//
// Created by Damian Li on 2024-08-27.
//

#ifndef TREENODE_H
#define TREENODE_H
#include "KeyValue.h"

enum Color {RED, BLACK, DOUBLE_BLACK, NA};

class TreeNode {
    public:
        KeyValueWrapper keyValue;
        TreeNode* left;
        TreeNode* right;
        TreeNode* parent;
        int color;

        // Constructor that directly takes key and value
        TreeNode(KeyValueWrapper kv);  // Insert RED node as default
        TreeNode(KeyValueWrapper kv, Color c);

        template<typename K, typename V>
        TreeNode(K key, V value);

        // Constructor that allows setting the color
        template<typename K, typename V>
        TreeNode(K key, V value, Color c);

        // Destructor
        inline ~TreeNode();

};

#include "TreeNode.tpp"

#endif //TREENODE_H
