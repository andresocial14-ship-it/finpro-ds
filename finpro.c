#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

// !! DEFINE !! //
#define account_file "users.txt"
#define film_file "films.txt"
#define studio_file "studios.txt"
#define schedule_file "schedules.txt"
#define booking_file "bookings.txt"
#define T 3               // minimum degree
#define MAX_KEYS (2*T-1)  // = 5, maksimum key per node
#define MIN_KEYS (T-1)    // = 2, minimum key per node

// !! STRUCT !! //
typedef struct {
    char username[100];
    char password[100];
    char name[100];
    char email[100];
} Account;

typedef struct {
    int id;
    char title[100];
    char genre[100];
    int duration;
    int age_rating;
    char detail[300];
} Film;

typedef struct {
    int id;
    char name[50];
    int capacity;
} Studio;

typedef struct {
    int id;
    int film_id;
    int studio_id;
    char date[20];
    char time[20];
    float price;
} Schedule;

typedef struct {
    char booking_code[10];
    char username[100];
    int schedule_id;
    char seat[10];
    float total_price;
    int status;
} Booking;

typedef struct BTreeNode {
    int n;                          // jumlah key saat ini
    Film keys[MAX_KEYS];            // array of film data (key = film.id)
    struct BTreeNode* children[MAX_KEYS + 1]; // pointer ke child nodes
    int leaf;                       // 1 jika node adalah daun, 0 jika tidak
} BTreeNode;

// !! GLOBAL VARIABLE !! //
char current_user[100];
BTreeNode* film_tree = NULL;

// !! FUNCTION STRUCT !! //
Account* find_username(char* username, char* password);

// !! PRINTILAN !! // 
void invalid_choice();
void invalid_file();    

// !! B-TREE !! //
BTreeNode* btree_create_node(int leaf);
void btree_insert(BTreeNode** root, Film film);
void btree_insert_nonfull(BTreeNode* node, Film film);
void btree_split_child(BTreeNode* parent, int i, BTreeNode* child);
Film* btree_search(BTreeNode* root, int id);
void btree_inorder(BTreeNode* root, Film* result, int* count);
void btree_delete(BTreeNode** root, int id);
void btree_delete_internal(BTreeNode* node, int id);
Film btree_get_predecessor(BTreeNode* node, int idx);
Film btree_get_successor(BTreeNode* node, int idx);
void btree_fill(BTreeNode* node, int idx);
void btree_borrow_from_prev(BTreeNode* node, int idx);
void btree_borrow_from_next(BTreeNode* node, int idx);
void btree_merge(BTreeNode* node, int idx);
void btree_free(BTreeNode* root);
int auto_id();

// !! MENU !! //
void main_menu();       
void login();           
void register_acc();

// ---------------- //
// !! MENU ADMIN !! //
// ---------------- //
void menu_admin();      

// ! FILM MANAGEMENT ! //
void btree_load_from_file();
void btree_save_to_file();
void film_manage();
void add_film();        
void del_film();        
void edit_film();       
void search_film();
void view_film();

// ! STUDIO & SCHEDULE MANAGEMENT ! //
void schedule_manage();
void add_studio();
void del_studio();
void add_schedule();
void edit_schedule();
void del_schedule();
void view_schedule();

// ! USER ACCOUNT MANAGEMENT ! //
void acc_manage();
void view_users();
void search_user();     
void delete_user();

// ------------------- //
// !! MENU CUSTOMER !! //
// ------------------- //
void menu_cust();
void view_film_cust();
void book_ticket();     
void history();
void cancel();

// ! EDIT PROFILE !//
void edit_profile();
void change_usn();
void change_name();
void change_email();
void change_pass();

// ------------------ //
// !! MENU CASHIER !! //
// ------------------ //
void menu_cashier();
void validate_ticket(); 
void sell();
void seat_status();

int main () {
    main_menu();
    return 0;
}

// CREATE B-TREE NODE //
BTreeNode* btree_create_node(int leaf) {
    BTreeNode* node = (BTreeNode*)malloc(sizeof(BTreeNode));
    if (!node) {
        printf("Memory allocation failed!\n");
        exit(1);
    }
    node->n    = 0;
    node->leaf = leaf;
    for (int i = 0; i <= MAX_KEYS; i++) {
        node->children[i] = NULL;
    }
    return node;
}

// ----------------------------------------
// INSERT: Main entry point
// Jika root penuh ? split root ? insert
// ----------------------------------------
void btree_insert(BTreeNode** root, Film film) {
    // Jika tree kosong
    if (*root == NULL) {
        *root = btree_create_node(1); // buat root sebagai leaf
        (*root)->keys[0] = film;
        (*root)->n = 1;
        return;
    }

    // Jika root penuh (n == MAX_KEYS), harus split dulu
    if ((*root)->n == MAX_KEYS) {
        // Buat root baru
        BTreeNode* new_root = btree_create_node(0); // bukan leaf
        new_root->children[0] = *root;

        // Split root lama sebagai child[0] dari new_root
        btree_split_child(new_root, 0, *root);

        // Tentukan ke child mana yang akan diinsert
        int i = 0;
        if (new_root->keys[0].id < film.id) {
            i = 1;
        }
        btree_insert_nonfull(new_root->children[i], film);

        *root = new_root;
    } else {
        // Root tidak penuh, langsung insert
        btree_insert_nonfull(*root, film);
    }
}

// ----------------------------------------
// INSERT ke node yang belum penuh
// ----------------------------------------
void btree_insert_nonfull(BTreeNode* node, Film film) {
    int i = node->n - 1;

    if (node->leaf) {
        // Geser key yang lebih besar ke kanan untuk beri ruang
        while (i >= 0 && node->keys[i].id > film.id) {
            node->keys[i + 1] = node->keys[i];
            i--;
        }
        node->keys[i + 1] = film;
        node->n++;
    } else {
        // Cari child yang tepat
        while (i >= 0 && node->keys[i].id > film.id) {
            i--;
        }
        i++;

        // Jika child penuh, split dulu
        if (node->children[i]->n == MAX_KEYS) {
            btree_split_child(node, i, node->children[i]);

            // Setelah split, tentukan ke child mana
            if (node->keys[i].id < film.id) {
                i++;
            }
        }
        btree_insert_nonfull(node->children[i], film);
    }
}

// ----------------------------------------
// SPLIT CHILD: Split child[i] dari parent
// ----------------------------------------
// Contoh split (t=3, max=5 keys):
// Before: child = [1,2,3,4,5]
// After:  left  = [1,2]   ? child[i]
//         mid   = [3]     ? naik ke parent
//         right = [4,5]   ? child[i+1] baru
// ----------------------------------------
void btree_split_child(BTreeNode* parent, int i, BTreeNode* child) {
    // Buat node baru untuk menampung right half dari child
    BTreeNode* new_node = btree_create_node(child->leaf);
    new_node->n = T - 1; // = 2 keys

    // Salin right half dari child ke new_node
    // keys[T] sampai keys[MAX_KEYS-1] ? keys[0] sampai keys[T-2]
    for (int j = 0; j < T - 1; j++) {
        new_node->keys[j] = child->keys[j + T];
    }

    // Salin children jika bukan leaf
    if (!child->leaf) {
        for (int j = 0; j < T; j++) {
            new_node->children[j] = child->children[j + T];
        }
    }

    // Kurangi jumlah key di child (sekarang hanya left half)
    child->n = T - 1; // = 2 keys

    // Geser children parent ke kanan untuk beri ruang child baru
    for (int j = parent->n; j >= i + 1; j--) {
        parent->children[j + 1] = parent->children[j];
    }
    parent->children[i + 1] = new_node;

    // Geser keys parent ke kanan
    for (int j = parent->n - 1; j >= i; j--) {
        parent->keys[j + 1] = parent->keys[j];
    }

    // Naikkan median key dari child ke parent
    parent->keys[i] = child->keys[T - 1]; // keys[2] = median
    parent->n++;
}

// ----------------------------------------
// SEARCH: Cari film by ID
// Return pointer ke Film jika ketemu, NULL jika tidak
// ----------------------------------------
Film* btree_search(BTreeNode* root, int id) {
    if (root == NULL) return NULL;

    int i = 0;

    // Cari posisi key pertama yang >= id
    while (i < root->n && id > root->keys[i].id) {
        i++;
    }

    // Cek apakah ketemu
    if (i < root->n && id == root->keys[i].id) {
        return &(root->keys[i]);
    }

    // Jika node adalah leaf dan tidak ketemu
    if (root->leaf) return NULL;

    // Rekursi ke child yang sesuai
    return btree_search(root->children[i], id);
}

// ----------------------------------------
// IN-ORDER TRAVERSAL: Kumpulkan semua film terurut by ID
// ----------------------------------------
void btree_inorder(BTreeNode* root, Film* result, int* count) {
    if (root == NULL) return;

    for (int i = 0; i < root->n; i++) {
        // Kunjungi left child sebelum key[i]
        if (!root->leaf) {
            btree_inorder(root->children[i], result, count);
        }
        // Tambahkan key[i] ke result
        result[(*count)++] = root->keys[i];
    }

    // Kunjungi rightmost child
    if (!root->leaf) {
        btree_inorder(root->children[root->n], result, count);
    }
}

// ============================================
// !! DELETE (HARD DELETE) !!
// ============================================

// Main delete entry point
void btree_delete(BTreeNode** root, int id) {
    if (*root == NULL) {
        printf("Film with ID %d not found!\n", id);
        return;
    }

    btree_delete_internal(*root, id);

    // Jika root sekarang kosong dan punya child, turunkan root
    if ((*root)->n == 0) {
        BTreeNode* old_root = *root;
        if ((*root)->leaf) {
            *root = NULL;
        } else {
            *root = (*root)->children[0];
        }
        free(old_root);
    }
}

// ----------------------------------------
// DELETE INTERNAL: rekursif
// ----------------------------------------
void btree_delete_internal(BTreeNode* node, int id) {
    int i = 0;

    // Cari posisi id di node ini
    while (i < node->n && id > node->keys[i].id) {
        i++;
    }

    // CASE 1: Key ditemukan di node ini
    if (i < node->n && id == node->keys[i].id) {

        if (node->leaf) {
            // CASE 1a: Node adalah leaf ? hapus langsung
            for (int j = i; j < node->n - 1; j++) {
                node->keys[j] = node->keys[j + 1];
            }
            node->n--;
        } else {
            // CASE 1b: Node bukan leaf
            if (node->children[i]->n >= T) {
                // Ganti dengan predecessor (max key dari left subtree)
                Film pred = btree_get_predecessor(node, i);
                node->keys[i] = pred;
                btree_delete_internal(node->children[i], pred.id);
            } else if (node->children[i + 1]->n >= T) {
                // Ganti dengan successor (min key dari right subtree)
                Film succ = btree_get_successor(node, i);
                node->keys[i] = succ;
                btree_delete_internal(node->children[i + 1], succ.id);
            } else {
                // Merge children[i] dan children[i+1]
                btree_merge(node, i);
                btree_delete_internal(node->children[i], id);
            }
        }
    } else {
        // CASE 2: Key tidak di node ini, cari di child
        if (node->leaf) {
            printf("Film with ID %d not found!\n", id);
            return;
        }

        // Tentukan apakah key ada di subtree child[i]
        int last_child = (i == node->n);

        // Pastikan child[i] punya cukup key sebelum rekursi
        if (node->children[i]->n < T) {
            btree_fill(node, i);
        }

        // Setelah fill, posisi bisa berubah karena merge
        if (last_child && i > node->n) {
            btree_delete_internal(node->children[i - 1], id);
        } else {
            btree_delete_internal(node->children[i], id);
        }
    }
}

// Ambil predecessor: key terbesar dari left subtree child[idx]
Film btree_get_predecessor(BTreeNode* node, int idx) {
    BTreeNode* cur = node->children[idx];
    while (!cur->leaf) {
        cur = cur->children[cur->n];
    }
    return cur->keys[cur->n - 1];
}

// Ambil successor: key terkecil dari right subtree child[idx+1]
Film btree_get_successor(BTreeNode* node, int idx) {
    BTreeNode* cur = node->children[idx + 1];
    while (!cur->leaf) {
        cur = cur->children[0];
    }
    return cur->keys[0];
}

// Fill: pastikan children[idx] punya minimal T keys
void btree_fill(BTreeNode* node, int idx) {
    if (idx != 0 && node->children[idx - 1]->n >= T) {
        // Pinjam dari sibling kiri
        btree_borrow_from_prev(node, idx);
    } else if (idx != node->n && node->children[idx + 1]->n >= T) {
        // Pinjam dari sibling kanan
        btree_borrow_from_next(node, idx);
    } else {
        // Merge dengan sibling
        if (idx != node->n) {
            btree_merge(node, idx);
        } else {
            btree_merge(node, idx - 1);
        }
    }
}

// Pinjam key dari child kiri (prev sibling)
void btree_borrow_from_prev(BTreeNode* node, int idx) {
    BTreeNode* child = node->children[idx];
    BTreeNode* sibling = node->children[idx - 1];

    // Geser keys child ke kanan
    for (int i = child->n - 1; i >= 0; i--) {
        child->keys[i + 1] = child->keys[i];
    }
    if (!child->leaf) {
        for (int i = child->n; i >= 0; i--) {
            child->children[i + 1] = child->children[i];
        }
    }

    // Turunkan key parent ke child
    child->keys[0] = node->keys[idx - 1];
    if (!child->leaf) {
        child->children[0] = sibling->children[sibling->n];
    }

    // Naikkan key terakhir sibling ke parent
    node->keys[idx - 1] = sibling->keys[sibling->n - 1];

    child->n++;
    sibling->n--;
}

// Pinjam key dari child kanan (next sibling)
void btree_borrow_from_next(BTreeNode* node, int idx) {
    BTreeNode* child = node->children[idx];
    BTreeNode* sibling = node->children[idx + 1];

    // Turunkan key parent ke akhir child
    child->keys[child->n] = node->keys[idx];
    if (!child->leaf) {
        child->children[child->n + 1] = sibling->children[0];
    }

    // Naikkan key pertama sibling ke parent
    node->keys[idx] = sibling->keys[0];

    // Geser keys sibling ke kiri
    for (int i = 1; i < sibling->n; i++) {
        sibling->keys[i - 1] = sibling->keys[i];
    }
    if (!sibling->leaf) {
        for (int i = 1; i <= sibling->n; i++) {
            sibling->children[i - 1] = sibling->children[i];
        }
    }

    child->n++;
    sibling->n--;
}

// Merge: gabungkan children[idx] dan children[idx+1]
// dengan keys[idx] sebagai median
void btree_merge(BTreeNode* node, int idx) {
    BTreeNode* child  = node->children[idx];
    BTreeNode* sibling = node->children[idx + 1];

    // Turunkan key parent ke posisi tengah child
    child->keys[T - 1] = node->keys[idx];

    // Salin keys sibling ke child
    for (int i = 0; i < sibling->n; i++) {
        child->keys[i + T] = sibling->keys[i];
    }
    if (!child->leaf) {
        for (int i = 0; i <= sibling->n; i++) {
            child->children[i + T] = sibling->children[i];
        }
    }

    // Geser keys dan children parent ke kiri
    for (int i = idx + 1; i < node->n; i++) {
        node->keys[i - 1] = node->keys[i];
    }
    for (int i = idx + 2; i <= node->n; i++) {
        node->children[i - 1] = node->children[i];
    }

    child->n += sibling->n + 1;
    node->n--;

    free(sibling);
}

// ----------------------------------------
// FREE: Bebaskan semua memory B-Tree
// ----------------------------------------
void btree_free(BTreeNode* root) {
    if (root == NULL) return;
    if (!root->leaf) {
        for (int i = 0; i <= root->n; i++) {
            btree_free(root->children[i]);
        }
    }
    free(root);
}

// ============================================
// !! LOAD & SAVE FILE !!
// ============================================

// Load semua film dari file ke B-Tree
void btree_load_from_file() {
    // Bebaskan tree lama jika ada
    if (film_tree != NULL) {
        btree_free(film_tree);
        film_tree = NULL;
    }

    FILE* fp = fopen(film_file, "r");
    if (fp == NULL) return; // file belum ada, tree kosong

    char buffer[600];
    while (fgets(buffer, sizeof(buffer), fp)) {
        Film film;
        buffer[strcspn(buffer, "\n")] = 0;
        sscanf(buffer, "%d=%[^=]=%[^=]=%d=%d=%[^\n]",
               &film.id, film.title, film.genre,
               &film.duration, &film.age_rating, film.detail);
        btree_insert(&film_tree, film);
    }
    fclose(fp);
}

// Simpan semua film dari B-Tree ke file (in-order = urut by ID)
void btree_save_to_file() {
    Film result[200];
    int count = 0;
    btree_inorder(film_tree, result, &count);

    FILE* fp = fopen(film_file, "w");
    if (fp == NULL) {
        printf("Failed to save to file!\n");
        return;
    }
    for (int i = 0; i < count; i++) {
        fprintf(fp, "%d=%s=%s=%d=%d=%s\n",
                result[i].id, result[i].title, result[i].genre,
                result[i].duration, result[i].age_rating, result[i].detail);
    }
    fclose(fp);
}

// AUTO ID //
int auto_id() {
    Film result[200];
    int count = 0;
    btree_inorder(film_tree, result, &count);
    if (count == 0) return 0;
    return result[count - 1].id; // ID terbesar (in-order = terurut)
}

// INVALID CHOICE //
void invalid_choice () {
    printf("Invalid choice! Try again\n");
    system("pause");
}

// INVALID OPEN FILE //
void invalid_file () {
    printf("Failed to open file!\n");
    system("pause");
    return;
}

// MAIN MENU //
void main_menu () {
    int choice;

    do {
        system("cls");
        printf("============================================\n");
        printf("            CINEMA BOOKING SYSTEM           \n");
        printf("--------------------------------------------\n");
        printf("[1] Login\n");
        printf("[2] Register New Account\n");
        printf("[0] Exit\n");
        printf("============================================\n");    
        printf("Choose : ");
        scanf("%d", &choice);
        
        if (choice < 0 || choice > 2) {
            invalid_choice();
        }
    } while(choice < 0 || choice > 2); 
    
    switch (choice) {
        case 1 : 
            login();
            break;
        case 2 : 
            register_acc();
            break;
        case 0 : 
            return;
    }
}

// LOGIN //
void login() {
    system("cls");

    char username[100];
    char password[100];
    char fileUsername[100];
    char filePassword[100];
    char buffer[200];
    int success = 0;

    printf("============================================\n");
    printf("                  LOGIN                    \n");
    printf("--------------------------------------------\n");

    // username
    do {
        printf("Username : ");
        scanf("%s", username);

        if (strlen(username) == 0) {
            printf("Username cannot be empty!\n");
        }
    } while (strlen(username) == 0);

    // password
    do {
        printf("Password : ");
        scanf("%s", password);

        if (strlen(password) == 0) {
            printf("Password cannot be empty!\n");
        }
    } while (strlen(password) == 0);
    printf("============================================\n");

    if (strcmp(username, "admin123@") == 0 && strcmp(password, "admin123@") == 0) {
        system("pause");
        menu_admin();
        return;
    }

    if (strcmp(username, "cashier123@") == 0 && strcmp(password, "cashier123@") == 0) {
        system("pause");
        menu_cashier();
        return;
    }

    FILE* fp = fopen(account_file, "r");
    if (fp == NULL) {
        invalid_file();
        return;
    }

    while (fgets(buffer, sizeof(buffer), fp)) {
        buffer[strcspn(buffer, "\n")] = 0;
        sscanf(buffer, "%[^,],%[^,]", fileUsername, filePassword);

        if (strcmp(username, fileUsername) == 0 && strcmp(password, filePassword) == 0) {
            success = 1;
            strcpy(current_user, username);
            break;
        }
    }
    fclose(fp);

    if (success) {
        menu_cust();
        return;
    }

    printf("Wrong username or password!\n");
    system("pause");
    login(); 
}

// REGISTER //
void register_acc() {
    system("cls");

    char fileUsername[100];
    char confirm_pass[100];
    char buffer[1024];
    int found;
    int valid;
    int validasi_at;

    FILE *reg = fopen(account_file, "a");
    if (reg == NULL) {
        invalid_file();
    }

    Account customer;

    printf("==========================================\n");
    printf("                 REGISTER                 \n");
    printf("==========================================\n");
    printf("Complete the registration form to continue\n");
    printf("------------------------------------------\n");

    // full name
    do {
        printf("Full Name     : ");
        scanf(" %[^\n]", customer.name);

        valid = 1;
        if (strlen(customer.name) == 0) {
            printf("Full name cannot be empty!\n");   
            valid = 0;
        } 
        else {
            for (int i = 0; customer.name[i] != '\0'; i++) {
                if (isdigit(customer.name[i])) {
                    printf("Full name cannot contain numbers!\n");
                    valid = 0;
                    break;
                }
            }
        }
    } while(strlen(customer.name) == 0 || !valid);

    // email
    do {
        printf("Email         : ");
        scanf("%s", customer.email);

        // validasi @
        validasi_at = 0;
        for (int i = 0; customer.email[i] != '\0'; i++) {
            if (customer.email[i] == '@') {
                validasi_at = 1;
                break;
            }
        }

        if (strlen(customer.email) == 0) {
            printf("Email cannot be empty!\n");     
        } 
        else if (!validasi_at) {
            printf("Email must contain '@'!\n");
        }
    } while (strlen(customer.email) == 0 || !validasi_at);

    // username
    do {
        printf("Username      : ");
        scanf("%s", customer.username);

        found = 0;
        FILE *check = fopen("users.txt", "r");

        if (check != NULL) {
            while (fgets(buffer, sizeof(buffer), check)) {
                sscanf(buffer, "%[^,]", fileUsername);
                if (strcmp(fileUsername, customer.username) == 0) {
                    found = 1;
                    printf("Username already exists! Try another username!\n");
                    break;
                }
            }
            fclose(check);
        }
    } while(found);

    // password
    do {
        printf("Password      : ");
        scanf("%s", customer.password);

        if (strlen(customer.password) == 0) {
            printf("Password cannot be empty!\n");
        } 
        else if (strlen(customer.password) < 5) {
            printf("Password must be at least 5 characters!\n");
        }
    } while(strlen(customer.password) < 5);

    // confirm password
    do {
        printf("Confirm Password  : ");
        scanf("%s", confirm_pass);
        if (strlen(confirm_pass) == 0) {
            printf("Confirm password cannot be empty!\n");
        } 
        else if (strcmp(customer.password, confirm_pass) != 0) {
            printf("Password does not match! Try again!\n");
        }
    } while(strcmp(customer.password, confirm_pass) != 0);

    fprintf(reg, "%s,%s,%s,%s\n", customer.username, customer.password, customer.name, customer.email);
    fclose(reg);

    system("cls");
    printf("============================================\n");
    printf("       Account created successfully!       \n");
    printf("============================================\n");
    system("pause");
    main_menu();
}

// MENU ADMIN //
void menu_admin () {
    int choice;
    btree_load_from_file();

    do {
        system("cls");
        printf("============================================\n");
        printf("                 ADMIN MENU                 \n");
        printf("--------------------------------------------\n");
        printf("Welcome Back, Admin!\n");
        printf("[1] Film Management\n");
        printf("[2] Studio & Schedule Management\n");
        printf("[3] User Account Management\n");
        printf("[0] Logout\n");
        printf("============================================\n");    
        printf("Choose : ");
        scanf("%d", &choice);
        
        if (choice < 0 || choice > 3) {
            invalid_choice();
        }
    } while(choice < 0 || choice > 3); 

    switch (choice) {
        case 1 : 
            film_manage();
            break;
        case 2 : 
            schedule_manage();
            break;
        case 3 : 
            acc_manage();
            break;
        case 0 : 
            main_menu();
    }
}

// SUB-MENU FILM MANAGEMENT //
void film_manage () {
    int choice;

    do {
        system("cls");
        printf("============================================\n");
        printf("               FILM MANAGEMENT              \n");
        printf("--------------------------------------------\n");
        printf("[1] Add New Film\n");
        printf("[2] Delete Film\n");
        printf("[3] Edit Film Info\n");
        printf("[4] View All Films\n");
        printf("[5] Search Film by ID\n");
        printf("[0] Back\n");
        printf("============================================\n");    
        printf("Choose : ");
        scanf("%d", &choice);
        
        if (choice < 0 || choice > 5) {
            invalid_choice();
        }
    } while(choice < 0 || choice > 5); 

    switch (choice) {
        case 1 : 
            add_film();
            break;
        case 2 : 
            del_film();
            break;
        case 3 : 
            edit_film();
            break;
        case 4 : 
            view_film();
            break;
        case 5 : 
            search_film();
            break;
        case 0 : 
            menu_admin();
            break;
    }
}

// ADD FILM //
void add_film() {
    system("cls");

    Film film;
    int valid;

    printf("============================================\n");
    printf("                 ADD FILM                   \n");
    printf("--------------------------------------------\n");

    // title
    do {
        printf("Title           : ");
        scanf(" %[^\n]", film.title);

        valid = 1;
        if (strlen(film.title) == 0) {
            printf("Title cannot be empty!\n");
            valid = 0;
        }
    } while(!valid);

    // genre
    do {
        printf("Genre           : ");
        scanf(" %[^\n]", film.genre);

        valid = 1;
        if (strlen(film.genre) == 0) {
            printf("Genre cannot be empty!\n");
            valid = 0;
        }
    } while(!valid);

    // duration
    do {
        printf("Duration (min)  : ");
        scanf("%d", &film.duration);

        if (film.duration <= 0) {
            printf("Duration must be greater than 0!\n");
        }
    } while(film.duration <= 0);

    // age rating
    do {
        printf("Age Rating (n+) : ");
        scanf("%d", &film.age_rating);

        if (film.age_rating < 0) {
            printf("Age rating cannot be negative!\n");
        }
    } while(film.age_rating < 0);

    // synopsis
    do {
        printf("Synopsis        : ");
        scanf(" %[^\n]", film.detail);

        valid = 1;
        if (strlen(film.detail) == 0) {
            printf("Detail cannot be empty!\n");
            valid = 0;
        }
    } while(!valid);

    // auto ID
    film.id = auto_id() + 1;

    // insert B-tree
    btree_insert(&film_tree, film);

    // simpan
    btree_save_to_file();

    system("cls");
    printf("============================================\n");
    printf("          Film added successfully!          \n");
    printf("--------------------------------------------\n");
    printf("  ID       : %d\n", film.id);
    printf("  Title    : %s\n", film.title);
    printf("  Genre    : %s\n", film.genre);
    printf("  Duration : %d min\n", film.duration);
    printf("  Age      : %d+\n", film.age_rating);
    printf("  Detail   : %s\n", film.detail);
    printf("============================================\n");
    system("pause");
    film_manage();
}

// DELETE FILM //
void del_film() {
    system("cls");

    Film result[200];
    int count = 0;
    btree_inorder(film_tree, result, &count);

    printf("==============================\n");
    printf("          DELETE FILM         \n");
    printf("==============================\n");
    printf("%-5s %-20s\n", "ID", "Title");
    printf("------------------------------\n");

    for (int i = 0; i < count; i++) {
        printf("%-5d %-20s\n", result[i].id, result[i].title);
    }

    if (count == 0) {
        printf("No films available.\n");
        printf("==============================\n");
        system("pause");
        film_manage();
        return;
    }

    printf("==============================\n");

    char input[10];
    int del_id;
    int valid;

    do {
        printf("Enter Film ID to delete : ");
        scanf("%s", input);
        valid = 1;
        for (int i = 0; input[i] != '\0'; i++) {
            if (!isdigit(input[i])) { valid = 0; break; }
        }
        if (!valid) printf("ID must be a number!\n");
    } while (!valid);

    del_id = atoi(input);

    // cek id
    Film* found = btree_search(film_tree, del_id);
    if (found == NULL) {
        printf("Film with ID %d not found!\n", del_id);
        system("pause");
        film_manage();
        return;
    }

    // delete B-tree
    btree_delete(&film_tree, del_id);

    // simpan
    btree_save_to_file();

    printf("Film with ID %d deleted successfully!\n", del_id);
    system("pause");
    film_manage();
}

// EDIT FILM // 
void edit_film() {
    system("cls");

    Film result[200];
    int count = 0;
    btree_inorder(film_tree, result, &count);

    printf("==============================\n");
    printf("           EDIT FILM          \n");
    printf("==============================\n");
    printf("%-5s %-20s\n", "ID", "Title");
    printf("------------------------------\n");

    for (int i = 0; i < count; i++) {
        printf("%-5d %-20s\n", result[i].id, result[i].title);
    }

    if (count == 0) {
        printf("No films available.\n");
        printf("==============================\n");
        system("pause");
        film_manage();
        return;
    }

    printf("==============================\n");

    char input[10];
    int edit_id;
    int valid;

    do {
        printf("Enter Film ID to edit : ");
        scanf("%s", input);
        valid = 1;
        for (int i = 0; input[i] != '\0'; i++) {
            if (!isdigit(input[i])) { valid = 0; break; }
        }
        if (!valid) printf("ID must be a number!\n");
    } while (!valid);

    edit_id = atoi(input);

    // search B-tree
    Film* target = btree_search(film_tree, edit_id);
    if (target == NULL) {
        printf("Film with ID %d not found!\n", edit_id);
        system("pause");
        film_manage();
        return;
    }

    printf("------------------------------\n");
    printf("Leave blank to keep current value\n");
    printf("------------------------------\n");

    char new_val[300];
    while (getchar() != '\n');

    // title
    printf("Title [%s] : ", target->title);
    fgets(new_val, sizeof(new_val), stdin);
    new_val[strcspn(new_val, "\n")] = '\0';
    if (strlen(new_val) != 0) strcpy(target->title, new_val);

    // genre
    printf("Genre [%s] : ", target->genre);
    fgets(new_val, sizeof(new_val), stdin);
    new_val[strcspn(new_val, "\n")] = '\0';
    if (strlen(new_val) != 0) strcpy(target->genre, new_val);

    // duration 
    printf("Duration [%d] : ", target->duration);
    fgets(new_val, sizeof(new_val), stdin);
    new_val[strcspn(new_val, "\n")] = '\0';
    if (strlen(new_val) != 0) {
        valid = 1;
        for (int i = 0; new_val[i] != '\0'; i++) {
            if (!isdigit(new_val[i])) { valid = 0; break; }
        }
        if (valid && atoi(new_val) > 0) target->duration = atoi(new_val);
        else printf("Invalid duration, keeping current value.\n");
    }

    // age rating
    printf("Age Rating [%d] : ", target->age_rating);
    fgets(new_val, sizeof(new_val), stdin);
    new_val[strcspn(new_val, "\n")] = '\0';
    if (strlen(new_val) != 0) {
        valid = 1;
        for (int i = 0; new_val[i] != '\0'; i++) {
            if (!isdigit(new_val[i])) { valid = 0; break; }
        }
        if (valid && atoi(new_val) >= 0) target->age_rating = atoi(new_val);
        else printf("Invalid age rating, keeping current value.\n");
    }

    // detail
    printf("Detail [%s] : ", target->detail);
    fgets(new_val, sizeof(new_val), stdin);
    new_val[strcspn(new_val, "\n")] = '\0';
    if (strlen(new_val) != 0) strcpy(target->detail, new_val);

    // simpan
    btree_save_to_file();

    printf("Film with ID %d updated successfully!\n", edit_id);
    system("pause");
    film_manage();
}

// VIEW FILM //
void view_film() {
    system("cls");

    Film result[200];
    int count = 0;

    btree_inorder(film_tree, result, &count);

    printf("============================================\n");
    printf("                 ALL FILMS                  \n");
    printf("    (Displayed via B-Tree In-Order)         \n");
    printf("============================================\n");
    printf("%-5s %-20s %-15s %-8s %-5s %-30s\n",
           "ID", "Title", "Genre", "Duration", "Age", "Detail");
    printf("--------------------------------------------\n");

    for (int i = 0; i < count; i++) {
        printf("%-5d %-20s %-15s %-8d %-5d %-30s\n", result[i].id, result[i].title, result[i].genre, result[i].duration, result[i].age_rating, result[i].detail);
    }

    printf("--------------------------------------------\n");
    if (count == 0) {
        printf("No films available.\n");
    } 
    else {
        printf("Total: %d film(s)\n", count);
    }
    printf("============================================\n");
    system("pause");
    film_manage();
}

// SEARCH FILM //
void search_film() {
    system("cls");

    printf("============================================\n");
    printf("              SEARCH FILM BY ID             \n");
    printf("      (Using B-Tree Search O(log n))        \n");
    printf("--------------------------------------------\n");

    char input[10];
    int search_id;
    int valid;

    do {
        printf("Enter Film ID to search : ");
        scanf("%s", input);
        valid = 1;
        for (int i = 0; input[i] != '\0'; i++) {
            if (!isdigit(input[i])) { 
                valid = 0; break; 
            }
        }
        if (!valid) printf("ID must be a number!\n");
    } while(!valid);

    search_id = atoi(input);

    // search B-tree
    Film* found = btree_search(film_tree, search_id);

    printf("--------------------------------------------\n");
    if (found != NULL) {
        printf("Film Found!\n");
        printf("--------------------------------------------\n");
        printf("  ID       : %d\n", found->id);
        printf("  Title    : %s\n", found->title);
        printf("  Genre    : %s\n", found->genre);
        printf("  Duration : %d min\n", found->duration);
        printf("  Age      : %d+\n", found->age_rating);
        printf("  Detail   : %s\n", found->detail);
    } 
    else {
        printf("Film with ID %d not found!\n", search_id);
    }
    printf("============================================\n");
    system("pause");
    film_manage();
}

// SUB-MENU STUDIO & SCHEDULE MANAGEMENT // 
void schedule_manage () {
    int choice;

    do {
        system("cls");
        printf("============================================\n");
        printf("          STUDIO & SCHEDULE MANAGEMENT      \n");
        printf("--------------------------------------------\n");
        printf("[1] Add New Studio\n");
        printf("[2] Delete Studio\n");
        printf("[3] Add Schedule\n");
        printf("[4] Edit Schedule\n");
        printf("[5] Delete Schedule\n");
        printf("[6] View All Active Schedules\n");
        printf("[0] Back\n");
        printf("============================================\n");    
        printf("Choose : ");
        scanf("%d", &choice);
        
        if (choice < 0 || choice > 6) {
            invalid_choice();
        }
    } while(choice < 0 || choice > 6); 

    switch (choice) {
        case 1 : 
            add_studio();
            break;
        case 2 : 
            del_studio();
            break;
        case 3 : 
            add_schedule();
            break;
        case 4 : 
            edit_schedule();
            break;
        case 5 : 
            del_schedule();
            break;
        case 6 : 
            view_schedule();
            break;
        case 0 : 
            menu_admin();
            break;
    }
}

// ADD STUDIO //
void add_studio () {

}

// DELETE STUDIO //
void del_studio () {

}

// ADD SCHEDULE //
void add_schedule () {

}

// EDIT SCHEDULE //
void edit_schedule () {

}

// DELETE SCHEDULE //
void del_schedule () {

}

// VIEW SCHEDULE //
void view_schedule () {
    
}

// SUB-MENU USER ACCOUNT MANAGEMENT // 
void acc_manage () {
    int choice;

    do {
        system("cls");
        printf("============================================\n");
        printf("           USER ACCOUNT MANAGEMENT          \n");
        printf("--------------------------------------------\n");
        printf("[1] View All Users\n");
        printf("[2] Search User by Username\n");
        printf("[3] Delete User Account\n");
        printf("[0] Back\n");
        printf("============================================\n");    
        printf("Choose : ");
        scanf("%d", &choice);
        
        if (choice < 0 || choice > 3) {
            invalid_choice();
        }
    } while(choice < 0 || choice > 3); 

    switch (choice) {
        case 1 : 
            view_users();
            break;
        case 2 : 
            search_user();
            break;
        case 3 : 
            delete_user();
            break;
        case 0 : 
            menu_admin();
            break;
    }
}

// VIEW USERS //
void view_users() {
    system("cls");

    FILE* data = fopen(account_file, "r");
    if (data == NULL) {
        invalid_file();
        return;
    }

    char buffer[500];
    int count = 0;

    printf("============================================\n");
    printf("               ALL USERS                    \n");
    printf("============================================\n");
    printf("%-20s %-25s %-30s\n", "Username", "Full Name", "Email");
    printf("--------------------------------------------\n");

    while (fgets(buffer, sizeof(buffer), data)) {
        Account accounts;
        sscanf(buffer, "%[^,],%[^,],%[^,],%[^\n]", accounts.username, accounts.password, accounts.name, accounts.email);

        printf("%-20s %-25s %-30s\n", accounts.username, accounts.name, accounts.email);
        count++;
    }

    fclose(data);
    printf("--------------------------------------------\n");

    if (count == 0) {
        printf("No users available.\n");
    } 
    else {
        printf("Total: %d user(s)\n", count);
    }

    printf("============================================\n");
    system("pause");
    acc_manage();
}

// SEARCH USER //  --- DIBAIKIN LAGI
void search_user() {
    system("cls");

    char keyword[100];
    Account accounts[100];
    int n = 0;
    int found = 0;
    char buffer[500];

    FILE* data = fopen(account_file, "r");
    if (data == NULL) {
        invalid_file();
        return;
    }

    while (fgets(buffer, sizeof(buffer), data)) {
        buffer[strcspn(buffer, "\n")] = 0;
        sscanf(buffer, "%[^,],%[^,],%[^,],%[^\n]", accounts[n].username, accounts[n].password, accounts[n].name, accounts[n].email);
        n++;
    }
    fclose(data);

    printf("============================================\n");
    printf("           SEARCH USER BY KEYWORD           \n");
    printf("--------------------------------------------\n");

    printf("Input keyword to search : ");
    scanf("%s", keyword);

    printf("\n%-20s %-25s %-30s\n", "Username", "Full Name", "Email");
    printf("--------------------------------------------\n");

    for (int i = 0; i < n; i++) {
        if (strstr(accounts[i].username, keyword) != NULL) {
            printf("%-20s %-25s %-30s\n", accounts[i].username, accounts[i].name, accounts[i].email);
            found = 1;
        }
    }

    if (found == 0)
        printf("\nUser \"%s\" not found.\n", keyword);

    printf("\n============================================\n");
    system("pause");

    acc_manage();
}

// DELETE USER //
void delete_user() {
    system("cls");

    Account accounts[100];
    int n = 0;
    char buffer[500];
    char keyword[100];
    int found = 0;

    FILE* data = fopen(account_file, "r");
    FILE* temp = fopen("temp.txt", "w");

    if (data == NULL || temp == NULL) {
        invalid_file();
        return;
    }

    while (fgets(buffer, sizeof(buffer), data)) {
        buffer[strcspn(buffer, "\n")] = 0;
        sscanf(buffer, "%[^,],%[^,],%[^,],%[^\n]", accounts[n].username, accounts[n].password, accounts[n].name, accounts[n].email);
        n++;
    }
    fclose(data);

    printf("==============================\n");
    printf("          DELETE USER         \n");
    printf("==============================\n");
    printf("%-20s %-25s\n", "Username", "Full Name");
    printf("------------------------------\n");

    for (int i = 0; i < n; i++) {
        printf("%-20s %-25s\n", accounts[i].username, accounts[i].name);
    }
    printf("------------------------------\n");

    if (n == 0) {
        printf("No users available.\n");
        printf("==============================\n");
        fclose(temp);
        system("pause");
        acc_manage();
        return;
    }

    printf("==============================\n");

    // input username
    printf("Enter Username to delete : ");
    scanf("%s", keyword);

    for (int i = 0; i < n; i++) {
        if (strcmp(accounts[i].username, keyword) == 0) {
            found = 1;
        } else {
            fprintf(temp, "%s,%s,%s,%s\n", accounts[i].username, accounts[i].password, accounts[i].name, accounts[i].email);
        }
    }

    fclose(temp);

    if (found) {
        remove(account_file);
        rename("temp.txt", account_file);
        printf("User \"%s\" deleted successfully!\n", keyword);
    } 
    else {
        remove("temp.txt");
        printf("User \"%s\" not found!\n", keyword);
    }

    system("pause");
    acc_manage();
}

// MENU CUSTOMER // 
void menu_cust () {
    int choice;

    do {
        system("cls");
        printf("============================================\n");
        printf("               CUSTOMER MENU                \n");
        printf("--------------------------------------------\n");
        view_film_cust();
        printf("Welcome Back, %s!\n", current_user);
        printf("[1] Book Ticket\n");
        printf("[2] History\n");
        printf("[3] Cancel Ticket\n");
        printf("[4] Edit Profile\n");
        printf("[0] Logout\n");
        printf("============================================\n");    
        printf("Choose : ");
        scanf("%d", &choice);
        
        if (choice < 0 || choice > 4) {
            invalid_choice();
        }
    } while(choice < 0 || choice > 4); 

    switch (choice) {
        case 1 : 
            book_ticket();
            break;
        case 2 : 
            history();
            break;
        case 3 : 
            cancel();
            break;
        case 4 : 
            edit_profile();
            break;
        case 0 : 
            main_menu();
            break;
    }
}

// VIEW FILM CUSTOMER //
void view_film_cust () {
    system("cls");

    FILE* data = fopen(film_file, "r");
    if (data == NULL) {
        invalid_file();
        return;
    }

    char buffer[500];
    int count = 0;

    printf("============================================\n");
    printf("                 NOW SHOWING                \n");
    printf("============================================\n");
    printf("%-5s %-20s %-15s %-8s %-5s %-30s\n", "ID", "Title", "Genre", "Duration", "Age", "Detail");
    printf("--------------------------------------------\n");

    while (fgets(buffer, sizeof(buffer), data)) {
        Film film;
        sscanf(buffer, "%d=%[^=]=%[^=]=%d=%d=%[^\n]", &film.id, film.title, film.genre, &film.duration, &film.age_rating, film.detail);

        printf("%-5d %-20s %-15s %-8d %-5d %-30s\n", film.id, film.title, film.genre, film.duration, film.age_rating, film.detail);
        count++;
    }

    fclose(data);
    printf("--------------------------------------------\n");
    printf("============================================\n");
}

// SUB-MENU BOOK TICKET //
void book_ticket () {
    // select film, schedule, seat, pay
}

// SUB-MENU HISTORY // 
void history () {
    // view all history transactions
}

// SUB-MENU CANCEL // 
void cancel () {
    // cancel pake booking code
}

// SUB-MENU EDIT PROFILE //
void edit_profile () {
    int choice;

    do {
        system("cls");
        printf("============================================\n");
        printf("               EDIT PROFILE                 \n");
        printf("--------------------------------------------\n");
        printf("[1] Change Full Name\n");
        printf("[2] Change Username\n");
        printf("[3] Change Email\n");
        printf("[4] Change Password\n");
        printf("[0] Back\n");
        printf("============================================\n");    
        printf("Choose : ");
        scanf("%d", &choice);
        
        if (choice < 0 || choice > 4) {
            invalid_choice();
        }
    } while(choice < 0 || choice > 4); 

    switch (choice) {
        case 1 : 
            change_name();
            break;
        case 2 : 
            change_usn();
            break;
        case 3 : 
            change_email();
            break;
        case 4 : 
            change_pass();
            break;
        case 0 : 
            menu_cust();
            break;
    }
}

// CHANGE NAME //
void change_name () {

}

// CHANGE USERNAME //
void change_usn () {

}

// CHANGE EMAIL //
void change_email () {

}

// CHANGE PASSWORD //
void change_pass () {

}

// MENU KASIR // 
void menu_cashier () {
    int choice;

    do {
        system("cls");
        printf("============================================\n");
        printf("               CASHIER MENU                 \n");
        printf("--------------------------------------------\n");
        printf("Welcome Back, Cashier!\n");
        printf("[1] Validate Ticket (Check-in)\n");
        printf("[2] Sell Ticket\n");
        printf("[3] Seat Status per Schedule\n");
        printf("[0] Logout\n");
        printf("============================================\n");    
        printf("Choose : ");
        scanf("%d", &choice);
        
        if (choice < 0 || choice > 3) {
            invalid_choice();
        }
    } while(choice < 0 || choice > 3); 

    switch (choice) {
        case 1 : 
            validate_ticket();
            break;
        case 2 : 
            sell();
            break;
        case 3 : 
            seat_status();
            break;
        case 0 : 
            main_menu();
            break;
    }
}

// VALIDATE TICKET //
void validate_ticket () {
    // konfirmasi tiket online
}

// SELL TICKET OFFLINE //
void sell () {
    // jual tiket offline
}

// SEAT STATUS //
void seat_status () {
    // ngatur kursi booked, tersedia
}