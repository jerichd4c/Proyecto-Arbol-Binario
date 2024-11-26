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

// Encuentra al primer descendiente vivo en preorden
Node* findFirstLivingDescendant(Node* node) {
    if (!node) return nullptr;
    if (!node->is_dead) return node;

    Node* leftDescendant = findFirstLivingDescendant(node->left);
    if (leftDescendant) return leftDescendant;

    return findFirstLivingDescendant(node->right);
}

// Encuentra al primer hermano vivo
Node* findLivingSibling(Node* parent, Node* current) {
    if (!parent) return nullptr;

    if (parent->left && parent->left != current && !parent->left->is_dead)
        return parent->left;
    if (parent->right && parent->right != current && !parent->right->is_dead)
        return parent->right;

    return nullptr;
}

// Encuentra al primer ancestro con hijos vivos
Node* findLivingAncestorWithChildren(Node* root, Node* current) {
    if (!root) return nullptr;

    // Verificar si el ancestro tiene dos hijos vivos
    if ((root->left && !root->left->is_dead) || (root->right && !root->right->is_dead)) {
        return root;
    }

    Node* leftSearch = findLivingAncestorWithChildren(root->left, current);
    if (leftSearch) return leftSearch;

    return findLivingAncestorWithChildren(root->right, current);
}

// Asignar al nuevo rey
Node* assignNewKing(Node* root, Node* current) {
    if (!root || !current) return nullptr;

    // Verificar descendientes vivos
    Node* newKing = findFirstLivingDescendant(current->left);
    if (newKing) return newKing;

    // Verificar hermanos vivos
    newKing = findLivingSibling(root, current);
    if (newKing) return newKing;

    // Verificar tíos vivos
    if (root->id_father != -1) {  // Verificar si el padre tiene hermanos
        Node* grandparent = findFirstLivingDescendant(root->left);  // Buscar el abuelo
        if (grandparent) {
            newKing = findLivingSibling(grandparent, root);
            if (newKing) return newKing;
        }
    }

    // Verificar ancestros con hijos vivos
    newKing = findLivingAncestorWithChildren(root, current);
    return newKing;
}

// Actualizar el estado del rey actual y del nuevo rey
void updateKingStatus(Node* root, Node* current, Node* newKing) {
    if (current) current->is_king = false; // Eliminar el estatus de rey
    if (newKing) {
        newKing->is_king = true;   // Asignar nuevo rey
        newKing->was_king = true; // Marcar que fue rey
    }
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

// Buscar nodo por ID
Node* findNodeByID(Node* root, int id) {
    if (!root) return nullptr;
    if (root->id == id) return root;

    Node* leftResult = findNodeByID(root->left, id);
    if (leftResult) return leftResult;

    return findNodeByID(root->right, id);
}

// Validar y modificar los datos de un nodo
void modifyNode(Node* root) {
    if (!root) {
        std::cout << "El árbol está vacío.\n";
        return;
    }

    int id;
    std::cout << "Ingrese el ID del nodo a modificar: ";
    std::cin >> id;

    Node* node = findNodeByID(root, id);
    if (!node) {
        std::cout << "No se encontró un nodo con el ID especificado.\n";
        return;
    }

    std::cout << "Nodo encontrado. Datos actuales:\n";
    std::cout << "ID: " << node->id << ", Nombre: " << node->name << " " << node->last_name
              << ", Género: " << node->gender << ", Edad: " << node->age
              << ", Vivo: " << (node->is_dead ? "No" : "Sí")
              << ", Rey actual: " << (node->is_king ? "Sí" : "No")
              << ", Fue rey: " << (node->was_king ? "Sí" : "No") << "\n";

    int option = 0;
    do {
        std::cout << "\nCampos Modificables:\n";
        std::cout << "1. Nombre\n";
        std::cout << "2. Apellido\n";
        std::cout << "3. Género\n";
        std::cout << "4. Edad\n";
        std::cout << "5. Estado de vida\n";
        std::cout << "6. Estado de rey actual\n";
        std::cout << "7. Salir\n";
        std::cout << "Seleccione un campo para modificar: ";
        std::cin >> option;

        switch (option) {
            case 1: {
                std::string newName;
                std::cout << "Ingrese el nuevo nombre: ";
                std::cin >> newName;
                node->name = newName;
                break;
            }
            case 2: {
                std::string newLastName;
                std::cout << "Ingrese el nuevo apellido: ";
                std::cin >> newLastName;
                node->last_name = newLastName;
                break;
            }
            case 3: {
                char newGender;
                do {
                    std::cout << "Ingrese el nuevo género (H/M): ";
                    std::cin >> newGender;
                    newGender = toupper(newGender);
                } while (newGender != 'H' && newGender != 'M');
                node->gender = newGender;
                break;
            }
            case 4: {
                int newAge;
                do {
                    std::cout << "Ingrese la nueva edad (valor positivo): ";
                    std::cin >> newAge;
                } while (newAge <= 0);
                node->age = newAge;
                break;
            }
            case 5: {
                int newIsDead;
                do {
                    std::cout << "Ingrese el estado de vida (0: Vivo, 1: Muerto): ";
                    std::cin >> newIsDead;
                } while (newIsDead != 0 && newIsDead != 1);
                node->is_dead = newIsDead;
                break;
            }
            case 6: {
                int newIsKing;
                do {
                    std::cout << "Ingrese el estado de rey actual (0: No es rey, 1: Es rey): ";
                    std::cin >> newIsKing;
                } while (newIsKing != 0 && newIsKing != 1);

                if (newIsKing == 1) {
                    // Asegurar que no haya múltiples reyes vivos
                    Node* currentKing = findFirstLivingDescendant(root);
                    if (currentKing && currentKing->is_king) {
                        std::cout << "Error: Ya existe un rey vivo. No se puede asignar este nodo como rey.\n";
                        break;
                    }
                }
                node->is_king = newIsKing;
                break;
            }
            case 7:
                std::cout << "Saliendo de la modificación de nodos.\n";
                break;
            default:
                std::cout << "Opción inválida. Intente nuevamente.\n";
        }

    } while (option != 7);
}

// Función para escribir el árbol binario en el archivo CSV
void updateCSV(Node* root, const std::string& filename) {
    if (!root) {
        std::cout << "El árbol está vacío, no hay datos para guardar.\n";
        return;
    }

    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error al abrir el archivo: " << filename << "\n";
        return;
    }

    // Recorrido preorden para guardar los datos
    //writeNodeToFile(root, file);
    file.close();
    std::cout << "Datos actualizados en el archivo CSV correctamente.\n";
}

// Recorrer el árbol y escribir cada nodo en el archivo
void writeNodeToFile(Node* node, std::ofstream& file) {
    if (!node) return;

    // Escribir datos del nodo en formato CSV
    file << node->id << ";"
         << node->name << ";"
         << node->last_name << ";"
         << node->gender << ";"
         << node->age << ";"
         << (node->id_father == -1 ? "" : std::to_string(node->id_father)) << ";"
         << node->is_dead << ";"
         << node->was_king << ";"
         << node->is_king << "\n";

    // Recursivamente escribir hijos
    writeNodeToFile(node->left, file);
    writeNodeToFile(node->right, file);
}

int main() {
    Node* root = readCSV("family_tree_ordered.csv");
    if (!root) return 1;

    int option = 0;
    do {
        std::cout << "\nMenú Principal\n";
        std::cout << "1. Mostrar línea de sucesión\n";
        std::cout << "2. Asignar nuevo rey automáticamente\n";
        std::cout << "3. Salir\n";
        std::cout << "Seleccione una opción: ";
        std::cin >> option;

        switch (option) {
            case 1:
                std::cout << "\nLínea de sucesión actual (familiares vivos):\n";
                printSuccessionLine(root);
                break;

            case 2: {
                Node* currentKing = findFirstLivingDescendant(root);
                if (currentKing && currentKing->is_king && currentKing->is_dead) {
                    Node* newKing = assignNewKing(root, currentKing);
                    if (newKing) {
                        updateKingStatus(root, currentKing, newKing);
                        std::cout << "Nuevo rey asignado: " << newKing->name << " " << newKing->last_name << "\n";
                    } else {
                        std::cout << "No se pudo asignar un nuevo rey.\n";
                    }
                } else {
                    std::cout << "El rey actual sigue vivo o no se encuentra disponible.\n";
                }
                break;
            }
            case 3:
                std::cout << "Modificar datos de un nodo.\n";
                modifyNode(root);
                break;
            case 4:
                std::cout << "Guardando cambios en el archivo CSV...\n";
                updateCSV(root, "family_tree_ordered.csv");
                std::cout << "Saliendo del programa.\n";
                break;
            case 5:
                std::cout << "Saliendo del programa.\n";
                break;
            default:
                std::cout << "Opción inválida. Intente nuevamente.\n";
        }
    } while (option != 4);

    return 0;
}
