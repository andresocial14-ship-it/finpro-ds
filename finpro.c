#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

// !! DEFINE !! //
#define account_file  "users.txt"
#define film_file     "films.txt"
#define studio_file   "studios.txt"
#define schedule_file "schedules.txt"
#define booking_file  "bookings.txt"
#define T         3
#define MAX_KEYS  (2*T-1)
#define MIN_KEYS  (T-1)

// ============================================================
// !! STRUCT !!
// ============================================================
typedef struct {
    char username[100];
    char password[100];
    char name[100];
    char email[100];
} Account;

typedef struct {
    int  id;
    char title[100];
    char genre[100];
    int  duration;
    int  age_rating;
    char detail[300];
} Film;

typedef struct {
    int  id;
    char name[50];
    int  capacity;
    int  rows;
    int  cols;
} Studio;

typedef struct {
    int   id;
    int   film_id;
    int   studio_id;
    char  date[20];
    char  time[20];
    float price;
} Schedule;

typedef struct {
    char  booking_code[10];
    char  username[100];
    int   schedule_id;
    char  seat[10];
    float total_price;
    int   status;
} Booking;

typedef struct BTreeNode {
    int             n;
    Film            keys[MAX_KEYS];
    struct BTreeNode* children[MAX_KEYS + 1];
    int             leaf;
} BTreeNode;

// ============================================================
// !! GLOBAL VARIABLE !!
// ============================================================
char      current_user[100];
BTreeNode* film_tree = NULL;

// ============================================================
// !! FUNCTION PROTOTYPES !!
// ============================================================

// Utility
void invalid_choice();
void invalid_file();

// B-Tree
BTreeNode* btree_create_node(int leaf);
void       btree_insert(BTreeNode** root, Film film);
void       btree_insert_nonfull(BTreeNode* node, Film film);
void       btree_split_child(BTreeNode* parent, int i, BTreeNode* child);
Film*      btree_search(BTreeNode* root, int id);
void       btree_inorder(BTreeNode* root, Film* result, int* count);
void       btree_delete(BTreeNode** root, int id);
void       btree_delete_internal(BTreeNode* node, int id);
Film       btree_get_predecessor(BTreeNode* node, int idx);
Film       btree_get_successor(BTreeNode* node, int idx);
void       btree_fill(BTreeNode* node, int idx);
void       btree_borrow_from_prev(BTreeNode* node, int idx);
void       btree_borrow_from_next(BTreeNode* node, int idx);
void       btree_merge(BTreeNode* node, int idx);
void       btree_free(BTreeNode* root);
int        auto_id();

// Film file
void btree_load_from_file();
void btree_save_to_file();

// Studio & Schedule helpers
int  time_to_minutes(const char* time_str);
int  validate_date(const char* date);
int  validate_time(const char* time_str);
int  auto_id_studio();
int  auto_id_schedule();
int  studio_in_use(int studio_id);
int  check_schedule_conflict(int studio_id, const char* date, const char* new_time,
                              int new_duration_min, int skip_id);
int  find_studio(int id, Studio* out);
int  find_schedule(int id, Schedule* out);
void print_all_studios();
void print_all_films_simple();
void minutes_to_time_str(int minutes, char* out);

// Main menu
void main_menu();
void login();
void register_acc();

// Admin
void menu_admin();
void film_manage();
void add_film();
void del_film();
void edit_film();
void search_film();
void view_film();
void schedule_manage();
void add_studio();
void view_studios();
void del_studio();
void add_schedule();
void edit_schedule();
void del_schedule();
void view_schedule();
void acc_manage();
void view_users();
void search_user();
void delete_user();

// Customer
void menu_cust();
void view_film_cust();
void book_ticket();
void history();
void cancel();
void edit_profile();
void change_usn();
void change_name();
void change_email();
void change_pass();

// Cashier
void menu_cashier();
void validate_ticket();
void sell();
void seat_status();

// ============================================================
// MAIN
// ============================================================
int main() {
    main_menu();
    return 0;
}

// ============================================================
// !! UTILITY !!
// ============================================================
void invalid_choice() {
    printf("Invalid choice! Try again\n");
    system("pause");
}

void invalid_file() {
    printf("Failed to open file!\n");
    system("pause");
}

// ============================================================
// !! B-TREE IMPLEMENTATION !!
// ============================================================
BTreeNode* btree_create_node(int leaf) {
    BTreeNode* node = (BTreeNode*)malloc(sizeof(BTreeNode));
    if (!node) { printf("Memory allocation failed!\n"); exit(1); }
    node->n    = 0;
    node->leaf = leaf;
    for (int i = 0; i <= MAX_KEYS; i++) node->children[i] = NULL;
    return node;
}

void btree_insert(BTreeNode** root, Film film) {
    if (*root == NULL) {
        *root = btree_create_node(1);
        (*root)->keys[0] = film;
        (*root)->n = 1;
        return;
    }
    if ((*root)->n == MAX_KEYS) {
        BTreeNode* new_root = btree_create_node(0);
        new_root->children[0] = *root;
        btree_split_child(new_root, 0, *root);
        int i = 0;
        if (new_root->keys[0].id < film.id) i = 1;
        btree_insert_nonfull(new_root->children[i], film);
        *root = new_root;
    } else {
        btree_insert_nonfull(*root, film);
    }
}

void btree_insert_nonfull(BTreeNode* node, Film film) {
    int i = node->n - 1;
    if (node->leaf) {
        while (i >= 0 && node->keys[i].id > film.id) {
            node->keys[i + 1] = node->keys[i];
            i--;
        }
        node->keys[i + 1] = film;
        node->n++;
    } else {
        while (i >= 0 && node->keys[i].id > film.id) i--;
        i++;
        if (node->children[i]->n == MAX_KEYS) {
            btree_split_child(node, i, node->children[i]);
            if (node->keys[i].id < film.id) i++;
        }
        btree_insert_nonfull(node->children[i], film);
    }
}

void btree_split_child(BTreeNode* parent, int i, BTreeNode* child) {
    BTreeNode* new_node = btree_create_node(child->leaf);
    new_node->n = T - 1;
    for (int j = 0; j < T - 1; j++) new_node->keys[j] = child->keys[j + T];
    if (!child->leaf)
        for (int j = 0; j < T; j++) new_node->children[j] = child->children[j + T];
    child->n = T - 1;
    for (int j = parent->n; j >= i + 1; j--) parent->children[j + 1] = parent->children[j];
    parent->children[i + 1] = new_node;
    for (int j = parent->n - 1; j >= i; j--) parent->keys[j + 1] = parent->keys[j];
    parent->keys[i] = child->keys[T - 1];
    parent->n++;
}

Film* btree_search(BTreeNode* root, int id) {
    if (root == NULL) return NULL;
    int i = 0;
    while (i < root->n && id > root->keys[i].id) i++;
    if (i < root->n && id == root->keys[i].id) return &(root->keys[i]);
    if (root->leaf) return NULL;
    return btree_search(root->children[i], id);
}

void btree_inorder(BTreeNode* root, Film* result, int* count) {
    if (root == NULL) return;
    for (int i = 0; i < root->n; i++) {
        if (!root->leaf) btree_inorder(root->children[i], result, count);
        result[(*count)++] = root->keys[i];
    }
    if (!root->leaf) btree_inorder(root->children[root->n], result, count);
}

void btree_delete(BTreeNode** root, int id) {
    if (*root == NULL) { printf("Film with ID %d not found!\n", id); return; }
    btree_delete_internal(*root, id);
    if ((*root)->n == 0) {
        BTreeNode* old_root = *root;
        *root = (*root)->leaf ? NULL : (*root)->children[0];
        free(old_root);
    }
}

void btree_delete_internal(BTreeNode* node, int id) {
    int i = 0;
    while (i < node->n && id > node->keys[i].id) i++;
    if (i < node->n && id == node->keys[i].id) {
        if (node->leaf) {
            for (int j = i; j < node->n - 1; j++) node->keys[j] = node->keys[j + 1];
            node->n--;
        } else {
            if (node->children[i]->n >= T) {
                Film pred = btree_get_predecessor(node, i);
                node->keys[i] = pred;
                btree_delete_internal(node->children[i], pred.id);
            } else if (node->children[i + 1]->n >= T) {
                Film succ = btree_get_successor(node, i);
                node->keys[i] = succ;
                btree_delete_internal(node->children[i + 1], succ.id);
            } else {
                btree_merge(node, i);
                btree_delete_internal(node->children[i], id);
            }
        }
    } else {
        if (node->leaf) { printf("Film with ID %d not found!\n", id); return; }
        int last_child = (i == node->n);
        if (node->children[i]->n < T) btree_fill(node, i);
        if (last_child && i > node->n)
            btree_delete_internal(node->children[i - 1], id);
        else
            btree_delete_internal(node->children[i], id);
    }
}

Film btree_get_predecessor(BTreeNode* node, int idx) {
    BTreeNode* cur = node->children[idx];
    while (!cur->leaf) cur = cur->children[cur->n];
    return cur->keys[cur->n - 1];
}

Film btree_get_successor(BTreeNode* node, int idx) {
    BTreeNode* cur = node->children[idx + 1];
    while (!cur->leaf) cur = cur->children[0];
    return cur->keys[0];
}

void btree_fill(BTreeNode* node, int idx) {
    if (idx != 0 && node->children[idx - 1]->n >= T)
        btree_borrow_from_prev(node, idx);
    else if (idx != node->n && node->children[idx + 1]->n >= T)
        btree_borrow_from_next(node, idx);
    else {
        if (idx != node->n) btree_merge(node, idx);
        else btree_merge(node, idx - 1);
    }
}

void btree_borrow_from_prev(BTreeNode* node, int idx) {
    BTreeNode* child   = node->children[idx];
    BTreeNode* sibling = node->children[idx - 1];
    for (int i = child->n - 1; i >= 0; i--) child->keys[i + 1] = child->keys[i];
    if (!child->leaf)
        for (int i = child->n; i >= 0; i--) child->children[i + 1] = child->children[i];
    child->keys[0] = node->keys[idx - 1];
    if (!child->leaf) child->children[0] = sibling->children[sibling->n];
    node->keys[idx - 1] = sibling->keys[sibling->n - 1];
    child->n++;
    sibling->n--;
}

void btree_borrow_from_next(BTreeNode* node, int idx) {
    BTreeNode* child   = node->children[idx];
    BTreeNode* sibling = node->children[idx + 1];
    child->keys[child->n] = node->keys[idx];
    if (!child->leaf) child->children[child->n + 1] = sibling->children[0];
    node->keys[idx] = sibling->keys[0];
    for (int i = 1; i < sibling->n; i++) sibling->keys[i - 1] = sibling->keys[i];
    if (!sibling->leaf)
        for (int i = 1; i <= sibling->n; i++) sibling->children[i - 1] = sibling->children[i];
    child->n++;
    sibling->n--;
}

void btree_merge(BTreeNode* node, int idx) {
    BTreeNode* child   = node->children[idx];
    BTreeNode* sibling = node->children[idx + 1];
    child->keys[T - 1] = node->keys[idx];
    for (int i = 0; i < sibling->n; i++) child->keys[i + T] = sibling->keys[i];
    if (!child->leaf)
        for (int i = 0; i <= sibling->n; i++) child->children[i + T] = sibling->children[i];
    for (int i = idx + 1; i < node->n; i++) node->keys[i - 1] = node->keys[i];
    for (int i = idx + 2; i <= node->n; i++) node->children[i - 1] = node->children[i];
    child->n += sibling->n + 1;
    node->n--;
    free(sibling);
}

void btree_free(BTreeNode* root) {
    if (root == NULL) return;
    if (!root->leaf)
        for (int i = 0; i <= root->n; i++) btree_free(root->children[i]);
    free(root);
}

// ============================================================
// !! FILM FILE I/O !!
// ============================================================
void btree_load_from_file() {
    if (film_tree != NULL) { btree_free(film_tree); film_tree = NULL; }
    FILE* fp = fopen(film_file, "r");
    if (fp == NULL) return;
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

void btree_save_to_file() {
    Film result[200];
    int count = 0;
    btree_inorder(film_tree, result, &count);
    FILE* fp = fopen(film_file, "w");
    if (fp == NULL) { printf("Failed to save to file!\n"); return; }
    for (int i = 0; i < count; i++)
        fprintf(fp, "%d=%s=%s=%d=%d=%s\n",
                result[i].id, result[i].title, result[i].genre,
                result[i].duration, result[i].age_rating, result[i].detail);
    fclose(fp);
}

int auto_id() {
    Film result[200];
    int count = 0;
    btree_inorder(film_tree, result, &count);
    if (count == 0) return 0;
    return result[count - 1].id;
}

// ============================================================
// !! STUDIO & SCHEDULE HELPERS !!
// ============================================================

// Konversi "HH:MM" ke total menit
int time_to_minutes(const char* time_str) {
    int hh, mm;
    sscanf(time_str, "%d:%d", &hh, &mm);
    return hh * 60 + mm;
}

// Format menit -> "HH:MM"
void minutes_to_time_str(int minutes, char* out) {
    sprintf(out, "%02d:%02d", minutes / 60, minutes % 60);
}

// Validasi tanggal YYYY-MM-DD
int validate_date(const char* date) {
    if (strlen(date) != 10) return 0;
    if (date[4] != '-' || date[7] != '-') return 0;
    for (int i = 0; i < 10; i++) {
        if (i == 4 || i == 7) continue;
        if (!isdigit(date[i])) return 0;
    }
    int year, month, day;
    sscanf(date, "%d-%d-%d", &year, &month, &day);
    if (month < 1 || month > 12) return 0;
    if (day < 1) return 0;
    int days_in_month[] = {0,31,28,31,30,31,30,31,31,30,31,30,31};
    if ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0))
        days_in_month[2] = 29;
    if (day > days_in_month[month]) return 0;
    return 1;
}

// Validasi jam HH:MM
int validate_time(const char* time_str) {
    if (strlen(time_str) != 5) return 0;
    if (time_str[2] != ':') return 0;
    for (int i = 0; i < 5; i++) {
        if (i == 2) continue;
        if (!isdigit(time_str[i])) return 0;
    }
    int hh, mm;
    sscanf(time_str, "%d:%d", &hh, &mm);
    if (hh < 0 || hh > 23) return 0;
    if (mm < 0 || mm > 59) return 0;
    return 1;
}

// Auto ID studio
int auto_id_studio() {
    FILE* fp = fopen(studio_file, "r");
    if (fp == NULL) return 1;
    char buffer[200];
    int max_id = 0;
    while (fgets(buffer, sizeof(buffer), fp)) {
        int id; sscanf(buffer, "%d=", &id);
        if (id > max_id) max_id = id;
    }
    fclose(fp);
    return max_id + 1;
}

// Auto ID schedule
int auto_id_schedule() {
    FILE* fp = fopen(schedule_file, "r");
    if (fp == NULL) return 1;
    char buffer[300];
    int max_id = 0;
    while (fgets(buffer, sizeof(buffer), fp)) {
        int id; sscanf(buffer, "%d=", &id);
        if (id > max_id) max_id = id;
    }
    fclose(fp);
    return max_id + 1;
}

// Cek apakah studio masih dipakai schedule
int studio_in_use(int studio_id) {
    FILE* fp = fopen(schedule_file, "r");
    if (fp == NULL) return 0;
    char buffer[300];
    while (fgets(buffer, sizeof(buffer), fp)) {
        Schedule sch;
        sscanf(buffer, "%d=%d=%d=%[^=]=%[^=]=%f",
               &sch.id, &sch.film_id, &sch.studio_id,
               sch.date, sch.time, &sch.price);
        if (sch.studio_id == studio_id) { fclose(fp); return 1; }
    }
    fclose(fp);
    return 0;
}

// Cek bentrok jadwal; return ID schedule yang bentrok, atau -1
int check_schedule_conflict(int studio_id, const char* date, const char* new_time,
                             int new_duration_min, int skip_id) {
    FILE* fp = fopen(schedule_file, "r");
    if (fp == NULL) return -1;
    int new_start = time_to_minutes(new_time);
    int new_end   = new_start + new_duration_min;
    char buffer[300];
    while (fgets(buffer, sizeof(buffer), fp)) {
        buffer[strcspn(buffer, "\n")] = 0;
        Schedule sch;
        sscanf(buffer, "%d=%d=%d=%[^=]=%[^=]=%f",
               &sch.id, &sch.film_id, &sch.studio_id,
               sch.date, sch.time, &sch.price);
        if (sch.id == skip_id) continue;
        if (sch.studio_id != studio_id) continue;
        if (strcmp(sch.date, date) != 0) continue;
        Film* f = btree_search(film_tree, sch.film_id);
        if (f == NULL) continue;
        int existing_start = time_to_minutes(sch.time);
        int existing_end   = existing_start + f->duration;
        if (new_start < existing_end && new_end > existing_start) {
            fclose(fp);
            return sch.id;
        }
    }
    fclose(fp);
    return -1;
}

// Cari studio by ID
int find_studio(int id, Studio* out) {
    FILE* fp = fopen(studio_file, "r");
    if (fp == NULL) return 0;
    char buffer[200];
    while (fgets(buffer, sizeof(buffer), fp)) {
        buffer[strcspn(buffer, "\n")] = 0;
        Studio s;
        sscanf(buffer, "%d=%[^=]=%d=%d=%d",
               &s.id, s.name, &s.capacity, &s.rows, &s.cols);
        if (s.id == id) {
            if (out != NULL) *out = s;
            fclose(fp); return 1;
        }
    }
    fclose(fp);
    return 0;
}

// Cari schedule by ID
int find_schedule(int id, Schedule* out) {
    FILE* fp = fopen(schedule_file, "r");
    if (fp == NULL) return 0;
    char buffer[300];
    while (fgets(buffer, sizeof(buffer), fp)) {
        buffer[strcspn(buffer, "\n")] = 0;
        Schedule sch;
        sscanf(buffer, "%d=%d=%d=%[^=]=%[^=]=%f",
               &sch.id, &sch.film_id, &sch.studio_id,
               sch.date, sch.time, &sch.price);
        if (sch.id == id) {
            if (out != NULL) *out = sch;
            fclose(fp); return 1;
        }
    }
    fclose(fp);
    return 0;
}

// Tampilkan semua studio (tabel)
void print_all_studios() {
    FILE* fp = fopen(studio_file, "r");
    printf("%-5s %-20s %-10s %-6s %-6s\n", "ID", "Name", "Capacity", "Rows", "Cols");
    printf("----------------------------------------------------\n");
    int count = 0;
    if (fp != NULL) {
        char buffer[200];
        while (fgets(buffer, sizeof(buffer), fp)) {
            buffer[strcspn(buffer, "\n")] = 0;
            Studio s;
            sscanf(buffer, "%d=%[^=]=%d=%d=%d",
                   &s.id, s.name, &s.capacity, &s.rows, &s.cols);
            printf("%-5d %-20s %-10d %-6d %-6d\n",
                   s.id, s.name, s.capacity, s.rows, s.cols);
            count++;
        }
        fclose(fp);
    }
    if (count == 0) printf("No studios available.\n");
}

// Tampilkan semua film (ringkas, untuk pemilihan)
void print_all_films_simple() {
    Film result[200];
    int count = 0;
    btree_inorder(film_tree, result, &count);
    printf("%-5s %-25s %-15s %-8s\n", "ID", "Title", "Genre", "Duration");
    printf("----------------------------------------------------\n");
    for (int i = 0; i < count; i++)
        printf("%-5d %-25s %-15s %-4d min\n",
               result[i].id, result[i].title,
               result[i].genre, result[i].duration);
    if (count == 0) printf("No films available.\n");
}

// ============================================================
// !! MAIN MENU !!
// ============================================================
void main_menu() {
    int choice;
    do {
        system("cls");
        printf("============================================\n");
        printf("          CINEMA BOOKING SYSTEM             \n");
        printf("--------------------------------------------\n");
        printf("[1] Login\n");
        printf("[2] Register New Account\n");
        printf("[0] Exit\n");
        printf("============================================\n");
        printf("Choose : ");
        scanf("%d", &choice);
        if (choice < 0 || choice > 2) invalid_choice();
    } while (choice < 0 || choice > 2);

    switch (choice) {
        case 1: login();        break;
        case 2: register_acc(); break;
        case 0: return;
    }
}

// ============================================================
// !! LOGIN !!
// ============================================================
void login() {
    system("cls");
    char username[100], password[100];
    char fileUsername[100], filePassword[100];
    char buffer[200];
    int  success = 0;

    printf("============================================\n");
    printf("                   LOGIN                    \n");
    printf("--------------------------------------------\n");

    do {
        printf("Username : ");
        scanf("%s", username);
        if (strlen(username) == 0) printf("Username cannot be empty!\n");
    } while (strlen(username) == 0);

    do {
        printf("Password : ");
        scanf("%s", password);
        if (strlen(password) == 0) printf("Password cannot be empty!\n");
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
    if (fp == NULL) { invalid_file(); return; }

    while (fgets(buffer, sizeof(buffer), fp)) {
        buffer[strcspn(buffer, "\n")] = 0;
        sscanf(buffer, "%[^,],%[^,]", fileUsername, filePassword);
        if (strcmp(username, fileUsername) == 0 &&
            strcmp(password, filePassword) == 0) {
            success = 1;
            strcpy(current_user, username);
            break;
        }
    }
    fclose(fp);

    if (success) { menu_cust(); return; }

    printf("Wrong username or password!\n");
    system("pause");
    login();
}

// ============================================================
// !! REGISTER !!
// ============================================================
void register_acc() {
    system("cls");

    char fileUsername[100];
    char confirm_pass[100];
    char buffer[1024];
    int  found, valid, validasi_at;

    FILE* reg = fopen(account_file, "a");
    if (reg == NULL) { invalid_file(); return; }

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
            printf("Full name cannot be empty!\n"); valid = 0;
        } else {
            for (int i = 0; customer.name[i] != '\0'; i++) {
                if (isdigit(customer.name[i])) {
                    printf("Full name cannot contain numbers!\n");
                    valid = 0; break;
                }
            }
        }
    } while (strlen(customer.name) == 0 || !valid);

    // email
    do {
        printf("Email         : ");
        scanf("%s", customer.email);
        validasi_at = 0;
        for (int i = 0; customer.email[i] != '\0'; i++)
            if (customer.email[i] == '@') { validasi_at = 1; break; }
        if (strlen(customer.email) == 0) printf("Email cannot be empty!\n");
        else if (!validasi_at) printf("Email must contain '@'!\n");
    } while (strlen(customer.email) == 0 || !validasi_at);

    // username
    do {
        printf("Username      : ");
        scanf("%s", customer.username);
        found = 0;
        FILE* check = fopen(account_file, "r");
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
    } while (found);

    // password
    do {
        printf("Password      : ");
        scanf("%s", customer.password);
        if (strlen(customer.password) == 0) printf("Password cannot be empty!\n");
        else if (strlen(customer.password) < 5)
            printf("Password must be at least 5 characters!\n");
    } while (strlen(customer.password) < 5);

    // confirm password
    do {
        printf("Confirm Password : ");
        scanf("%s", confirm_pass);
        if (strlen(confirm_pass) == 0) printf("Confirm password cannot be empty!\n");
        else if (strcmp(customer.password, confirm_pass) != 0)
            printf("Password does not match! Try again!\n");
    } while (strcmp(customer.password, confirm_pass) != 0);

    fprintf(reg, "%s,%s,%s,%s\n",
            customer.username, customer.password,
            customer.name, customer.email);
    fclose(reg);

    system("cls");
    printf("============================================\n");
    printf("       Account created successfully!        \n");
    printf("============================================\n");
    system("pause");
    main_menu();
}

// ============================================================
// !! ADMIN MENU !!
// ============================================================
void menu_admin() {
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
        if (choice < 0 || choice > 3) invalid_choice();
    } while (choice < 0 || choice > 3);

    switch (choice) {
        case 1: film_manage();    break;
        case 2: schedule_manage(); break;
        case 3: acc_manage();     break;
        case 0: main_menu();      break;
    }
}

// ============================================================
// !! FILM MANAGEMENT !!
// ============================================================
void film_manage() {
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
        if (choice < 0 || choice > 5) invalid_choice();
    } while (choice < 0 || choice > 5);

    switch (choice) {
        case 1: add_film();    break;
        case 2: del_film();    break;
        case 3: edit_film();   break;
        case 4: view_film();   break;
        case 5: search_film(); break;
        case 0: menu_admin();  break;
    }
}

void add_film() {
    system("cls");
    Film film;
    int valid;

    printf("============================================\n");
    printf("                 ADD FILM                   \n");
    printf("--------------------------------------------\n");

    do {
        printf("Title           : ");
        scanf(" %[^\n]", film.title);
        valid = 1;
        if (strlen(film.title) == 0) { printf("Title cannot be empty!\n"); valid = 0; }
    } while (!valid);

    do {
        printf("Genre           : ");
        scanf(" %[^\n]", film.genre);
        valid = 1;
        if (strlen(film.genre) == 0) { printf("Genre cannot be empty!\n"); valid = 0; }
    } while (!valid);

    do {
        printf("Duration (min)  : ");
        scanf("%d", &film.duration);
        if (film.duration <= 0) printf("Duration must be greater than 0!\n");
    } while (film.duration <= 0);

    do {
        printf("Age Rating (n+) : ");
        scanf("%d", &film.age_rating);
        if (film.age_rating < 0) printf("Age rating cannot be negative!\n");
    } while (film.age_rating < 0);

    do {
        printf("Synopsis        : ");
        scanf(" %[^\n]", film.detail);
        valid = 1;
        if (strlen(film.detail) == 0) { printf("Detail cannot be empty!\n"); valid = 0; }
    } while (!valid);

    film.id = auto_id() + 1;
    btree_insert(&film_tree, film);
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
    for (int i = 0; i < count; i++)
        printf("%-5d %-20s\n", result[i].id, result[i].title);

    if (count == 0) {
        printf("No films available.\n");
        printf("==============================\n");
        system("pause");
        film_manage();
        return;
    }
    printf("==============================\n");

    char input[10];
    int del_id, valid;

    do {
        printf("Enter Film ID to delete : ");
        scanf("%s", input);
        valid = 1;
        for (int i = 0; input[i] != '\0'; i++)
            if (!isdigit(input[i])) { valid = 0; break; }
        if (!valid) printf("ID must be a number!\n");
    } while (!valid);

    del_id = atoi(input);
    Film* found = btree_search(film_tree, del_id);
    if (found == NULL) {
        printf("Film with ID %d not found!\n", del_id);
        system("pause");
        film_manage();
        return;
    }

    btree_delete(&film_tree, del_id);
    btree_save_to_file();
    printf("Film with ID %d deleted successfully!\n", del_id);
    system("pause");
    film_manage();
}

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
    for (int i = 0; i < count; i++)
        printf("%-5d %-20s\n", result[i].id, result[i].title);

    if (count == 0) {
        printf("No films available.\n");
        printf("==============================\n");
        system("pause");
        film_manage();
        return;
    }
    printf("==============================\n");

    char input[10];
    int edit_id, valid;

    do {
        printf("Enter Film ID to edit : ");
        scanf("%s", input);
        valid = 1;
        for (int i = 0; input[i] != '\0'; i++)
            if (!isdigit(input[i])) { valid = 0; break; }
        if (!valid) printf("ID must be a number!\n");
    } while (!valid);

    edit_id = atoi(input);
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

    printf("Title [%s] : ", target->title);
    fgets(new_val, sizeof(new_val), stdin);
    new_val[strcspn(new_val, "\n")] = '\0';
    if (strlen(new_val) != 0) strcpy(target->title, new_val);

    printf("Genre [%s] : ", target->genre);
    fgets(new_val, sizeof(new_val), stdin);
    new_val[strcspn(new_val, "\n")] = '\0';
    if (strlen(new_val) != 0) strcpy(target->genre, new_val);

    printf("Duration [%d] : ", target->duration);
    fgets(new_val, sizeof(new_val), stdin);
    new_val[strcspn(new_val, "\n")] = '\0';
    if (strlen(new_val) != 0) {
        valid = 1;
        for (int i = 0; new_val[i] != '\0'; i++)
            if (!isdigit(new_val[i])) { valid = 0; break; }
        if (valid && atoi(new_val) > 0) target->duration = atoi(new_val);
        else printf("Invalid duration, keeping current value.\n");
    }

    printf("Age Rating [%d] : ", target->age_rating);
    fgets(new_val, sizeof(new_val), stdin);
    new_val[strcspn(new_val, "\n")] = '\0';
    if (strlen(new_val) != 0) {
        valid = 1;
        for (int i = 0; new_val[i] != '\0'; i++)
            if (!isdigit(new_val[i])) { valid = 0; break; }
        if (valid && atoi(new_val) >= 0) target->age_rating = atoi(new_val);
        else printf("Invalid age rating, keeping current value.\n");
    }

    printf("Detail [%s] : ", target->detail);
    fgets(new_val, sizeof(new_val), stdin);
    new_val[strcspn(new_val, "\n")] = '\0';
    if (strlen(new_val) != 0) strcpy(target->detail, new_val);

    btree_save_to_file();
    printf("Film with ID %d updated successfully!\n", edit_id);
    system("pause");
    film_manage();
}

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
    for (int i = 0; i < count; i++)
        printf("%-5d %-20s %-15s %-8d %-5d %-30s\n",
               result[i].id, result[i].title, result[i].genre,
               result[i].duration, result[i].age_rating, result[i].detail);
    printf("--------------------------------------------\n");
    if (count == 0) printf("No films available.\n");
    else printf("Total: %d film(s)\n", count);
    printf("============================================\n");
    system("pause");
    film_manage();
}

void search_film() {
    system("cls");
    printf("============================================\n");
    printf("              SEARCH FILM BY ID             \n");
    printf("      (Using B-Tree Search O(log n))        \n");
    printf("--------------------------------------------\n");

    char input[10];
    int search_id, valid;

    do {
        printf("Enter Film ID to search : ");
        scanf("%s", input);
        valid = 1;
        for (int i = 0; input[i] != '\0'; i++)
            if (!isdigit(input[i])) { valid = 0; break; }
        if (!valid) printf("ID must be a number!\n");
    } while (!valid);

    search_id = atoi(input);
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
    } else {
        printf("Film with ID %d not found!\n", search_id);
    }
    printf("============================================\n");
    system("pause");
    film_manage();
}

// ============================================================
// !! STUDIO & SCHEDULE MANAGEMENT !!
// ============================================================
void schedule_manage() {
    int choice;
    do {
        system("cls");
        printf("============================================\n");
        printf("       STUDIO & SCHEDULE MANAGEMENT         \n");
        printf("--------------------------------------------\n");
        printf("[1] Add New Studio\n");
        printf("[2] View All Studios\n");
        printf("[3] Delete Studio\n");
        printf("[4] Add Schedule\n");
        printf("[5] Edit Schedule\n");
        printf("[6] Delete Schedule\n");
        printf("[7] View All Schedules\n");
        printf("[0] Back\n");
        printf("============================================\n");
        printf("Choose : ");
        scanf("%d", &choice);
        if (choice < 0 || choice > 7) invalid_choice();
    } while (choice < 0 || choice > 7);

    switch (choice) {
        case 1: add_studio();    break;
        case 2: view_studios();  break;
        case 3: del_studio();    break;
        case 4: add_schedule();  break;
        case 5: edit_schedule(); break;
        case 6: del_schedule();  break;
        case 7: view_schedule(); break;
        case 0: menu_admin();    break;
    }
}

// ADD STUDIO
void add_studio() {
    system("cls");
    printf("============================================\n");
    printf("               ADD NEW STUDIO               \n");
    printf("--------------------------------------------\n");

    Studio s;

    do {
        printf("Studio Name : ");
        scanf(" %[^\n]", s.name);
        if (strlen(s.name) == 0) printf("Name cannot be empty!\n");
    } while (strlen(s.name) == 0);

    do {
        printf("Rows        : ");
        scanf("%d", &s.rows);
        if (s.rows <= 0) printf("Rows must be greater than 0!\n");
    } while (s.rows <= 0);

    do {
        printf("Cols        : ");
        scanf("%d", &s.cols);
        if (s.cols <= 0) printf("Cols must be greater than 0!\n");
    } while (s.cols <= 0);

    s.capacity = s.rows * s.cols;
    s.id = auto_id_studio();

    printf("Capacity    : %d (auto = %d rows x %d cols)\n",
           s.capacity, s.rows, s.cols);

    FILE* fp = fopen(studio_file, "a");
    if (fp == NULL) { invalid_file(); return; }
    fprintf(fp, "%d=%s=%d=%d=%d\n", s.id, s.name, s.capacity, s.rows, s.cols);
    fclose(fp);

    printf("--------------------------------------------\n");
    printf("Studio \"%s\" added successfully! (ID: %d)\n", s.name, s.id);
    printf("============================================\n");
    system("pause");
    schedule_manage();
}

// VIEW ALL STUDIOS
void view_studios() {
    system("cls");
    printf("============================================\n");
    printf("               ALL STUDIOS                  \n");
    printf("============================================\n");

    FILE* fp = fopen(studio_file, "r");
    int count = 0;
    printf("%-5s %-20s %-10s %-6s %-6s\n", "ID", "Name", "Capacity", "Rows", "Cols");
    printf("--------------------------------------------\n");
    if (fp != NULL) {
        char buffer[200];
        while (fgets(buffer, sizeof(buffer), fp)) {
            buffer[strcspn(buffer, "\n")] = 0;
            Studio s;
            sscanf(buffer, "%d=%[^=]=%d=%d=%d",
                   &s.id, s.name, &s.capacity, &s.rows, &s.cols);
            printf("%-5d %-20s %-10d %-6d %-6d\n",
                   s.id, s.name, s.capacity, s.rows, s.cols);
            count++;
        }
        fclose(fp);
    }
    printf("--------------------------------------------\n");
    if (count == 0) printf("No studios available.\n");
    else printf("Total: %d studio(s)\n", count);
    printf("============================================\n");
    system("pause");
    schedule_manage();
}

// DELETE STUDIO
void del_studio() {
    system("cls");
    printf("============================================\n");
    printf("               DELETE STUDIO                \n");
    printf("--------------------------------------------\n");
    print_all_studios();
    printf("============================================\n");

    // cek file ada isinya
    FILE* fp = fopen(studio_file, "r");
    if (fp == NULL) {
        printf("No studios available.\n");
        system("pause");
        schedule_manage();
        return;
    }
    char buf[10];
    int empty = (fgets(buf, sizeof(buf), fp) == NULL);
    fclose(fp);
    if (empty) {
        printf("No studios available.\n");
        system("pause");
        schedule_manage();
        return;
    }

    char input[10];
    int del_id, valid;

    do {
        printf("Enter Studio ID to delete : ");
        scanf("%s", input);
        valid = 1;
        for (int i = 0; input[i] != '\0'; i++)
            if (!isdigit(input[i])) { valid = 0; break; }
        if (!valid) printf("ID must be a number!\n");
    } while (!valid);

    del_id = atoi(input);

    Studio s;
    if (!find_studio(del_id, &s)) {
        printf("Studio with ID %d not found!\n", del_id);
        system("pause");
        schedule_manage();
        return;
    }

    if (studio_in_use(del_id)) {
        printf("--------------------------------------------\n");
        printf("Cannot delete \"%s\"!\n", s.name);
        printf("This studio is still used by active schedules.\n");
        printf("Please delete the related schedules first.\n");
        printf("--------------------------------------------\n");
        system("pause");
        schedule_manage();
        return;
    }

    printf("Are you sure you want to delete studio \"%s\"? (y/n) : ", s.name);
    char confirm;
    scanf(" %c", &confirm);
    if (confirm != 'y' && confirm != 'Y') {
        printf("Deletion cancelled.\n");
        system("pause");
        schedule_manage();
        return;
    }

    FILE* in  = fopen(studio_file, "r");
    FILE* tmp = fopen("temp_studio.txt", "w");
    if (in == NULL || tmp == NULL) { invalid_file(); return; }

    char buffer[200];
    while (fgets(buffer, sizeof(buffer), in)) {
        buffer[strcspn(buffer, "\n")] = 0;
        int id; sscanf(buffer, "%d=", &id);
        if (id != del_id) fprintf(tmp, "%s\n", buffer);
    }
    fclose(in); fclose(tmp);
    remove(studio_file);
    rename("temp_studio.txt", studio_file);

    printf("Studio \"%s\" deleted successfully!\n", s.name);
    system("pause");
    schedule_manage();
}

// ADD SCHEDULE
void add_schedule() {
    system("cls");
    printf("============================================\n");
    printf("               ADD SCHEDULE                 \n");
    printf("--------------------------------------------\n");

    // cek ada film
    Film all_films[200];
    int film_count = 0;
    btree_inorder(film_tree, all_films, &film_count);
    if (film_count == 0) {
        printf("No films available. Please add a film first.\n");
        system("pause");
        schedule_manage();
        return;
    }

    Schedule sch;
    char input[20];
    int valid;

    // Pilih Film
    printf("=== SELECT FILM ===\n");
    print_all_films_simple();
    printf("--------------------------------------------\n");
    do {
        printf("Enter Film ID : ");
        scanf("%s", input);
        valid = 1;
        for (int i = 0; input[i] != '\0'; i++)
            if (!isdigit(input[i])) { valid = 0; break; }
        if (!valid) { printf("ID must be a number!\n"); continue; }
        sch.film_id = atoi(input);
        if (btree_search(film_tree, sch.film_id) == NULL) {
            printf("Film with ID %d not found!\n", sch.film_id);
            valid = 0;
        }
    } while (!valid);

    Film* selected_film = btree_search(film_tree, sch.film_id);

    // Pilih Studio
    printf("\n=== SELECT STUDIO ===\n");
    print_all_studios();
    printf("--------------------------------------------\n");

    // cek ada studio
    FILE* sfp = fopen(studio_file, "r");
    if (sfp == NULL) {
        printf("No studios available. Please add a studio first.\n");
        system("pause");
        schedule_manage();
        return;
    }
    char sbuf[10];
    int no_studio = (fgets(sbuf, sizeof(sbuf), sfp) == NULL);
    fclose(sfp);
    if (no_studio) {
        printf("No studios available. Please add a studio first.\n");
        system("pause");
        schedule_manage();
        return;
    }

    do {
        printf("Enter Studio ID : ");
        scanf("%s", input);
        valid = 1;
        for (int i = 0; input[i] != '\0'; i++)
            if (!isdigit(input[i])) { valid = 0; break; }
        if (!valid) { printf("ID must be a number!\n"); continue; }
        sch.studio_id = atoi(input);
        if (!find_studio(sch.studio_id, NULL)) {
            printf("Studio with ID %d not found!\n", sch.studio_id);
            valid = 0;
        }
    } while (!valid);

    // Input Tanggal
    printf("\n=== DATE & TIME ===\n");
    do {
        printf("Date (YYYY-MM-DD) : ");
        scanf("%s", sch.date);
        if (!validate_date(sch.date)) {
            printf("Invalid date! Format must be YYYY-MM-DD.\n");
            printf("  - Month : 01-12\n");
            printf("  - Day   : must match the month\n");
        }
    } while (!validate_date(sch.date));

    // Input Jam + Cek Bentrok
    int conflict_id;
    do {
        do {
            printf("Time (HH:MM)      : ");
            scanf("%s", sch.time);
            if (!validate_time(sch.time)) {
                printf("Invalid time! Format must be HH:MM.\n");
                printf("  - Hour   : 00-23\n");
                printf("  - Minute : 00-59\n");
            }
        } while (!validate_time(sch.time));

        conflict_id = check_schedule_conflict(
            sch.studio_id, sch.date, sch.time,
            selected_film->duration, -1);

        if (conflict_id != -1) {
            Schedule conflict_sch;
            find_schedule(conflict_id, &conflict_sch);
            Film*  cf = btree_search(film_tree, conflict_sch.film_id);
            Studio cs; find_studio(conflict_sch.studio_id, &cs);
            char end_time_str[10];
            int end_min = time_to_minutes(conflict_sch.time) + (cf ? cf->duration : 0);
            minutes_to_time_str(end_min, end_time_str);

            printf("--------------------------------------------\n");
            printf("SCHEDULE CONFLICT detected!\n");
            printf("  Conflicting Schedule ID : %d\n", conflict_id);
            printf("  Film    : %s\n", cf ? cf->title : "Unknown");
            printf("  Studio  : %s\n", cs.name);
            printf("  Date    : %s\n", conflict_sch.date);
            printf("  Time    : %s - %s\n", conflict_sch.time, end_time_str);
            printf("Your new schedule (%s, ~%d min) overlaps with the above.\n",
                   sch.time, selected_film->duration);
            printf("Please enter a different time.\n");
            printf("--------------------------------------------\n");
        }
    } while (conflict_id != -1);

    // Input Harga
    printf("\n=== PRICE ===\n");
    do {
        printf("Price (Rp) : ");
        scanf("%f", &sch.price);
        if (sch.price <= 0) printf("Price must be greater than 0!\n");
    } while (sch.price <= 0);

    sch.id = auto_id_schedule();

    FILE* fp = fopen(schedule_file, "a");
    if (fp == NULL) { invalid_file(); return; }
    fprintf(fp, "%d=%d=%d=%s=%s=%.0f\n",
            sch.id, sch.film_id, sch.studio_id,
            sch.date, sch.time, sch.price);
    fclose(fp);

    char end_time_str[10];
    minutes_to_time_str(time_to_minutes(sch.time) + selected_film->duration, end_time_str);
    Studio stu; find_studio(sch.studio_id, &stu);

    printf("--------------------------------------------\n");
    printf("Schedule added successfully!\n");
    printf("  ID      : %d\n", sch.id);
    printf("  Film    : %s\n", selected_film->title);
    printf("  Studio  : %s\n", stu.name);
    printf("  Date    : %s\n", sch.date);
    printf("  Time    : %s - %s\n", sch.time, end_time_str);
    printf("  Price   : Rp %.0f\n", sch.price);
    printf("============================================\n");
    system("pause");
    schedule_manage();
}

// EDIT SCHEDULE
void edit_schedule() {
    system("cls");
    printf("============================================\n");
    printf("               EDIT SCHEDULE                \n");
    printf("--------------------------------------------\n");

    FILE* fp = fopen(schedule_file, "r");
    if (fp == NULL) {
        printf("No schedules available.\n");
        system("pause");
        schedule_manage();
        return;
    }

    char buffer[300];
    int count = 0;
    printf("%-4s %-22s %-12s %-12s %-6s %s\n",
           "ID", "Film", "Studio", "Date", "Time", "Price");
    printf("--------------------------------------------\n");
    while (fgets(buffer, sizeof(buffer), fp)) {
        buffer[strcspn(buffer, "\n")] = 0;
        Schedule sch;
        sscanf(buffer, "%d=%d=%d=%[^=]=%[^=]=%f",
               &sch.id, &sch.film_id, &sch.studio_id,
               sch.date, sch.time, &sch.price);
        Film*  f = btree_search(film_tree, sch.film_id);
        Studio st; find_studio(sch.studio_id, &st);
        printf("%-4d %-22s %-12s %-12s %-6s Rp %.0f\n",
               sch.id, f ? f->title : "Unknown", st.name,
               sch.date, sch.time, sch.price);
        count++;
    }
    fclose(fp);

    if (count == 0) {
        printf("No schedules available.\n");
        system("pause");
        schedule_manage();
        return;
    }
    printf("--------------------------------------------\n");

    char input[20];
    int edit_id, valid;

    do {
        printf("Enter Schedule ID to edit : ");
        scanf("%s", input);
        valid = 1;
        for (int i = 0; input[i] != '\0'; i++)
            if (!isdigit(input[i])) { valid = 0; break; }
        if (!valid) { printf("ID must be a number!\n"); continue; }
        edit_id = atoi(input);
        Schedule tmp;
        if (!find_schedule(edit_id, &tmp)) {
            printf("Schedule with ID %d not found!\n", edit_id);
            valid = 0;
        }
    } while (!valid);

    Schedule sch;
    find_schedule(edit_id, &sch);

    printf("--------------------------------------------\n");
    printf("Leave blank to keep current value.\n");
    printf("--------------------------------------------\n");

    while (getchar() != '\n');
    char new_val[50];

    // Edit Studio
    printf("\n=== STUDIO ===\n");
    print_all_studios();
    printf("Current Studio ID [%d] : ", sch.studio_id);
    fgets(new_val, sizeof(new_val), stdin);
    new_val[strcspn(new_val, "\n")] = '\0';
    if (strlen(new_val) != 0) {
        valid = 1;
        for (int i = 0; new_val[i] != '\0'; i++)
            if (!isdigit(new_val[i])) { valid = 0; break; }
        if (valid) {
            int ns = atoi(new_val);
            if (find_studio(ns, NULL)) sch.studio_id = ns;
            else printf("Studio ID %d not found. Keeping current.\n", ns);
        } else printf("Invalid input. Keeping current studio.\n");
    }

    // Edit Film
    printf("\n=== FILM ===\n");
    print_all_films_simple();
    printf("Current Film ID [%d] : ", sch.film_id);
    fgets(new_val, sizeof(new_val), stdin);
    new_val[strcspn(new_val, "\n")] = '\0';
    if (strlen(new_val) != 0) {
        valid = 1;
        for (int i = 0; new_val[i] != '\0'; i++)
            if (!isdigit(new_val[i])) { valid = 0; break; }
        if (valid) {
            int nf = atoi(new_val);
            if (btree_search(film_tree, nf) != NULL) sch.film_id = nf;
            else printf("Film ID %d not found. Keeping current.\n", nf);
        } else printf("Invalid input. Keeping current film.\n");
    }

    Film* selected_film = btree_search(film_tree, sch.film_id);

    // Edit Tanggal
    printf("\n=== DATE ===\n");
    printf("Current Date [%s] : ", sch.date);
    fgets(new_val, sizeof(new_val), stdin);
    new_val[strcspn(new_val, "\n")] = '\0';
    if (strlen(new_val) != 0) {
        if (validate_date(new_val)) strcpy(sch.date, new_val);
        else printf("Invalid date format. Keeping current date.\n");
    }

    // Edit Jam + Cek Bentrok
    printf("\n=== TIME ===\n");
    char temp_time[20];
    printf("Current Time [%s] (leave blank to keep) : ", sch.time);
    fgets(new_val, sizeof(new_val), stdin);
    new_val[strcspn(new_val, "\n")] = '\0';
    strcpy(temp_time, (strlen(new_val) != 0) ? new_val : sch.time);

    int conflict_id;
    while (1) {
        if (!validate_time(temp_time)) {
            printf("Invalid time format (HH:MM, 00:00-23:59).\n");
            printf("Re-enter Time : ");
            scanf("%s", temp_time);
            while (getchar() != '\n');
            continue;
        }
        conflict_id = check_schedule_conflict(
            sch.studio_id, sch.date, temp_time,
            selected_film->duration, edit_id);

        if (conflict_id != -1) {
            Schedule cs; find_schedule(conflict_id, &cs);
            Film*  cf = btree_search(film_tree, cs.film_id);
            Studio cst; find_studio(cs.studio_id, &cst);
            char end_time_str[10];
            int end_min = time_to_minutes(cs.time) + (cf ? cf->duration : 0);
            minutes_to_time_str(end_min, end_time_str);
            printf("--------------------------------------------\n");
            printf("SCHEDULE CONFLICT detected!\n");
            printf("  Conflicting Schedule ID : %d\n", conflict_id);
            printf("  Film    : %s\n", cf ? cf->title : "Unknown");
            printf("  Studio  : %s\n", cst.name);
            printf("  Date    : %s\n", cs.date);
            printf("  Time    : %s - %s\n", cs.time, end_time_str);
            printf("Please enter a different time.\n");
            printf("--------------------------------------------\n");
            printf("Re-enter Time (HH:MM) : ");
            scanf("%s", temp_time);
            while (getchar() != '\n');
        } else break;
    }
    strcpy(sch.time, temp_time);

    // Edit Harga
    printf("\n=== PRICE ===\n");
    printf("Current Price [%.0f] : ", sch.price);
    fgets(new_val, sizeof(new_val), stdin);
    new_val[strcspn(new_val, "\n")] = '\0';
    if (strlen(new_val) != 0) {
        float np = atof(new_val);
        if (np > 0) sch.price = np;
        else printf("Invalid price. Keeping current.\n");
    }

    // Tulis ulang file
    FILE* in  = fopen(schedule_file, "r");
    FILE* tmp = fopen("temp_schedule.txt", "w");
    if (in == NULL || tmp == NULL) { invalid_file(); return; }
    char buf[300];
    while (fgets(buf, sizeof(buf), in)) {
        buf[strcspn(buf, "\n")] = 0;
        int id; sscanf(buf, "%d=", &id);
        if (id == edit_id)
            fprintf(tmp, "%d=%d=%d=%s=%s=%.0f\n",
                    sch.id, sch.film_id, sch.studio_id,
                    sch.date, sch.time, sch.price);
        else
            fprintf(tmp, "%s\n", buf);
    }
    fclose(in); fclose(tmp);
    remove(schedule_file);
    rename("temp_schedule.txt", schedule_file);

    printf("--------------------------------------------\n");
    printf("Schedule ID %d updated successfully!\n", edit_id);
    printf("============================================\n");
    system("pause");
    schedule_manage();
}

// DELETE SCHEDULE
void del_schedule() {
    system("cls");
    printf("============================================\n");
    printf("               DELETE SCHEDULE              \n");
    printf("--------------------------------------------\n");

    FILE* fp = fopen(schedule_file, "r");
    if (fp == NULL) {
        printf("No schedules available.\n");
        system("pause");
        schedule_manage();
        return;
    }

    char buffer[300];
    int count = 0;
    printf("%-4s %-22s %-12s %-12s %-6s %s\n",
           "ID", "Film", "Studio", "Date", "Time", "Price");
    printf("--------------------------------------------\n");
    while (fgets(buffer, sizeof(buffer), fp)) {
        buffer[strcspn(buffer, "\n")] = 0;
        Schedule sch;
        sscanf(buffer, "%d=%d=%d=%[^=]=%[^=]=%f",
               &sch.id, &sch.film_id, &sch.studio_id,
               sch.date, sch.time, &sch.price);
        Film*  f = btree_search(film_tree, sch.film_id);
        Studio st; find_studio(sch.studio_id, &st);
        printf("%-4d %-22s %-12s %-12s %-6s Rp %.0f\n",
               sch.id, f ? f->title : "Unknown", st.name,
               sch.date, sch.time, sch.price);
        count++;
    }
    fclose(fp);

    if (count == 0) {
        printf("No schedules available.\n");
        system("pause");
        schedule_manage();
        return;
    }
    printf("--------------------------------------------\n");

    char input[10];
    int del_id, valid;

    do {
        printf("Enter Schedule ID to delete : ");
        scanf("%s", input);
        valid = 1;
        for (int i = 0; input[i] != '\0'; i++)
            if (!isdigit(input[i])) { valid = 0; break; }
        if (!valid) { printf("ID must be a number!\n"); continue; }
        del_id = atoi(input);
        Schedule tmp;
        if (!find_schedule(del_id, &tmp)) {
            printf("Schedule with ID %d not found!\n", del_id);
            valid = 0;
        }
    } while (!valid);

    Schedule sch; find_schedule(del_id, &sch);
    Film*  f = btree_search(film_tree, sch.film_id);
    Studio st; find_studio(sch.studio_id, &st);

    printf("--------------------------------------------\n");
    printf("Schedule to delete:\n");
    printf("  Film   : %s\n", f ? f->title : "Unknown");
    printf("  Studio : %s\n", st.name);
    printf("  Date   : %s  Time: %s\n", sch.date, sch.time);
    printf("  Price  : Rp %.0f\n", sch.price);
    printf("--------------------------------------------\n");
    printf("Are you sure? (y/n) : ");
    char confirm;
    scanf(" %c", &confirm);

    if (confirm != 'y' && confirm != 'Y') {
        printf("Deletion cancelled.\n");
        system("pause");
        schedule_manage();
        return;
    }

    FILE* in  = fopen(schedule_file, "r");
    FILE* tmp = fopen("temp_schedule.txt", "w");
    if (in == NULL || tmp == NULL) { invalid_file(); return; }
    char buf[300];
    while (fgets(buf, sizeof(buf), in)) {
        buf[strcspn(buf, "\n")] = 0;
        int id; sscanf(buf, "%d=", &id);
        if (id != del_id) fprintf(tmp, "%s\n", buf);
    }
    fclose(in); fclose(tmp);
    remove(schedule_file);
    rename("temp_schedule.txt", schedule_file);

    printf("Schedule ID %d deleted successfully!\n", del_id);
    system("pause");
    schedule_manage();
}

// VIEW ALL SCHEDULES
void view_schedule() {
    system("cls");
    printf("============================================\n");
    printf("               ALL SCHEDULES                \n");
    printf("============================================\n");

    FILE* fp = fopen(schedule_file, "r");
    int count = 0;
    printf("%-4s %-22s %-12s %-12s %-8s %-8s %s\n",
           "ID", "Film", "Studio", "Date", "Start", "End", "Price");
    printf("--------------------------------------------\n");
    if (fp != NULL) {
        char buffer[300];
        while (fgets(buffer, sizeof(buffer), fp)) {
            buffer[strcspn(buffer, "\n")] = 0;
            Schedule sch;
            sscanf(buffer, "%d=%d=%d=%[^=]=%[^=]=%f",
                   &sch.id, &sch.film_id, &sch.studio_id,
                   sch.date, sch.time, &sch.price);
            Film*  f = btree_search(film_tree, sch.film_id);
            Studio st; find_studio(sch.studio_id, &st);
            char end_time_str[10] = "?";
            if (f != NULL)
                minutes_to_time_str(time_to_minutes(sch.time) + f->duration, end_time_str);
            printf("%-4d %-22s %-12s %-12s %-8s %-8s Rp %.0f\n",
                   sch.id, f ? f->title : "Unknown", st.name,
                   sch.date, sch.time, end_time_str, sch.price);
            count++;
        }
        fclose(fp);
    }
    printf("--------------------------------------------\n");
    if (count == 0) printf("No schedules available.\n");
    else printf("Total: %d schedule(s)\n", count);
    printf("============================================\n");
    system("pause");
    schedule_manage();
}

// ============================================================
// !! USER ACCOUNT MANAGEMENT !!
// ============================================================
void acc_manage() {
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
        if (choice < 0 || choice > 3) invalid_choice();
    } while (choice < 0 || choice > 3);

    switch (choice) {
        case 1: view_users();   break;
        case 2: search_user();  break;
        case 3: delete_user();  break;
        case 0: menu_admin();   break;
    }
}

void view_users() {
    system("cls");
    FILE* data = fopen(account_file, "r");
    if (data == NULL) { invalid_file(); return; }

    char buffer[500];
    int count = 0;

    printf("============================================\n");
    printf("                  ALL USERS                  \n");
    printf("============================================\n");
    printf("%-20s %-25s %-30s\n", "Username", "Full Name", "Email");
    printf("--------------------------------------------\n");

    while (fgets(buffer, sizeof(buffer), data)) {
        Account acc;
        sscanf(buffer, "%[^,],%[^,],%[^,],%[^\n]",
               acc.username, acc.password, acc.name, acc.email);
        printf("%-20s %-25s %-30s\n", acc.username, acc.name, acc.email);
        count++;
    }
    fclose(data);

    printf("--------------------------------------------\n");
    if (count == 0) printf("No users available.\n");
    else printf("Total: %d user(s)\n", count);
    printf("============================================\n");
    system("pause");
    acc_manage();
}

void search_user() {
    system("cls");
    char keyword[100];
    Account accounts[100];
    int n = 0, found = 0;
    char buffer[500];

    FILE* data = fopen(account_file, "r");
    if (data == NULL) { invalid_file(); return; }

    while (fgets(buffer, sizeof(buffer), data)) {
        buffer[strcspn(buffer, "\n")] = 0;
        sscanf(buffer, "%[^,],%[^,],%[^,],%[^\n]",
               accounts[n].username, accounts[n].password,
               accounts[n].name, accounts[n].email);
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
            printf("%-20s %-25s %-30s\n",
                   accounts[i].username, accounts[i].name, accounts[i].email);
            found = 1;
        }
    }
    if (!found) printf("\nUser \"%s\" not found.\n", keyword);

    printf("\n============================================\n");
    system("pause");
    acc_manage();
}

void delete_user() {
    system("cls");
    Account accounts[100];
    int n = 0;
    char buffer[500];
    char keyword[100];
    int found = 0;

    FILE* data = fopen(account_file, "r");
    FILE* temp = fopen("temp.txt", "w");
    if (data == NULL || temp == NULL) { invalid_file(); return; }

    while (fgets(buffer, sizeof(buffer), data)) {
        buffer[strcspn(buffer, "\n")] = 0;
        sscanf(buffer, "%[^,],%[^,],%[^,],%[^\n]",
               accounts[n].username, accounts[n].password,
               accounts[n].name, accounts[n].email);
        n++;
    }
    fclose(data);

    printf("==============================\n");
    printf("          DELETE USER         \n");
    printf("==============================\n");
    printf("%-20s %-25s\n", "Username", "Full Name");
    printf("------------------------------\n");
    for (int i = 0; i < n; i++)
        printf("%-20s %-25s\n", accounts[i].username, accounts[i].name);
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

    printf("Enter Username to delete : ");
    scanf("%s", keyword);

    for (int i = 0; i < n; i++) {
        if (strcmp(accounts[i].username, keyword) == 0) found = 1;
        else fprintf(temp, "%s,%s,%s,%s\n",
                     accounts[i].username, accounts[i].password,
                     accounts[i].name, accounts[i].email);
    }
    fclose(temp);

    if (found) {
        remove(account_file);
        rename("temp.txt", account_file);
        printf("User \"%s\" deleted successfully!\n", keyword);
    } else {
        remove("temp.txt");
        printf("User \"%s\" not found!\n", keyword);
    }
    system("pause");
    acc_manage();
}

// ============================================================
// !! CUSTOMER MENU !!
// ============================================================
void menu_cust() {
    int choice;
    do {
        system("cls");
        view_film_cust();
        printf("============================================\n");
        printf("               CUSTOMER MENU                \n");
        printf("--------------------------------------------\n");
        printf("Welcome Back, %s!\n", current_user);
        printf("[1] Book Ticket\n");
        printf("[2] History\n");
        printf("[3] Cancel Ticket\n");
        printf("[4] Edit Profile\n");
        printf("[0] Logout\n");
        printf("============================================\n");
        printf("Choose : ");
        scanf("%d", &choice);
        if (choice < 0 || choice > 4) invalid_choice();
    } while (choice < 0 || choice > 4);

    switch (choice) {
        case 1: book_ticket();  break;
        case 2: history();      break;
        case 3: cancel();       break;
        case 4: edit_profile(); break;
        case 0: main_menu();    break;
    }
}

void view_film_cust() {
    FILE* data = fopen(film_file, "r");
    if (data == NULL) { invalid_file(); return; }

    char buffer[500];
    int count = 0;

    printf("============================================\n");
    printf("                 NOW SHOWING                \n");
    printf("============================================\n");
    printf("%-5s %-20s %-15s %-8s %-5s %-30s\n",
           "ID", "Title", "Genre", "Duration", "Age", "Detail");
    printf("--------------------------------------------\n");
    while (fgets(buffer, sizeof(buffer), data)) {
        Film film;
        sscanf(buffer, "%d=%[^=]=%[^=]=%d=%d=%[^\n]",
               &film.id, film.title, film.genre,
               &film.duration, &film.age_rating, film.detail);
        printf("%-5d %-20s %-15s %-8d %-5d %-30s\n",
               film.id, film.title, film.genre,
               film.duration, film.age_rating, film.detail);
        count++;
    }
    fclose(data);
    printf("--------------------------------------------\n");
    printf("============================================\n");
}

void book_ticket() {
    // TODO: select film, schedule, seat, pay
}

void history() {
    // TODO: view all history transactions
}

void cancel() {
    // TODO: cancel pake booking code
}

void edit_profile() {
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
        if (choice < 0 || choice > 4) invalid_choice();
    } while (choice < 0 || choice > 4);

    switch (choice) {
        case 1: change_name();  break;
        case 2: change_usn();   break;
        case 3: change_email(); break;
        case 4: change_pass();  break;
        case 0: menu_cust();    break;
    }
}

void change_name()  { /* TODO */ }
void change_usn()   { /* TODO */ }
void change_email() { /* TODO */ }
void change_pass()  { /* TODO */ }

// ============================================================
// !! CASHIER MENU !!
// ============================================================
void menu_cashier() {
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
        if (choice < 0 || choice > 3) invalid_choice();
    } while (choice < 0 || choice > 3);

    switch (choice) {
        case 1: validate_ticket(); break;
        case 2: sell();            break;
        case 3: seat_status();     break;
        case 0: main_menu();       break;
    }
}

void validate_ticket() { /* TODO: konfirmasi tiket online  */ }
void sell()            { /* TODO: jual tiket offline       */ }
void seat_status()     { /* TODO: tampilkan status kursi   */ }
