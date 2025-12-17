#include <iostream>
#include <cstring>
#include <cstdlib>

#define MAX_INPUT_SIZE 1024 

struct Node {
    char* str;     
    Node* next;    
};

void appendNode(Node** head, const char* input) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    if (!newNode) {
        std::cerr << "Memory allocation failed for node" << std::endl;
        exit(1);
    }

    size_t len = strlen(input);
    newNode->str = (char*)malloc(len + 1);
    if (!newNode->str) {
        std::cerr << "Memory allocation failed for string" << std::endl;
        free(newNode);
        exit(1);
    }

    strcpy(newNode->str, input);
    newNode->next = nullptr;

    if (*head == nullptr) {
        *head = newNode;
        return;
    }

    Node* current = *head;
    while (current->next != nullptr) {
        current = current->next;
    }

    current->next = newNode;
}


void printList(Node* head) {
    Node* current = head;
    while (current != nullptr) {
        std::cout << current->str;
        current = current->next;
    }
}


void freeList(Node* head) {
    Node* current = head;
    while (current != nullptr) {
        Node* temp = current;
        current = current->next;
        free(temp->str); 
        free(temp);     
    }
}

int main() {
    Node* head = nullptr; 
    char buffer[MAX_INPUT_SIZE]; 

    std::cout << "Enter strings (start with '.' to stop):\n";

    while (true) {

        if (!fgets(buffer, MAX_INPUT_SIZE, stdin)) {
            std::cerr << "Error reading input" << std::endl;
            freeList(head);
            return 1;
        }

        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
            len--;
        }

        if (len > 0 && buffer[0] == '.') {
            break;
        }

        if (len > 0) {
            appendNode(&head, buffer);
        }
    }

    
    std::cout << "\nStored strings:";
    printList(head);

    
    freeList(head);

    return 0;
}