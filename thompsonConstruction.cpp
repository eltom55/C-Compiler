
#include <iostream>
#include <string> 
#include <vector>
#include <stack>
#include <unordered_map>
#include <algorithm>
#include <unordered_map>



using namespace std; 

//1) this part is to get the postfix because its used to make the tree
std::string reverseString(const std::string& str) {
    std::string reversed = str; // Create a copy of the input string
    std::reverse(reversed.begin(), reversed.end()); // Reverse the copy
    return reversed;
}

bool isOperand(char token) {
    return (token >= 'a' && token <= 'z') || (token >= 'A' && token <= 'Z');
}

// Function to get the precedence of an operator
int getPrecedence(char op) {
    if (op == '|')
        return 1;
    if (op == '+')
        return 2;
    if (op == '*')
        return 3;
    return 0; // Default precedence for other characters
}

// Function to convert infix regular expression to postfix
string infixToPostfix(const string& infix) {
    stack<char> operatorStack;
    vector<char> postfix;
    
    for (char token : infix) {
        if (isOperand(token)) {
            postfix.push_back(token);
        } 
        else if (token == '(') {
            operatorStack.push(token);
        } 
        else if (token == ')') {
            while (!operatorStack.empty() && operatorStack.top() != '(') {
                postfix.push_back(operatorStack.top());
                operatorStack.pop();
            }
            if (!operatorStack.empty() && operatorStack.top() == '(') {
                operatorStack.pop(); // Pop and discard '('
            } 
            else {
                
                cerr << "Error: Mismatched parentheses." << endl;
                return "";
            }
        } 
        else { 
            while (!operatorStack.empty() && operatorStack.top() != '(' && getPrecedence(token) <= getPrecedence(operatorStack.top())) {
                postfix.push_back(operatorStack.top());
                operatorStack.pop();
            }
            operatorStack.push(token);
        }
    }
    
    while (!operatorStack.empty()) {
        if (operatorStack.top() == '(') {
            cerr << "Error: Mismatched parentheses." << endl;
            return "";
        }
        postfix.push_back(operatorStack.top());
        operatorStack.pop();
    }
    
    return string(postfix.begin(), postfix.end());
}

// This is a tree data structure that helps build the expression tree
struct TreeNode {
    char data;
    TreeNode* left;
    TreeNode* right;
    TreeNode* parent;

    TreeNode(char val) : data(val), left(nullptr), right(nullptr), parent(nullptr) {}
};

TreeNode* insert(TreeNode* root, char data, TreeNode* parent = nullptr) {
    if (root == nullptr) {
        TreeNode* newNode = new TreeNode(data);
        newNode->parent = parent; // Set the parent pointer for the new node
        return newNode;
    }
    // If the current node is an operator ('|', '+', '*'), insert to the right subtree
    if ((root->data == '|' || root->data == '+' || root->data == '*') && root->right == nullptr) {
        root->right = insert(root->right, data, root);
        return root->right;
    }
    // If the current node is an operand or '*' with one child, insert to the left subtree
    else {
        if (parent->left == nullptr && parent->data != '*') {
            parent->left = insert(parent->left, data, parent);
            return parent->left;
        }
        else {
            while (parent->left != nullptr || parent->data == '*') {
                parent = parent->parent;
            }
            parent->left = insert(parent->left, data, parent);
            return parent->left;
        }
    }

    return root;
}

void printTree(TreeNode* root, int space = 0, int indent = 4) {
    if (root == nullptr) {
        return;
    }

    space += indent;

    printTree(root->right, space);

    cout << endl;
    for (int i = indent; i < space; i++) {
        cout << " ";
    }
    cout << root->data << "\n";

    printTree(root->left, space);
}


class NFA_NODE
{
    private:
        unordered_map<char, vector<NFA_NODE*>> transitions;
        bool isFinal = false;

    public:

        NFA_NODE(bool final) {isFinal = final;}

        NFA_NODE(char c, NFA_NODE *node, bool final)
        {
            isFinal = final;
            transitions.insert({c, vector<NFA_NODE*>({node})});
        }
        NFA_NODE addtransition(char c, NFA_NODE *node)
        {
            if(transitions.find(c) == transitions.end())
            {
                transitions.insert({c, vector<NFA_NODE*>({node})});
            }
            else
            {
                transitions[c].push_back(node);
            }
        }

};


class NFA 
{
    private:
        //bool isStart;
        NFA_NODE* StartState;
    
    public:
        NFA() {};

        NFA(TreeNode* root)
        {
            NFA left;
            NFA right;

            if (root->left != nullptr) {
                left = NFA(root->left);
            }
            if (root->right != nullptr) {
                right = NFA(root->right);
            }

            if(isOperand(root->data))
                makeNFAFromChar(root->data);
            else
                makeNFAFromOp(left, right, root->data);
        }

        void makeNFAFromChar(char c)
        {
            NFA_NODE *final = new NFA_NODE(true);
            NFA_NODE *start = new NFA_NODE(c, final, false);

            StartState = start;
        }

        void makeNFAFromOp(NFA nfa1, NFA nfa2, char oper)
        {
            if(oper == '|')
            {
                mergeOr(nfa1,nfa2);
            }
            else if (oper == '+')
            {
                mergeConcat(nfa1,nfa2);
            }
            else
            {
                mergeLoop(nfa1, nfa2);
            }
            
        }   
        void mergeOr(NFA nfa1, NFA nfa2)
        {
            NFA_NODE *newStart = new NFA_NODE(false);
            newStart->addtransition('\0', nfa1.StartState);
            newStart->addtransition('\0', nfa2.StartState);

            StartState = newStart;

        }

        void mergeConcat(NFA nfa1, NFA nfa2)
        {
            
        }

        void mergeLoop(NFA nfa1, NFA nfa2)
        {
            
        }
};



int main()
{
    cout << "Enter a Regular Expression: ";
    string input;
    cin >> input;
    string expressionstr = infixToPostfix(input);
    string expression = reverseString(expressionstr);
    cout<< expression << endl;
    //char expression[] = "+*|b|dc|ea";
    TreeNode* root = nullptr;
    TreeNode* firstroot = nullptr;
    
    char c;
    for (int i = 0; i < expression.size(); i++) {
        c = expression[i];
        if (root == nullptr) {
            root = insert(root, c, nullptr);
            firstroot = root;
        }
        else {
            root = insert(root, c, root->parent);
        }
    }

    //(a|e)+(c|d|b)*  
    cout << "Tree Structure:" << endl;
    //printTree(firstroot);
    cout << firstroot->right->data << endl;
    //cout << firstroot->right->left->data << endl;
    cout << firstroot->right->right->data << endl;
    cout << firstroot->right->right->left->data << endl;
    cout << firstroot->right->right->left->right->data << endl;
    cout << firstroot->right->right->left->left->data << endl;
    return 0;
}


