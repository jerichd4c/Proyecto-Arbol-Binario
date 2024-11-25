#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

// Nodo para el árbol binario
struct Node {
    int id;
    std::string name;
    std::string last_name;
    char gender;
    int age;
    int id_father;
    bool is_dead;
    bool was_king;
    bool is_king;
    Node* left;
    Node* right;

    Node(int i, const std::string& n, const std::string& ln, char g, int a, int idf, bool dead, bool wk, bool ik)
        : id(i), name(n), last_name(ln), gender(g), age(a), id_father(idf), is_dead(dead), was_king(wk), is_king(ik), left(nullptr), right(nullptr) {}
};

// Función para leer el .csv y crear el árbol binario
Node* readCSV(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: No se pudo abrir el archivo " << filename << "\n";
        return nullptr;
    }

    Node* root = nullptr;
    Node* nodes[21] = {nullptr}; // Máximo 20 familiares + 1 (índice base 1)
    std::string line;

    while (std::getline(file, line)) {
        std::istringstream ss(line);
        std::string word;
        
        int id;
        std::string name;
        std::string last_name;
        char gender;
        int age;
        int id_father;
        bool is_dead, was_king, is_king;

        // Leer línea separada por ';'
        std::getline(ss, word, ';');
        id = std::stoi(word);

        std::getline(ss, name, ';');
        std::getline(ss, last_name, ';');

        std::getline(ss, word, ';');
        gender = word[0];

        std::getline(ss, word, ';');
        age = std::stoi(word);

        std::getline(ss, word, ';');
        id_father = word.empty() ? -1 : std::stoi(word);

        std::getline(ss, word, ';');
        is_dead = std::stoi(word);

        std::getline(ss, word, ';');
        was_king = std::stoi(word);

        std::getline(ss, word, ';');
        is_king = std::stoi(word);

        // Crear nodo
        Node* newNode = new Node(id, name, last_name, gender, age, id_father, is_dead, was_king, is_king);
        nodes[id] = newNode;

        if (id_father == -1) {
            root = newNode; // Asignar raíz
        } else {
            Node* fatherNode = nodes[id_father];
            if (!fatherNode) {
                std::cerr << "Error: ID del padre no encontrado para el nodo con ID " << id << "\n";
                delete newNode;
                continue;
            }

            if (!fatherNode->left) {
                fatherNode->left = newNode;
            } else if (!fatherNode->right) {
                fatherNode->right = newNode;
            } else {
                std::cerr << "Error: Más de dos hijos asignados al nodo con ID " << id_father << "\n";
                delete newNode;
                continue;
            }
        }
    }

    file.close();
    return root;
}

// Función para imprimir los datos del árbol en orden de sucesión
void printSuccessionLine(Node* root) {
    if (root == nullptr) return;

    if (!root->is_dead) {
        std::cout << "ID: " << root->id << ", Name: " << root->name << " " << root->last_name
                  << ", Age: " << root->age << "\n";
    }

    printSuccessionLine(root->left);
    printSuccessionLine(root->right);
}

int main() {
    Node* root = readCSV("family_tree_ordered.csv");
    if (!root) return 1;

    std::cout << "\nLínea de sucesión actual (familiares vivos):\n";
    printSuccessionLine(root);

    return 0;
}
