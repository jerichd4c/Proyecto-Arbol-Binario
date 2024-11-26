#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>

// Definición del nodo del árbol
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

    Node(int i, std::string n, std::string ln, char g, int a, int idf, bool dead, bool wk, bool ik)
        : id(i), name(n), last_name(ln), gender(g), age(a), id_father(idf), is_dead(dead), was_king(wk), is_king(ik), left(nullptr), right(nullptr) {}
};

// Prototipos de funciones
Node* readCSV(const std::string& filename);
bool validateCSVLine(const std::string& line, int expected_columns);
void printSuccessionLine(Node* root);
Node* findSuccessor(Node* root);
Node* findFirstLivingDescendant(Node* node);
Node* findSibling(Node* root);
Node* findUncle(Node* root);
Node* findAncestorWithMultipleChildren(Node* root);
void updateCSV(Node* root, const std::string& filename);
void writeNodeToFile(Node* node, std::ofstream& file);
void modifyNode(Node* root);
Node* findNodeByID(Node* root, int id);
void runTests(Node* root);
void createComplexCSV(const std::string& filename);
void simulateComplexRun();

// Función para leer el CSV y construir el árbol binario
Node* readCSV(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error al abrir el archivo CSV.\n";
        return nullptr;
    }

    Node* root = nullptr;
    Node* nodes[21] = {nullptr}; // Arreglo para guardar nodos por su ID (máximo 20 nodos)

    std::string line;
    bool isFirstLine = true;
    const int expected_columns = 9;

    while (std::getline(file, line)) {
        if (isFirstLine) { // Saltar los headers
            isFirstLine = false;
            continue;
        }

        if (!validateCSVLine(line, expected_columns)) {
            std::cerr << "Línea inválida encontrada: " << line << "\n";
            continue; // Ignorar líneas inválidas
        }

        std::stringstream ss(line);
        std::string word;
        std::string data[9];
        int index = 0;

        while (std::getline(ss, word, ';') && index < 9) {
            data[index++] = word;
        }

        int id = std::stoi(data[0]);
        std::string name = data[1];
        std::string last_name = data[2];
        char gender = data[3][0];
        int age = std::stoi(data[4]);
        int id_father = data[5].empty() ? -1 : std::stoi(data[5]);
        bool is_dead = std::stoi(data[6]);
        bool was_king = std::stoi(data[7]);
        bool is_king = std::stoi(data[8]);

        Node* newNode = new Node(id, name, last_name, gender, age, id_father, is_dead, was_king, is_king);
        nodes[id] = newNode;

        if (id_father == -1) {
            root = newNode;
        } else if (nodes[id_father]) {
            Node* parent = nodes[id_father];
            if (!parent->left) {
                parent->left = newNode;
            } else if (!parent->right) {
                parent->right = newNode;
            }
        }
    }

    file.close();
    return root;
}

// Funcion para validar el .CSV

bool validateCSVLine(const std::string& line, int expected_columns) { 
    int count = std::count(line.begin(), line.end(), ';') + 1;
    return count == expected_columns;
}

// Mostrar línea de sucesión
void printSuccessionLine(Node* root) {
    if (!root) return;
    if (!root->is_dead) {
        std::cout << "ID: " << root->id << ", Nombre: " << root->name << " " << root->last_name
                  << ", Edad: " << root->age << "\n";
    }
    printSuccessionLine(root->left);
    printSuccessionLine(root->right);
}

// Buscar sucesor del rey
Node* findSuccessor(Node* root) {
    if (!root) return nullptr;

    // Si es rey y ha muerto o tiene más de 70 años
    if (root->is_king && (root->is_dead || root->age > 70)) {
        // Buscar en hijos directos
        Node* successor = findFirstLivingDescendant(root->left);
        if (!successor) successor = findFirstLivingDescendant(root->right);

        // Si no hay hijos vivos, buscar en hermanos
        if (!successor) {
            Node* sibling = findSibling(root);
            successor = findFirstLivingDescendant(sibling);
        }

        // Si no hay hermanos vivos, buscar en tíos
        if (!successor) {
            Node* uncle = findUncle(root);
            successor = findFirstLivingDescendant(uncle);
        }

        // Si todo falla, buscar en ancestros con múltiples hijos
        if (!successor) {
            Node* ancestor = findAncestorWithMultipleChildren(root);
            successor = findFirstLivingDescendant(ancestor);
        }

        return successor;
    }

    // Recursión en subárbol izquierdo y derecho
    Node* leftSuccessor = findSuccessor(root->left);
    if (leftSuccessor) return leftSuccessor;
    return findSuccessor(root->right);
}

// Buscar hermano

Node* findSibling(Node* root) {
    if (!root || root->id_father == -1) return nullptr; // Sin padre => Sin hermanos

    Node* parent = findNodeByID(root, root->id_father);
    if (!parent) return nullptr;

    // Hermano es el otro hijo del padre
    if (parent->left && parent->left != root) return parent->left;
    if (parent->right && parent->right != root) return parent->right;
    return nullptr;
}

// Buscar tios

Node* findUncle(Node* root) {
    if (!root || root->id_father == -1) return nullptr;

    Node* grandparent = findNodeByID(root, findNodeByID(root, root->id_father)->id_father);
    if (!grandparent) return nullptr;

    // Buscar los hijos del abuelo que no sean el padre del nodo
    if (grandparent->left && grandparent->left->id != root->id_father) return grandparent->left;
    if (grandparent->right && grandparent->right->id != root->id_father) return grandparent->right;
    return nullptr;
}

// Buscar ancenstros con multiples hijos

Node* findAncestorWithMultipleChildren(Node* root) {
    if (!root || root->id_father == -1) return nullptr;

    Node* parent = findNodeByID(root, root->id_father);
    if (parent && parent->left && parent->right) return parent; // Padre con 2 hijos
    return findAncestorWithMultipleChildren(parent); // Subir un nivel
}

// Encontrar el primer descendiente vivo
Node* findFirstLivingDescendant(Node* node) {
    if (!node) return nullptr;
    if (!node->is_dead) return node;
    Node* leftDescendant = findFirstLivingDescendant(node->left);
    return leftDescendant ? leftDescendant : findFirstLivingDescendant(node->right);
}

// Actualizar el archivo CSV
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

    writeNodeToFile(root, file);
    file.close();
    std::cout << "Datos actualizados en el archivo CSV correctamente.\n";
}

// Escribir un nodo en el archivo
void writeNodeToFile(Node* node, std::ofstream& file) {
    if (!node) return;
    file << node->id << ";"
         << node->name << ";"
         << node->last_name << ";"
         << node->gender << ";"
         << node->age << ";"
         << (node->id_father == -1 ? "" : std::to_string(node->id_father)) << ";"
         << node->is_dead << ";"
         << node->was_king << ";"
         << node->is_king << "\n";
    writeNodeToFile(node->left, file);
    writeNodeToFile(node->right, file);
}

// Modificar un nodo del árbol
void modifyNode(Node* root) {
    int id;
    std::cout << "Ingrese el ID del nodo a modificar: ";
    std::cin >> id;

    Node* node = findNodeByID(root, id);
    if (!node) {
        std::cout << "Nodo no encontrado.\n";
        return;
    }

    std::cout << "Ingrese nuevo nombre: ";
    std::cin >> node->name;
    std::cout << "Ingrese nuevo apellido: ";
    std::cin >> node->last_name;
    std::cout << "Ingrese género (H/M): ";
    std::cin >> node->gender;
    std::cout << "Ingrese nueva edad: ";
    std::cin >> node->age;
    std::cout << "¿Está muerto? (0/1): ";
    std::cin >> node->is_dead;
    std::cout << "¿Fue rey? (0/1): ";
    std::cin >> node->was_king;
    std::cout << "¿Es rey? (0/1): ";
    std::cin >> node->is_king;
}

// Buscar un nodo por su ID
Node* findNodeByID(Node* root, int id) {
    if (!root) return nullptr;
    if (root->id == id) return root;
    Node* left = findNodeByID(root->left, id);
    return left ? left : findNodeByID(root->right, id);
}

// Crear un archivo CSV con un caso complejo
void createComplexCSV(const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error al crear el archivo CSV complejo.\n";
        return;
    }

    file << "id;name;last_name;gender;age;id_father;is_dead;was_king;is_king\n";
    file << "1;Arthur;Royal;H;80;;0;1;1\n";
    file << "2;Richard;Royal;H;68;1;0;1;0\n";
    file << "3;Catherine;Royal;M;44;2;0;0;0\n";
    file << "4;John;Royal;H;30;2;0;0;0\n";
    file << "5;Alice;Royal;M;40;3;1;0;0\n";
    file << "6;George;Royal;H;25;3;0;0;0\n";
    file << "7;Edward;Royal;H;20;4;0;0;0\n";
    file << "8;Sophia;Royal;M;18;4;0;0;0\n";

    file.close();
}

// Simulación de caso complejo
void simulateComplexRun() {
    createComplexCSV("complex_family_tree.csv");
    Node* root = readCSV("complex_family_tree.csv");
    std::cout << "\n*** LÍNEA DE SUCESIÓN INICIAL ***\n";
    printSuccessionLine(root);

    Node* currentKing = findNodeByID(root, 1);
    if (currentKing) currentKing->is_dead = true;

    Node* newKing = findSuccessor(root);
    if (newKing) newKing->is_king = true;

    std::cout << "\n*** NUEVO REY ***\n";
    if (newKing) {
        std::cout << "ID: " << newKing->id << ", Nombre: " << newKing->name << " " << newKing->last_name << "\n";
    }

    std::cout << "\n*** LÍNEA DE SUCESIÓN ACTUALIZADA ***\n";
    printSuccessionLine(root);

    updateCSV(root, "complex_family_tree_updated.csv");
}

// Ejecutar simulaciones
void runTests(Node* root) {
    std::cout << "\n*** LÍNEA DE SUCESIÓN ***\n";
    printSuccessionLine(root);

    Node* newKing = findSuccessor(root);
    if (newKing) {
        std::cout << "El sucesor es ID: " << newKing->id << ", Nombre: " << newKing->name << " " << newKing->last_name << "\n";
    }

    updateCSV(root, "family_tree_updated.csv");
}

// Main
int main() {
    // Variables iniciales
    std::string inputFile = "family_tree_ordered.csv";
    std::string outputFile = "family_tree_updated.csv";

    // Crear o leer el árbol inicial
    Node* root = readCSV(inputFile);
    if (!root) {
        std::cerr << "Error al cargar el árbol desde el archivo " << inputFile << ". Finalizando programa.\n";
        return 1;
    }

    int option;
    do {
        std::cout << "\n*** SISTEMA DE LÍNEA DE SUCESIÓN ***\n";
        std::cout << "1. Mostrar línea de sucesión\n";
        std::cout << "2. Buscar sucesor del rey\n";
        std::cout << "3. Modificar un nodo\n";
        std::cout << "4. Actualizar archivo CSV\n";
        std::cout << "5. Simulación de caso complejo\n";
        std::cout << "6. Salir\n";
        std::cout << "Seleccione una opción: ";
        std::cin >> option;

        switch (option) {
            case 1: {
                std::cout << "\n*** LÍNEA DE SUCESIÓN ***\n";
                printSuccessionLine(root);
                break;
            }
            case 2: {
                Node* successor = findSuccessor(root);
                if (successor) {
                    std::cout << "El sucesor del rey es:\n";
                    std::cout << "ID: " << successor->id << ", Nombre: " << successor->name << " " << successor->last_name
                              << ", Edad: " << successor->age << "\n";
                } else {
                    std::cout << "No hay sucesor disponible en la línea actual.\n";
                }
                break;
            }
            case 3: {
                modifyNode(root);
                break;
            }
            case 4: {
                updateCSV(root, outputFile);
                break;
            }
            case 5: {
                simulateComplexRun();
                break;
            }
            case 6: {
                std::cout << "Saliendo del programa. ¡Hasta luego!\n";
                break;
            }
            default: {
                std::cout << "Opción inválida. Intente de nuevo.\n";
                break;
            }
        }
    } while (option != 6);

    return 0;
}