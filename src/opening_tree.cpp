#include "h_files/opening_tree.h"

#include <vector>
#include <string>
#include <iostream>


std::string traverse_tree(Node* root, std::vector<std::string> move_history, int move_count){
    if (!move_history.size()){
        return root->child_strings[0];
    }

    for (int i = 0; i < 3; i++){
        if (root->child_strings[i] == move_history[move_count]){

            return traverse_tree(root->child_nodes[i], move_history, move_count+1);
        }
    }

    //break out of openings when 
    return "?";
}

Node* fill_tree() {
    std::string e2e4_child_strings1[3] = {"c7c5", "e7e5", "g8f6"};
    Node* e2e4_child_nodes1[3] = {nullptr, nullptr, nullptr};
    Node* e2e4_node1 = new Node(e2e4_child_strings1, e2e4_child_nodes1);

    std::string e2e4_child_strings2[3] = {"d2d4", "b1c3", "f1b5"};
    Node* e2e4_child_nodes2[3] = {nullptr, nullptr, nullptr};
    Node* e2e4_node2 = new Node(e2e4_child_strings2, e2e4_child_nodes2);

    Node* e2e4_nodes[3] = {e2e4_node1, e2e4_node2, nullptr};
    std::string e2e4_strings[3] = {"e2e4", "", ""};
    Node* e2e4_root = new Node(e2e4_strings, e2e4_nodes);

    // d2d4 Opening Branch
    std::string d2d4_child_strings1[3] = {"d7d5", "g8f6", "c7c5"};
    Node* d2d4_child_nodes1[3] = {nullptr, nullptr, nullptr};
    Node* d2d4_node1 = new Node(d2d4_child_strings1, d2d4_child_nodes1);

    std::string d2d4_child_strings2[3] = {"c2c4", "g1f3", "e2e3"};
    Node* d2d4_child_nodes2[3] = {nullptr, nullptr, nullptr};
    Node* d2d4_node2 = new Node(d2d4_child_strings2, d2d4_child_nodes2);

    Node* d2d4_nodes[3] = {d2d4_node1, d2d4_node2, nullptr};
    std::string d2d4_strings[3] = {"d2d4", "", ""};
    Node* d2d4_root = new Node(d2d4_strings, d2d4_nodes);

    // c2c4 Opening Branch
    std::string c2c4_child_strings1[3] = {"e7e5", "g8f6", "c7c5"};
    Node* c2c4_child_nodes1[3] = {nullptr, nullptr, nullptr};
    Node* c2c4_node1 = new Node(c2c4_child_strings1, c2c4_child_nodes1);

    std::string c2c4_child_strings2[3] = {"d2d4", "g1f3", "b1c3"};
    Node* c2c4_child_nodes2[3] = {nullptr, nullptr, nullptr};
    Node* c2c4_node2 = new Node(c2c4_child_strings2, c2c4_child_nodes2);

    Node* c2c4_nodes[3] = {c2c4_node1, c2c4_node2, nullptr};
    std::string c2c4_strings[3] = {"c2c4", "", ""};
    Node* c2c4_root = new Node(c2c4_strings, c2c4_nodes);

    // Root Node
    std::string root_child_strings[3] = {"e2e4", "d2d4", "c2c4"};
    Node* root_child_nodes[3] = {e2e4_root, d2d4_root, c2c4_root};
    Node* root = new Node(root_child_strings, root_child_nodes);

    return root;
}

//to put into main
int main() {
    Node* root = fill_tree();
    

    std::vector<std::string> move_history = {"e2e4", "c2c4"};
    if (move_history.size() < our_depth){
        std::string result = traverse_tree(root, move_history, 0);
    }
    
}

Node::Node(std::string child_strings[3], Node* child_nodes[3]){

}

