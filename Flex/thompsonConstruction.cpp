
#include <iostream>
#include <string> 
#include <vector>
#include <stack>
#include <unordered_map>
#include <algorithm>
#include <unordered_map>
#include <queue>
#include <unordered_set>




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
        void addtransition(char c, NFA_NODE *node)
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
            // Adding a getter function for transitions
        unordered_map<char, vector<NFA_NODE*>>& getTransitions() {
            return transitions;
        }

        // Adding a function to check if the node is final
        bool isFinalState() {
            return isFinal;
        }
};


class NFA 
{
    private:
        //bool isStart;
        NFA_NODE* StartState;
        vector<NFA_NODE*> FinalStates; 
        vector<NFA_NODE*> states;
    
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
            FinalStates.push_back(final);
            StartState = start;
            states.push_back(start);
            states.push_back(final);
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
            else if (oper == '*')
            {
                mergeLoop(nfa2);
            }
            
        }   
        void mergeOr(NFA nfa1, NFA nfa2)
        {
            NFA_NODE *newStart = new NFA_NODE(false);
            NFA_NODE *newFinal = new NFA_NODE(true);
            newStart->addtransition('\0', nfa1.StartState);
            newStart->addtransition('\0', nfa2.StartState);
            for(int i =0; i < nfa1.FinalStates.size(); i++)
            {
                nfa1.FinalStates[i]->addtransition('\0',newFinal);
            }
            for(int i =0; i < nfa2.FinalStates.size(); i++)
            {
                nfa2.FinalStates[i]->addtransition('\0',newFinal);
            }
            StartState = newStart;
            FinalStates.push_back(newFinal);
            states.push_back(StartState);
            for(int i =0; i < nfa1.states.size(); i++)
            {
                states.push_back(nfa1.states[i]);
            }
            for(int i =0; i < nfa2.states.size(); i++)
            {
                states.push_back(nfa2.states[i]);
            }
            states.push_back(newFinal);

        }

        void mergeConcat(NFA nfa1, NFA nfa2)
        {
            NFA_NODE *ConnectingState = new NFA_NODE(false);
            for(int i =0; i < nfa1.FinalStates.size(); i++)
            {
                nfa1.FinalStates[i]->addtransition('\0',ConnectingState);
            }

            ConnectingState->addtransition('\0',nfa2.StartState);

            for(int i =0; i < nfa2.FinalStates.size(); i++)
            {
                FinalStates.push_back(nfa2.FinalStates[i]);
            }
            StartState = nfa1.StartState;
            for(int i =0; i < nfa1.states.size(); i++)
            {
                states.push_back(nfa1.states[i]);
            }
            states.push_back(ConnectingState);
            for(int i =0; i < nfa2.states.size(); i++)
            {
                states.push_back(nfa2.states[i]);
            }
        }

        void mergeLoop(NFA nfa1)
        {

            NFA_NODE *newStart = new NFA_NODE(false);
            NFA_NODE *newFinal = new NFA_NODE(true);
            for(int i =0; i < nfa1.FinalStates.size(); i++)
            {
                nfa1.FinalStates[i]->addtransition('\0',newFinal);
            }
            newStart->addtransition('\0', nfa1.StartState);
            newFinal->addtransition('\0',newStart);
            newStart->addtransition('\0', newFinal);
            FinalStates.push_back(newFinal);
            StartState = newStart;
            states.push_back(StartState);
            for(int i =0; i < nfa1.states.size(); i++)
            {
                states.push_back(nfa1.states[i]);
            }
            states.push_back(newFinal);
        }

        NFA_NODE* getStartState() {
            return StartState;
        }

        vector<NFA_NODE*> getFinals()
        {
            return FinalStates;
        }

         vector<NFA_NODE*> getStates()
        {
            return states;
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

    NFA nfa(firstroot);

    for (int i = 0; i < nfa.getStates().size(); i++) {
        std::cout << "Node " << i << ":\n";
        for (int j = 0; j < nfa.getFinals().size(); j++) {
            if (nfa.getFinals()[j] == nfa.getStates()[i]) {
                cout << "This is a final state\n";
                break;
            }
        }
        for (const auto& pair : nfa.getStates()[i]->getTransitions()) {
            for (int j = 0; j < pair.second.size(); j++) {
                for (int k = 0; k < nfa.getStates().size(); k++) {
                    if (nfa.getStates()[k] == pair.second[j]) {
                        std::cout << "Key: " << pair.first << " transitions to node " << k << "\n";
                        break;
                    }
                }
            }
        }
        std::cout << "\n";
    }
    //cout << nfa.getStartState()->getTransitions()[0][0] << endl;

    /*
    cout << firstroot->right->data << endl;
    //cout << firstroot->right->left->data << endl;
    cout << firstroot->right->right->data << endl;
    cout << firstroot->right->right->left->data << endl;
    cout << firstroot->right->right->left->right->data << endl;
    cout << firstroot->right->right->left->left->data << endl;
    */

   return 0;
}


