//
// Created by Damian Li on 2024-08-26.
//

#ifndef BINARYTREE_H
#define BINARYTREE_H

#include "TreeNode.h"
#include "KeyValue.h"
#include <set>

using namespace std;

class BinaryTree {
public:
    BinaryTree();
    virtual ~BinaryTree();

    // Templated insert method
    template<typename K, typename V>
    void insert(K key, V value);

    // Search methods
    bool search(const KeyValueWrapper& kv);  // Search using KeyValueWrapper
    template<typename K>
    bool search(K key);

    TreeNode*& getRoot();

    // Traverse methods
    virtual void inorderTraversal();
    void preorderTraversal();
    void postorderTraversal();

    // Scan method using KeyValueWrapper
    void Scan(TreeNode* node, const KeyValueWrapper& small_key, const KeyValueWrapper& large_key, std::set<KeyValueWrapper>& res);

protected:
    TreeNode *root;

    // Internal insert method using KeyValueWrapper
    void insert(TreeNode*& node, KeyValueWrapper kv);

    // Internal search method
    bool search(TreeNode* node, const KeyValueWrapper& kv);

    // Traverse methods
    void inorderTraversal(TreeNode* node);
    void preorderTraversal(TreeNode* node);
    void postorderTraversal(TreeNode* node);

    // Utility to destroy the tree
    static void destroyTree(const TreeNode* node);
};

#include "BinaryTree.tpp"  // Include the templated method implementations

#endif // BINARYTREE_H
