//
// Created by Damian Li on 2024-08-26.
//

#ifndef REDBLACKTREE_H
#define REDBLACKTREE_H
#include "TreeNode.h"
#include "BinaryTree.h"
#include <fstream>
#include <utility>
#include <vector>
#include <unordered_map>
#include <functional>
using namespace std;

class RedBlackTree final : public BinaryTree {
    public:
        void merge(RedBlackTree);
        void inorderTraversal() override;
        void preorder();
        // update with KeyValueWrapper Class
        vector<KeyValueWrapper> inOrderFlushToSst(); // tested
        KeyValueWrapper getValue(const KeyValueWrapper& kv); // tested
        void insert(KeyValueWrapper kv);   // done tested
        void updateExistedKeyValue(TreeNode *&root, KeyValueWrapper& kv); // tested
        void deleteKey(KeyValueWrapper kv);  // added
        static int getColor(TreeNode *&);  // done tested
        int getBlackHeight(TreeNode *); // added tested

        void inOrderTraversal(std::function<void(KeyValueWrapper&)> callback) ;
        void inOrderTraversal(TreeNode* node, std::function<void(KeyValueWrapper&)> callback) ;


    protected:
        void rotateLeft(TreeNode *&); // done
        void rotateRight(TreeNode *&); // done
        void fixInsertRBTree(TreeNode *&);  // done
        void fixDeleteRBTree(TreeNode *&);  // added
        void inorderRBT(TreeNode *&); // done
        void preorderRBT(TreeNode *&);
        static void setColor(TreeNode *&, int);    // done
        TreeNode *minValueNode(TreeNode *&); // added
        TreeNode *maxValueNode(TreeNode *&); // added
        // update with KeyValueWrapper Class
        void inorderTraversal(TreeNode *, vector<KeyValueWrapper> &);
        KeyValueWrapper getValue(TreeNode *&, const KeyValueWrapper& kv);
        TreeNode* insert(TreeNode *&, TreeNode *&); // done
        TreeNode* deleteBST(TreeNode *&, KeyValueWrapper kv);    // added

};

#endif //REDBLACKTREE_H
