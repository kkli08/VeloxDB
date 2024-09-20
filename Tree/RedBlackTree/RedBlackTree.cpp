//
// Created by Damian Li on 2024-08-26.
// RBTree ideas from : https://github.com/anandarao/Red-Black-Tree
#include <utility>
#include <iostream>
#include "RedBlackTree.h"

using namespace std;
// helper functions:
int RedBlackTree::getColor(TreeNode *&node){
  if (node == nullptr) return BLACK;
  return node->color;
}
void RedBlackTree::setColor(TreeNode *&node, int color){
  if (node == nullptr) return;
  node->color = color;
}

void RedBlackTree::inOrderTraversal(std::function<void(KeyValueWrapper&)> callback) {
    inOrderTraversal(root, callback);
}

void RedBlackTree::inOrderTraversal(TreeNode* node, std::function<void( KeyValueWrapper&)> callback) {
    if (node == nullptr) return;

    inOrderTraversal(node->left, callback);  // Visit left subtree
    callback(node->keyValue);                                   // Pass const KeyValueWrapper to callback
    inOrderTraversal(node->right, callback); // Visit right subtree
}

// TreeNode* RedBlackTree::getRoot() {
//     return root;
// }

void RedBlackTree::inorderTraversal() {
    inorderRBT(root);
}

void RedBlackTree::inorderRBT(TreeNode *&node) {
    if (node == nullptr) {
        return; // Base case: if the node is null, return
    }
    // Recursively traverse the left subtree
    inorderRBT(node->left);
    // Print the key and value of the current node
    node->keyValue.printKeyValue();
    std::cout << "Color: "
              << (node->color == RED ? "Red" : (node->color == BLACK ? "Black" :
                  (node->color == DOUBLE_BLACK ? "Double Black" : "N/A"))) << std::endl;
    // Recursively traverse the right subtree
    inorderRBT(node->right);
}

vector<KeyValueWrapper> RedBlackTree::inOrderFlushToSst() {
    vector<KeyValueWrapper> kv_pairs;  // Store KeyValueWrapper objects
    inorderTraversal(root, kv_pairs);  // Perform in-order traversal
    return kv_pairs;  // Return the vector of KeyValueWrapper objects
}


void RedBlackTree::inorderTraversal(TreeNode* node, vector<KeyValueWrapper> &kv_pairs) {
    if (node == nullptr) {
        return;
    }
    inorderTraversal(node->left, kv_pairs);
    kv_pairs.push_back(node->keyValue);
    inorderTraversal(node->right, kv_pairs);
}

KeyValueWrapper RedBlackTree::getValue(const KeyValueWrapper& kv) {
    if (search(kv)) {
        // Return the found KeyValueWrapper
        return getValue(root, kv);
    }
    // Return an empty KeyValueWrapper (or handle not found in another way)
    // currently just make it empty 2024-09-08
    return KeyValueWrapper();
}

KeyValueWrapper RedBlackTree::getValue(TreeNode *&node, const KeyValueWrapper& kv) {
    if (node->keyValue == kv) {
        // Return the value from the matching node
        return node->keyValue;
    } else if (kv < node->keyValue) {
        return getValue(node->left, kv);  // Search in the left subtree
    } else {
        return getValue(node->right, kv);  // Search in the right subtree
    }
}


/*
 * Red Black Tree Insertions
 *  - rotateLeft
 *  - rotateRight
 *  - public insert
 *  - insert
 *  - fixInsertRBTree
 */
// RBTree insert method
void RedBlackTree::insert(KeyValueWrapper kv) {
    // First, search for the key in the tree
    if (search(kv)) {
        // If the key exists, update the value
        updateExistedKeyValue(root, kv);
    } else {
        // Create a new TreeNode with the KeyValue
        TreeNode* newNode = new TreeNode(kv);
        root = insert(root, newNode);  // Insert the new node in the tree
        fixInsertRBTree(newNode);      // Fix any red-black tree property violations
    }
}


void RedBlackTree::updateExistedKeyValue(TreeNode *&node, KeyValueWrapper& kv) {
    if (node == nullptr) {
        return;  // If node is null, there's nothing to update
    } else if (node->keyValue == kv) {
        // If the key matches, update the value
        node->keyValue = kv;  // Replace the whole KeyValue
    } else if (kv < node->keyValue) {
        // If the key is smaller, traverse the left subtree
        updateExistedKeyValue(node->left, kv);
    } else {
        // Otherwise, traverse the right subtree
        updateExistedKeyValue(node->right, kv);
    }
}


// BST insertions
TreeNode* RedBlackTree::insert(TreeNode*& root, TreeNode*& newNode){
  if (root == nullptr)
    return newNode;

  if (newNode->keyValue < root->keyValue) {
    root->left = insert(root->left, newNode);
    root->left->parent = root;
  } else if (newNode->keyValue > root->keyValue) {
    root->right = insert(root->right, newNode);
    root->right->parent = root;
  }

  return root;
}

void RedBlackTree::fixInsertRBTree(TreeNode *&ptr){
  TreeNode *parent = nullptr;
  TreeNode *grandparent = nullptr;
  while (ptr != root && getColor(ptr) == RED && getColor(ptr->parent) == RED) {
    parent = ptr->parent;
    grandparent = parent->parent;
    if (parent == grandparent->left) {
      TreeNode *uncle = grandparent->right;
      if (getColor(uncle) == RED) {
        setColor(uncle, BLACK);
        setColor(parent, BLACK);
        setColor(grandparent, RED);
        ptr = grandparent;
      } else {
        if (ptr == parent->right) {
          rotateLeft(parent);
          ptr = parent;
          parent = ptr->parent;
        }
        rotateRight(grandparent);
        std::swap(parent->color, grandparent->color);
        ptr = parent;
      }
    } else {
      TreeNode *uncle = grandparent->left;
      if (getColor(uncle) == RED) {
        setColor(uncle, BLACK);
        setColor(parent, BLACK);
        setColor(grandparent, RED);
        ptr = grandparent;
      } else {
        if (ptr == parent->left) {
          rotateRight(parent);
          ptr = parent;
          parent = ptr->parent;
        }
        rotateLeft(grandparent);
        std::swap(parent->color, grandparent->color);
        ptr = parent;
      }
    }
  }
  setColor(root, BLACK);
}

void RedBlackTree::rotateLeft(TreeNode *&ptr) {
  TreeNode *right_child = ptr->right;
  ptr->right = right_child->left;

  if (ptr->right != nullptr)
    ptr->right->parent = ptr;

  right_child->parent = ptr->parent;

  if (ptr->parent == nullptr)
    root = right_child;
  else if (ptr == ptr->parent->left)
    ptr->parent->left = right_child;
  else
    ptr->parent->right = right_child;

  right_child->left = ptr;
  ptr->parent = right_child;
}

void RedBlackTree::rotateRight(TreeNode *&ptr) {
  TreeNode *left_child = ptr->left;
  ptr->left = left_child->right;

  if (ptr->left != nullptr)
    ptr->left->parent = ptr;

  left_child->parent = ptr->parent;

  if (ptr->parent == nullptr)
    root = left_child;
  else if (ptr == ptr->parent->left)
    ptr->parent->left = left_child;
  else
    ptr->parent->right = left_child;

  left_child->right = ptr;
  ptr->parent = left_child;
}



/*
 * Delete Key
 * - deleteKey public
 * - deleteBST
 * - fixDeleteRBTree
 */
void RedBlackTree::deleteKey(KeyValueWrapper kv) {
    TreeNode *node = deleteBST(root, kv);  // Call the updated deleteBST with KeyValueWrapper
    fixDeleteRBTree(node);  // Fix red-black tree properties after deletion
}


TreeNode* RedBlackTree::deleteBST(TreeNode *&root, KeyValueWrapper kv) {
    if (root == nullptr) {
        return root;  // Base case: key not found
    }

    if (kv < root->keyValue) {
        // If the key is smaller, search in the left subtree
        return deleteBST(root->left, kv);
    }

    if (kv > root->keyValue) {
        // If the key is larger, search in the right subtree
        return deleteBST(root->right, kv);
    }

    // If the node has one child or no children
    if (root->left == nullptr || root->right == nullptr) {
        return root;
    }

    // If the node has two children, find the inorder successor (smallest in the right subtree)
    TreeNode *temp = minValueNode(root->right);

    // Replace the current node's key-value with the inorder successor's key-value
    root->keyValue = temp->keyValue;

    // Delete the inorder successor
    return deleteBST(root->right, temp->keyValue);
}


void RedBlackTree::fixDeleteRBTree(TreeNode *&node) {
    if (node == nullptr)
        return;

    if (node == root) {
        root = nullptr;
        return;
    }

    if (getColor(node) == RED || getColor(node->left) == RED || getColor(node->right) == RED) {
        TreeNode *child = node->left != nullptr ? node->left : node->right;

        if (node == node->parent->left) {
            node->parent->left = child;
            if (child != nullptr)
                child->parent = node->parent;
            setColor(child, BLACK);
            delete (node);
        } else {
            node->parent->right = child;
            if (child != nullptr)
                child->parent = node->parent;
            setColor(child, BLACK);
            delete (node);
        }
    } else {
        TreeNode *sibling = nullptr;
        TreeNode *parent = nullptr;
        TreeNode *ptr = node;
        setColor(ptr, DOUBLE_BLACK);
        while (ptr != root && getColor(ptr) == DOUBLE_BLACK) {
            parent = ptr->parent;
            if (ptr == parent->left) {
                sibling = parent->right;
                if (getColor(sibling) == RED) {
                    setColor(sibling, BLACK);
                    setColor(parent, RED);
                    rotateLeft(parent);
                } else {
                    if (getColor(sibling->left) == BLACK && getColor(sibling->right) == BLACK) {
                        setColor(sibling, RED);
                        if(getColor(parent) == RED)
                            setColor(parent, BLACK);
                        else
                            setColor(parent, DOUBLE_BLACK);
                        ptr = parent;
                    } else {
                        if (getColor(sibling->right) == BLACK) {
                            setColor(sibling->left, BLACK);
                            setColor(sibling, RED);
                            rotateRight(sibling);
                            sibling = parent->right;
                        }
                        setColor(sibling, parent->color);
                        setColor(parent, BLACK);
                        setColor(sibling->right, BLACK);
                        rotateLeft(parent);
                        break;
                    }
                }
            } else {
                sibling = parent->left;
                if (getColor(sibling) == RED) {
                    setColor(sibling, BLACK);
                    setColor(parent, RED);
                    rotateRight(parent);
                } else {
                    if (getColor(sibling->left) == BLACK && getColor(sibling->right) == BLACK) {
                        setColor(sibling, RED);
                        if (getColor(parent) == RED)
                            setColor(parent, BLACK);
                        else
                            setColor(parent, DOUBLE_BLACK);
                        ptr = parent;
                    } else {
                        if (getColor(sibling->left) == BLACK) {
                            setColor(sibling->right, BLACK);
                            setColor(sibling, RED);
                            rotateLeft(sibling);
                            sibling = parent->left;
                        }
                        setColor(sibling, parent->color);
                        setColor(parent, BLACK);
                        setColor(sibling->left, BLACK);
                        rotateRight(parent);
                        break;
                    }
                }
            }
        }
        if (node == node->parent->left)
            node->parent->left = nullptr;
        else
            node->parent->right = nullptr;
        delete(node);
        setColor(root, BLACK);
    }
}

TreeNode *RedBlackTree::minValueNode(TreeNode *&node) {

    TreeNode *ptr = node;

    while (ptr->left != nullptr)
        ptr = ptr->left;

    return ptr;
}

TreeNode* RedBlackTree::maxValueNode(TreeNode *&node) {
    TreeNode *ptr = node;

    while (ptr->right != nullptr)
        ptr = ptr->right;

    return ptr;
}

int RedBlackTree::getBlackHeight(TreeNode *node) {
    int blackheight = 0;
    while (node != nullptr) {
        if (getColor(node) == BLACK)
            blackheight++;
        node = node->left;
    }
    return blackheight;
}