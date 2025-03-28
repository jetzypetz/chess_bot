#include <string>

struct Node {

    std::string child_strings[3];
    Node* child_nodes[3];

    Node(std::string child_strings[3], Node* child_nodes[3]);
};

std::string traverse_tree(Node* root, std::string move);
Node* fill_tree();

// root node - history at 0
// if first move in the children node
// recursively go through nodes for each further children

// e2e4, d2d4, Ng1f3