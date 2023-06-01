#include <bits/types/FILE.h>
#include <cassert>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <fstream>

#include "red_black_tree.h"

RedBlackTree::RedBlackTree() 
    : m_root(-1)
{}

RedBlackTree::RedBlackTree(int bufferSize) 
    : RedBlackTree()
{
    m_nodeBuffer.resize(bufferSize);
};

RedBlackTree::RedBlackTree(std::ifstream &file)
    : RedBlackTree()
{
    loadTreeFromFile(file);
}

RedBlackTree::RedBlackTree(const std::string& filename) 
    : RedBlackTree()
{
    loadTreeFromFile(filename);
}

int RedBlackTree::key_compare(const char *key, int keyLen, int newNodeIndex) {
    return std::strncmp(key, &m_keysBuffer[m_nodeBuffer[newNodeIndex].keyOffset], keyLen);
}

void RedBlackTree::insert(int keyOffset, int keyLen, int value) 
{
    const char *key = &m_keysBuffer[keyOffset];


    m_nodeBuffer.push_back({keyOffset, keyLen, value});
    int newNodeIndex = m_nodeBuffer.size()-1;
    Node *nd = &m_nodeBuffer[newNodeIndex];

    // BST-style insertion
    int currentNodeIndex = m_root;
    int parentNodeIndex = -1;
    while (currentNodeIndex != -1) {
        parentNodeIndex = currentNodeIndex;
        if (key_compare(key, nd->keyLen, currentNodeIndex) < 0)  {
            currentNodeIndex = m_nodeBuffer[currentNodeIndex].left;
        } else {
            currentNodeIndex = m_nodeBuffer[currentNodeIndex].right;
        }
    }

    nd->parent = parentNodeIndex;
    if (parentNodeIndex == -1) {
        m_root = newNodeIndex;  // Tree is empty
    } else if (key_compare(key, nd->keyLen, parentNodeIndex) < 0) {
        m_nodeBuffer[parentNodeIndex].left = newNodeIndex;
    } else {
        m_nodeBuffer[parentNodeIndex].right = newNodeIndex;
    }

    insertFixup(newNodeIndex);
}

int RedBlackTree::search(std::string key) {
    int currentNodeIndex = m_root;
    while (currentNodeIndex != -1 && key_compare(key.c_str(), key.length(), currentNodeIndex) != 0) {
        if (key_compare(key.c_str(), key.length(), currentNodeIndex) < 0) {
            currentNodeIndex = m_nodeBuffer[currentNodeIndex].left;
        } else {
            currentNodeIndex = m_nodeBuffer[currentNodeIndex].right;
        }
    }
    if (currentNodeIndex == -1) 
        return -1;
    return m_nodeBuffer[currentNodeIndex].value;
}

void RedBlackTree::printTree() 
{
    inOrderTraversal(m_root);
}

void RedBlackTree::saveTreeToFile(const std::string& filename) {


    std::ofstream file(filename, std::ios::binary);
    if (file.is_open()) {
        saveTreeToFile(file);
        std::cout << "Tree saved to file: " << filename << std::endl;
    } else {
        std::cout << "Unable to open file: " << filename << std::endl;
    }
}

void RedBlackTree::saveTreeToFile(std::ofstream &file) {
    assert(file.is_open());
    
    fprintf(stdout, "hey\n");
    fprintf(stdout, "A: %u\n", m_root);
    file.write(reinterpret_cast<const char*>(&m_root), sizeof(m_root));
    fprintf(stdout, "A: %u\n", m_root);

    int bufferSize = m_nodeBuffer.size();
    file.write(reinterpret_cast<const char*>(&bufferSize), sizeof(bufferSize));
    file.write(reinterpret_cast<const char*>(&m_nodeBuffer[0]), sizeof(Node) * bufferSize);
    file.close();
}

void RedBlackTree::loadTreeFromFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (file.is_open()) {
        loadTreeFromFile(file);
        std::cout << "Tree loaded from file: " << filename << std::endl;
    } else {
        std::cout << "Unable to open file: " << filename << std::endl;
    }
}

void RedBlackTree::loadTreeFromFile(std::ifstream &file) {
    file.read(reinterpret_cast<char*>(&m_root), sizeof(m_root));

    int bufferSize;
    file.read(reinterpret_cast<char*>(&bufferSize), sizeof(bufferSize));
    m_nodeBuffer.resize(bufferSize);
    file.read(reinterpret_cast<char*>(&m_nodeBuffer[0]), sizeof(Node) * bufferSize);
    file.close();
}

void RedBlackTree::insertFixup(int newNodeIndex) 
{
    while (m_nodeBuffer[newNodeIndex].parent != -1 && m_nodeBuffer[m_nodeBuffer[newNodeIndex].parent].color == RED) {
        int parentIndex = m_nodeBuffer[newNodeIndex].parent;
        int grandparentIndex = m_nodeBuffer[parentIndex].parent;

        if (parentIndex == m_nodeBuffer[grandparentIndex].left) {
            int uncleIndex = m_nodeBuffer[grandparentIndex].right;

            if (uncleIndex != -1 && m_nodeBuffer[uncleIndex].color == RED) {
                m_nodeBuffer[parentIndex].color = BLACK;
                m_nodeBuffer[uncleIndex].color = BLACK;
                m_nodeBuffer[grandparentIndex].color = RED;
                newNodeIndex = grandparentIndex;
            } else {
                if (newNodeIndex == m_nodeBuffer[parentIndex].right) {
                    newNodeIndex = parentIndex;
                    leftRotate(newNodeIndex);
                    parentIndex = m_nodeBuffer[newNodeIndex].parent;
                }

                m_nodeBuffer[parentIndex].color = BLACK;
                m_nodeBuffer[grandparentIndex].color = RED;
                rightRotate(grandparentIndex);
            }
        } else {
            int uncleIndex = m_nodeBuffer[grandparentIndex].left;

            if (uncleIndex != -1 && m_nodeBuffer[uncleIndex].color == RED) {
                m_nodeBuffer[parentIndex].color = BLACK;
                m_nodeBuffer[uncleIndex].color = BLACK;
                m_nodeBuffer[grandparentIndex].color = RED;
                newNodeIndex = grandparentIndex;
            } else {
                if (newNodeIndex == m_nodeBuffer[parentIndex].left) {
                    newNodeIndex = parentIndex;
                    rightRotate(newNodeIndex);
                    parentIndex = m_nodeBuffer[newNodeIndex].parent;
                }

                m_nodeBuffer[parentIndex].color = BLACK;
                m_nodeBuffer[grandparentIndex].color = RED;
                leftRotate(grandparentIndex);
            }
        }
    }

    m_nodeBuffer[m_root].color = BLACK;
}

void RedBlackTree::leftRotate(int nodeIndex) 
{
    int rightChildIndex = m_nodeBuffer[nodeIndex].right;

    m_nodeBuffer[nodeIndex].right = m_nodeBuffer[rightChildIndex].left;
    if (m_nodeBuffer[rightChildIndex].left != -1)
        m_nodeBuffer[m_nodeBuffer[rightChildIndex].left].parent = nodeIndex;

    m_nodeBuffer[rightChildIndex].parent = m_nodeBuffer[nodeIndex].parent;
    if (m_nodeBuffer[nodeIndex].parent == -1)
        m_root = rightChildIndex;
    else if (nodeIndex == m_nodeBuffer[m_nodeBuffer[nodeIndex].parent].left)
        m_nodeBuffer[m_nodeBuffer[nodeIndex].parent].left = rightChildIndex;
    else
        m_nodeBuffer[m_nodeBuffer[nodeIndex].parent].right = rightChildIndex;

    m_nodeBuffer[rightChildIndex].left = nodeIndex;
    m_nodeBuffer[nodeIndex].parent = rightChildIndex;
}

void RedBlackTree::rightRotate(int nodeIndex) 
{
    int leftChildIndex = m_nodeBuffer[nodeIndex].left;

    m_nodeBuffer[nodeIndex].left = m_nodeBuffer[leftChildIndex].right;
    if (m_nodeBuffer[leftChildIndex].right != -1)
        m_nodeBuffer[m_nodeBuffer[leftChildIndex].right].parent = nodeIndex;

    m_nodeBuffer[leftChildIndex].parent = m_nodeBuffer[nodeIndex].parent;
    if (m_nodeBuffer[nodeIndex].parent == -1)
        m_root = leftChildIndex;
    else if (nodeIndex == m_nodeBuffer[m_nodeBuffer[nodeIndex].parent].right)
        m_nodeBuffer[m_nodeBuffer[nodeIndex].parent].right = leftChildIndex;
    else
        m_nodeBuffer[m_nodeBuffer[nodeIndex].parent].left = leftChildIndex;

    m_nodeBuffer[leftChildIndex].right = nodeIndex;
    m_nodeBuffer[nodeIndex].parent = leftChildIndex;
}

void RedBlackTree::inOrderTraversal(int nodeIndex)
{
    if (nodeIndex != -1) {
        inOrderTraversal(m_nodeBuffer[nodeIndex].left);
        // std::cout <<  << " ";
        fprintf(stdout, "¤ %u %.*s\n",  m_nodeBuffer[nodeIndex].keyLen, m_nodeBuffer[nodeIndex].keyLen, &m_keysBuffer[m_nodeBuffer[nodeIndex].keyOffset]);

        inOrderTraversal(m_nodeBuffer[nodeIndex].right);
    }
}

/* 
int main() {
    const char *keysBuffer = "A\nBaa\nC\nBbb\nD\nE\nF\n";
    RedBlackTree tree(keysBuffer, 100);

    tree.insert(16, 1); // F
    tree.insert(6 , 2);  // C
    tree.insert(8 , 3);  // Bbb
    tree.insert(2 , 4);  // Baa

    tree.printTree();

    int value = tree.search("Baa");
    fprintf(stdout, "%u\n", value);


    return 0;
}
*/
