#ifndef HUFFMANBUILDER_H
#define HUFFMANBUILDER_H

#include <vector>
#include <queue>
#include <cstdint>
#include "FrequencyTable.h"

struct Node {
    uint16_t symbol;
    uint32_t frequency;
    Node *left, *right;
    Node(uint16_t s, uint32_t f) : symbol(s), frequency(f), left(nullptr), right(nullptr) {}
    Node(Node* l, Node* r) : symbol(0), frequency(l->frequency + r->frequency), left(l), right(r) {}
    
    ~Node() {
        delete left;
        delete right;
    }
};

struct NodeCompare {
    bool operator()(Node* a, Node* b) {
        return a->frequency > b->frequency; 
    }
};

class HuffmanBuilder {
public:
    uint8_t codeLengths[65536];
    HuffmanBuilder() {
        for(int i=0; i<65536; i++) codeLengths[i] = 0;
    }
    void build(const FrequencyTable& freqTable) {
        std::priority_queue<Node*, std::vector<Node*>, NodeCompare> minHeap;
        for (int i = 0; i < FrequencyTable::MAX_SYMBOL; i++) {
            if (freqTable.counts[i] > 0) {
                minHeap.push(new Node((uint16_t)i, freqTable.counts[i]));
            }
        }
        if (minHeap.empty()) return;
        while (minHeap.size() > 1) {
            Node* left = minHeap.top(); minHeap.pop();
            Node* right = minHeap.top(); minHeap.pop();
            Node* parent = new Node(left, right);
            minHeap.push(parent);
        }

        Node* root = minHeap.top();
        assignDepths(root, 0);
        delete root; 
    }

private:
    void assignDepths(Node* node, uint8_t depth) {
        if (!node) return;

        if (!node->left && !node->right) {
            codeLengths[node->symbol] = depth;
        } else {
            assignDepths(node->left, depth + 1);
            assignDepths(node->right, depth + 1);
        }
    }
};

#endif