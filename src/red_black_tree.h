#pragma once

#include <vector>
#include <string>

enum Color { RED, BLACK };

struct Node {
    int keyOffset, keyLen;
    int value;

    Color color;
    int parent;
    int left;
    int right;

    Node() { }

    Node(int keyOffset, int keyLen, int value) 
        : keyOffset(keyOffset), keyLen(keyLen), value(value), 
            color(Color::RED), parent(-1), left(-1), right(-1)
    { }
};

class RedBlackTree {
public:
    RedBlackTree();
    RedBlackTree(int bufferSize);
    RedBlackTree(const std::string& filename);
    RedBlackTree(std::ifstream &file);

    void insert(int key, int keyLen, int value);
    int search(std::string key);

    void saveTreeToFile(const std::string& filename);
    void saveTreeToFile(std::ofstream &file);

    // void setKeyBuffer(char *keyBuffer);
    inline std::vector<char>& getKeyBuffer() {
        return m_keysBuffer;
    };

    void loadTreeFromFile(const std::string& filename);
    void loadTreeFromFile(std::ifstream &file);

    void printTree();

private:
    int m_root;
    std::vector<Node> m_nodeBuffer;
    std::vector<char> m_keysBuffer;

private:
    int key_compare(const char *key, int keyLen, int newNodeIndex);

    int allocateNode();
    void insertFixup(int newNodeIndex);
    void leftRotate(int nodeIndex);
    void rightRotate(int nodeIndex);
    void inOrderTraversal(int nodeIndex);
    void deserializeNodeBuffer(std::ifstream& file);
};
