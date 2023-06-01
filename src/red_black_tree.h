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

    Node() :
        keyOffset(-1), keyLen(-1), value(-1),
        color(Color::RED), parent(-1), left(-1), right(-1)
    { }

    Node(int keyOffset, int keyLen, int value) 
        : keyOffset(keyOffset), keyLen(keyLen), value(value), 
            color(Color::RED), parent(-1), left(-1), right(-1)
    { }
};

class RedBlackTree {
public:
    // RedBlackTree();

    void insert(const std::string &key, int value);
    int search(const std::string &key);

    void saveTreeToFile(std::ofstream &file);

    inline const std::vector<char>& getKeyBuffer() {
        return m_keyBuffer;
    };

    void loadTreeFromFile(std::ifstream &file);

    void printTree();

private:
    int m_root = -1;

    int m_nodeCount = 0;
    std::vector<Node> m_nodeBuffer;
    std::vector<char> m_keyBuffer;

private:
    int key_compare(const char *key, int keyLen, int newNodeIndex);

    int allocateNode();
    void insertFixup(int newNodeIndex);
    void leftRotate(int nodeIndex);
    void rightRotate(int nodeIndex);
    void inOrderTraversal(int nodeIndex);
    void deserializeNodeBuffer(std::ifstream& file);
};
