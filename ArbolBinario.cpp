#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>
#include <windows.h>  // Libreria para leer caracteres en español
using namespace std;

// Definición del nodo del árbol
struct Node {
    int id;
    string name;
    string last_name;
    char gender;
    int age;
    int id_father;
    bool is_dead;
    bool was_king;
    bool is_king;
    Node* left;
    Node* right;

    Node(int i, string n, string ln, char g, int a, int idf, bool dead, bool wk, bool ik)
        : id(i), name(n), last_name(ln), gender(g), age(a), id_father(idf), is_dead(dead), was_king(wk), is_king(ik), left(nullptr), right(nullptr) {}
};

// Constante del archivo CSV

string FILE_NAME;

// Prototipos de funciones
Node* readCSV();
bool validateCSVLine(const string& line, int expected_columns);
void printSuccessionLine(Node* root, int level);
Node* findSuccessor(Node* root);
Node* findFirstLivingDescendant(Node* node);
Node* findSibling(Node* root);
Node* findUncle(Node* root);
Node* findAncestorWithMultipleChildren(Node* root);
void updateCSV(Node* root);
void writeNodeToFile(Node* node, ofstream& file);
void modifyNode(Node* root);
Node* findNodeByID(Node* root, int id);
void addFamilyMember(Node*& root);
Node* findNodeAndParent(Node* root, int id, Node*& parent);
void processPendingNodes(Node*& root, Node** pending, int& pending_count);
void addNodeToTree(Node*& root, Node* newNode);

// Función para leer el CSV y construir el árbol binario
Node* readCSV() {
    ifstream file(FILE_NAME);
    if (!file.is_open()) {
        cerr << "Error al abrir el archivo CSV.\n";
        return nullptr;
    }

    Node* root = nullptr;
    string line;
    bool isFirstLine = true;
    const int expected_columns = 9;

    // Arreglo para nodos pendientes
    Node* pending[100];  // Máximo 100 nodos pendientes
    int pending_count = 0;

    while (getline(file, line)) {
        if (isFirstLine) { // Saltar los headers
            isFirstLine = false;
            continue;
        }

        if (!validateCSVLine(line, expected_columns)) {
            cerr << "Línea inválida encontrada: " << line << "\n";
            continue;
        }

        stringstream ss(line);
        string word;
        string data[9];
        int index = 0;

        while (getline(ss, word, ';') && index < 9) {
            data[index++] = word;
        }

        int id = stoi(data[0]);
        string name = data[1];
        string last_name = data[2];
        char gender = data[3][0];
        int age = stoi(data[4]);
        int id_father = data[5].empty() ? -1 : stoi(data[5]);
        bool is_dead = stoi(data[6]);
        bool was_king = stoi(data[7]);
        bool is_king = stoi(data[8]);

        Node* newNode = new Node(id, name, last_name, gender, age, id_father, is_dead, was_king, is_king);

        if (id_father == -1) {
            // Este nodo es la raíz
            if (!root) {
                root = newNode;
            } else {
                cerr << "Advertencia: Se encontró más de un nodo raíz. Ignorando nodo con ID " << id << ".\n";
                delete newNode;
            }
        } else {
            Node* parent = findNodeByID(root, id_father);
            if (parent) {
                addNodeToTree(parent, newNode);
            } else {
                // Si el padre no está disponible, agregar a pendientes
                pending[pending_count++] = newNode;
            }
        }
    }

    // Procesar nodos pendientes
    processPendingNodes(root, pending, pending_count);

    file.close();
    return root;
}

// Procesar nodos pendientes
void processPendingNodes(Node*& root, Node** pending, int& pending_count) {
    int iterations = 0;

    while (pending_count > 0) {
        bool progress = false;

        for (int i = 0; i < pending_count; ++i) {
            Node* node = pending[i];
            Node* parent = findNodeByID(root, node->id_father);
            if (parent) {
                addNodeToTree(parent, node);
                pending[i] = pending[--pending_count];  // Eliminar nodo procesado
                progress = true;
                break;  // Reiniciar la búsqueda
            }
        }

        if (!progress) {
            cerr << "Error: No se pudo procesar todos los nodos pendientes después de " << iterations << " iteraciones.\n";
            break;
        }

        ++iterations;
    }
}

// Agregar un nodo al árbol
void addNodeToTree(Node*& parent, Node* newNode) {
    if (!parent->left) {
        parent->left = newNode;
    } else if (!parent->right) {
        parent->right = newNode;
    } else {
        cerr << "El padre con ID " << parent->id << " ya tiene dos hijos. Ignorando nodo con ID " << newNode->id << ".\n";
        delete newNode;
    }
}

// Funcion para validar el .CSV

bool validateCSVLine(const string& line, int expected_columns) { 
    int count = std::count(line.begin(), line.end(), ';') + 1;
    return count == expected_columns;
}

// Mostrar línea de sucesión
void printSuccessionLine(Node* root, int level = 0) {
    if (!root) return;

    // Sangría para mostrar jerarquía
    string indent(level * 4, ' '); // Cada nivel agrega 4 espacios

    // Formato de impresión
    cout << indent << "|-- ID: " << root->id 
         << ", Nombre: " << root->name << " " << root->last_name
         << ", Edad: " << root->age
         << ", Estado: " << (root->is_dead ? "Muerto" : "Vivo")
         << ", " << (root->is_king ? "Rey Actual" : (root->was_king ? "Exrey" : ""))
         << "\n";

    // Recursión para hijos (primero izquierdo, luego derecho)
    printSuccessionLine(root->left, level + 1);
    printSuccessionLine(root->right, level + 1);
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

// Buscar tíos
Node* findUncle(Node* root) {
    if (!root || root->id_father == -1) return nullptr;

    Node* grandparent = findNodeByID(root, findNodeByID(root, root->id_father)->id_father);
    if (!grandparent) return nullptr;

    // Buscar los hijos del abuelo que no sean el padre del nodo
    if (grandparent->left && grandparent->left->id != root->id_father) return grandparent->left;
    if (grandparent->right && grandparent->right->id != root->id_father) return grandparent->right;
    return nullptr;
}

// Buscar ancestros con múltiples hijos
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
void updateCSV(Node* root) {
    if (!root) {
        cout << "El árbol está vacío, no hay datos para guardar.\n";
        return;
    }

    ofstream file(FILE_NAME);
    if (!file.is_open()) {
        cerr << "Error al abrir el archivo: " << FILE_NAME << "\n";
        return;
    }
    
    // Escribir el encabezado
    file << "id;name;last_name;gender;age;id_father;is_dead;was_king;is_king\n";

    writeNodeToFile(root, file);
    file.close();
    cout << "Datos actualizados en el archivo CSV correctamente.\n";
}

// Escribir un nodo en el archivo
void writeNodeToFile(Node* node, ofstream& file) {
    if (!node) return;
    file << node->id << ";"
         << node->name << ";"
         << node->last_name << ";"
         << node->gender << ";"
         << node->age << ";"
         << (node->id_father == -1 ? "" : to_string(node->id_father)) << ";"
         << node->is_dead << ";"
         << node->was_king << ";"
         << node->is_king << "\n";
    writeNodeToFile(node->left, file);
    writeNodeToFile(node->right, file);
}

// Modificar un nodo del árbol
void modifyNode(Node* root) {
    int id;
    cout << "Ingrese el ID del nodo a modificar: ";
    cin >> id;

    Node* node = findNodeByID(root, id);
    if (!node) {
        cout << "Nodo no encontrado.\n";
        return;
    }

    cout << "Ingrese nuevo nombre: ";
    cin >> node->name;
    cout << "Ingrese nuevo apellido: ";
    cin >> node->last_name;
    cout << "Ingrese género (H/M): ";
    cin >> node->gender;
    cout << "Ingrese nueva edad: ";
    cin >> node->age;
    cout << "¿Está muerto? (0/1): ";
    cin >> node->is_dead;
    cout << "¿Fue rey? (0/1): ";
    cin >> node->was_king;
    cout << "¿Es rey? (0/1): ";
    cin >> node->is_king;
}

// Buscar un nodo por su ID
Node* findNodeByID(Node* root, int id) {
    if (!root) return nullptr;
    if (root->id == id) return root;

    Node* leftSearch = findNodeByID(root->left, id);
    if (leftSearch) return leftSearch;

    return findNodeByID(root->right, id);
}

// Agregar un nuevo familiar
void addFamilyMember(Node*& root) {
    int id, id_father, age;
    string name, last_name;
    char gender;
    bool is_dead, was_king, is_king;

    cout << "Ingrese los datos del nuevo familiar:\n";
    cout << "ID: ";
    cin >> id;
    
    // Validación: Verificar si el ID ya está en uso
    if (findNodeByID(root, id)) {
        cout << "Error: El ID " << id << " ya está en uso. Por favor, ingrese un ID único.\n";
        return; // Salir sin agregar el nodo
    }

    cout << "Nombre: ";
    cin >> name;
    cout << "Apellido: ";
    cin >> last_name;
    cout << "Género (M/F): ";
    cin >> gender;
    cout << "Edad: ";
    cin >> age;
    cout << "ID del padre: ";
    cin >> id_father;
    cout << "¿Está muerto? (1 para Sí, 0 para No): ";
    cin >> is_dead;
    cout << "¿Fue rey? (1 para Sí, 0 para No): ";
    cin >> was_king;
    cout << "¿Es rey? (1 para Sí, 0 para No): ";
    cin >> is_king;

    Node* parent = findNodeByID(root, id_father);
    if (!parent) {
        cout << "No se encontró un padre con ID: " << id_father << ". No se puede agregar al familiar.\n";
        return;
    }

    if (parent->left && parent->right) {
        cout << "El padre ya tiene dos hijos. No se puede agregar más familiares.\n";
        return;
    }

    Node* newNode = new Node(id, name, last_name, gender, age, id_father, is_dead, was_king, is_king);
    if (!parent->left) {
        parent->left = newNode;
    } else {
        parent->right = newNode;
    }

    cout << "Familiar agregado correctamente.\n";

    // Actualizar el archivo CSV
    updateCSV(root);
}

// Buscar nodo y su padre
Node* findNodeAndParent(Node* root, int id, Node*& parent) {
    if (!root) return nullptr;

    if (root->left && root->left->id == id) {
        parent = root;
        return root->left;
    }

    if (root->right && root->right->id == id) {
        parent = root;
        return root->right;
    }

    // Buscar recursivamente en los hijos
    Node* leftResult = findNodeAndParent(root->left, id, parent);
    if (leftResult) return leftResult;

    return findNodeAndParent(root->right, id, parent);
}

// Main
int main() {

    // Configurar la consola para usar UTF-8
    SetConsoleOutputCP(CP_UTF8); // Establece la salida en UTF-8, caracteres especiales en español
    SetConsoleCP(CP_UTF8);       // Establece la entrada en UTF-8, caracteres especiales en español


    int option2;
    cout << "Seleccione el archivo CSV a usar:\n";
    cout << "1. family_tree_ordered.csv\n";
    cout << "2. family_tree_unordered.csv\n";
    cout << "Ingrese su opción (1 o 2): ";
    cin >> option2;

    if (option2 == 1) {
        FILE_NAME = "family_tree_ordered.csv";
    } else if (option2 == 2) {
        FILE_NAME = "family_tree_unordered.csv";
    } else {
        cerr << "Opción inválida. Finalizando programa.\n";
        return 1;
    }

    Node* root = readCSV();
    if (!root) {
        cerr << "Error al cargar el árbol desde el archivo " << FILE_NAME << ". Finalizando programa.\n";
        return 1;
    }

    int option;
    do {
        cout << "\n*** SISTEMA DE LÍNEA DE SUCESIÓN ***\n";
        cout << "1. Mostrar línea de sucesión\n";
        cout << "2. Buscar sucesor del rey\n";
        cout << "3. Modificar un nodo\n";
        cout << "4. Actualizar archivo CSV\n";
        cout << "5. Agregar un familiar\n";
        cout << "6. Salir\n";
        cout << "Seleccione una opción: ";
        cin >> option;

        switch (option) {
            case 1: {
                printSuccessionLine(root);
                break;
            }
            case 2: {
                Node* successor = findSuccessor(root);
                if (successor) {
                    cout << "El sucesor del rey es:\n";
                    cout << "ID: " << successor->id << ", Nombre: " << successor->name << " " << successor->last_name
                         << ", Edad: " << successor->age << "\n";
                } else {
                    cout << "No hay sucesor disponible en la línea actual.\n";
                }
                break;
            }
            case 3: {
                modifyNode(root);
                break;
            }
            case 4: {
                updateCSV(root);
                break;
            }
            case 5: {
                addFamilyMember(root);
                break;
            }
            case 6: {
                cout << "Saliendo del programa. ¡Hasta luego!\n";
                break;
            }
            default: {
                cout << "Opción inválida. Intente de nuevo.\n";
                break;
            }
        }
    } while (option != 6);

    return 0;
}