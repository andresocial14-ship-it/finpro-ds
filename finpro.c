#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>

// =========================== DESAIN ==============================
#define CENTER_PAD "\t\t\t\t\t\t"
#define LOAD_SPEED 25000
#define GOLD "\033[1;33m"
#define RED "\033[1;31m"
#define GREEN "\033[1;32m"
#define DIM_GOLD "\033[2;33m"
#define CINEMA_RED "\033[38;2;139;0;0m"
#define RESET "\033[0m"
#define FORM_PAD "\t\t\t\t\t\t\t   "
// ================================================================

// ============================ DEFINES ============================
#define account_file  "users.txt"
#define film_file     "films.txt"
#define studio_file   "studios.txt"
#define schedule_file "schedules.txt"
#define booking_file  "bookings.txt"
#define T         3
#define MAX_KEYS  (2*T-1)
#define MIN_KEYS  (T-1)
#define MAX_BOOKINGS 500
#define MAX_SEATS_PER_ORDER 8

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
    char  booking_code[20];
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

// Booking helpers
int  auto_id_booking();
void generate_booking_code(int n, char* out);
int  is_seat_booked(int schedule_id, const char* seat);
void display_seat_map(int schedule_id, Studio* studio);
int  find_booking(const char* booking_code, Booking* out);
int  load_bookings(Booking* arr, int max_count);
int  validate_seat_format(const char* seat, Studio* studio);

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
void edit_studio();
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
void print_table_line(int w_id, int w_title, int w_genre, int w_dur, int w_age, int w_detail);
void book_ticket();
void history();
void cancel();
void edit_profile();
void change_usn();
void change_name();
void change_email();
void change_pass();
void view_profile();
void delete_account_cust();

// Cashier
void menu_cashier();
void validate_ticket();
void sell();
void seat_status();


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
// !! ASCII BANNER IMPLEMENTATION !!
// ============================================================
void XXI_Banner(){
	
    /* Bersihkan layar */
    printf("\033[2J\033[H");

    /* Padding vertikal */
    printf("\n\n\n\n\n");

    /* Warna Gold */
    printf("\033[1;33m");

    /* ================================================
       ASCII Art XXI
       ================================================ */

    printf("\t\t\t\t\t\tXXXXXXX       XXXXXXX   XXXXXXX       XXXXXXX   IIIIIIIIII\n");
    printf("\t\t\t\t\t\tX:::::X       X:::::X   X:::::X       X:::::X   I::::::::I\n");
    printf("\t\t\t\t\t\tX:::::X       X:::::X   X:::::X       X:::::X   I::::::::I\n");
    printf("\t\t\t\t\t\tX::::::X     X::::::X   X::::::X     X::::::X   II::::::II\n");
    printf("\t\t\t\t\t\tXXX:::::X   X:::::XXX   XXX:::::X   X:::::XXX     I::::I\n");
    printf("\t\t\t\t\t\t   X:::::X X:::::X         X:::::X X:::::X        I::::I\n");
    printf("\t\t\t\t\t\t    X:::::X:::::X           X:::::X:::::X         I::::I\n");
    printf("\t\t\t\t\t\t     X:::::::::X             X:::::::::X          I::::I\n");
    printf("\t\t\t\t\t\t     X:::::::::X             X:::::::::X          I::::I\n");
    printf("\t\t\t\t\t\t    X:::::X:::::X           X:::::X:::::X         I::::I\n");
    printf("\t\t\t\t\t\t   X:::::X X:::::X         X:::::X X:::::X        I::::I\n");
    printf("\t\t\t\t\t\tXXX:::::X   X:::::XXX   XXX:::::X   X:::::XXX   II::::::II\n");
    printf("\t\t\t\t\t\tX::::::X     X::::::X   X::::::X     X::::::X   I::::::::I\n");
    printf("\t\t\t\t\t\tX:::::X       X:::::X   X:::::X       X:::::X   I::::::::I\n");
    printf("\t\t\t\t\t\tX:::::X       X:::::X   X:::::X       X:::::X   I::::::::I\n");
    printf("\t\t\t\t\t\tXXXXXXX       XXXXXXX   XXXXXXX       XXXXXXX   IIIIIIIIII\n");

    /* Reset warna */
    printf("\033[0m");
}
void loading_bar(int total, const char *label) {
    int bar_width = 38;

    for (int i = 0; i <= total; i++) {
        int filled  = (i * bar_width) / total;
        int percent = (i * 100) / total;

        printf("\r" CENTER_PAD "\033[1;33m%-18s \033[0m[", label);

        printf("\033[1;33m");
        for (int j = 0; j < filled; j++)
            printf("=");

        printf("\033[2;33m");
        for (int j = filled; j < bar_width; j++)
            printf("=");

        printf("\033[0m] \033[1;33m%3d%%\033[0m", percent);

        fflush(stdout);
        usleep(LOAD_SPEED);
    }

    printf("\n");
}

void tampilkan_menu() {
    printf("\n");

    printf(GOLD "\t\t\t\t\t\t   +------------------------------------------------+\n");
    printf("\t\t\t\t\t\t   |             SELAMAT DATANG DI XXI              |\n");
    printf("\t\t\t\t\t\t   |------------------------------------------------|\n");

    printf("\t\t\t\t\t\t   |  [1] Login Ke Akun                             |\n");
    printf("\t\t\t\t\t\t   |  [2] Daftar Akun Baru (Register)               |\n");
    printf("\t\t\t\t\t\t   |  [0] Keluar Aplikasi                           |\n");

    printf("\t\t\t\t\t\t   +------------------------------------------------+\n" RESET);
}

void loginBanner() {
    printf(GOLD);

    printf("\t\t\t\t\t\t\t\t  _                _       \n");
    printf("\t\t\t\t\t\t\t\t | |    ___   __ _(_)_ __  \n");
    printf("\t\t\t\t\t\t\t\t | |   / _ \\ / _` | | '_ \\\\\n");
    printf("\t\t\t\t\t\t\t\t | |__| (_) | (_| | | | | |\n");
    printf("\t\t\t\t\t\t\t\t |_____\\___/ \\__, |_|_| |_|\n");
    printf("\t\t\t\t\t\t\t\t             |___/         \n");

    printf(RESET);
    printf("\n");
}

void registerBanner() {
    printf(GOLD);

    printf("\n");
    printf("\t\t\t\t\t\t\t ____  _____ ____ ___ ____ _____ _____ ____  \n");
    printf("\t\t\t\t\t\t\t|  _ \\| ____/ ___|_ _/ ___|_   _| ____|  _ \\ \n");
    printf("\t\t\t\t\t\t\t| |_) |  _|| |  _ | |\\___ \\ | | |  _| | |_) |\n");
    printf("\t\t\t\t\t\t\t|  _ <| |__| |_| || | ___) || | | |___|  _ < \n");
    printf("\t\t\t\t\t\t\t|_| \\_\\_____|____|___|____/ |_| |_____|_| \\_\\\n");
    printf("\n");

    printf(RESET);
}

void tampilkan_banner_train() {

    printf("\n");

    /* Tulisan NOW SHOWING */
    printf(GOLD);

    printf("\t\t\t\t    _  _     ___  __      __          ___    _  _     ___  __      __ ___    _  _     ___   \n");
    printf("\t\t\t\t   | \\| |   / _ \\ \\ \\    / /  o O O  / __|  | || |   / _ \\ \\ \\    / /|_ _|  | \\| |   / __|  \n");
    printf("\t\t\t\t   | .` |  | (_) | \\ \\/\\/ /  o       \\__ \\  | __ |  | (_) | \\ \\/\\/ /  | |   | .` |  | (_ |  \n");
    printf("\t\t\t\t   |_|\\_|   \\___/   \\_/\\_/  TS__[O]  |___/  |_||_|   \\___/   \\_/\\_/  |___|  |_|\\_|   \\___|  \n");

    /* Kereta Merah Bioskop */
    printf(CINEMA_RED);

    printf("\t\t\t\t  _|\"\"\"\"\"|_|\"\"\"\"\"|_|\"\"\"\"\"| {======|_|\"\"\"\"\"|_|\"\"\"\"\"|_|\"\"\"\"\"|_|\"\"\"\"\"|_|\"\"\"\"\"|_|\"\"\"\"\"|_|\"\"\"\"\"| \n");
    printf("\t\t\t\t  \"`-0-0-'\"`-0-0-'\"`-0-0-'./o--000'\"`-0-0-'\"`-0-0-'\"`-0-0-'\"`-0-0-'\"`-0-0-'\"`-0-0-'\"`-0-0-' \n");

    /* Border dan Judul */
    printf(GOLD);

    printf("\n");
    printf("\t\t\t\t----------------------------------------------------------------------------------------------\n");

    printf("\t\t\t\t                                    ! NOW SHOWING MOVIES !\n");

    printf("\t\t\t\t----------------------------------------------------------------------------------------------\n");

    printf(RESET);
    printf("\n");
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
// !! BOOKING HELPERS !!
// ============================================================

/*
 * Hitung jumlah total baris di bookings.txt (semua status).
 * Digunakan untuk generate ID booking berikutnya.
 */
int auto_id_booking() {
    FILE* fp = fopen(booking_file, "r");
    if (fp == NULL) return 1;  // file kosong, mulai dari 1

    int max_id = 0;
    char buf[300];

    while (fgets(buf, sizeof(buf), fp)) {
        // Format: BK-XXXXX=...
        // Ambil angka dari "BK-XXXXX"
        int id = 0;
        // Skip "BK-" lalu baca angkanya
        if (sscanf(buf, "BK-%d", &id) == 1) {
            if (id > max_id) max_id = id;
        }
    }
    fclose(fp);
    return max_id + 1;  // kembalikan ID berikutnya
}

/*
 * Buat string booking code dari nomor urut.
 * Contoh: n=3 ? "BK-00003"
 */
void generate_booking_code(int n, char* out) {
    sprintf(out, "BK-%05d", n);
}

/*
 * Cek apakah kursi sudah dipesan untuk schedule tertentu.
 * Hanya booking dengan status=1 yang dianggap aktif.
 * Return 1 jika sudah terisi, 0 jika kosong.
 */
int is_seat_booked(int schedule_id, const char* seat) {
    FILE* fp = fopen(booking_file, "r");
    if (fp == NULL) return 0;
    char buffer[300];
    while (fgets(buffer, sizeof(buffer), fp)) {
        buffer[strcspn(buffer, "\n")] = 0;
        Booking bk;
        sscanf(buffer, "%[^=]=%[^=]=%d=%[^=]=%f=%d",
               bk.booking_code, bk.username,
               &bk.schedule_id, bk.seat,
               &bk.total_price, &bk.status);
        if (bk.schedule_id == schedule_id &&
            strcmp(bk.seat, seat) == 0 &&
            bk.status == 1) {
            fclose(fp);
            return 1; /* kursi sudah terisi */
        }
    }
    fclose(fp);
    return 0;
}

/*
 * Tampilkan denah kursi studio untuk jadwal tertentu.
 * Kursi yang sudah dipesan (aktif) ditampilkan sebagai "BKD".
 * Format kursi: A1, A2, ..., B1, B2, ...
 * Baris ? huruf kapital (A=0, B=1, dst)
 * Kolom ? angka 1..cols
 */
void display_seat_map(int schedule_id, Studio* studio) {
    int rows = studio->rows;
    int cols = studio->cols;

    printf("\n");
    printf("  SEAT MAP - %s\n", studio->name);
    printf("  [ ] = Available   [BKD] = Booked\n");
    printf("  ----------------------------------------\n");

    /* Header kolom */
    printf("     ");
    for (int c = 1; c <= cols; c++) {
        printf(" %-4d", c);
    }
    printf("\n");

    /* Baris kursi */
    for (int r = 0; r < rows; r++) {
        char row_label = 'A' + r;
        printf("  %c  ", row_label);
        for (int c = 1; c <= cols; c++) {
            char seat[10];
            sprintf(seat, "%c%d", row_label, c);
            if (is_seat_booked(schedule_id, seat)) {
                printf("%-5s", "BKD");
            } else {
                printf("%-5s", seat);
            }
        }
        printf("\n");
    }
    printf("  ----------------------------------------\n");
}

/*
 * Cari booking berdasarkan booking_code.
 * Isi struct Booking jika ditemukan.
 * Return 1 jika ketemu, 0 jika tidak.
 */
int find_booking(const char* booking_code, Booking* out) {
    FILE* fp = fopen(booking_file, "r");
    if (fp == NULL) return 0;
    char buffer[300];
    while (fgets(buffer, sizeof(buffer), fp)) {
        buffer[strcspn(buffer, "\n")] = 0;
        Booking bk;
        sscanf(buffer, "%[^=]=%[^=]=%d=%[^=]=%f=%d",
               bk.booking_code, bk.username,
               &bk.schedule_id, bk.seat,
               &bk.total_price, &bk.status);
        if (strcmp(bk.booking_code, booking_code) == 0) {
            if (out != NULL) *out = bk;
            fclose(fp);
            return 1;
        }
    }
    fclose(fp);
    return 0;
}

/*
 * Muat semua booking dari file ke array.
 * Return jumlah booking yang berhasil dimuat.
 */
int load_bookings(Booking* arr, int max_count) {
    FILE* fp = fopen(booking_file, "r");
    if (fp == NULL) return 0;
    int count = 0;
    char buffer[300];
    while (fgets(buffer, sizeof(buffer), fp) && count < max_count) {
        buffer[strcspn(buffer, "\n")] = 0;
        sscanf(buffer, "%[^=]=%[^=]=%d=%[^=]=%f=%d",
               arr[count].booking_code, arr[count].username,
               &arr[count].schedule_id, arr[count].seat,
               &arr[count].total_price, &arr[count].status);
        count++;
    }
    fclose(fp);
    return count;
}

/*
 * Validasi format dan posisi kursi.
 * Format: huruf kapital diikuti angka, contoh A1, B10.
 * Validasi: huruf harus <= 'A' + rows - 1, angka harus 1..cols.
 * Return 1 jika valid, 0 jika tidak.
 */
int validate_seat_format(const char* seat, Studio* studio) {
    int len = strlen(seat);
    if (len < 2 || len > 4) return 0;           /* min "A1", max "Z99" */

    char row_char = seat[0];
    if (row_char < 'A' || row_char > 'Z') return 0;

    /* Semua karakter setelah huruf harus digit */
    for (int i = 1; i < len; i++) {
        if (!isdigit(seat[i])) return 0;
    }

    int col_num = atoi(seat + 1);
    if (col_num < 1) return 0;

    /* Cek apakah baris ada dalam studio */
    int row_idx = row_char - 'A';           /* A=0, B=1, dst */
    if (row_idx >= studio->rows) return 0;

    /* Cek apakah kolom ada dalam studio */
    if (col_num > studio->cols) return 0;

    return 1;
}

// ============================================================
// !! MAIN MENU !!
// ============================================================
void main_menu() {
    char input[100];
    char lower_input[100];
    int choice;
    int valid;

    do {
        system("cls");
        XXI_Banner();
        tampilkan_menu();
        printf(GOLD "\n\t\t\t\t\t\t   Choose Menu : " RESET);
        // Menggunakan fgets untuk membaca string, termasuk spasi
        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = '\0'; // Menghapus karakter newline (\n)

        // Mengonversi input menjadi lowercase (huruf kecil) agar case-insensitive
        for (int i = 0; input[i] != '\0'; i++) {
            lower_input[i] = tolower(input[i]);
        }
        lower_input[strlen(input)] = '\0';

        valid = 1; // Asumsikan input valid di awal

        // Pengecekan kondisi
        if (strcmp(lower_input, "1") == 0 || strcmp(lower_input, "login") == 0) {
            choice = 1;
        } else if (strcmp(lower_input, "2") == 0 || strcmp(lower_input, "register new account") == 0) {
            choice = 2;
        } else if (strcmp(lower_input, "0") == 0 || strcmp(lower_input, "exit") == 0) {
            choice = 0;
        } else {
            // Jika input di luar opsi
            printf("\n");
            printf(RED "\t\t\t\t\t\t   [ERORR] Invalid Choice!" RESET "\n");
            getchar();
            valid = 0; // Input tidak valid, loop akan diulang
        }
    } while (!valid);

    switch (choice) {
        case 1: login();        break;
        case 2: register_acc(); break;
        case 0: return;
    }
}

// ============================================================
// !! LOGIN !!
void login() {
    system("cls");

    char username[100], password[100];
    char fileUsername[100], filePassword[100];
    char buffer[200];
    int success;

    while (1) {

        success = 0;
        system("cls");
        printf("\n\n\n\n");
        loginBanner();

        printf("\n");

        printf(GOLD "\t\t\t\t\t\t\t  +--------------------------------------+\n");
        printf("\t\t\t\t\t\t\t  |                 LOGIN                |\n");
        printf("\t\t\t\t\t\t\t  |--------------------------------------|\n");
        printf(DIM_GOLD "\t\t\t\t\t\t\t  |    Enter 0 on username to go back    |\n");
        printf(GOLD "\t\t\t\t\t\t\t  +--------------------------------------+\n\n" RESET);

        /* ================= USERNAME ================= */

        do {

            printf(GOLD "\t\t\t\t\t\t\t  Username : " RESET);

            fgets(username, sizeof(username), stdin);
            username[strcspn(username, "\n")] = '\0';

            if (strcmp(username, "0") == 0) {
                main_menu();
                return;
            }

            if (strlen(username) == 0) {
                printf(RED "\n\t\t\t\t\t\t\t  ? Username cannot be empty!\n\n" RESET);
            }

        } while (strlen(username) == 0);

        /* ================= PASSWORD ================= */

        do {

            printf(GOLD "\t\t\t\t\t\t\t  Password : " RESET);

            fgets(password, sizeof(password), stdin);
            password[strcspn(password, "\n")] = '\0';

            if (strlen(password) == 0) {
                printf(RED "\n\t\t\t\t\t\t\t  ? Password cannot be empty!\n\n" RESET);
            }

        } while (strlen(password) == 0);

        printf("\n");

        /* ================= ADMIN ================= */

        if (strcmp(username, "admin123@") == 0 &&
            strcmp(password, "admin123@") == 0) {

            printf(GREEN "\t\t\t\t\t\t\t  ? Login Successful!\n" RESET);

            getchar();
            menu_admin();
            return;
        }

        /* ================= CASHIER ================= */

        if (strcmp(username, "cashier123@") == 0 &&
            strcmp(password, "cashier123@") == 0) {

            printf(GREEN "\t\t\t\t\t\t\t  ? Login Successful!\n" RESET);

            getchar();
            menu_cashier();
            return;
        }

        /* ================= CUSTOMER ================= */

        FILE *fp = fopen(account_file, "r");

        if (fp == NULL) {
            invalid_file();
            return;
        }

        while (fgets(buffer, sizeof(buffer), fp)) {

            buffer[strcspn(buffer, "\n")] = '\0';

            sscanf(buffer,
                   "%[^,],%[^,]",
                   fileUsername,
                   filePassword);

            if (strcmp(username, fileUsername) == 0 &&
                strcmp(password, filePassword) == 0) {

                success = 1;
                strcpy(current_user, username);
                break;
            }
        }

        fclose(fp);

        if (success) {

            printf(GREEN "\t\t\t\t\t\t\t  ? Login Successful!\n" RESET);

            getchar();
            menu_cust();
            return;
        }

        /* ================= ERROR ================= */

        printf(RED "\n\t\t\t\t\t\t\t  ? Wrong Username or Password!\n" RESET);

        printf(DIM_GOLD "\t\t\t\t\t\t\t  Press ENTER to try again..." RESET);

        getchar();
    }
}

// ============================================================
// !! REGISTER !!
// ============================================================
void register_acc() {
    system("cls");

    char fileUsername[100];
    char confirm_pass[100];
    char buffer[1024];
    int found, valid, validasi_at, has_space;

    FILE *reg = fopen(account_file, "a");
    if (reg == NULL) {
        invalid_file();
        return;
    }

    Account customer;
    printf("\n\n\n\n");
    registerBanner();

    printf(GOLD FORM_PAD "+--------------------------------------+\n");
    printf(FORM_PAD "¦               REGISTER               ¦\n");
    printf(FORM_PAD "¦--------------------------------------¦\n");
    printf(DIM_GOLD FORM_PAD "¦    Complete the form to continue     ¦\n");
    printf(FORM_PAD "¦   Enter 0 at any prompt to go back   ¦\n");
    printf(GOLD FORM_PAD "+--------------------------------------+\n\n" RESET);

    /* FULL NAME */
    do {
        printf(GOLD FORM_PAD "Full Name        : " RESET);

        fgets(customer.name, sizeof(customer.name), stdin);
        customer.name[strcspn(customer.name, "\n")] = '\0';

        if (strcmp(customer.name, "0") == 0) {
            fclose(reg);
            main_menu();
            return;
        }

        valid = 1;

        if (strlen(customer.name) == 0) {
            printf(RED FORM_PAD "? Full name cannot be empty!\n\n" RESET);
            valid = 0;
        } else {
            for (int i = 0; customer.name[i] != '\0'; i++) {
                if (isdigit(customer.name[i])) {
                    printf(RED FORM_PAD "? Name cannot contain numbers!\n\n" RESET);
                    valid = 0;
                    break;
                }
            }
        }

    } while (!valid);

    /* EMAIL */
    do {
        printf(GOLD FORM_PAD "Email            : " RESET);

        fgets(customer.email, sizeof(customer.email), stdin);
        customer.email[strcspn(customer.email, "\n")] = '\0';

        if (strcmp(customer.email, "0") == 0) {
            fclose(reg);
            main_menu();
            return;
        }

        validasi_at = 0;

        for (int i = 0; customer.email[i] != '\0'; i++) {
            if (customer.email[i] == '@') {
                validasi_at = 1;
                break;
            }
        }

        if (strlen(customer.email) == 0) {
            printf(RED FORM_PAD "? Email cannot be empty!\n\n" RESET);
        } else if (!validasi_at) {
            printf(RED FORM_PAD "? Email must contain @\n\n" RESET);
        }

    } while (strlen(customer.email) == 0 || !validasi_at);

    /* USERNAME */
    do {
        printf(GOLD FORM_PAD "Username         : " RESET);

        fgets(customer.username, sizeof(customer.username), stdin);
        customer.username[strcspn(customer.username, "\n")] = '\0';

        if (strcmp(customer.username, "0") == 0) {
            fclose(reg);
            main_menu();
            return;
        }

        if (strlen(customer.username) == 0) {
            printf(RED FORM_PAD "? Username cannot be empty!\n\n" RESET);
            found = 1;
            continue;
        }

        has_space = 0;

        for (int i = 0; customer.username[i] != '\0'; i++) {
            if (isspace(customer.username[i])) {
                has_space = 1;
                break;
            }
        }

        if (has_space) {
            printf(RED FORM_PAD "? Username cannot contain spaces!\n\n" RESET);
            found = 1;
            continue;
        }

        found = 0;

        FILE *check = fopen(account_file, "r");

        if (check != NULL) {
            while (fgets(buffer, sizeof(buffer), check)) {

                sscanf(buffer, "%[^,]", fileUsername);

                if (strcmp(fileUsername, customer.username) == 0) {
                    found = 1;
                    printf(RED FORM_PAD "? Username already exists!\n\n" RESET);
                    break;
                }
            }

            fclose(check);
        }

    } while (found);

    /* PASSWORD */
    do {
        printf(GOLD FORM_PAD "Password         : " RESET);

        fgets(customer.password, sizeof(customer.password), stdin);
        customer.password[strcspn(customer.password, "\n")] = '\0';

        if (strcmp(customer.password, "0") == 0) {
            fclose(reg);
            main_menu();
            return;
        }

        if (strlen(customer.password) == 0) {
            printf(RED FORM_PAD "? Password cannot be empty!\n\n" RESET);
        } else if (strlen(customer.password) < 5) {
            printf(RED FORM_PAD "? Minimum 5 characters!\n\n" RESET);
        }

    } while (strlen(customer.password) < 5);

    /* CONFIRM PASSWORD */
    do {
        printf(GOLD FORM_PAD "Confirm Password : " RESET);

        fgets(confirm_pass, sizeof(confirm_pass), stdin);
        confirm_pass[strcspn(confirm_pass, "\n")] = '\0';

        if (strcmp(confirm_pass, "0") == 0) {
            fclose(reg);
            main_menu();
            return;
        }

        if (strlen(confirm_pass) == 0) {
            printf(RED FORM_PAD "? Confirm password required!\n\n" RESET);
        } else if (strcmp(customer.password, confirm_pass) != 0) {
            printf(RED FORM_PAD "? Password does not match!\n\n" RESET);
        }

    } while (strcmp(customer.password, confirm_pass) != 0 ||
             strlen(confirm_pass) == 0);

    fprintf(reg,
            "%s,%s,%s,%s\n",
            customer.username,
            customer.password,
            customer.name,
            customer.email);

    fclose(reg);

    system("cls");

    registerBanner();

    printf("\n");
    printf(GREEN FORM_PAD "     ? Account Created Successfully!\n\n" RESET);

    printf(DIM_GOLD FORM_PAD "      Press ENTER to continue..." RESET);

    getchar();

    main_menu();
}

// ============================================================
// !! ADMIN MENU !!
// ============================================================
void menu_admin() {
    char input[100];
    char lower_input[100];
    int choice;
    int valid;
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
        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = '\0'; // Menghapus karakter newline (\n)

        // Mengonversi input menjadi lowercase (huruf kecil) agar case-insensitive
        for (int i = 0; input[i] != '\0'; i++) {
            lower_input[i] = tolower(input[i]);
        }
        lower_input[strlen(input)] = '\0';

        valid = 1; // Asumsikan input valid di awal

        // Pengecekan kondisi berdasarkan angka atau teks
        if (strcmp(lower_input, "1") == 0 || strcmp(lower_input, "film management") == 0) {
            choice = 1;
        } else if (strcmp(lower_input, "2") == 0 || strcmp(lower_input, "studio & schedule management") == 0) {
            choice = 2;
        } else if (strcmp(lower_input, "3") == 0 || strcmp(lower_input, "user account management") == 0) {
            choice = 3;
        } else if (strcmp(lower_input, "0") == 0 || strcmp(lower_input, "logout") == 0) {
            choice = 0;
        } else {
            // Jika input kosong atau di luar opsi
            printf("\n[ERROR] Invalid Input!\n");
            printf("Please enter the number (0-3) or the exact option text.\n");
            system("pause");
            valid = 0; // Input tidak valid, loop akan diulang
        }
    } while (!valid);

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
    char input[100];
    char lower_input[100];
    int choice;
    int valid;

    do {
        system("cls");
        printf("============================================\n");
        printf("               FILM MANAGEMENT              \n");
        printf("--------------------------------------------\n");
        printf("[1] View All Films\n");
        printf("[2] Search Film\n");
        printf("[3] Add New Film\n");
        printf("[4] Delete Film\n");
        printf("[5] Edit Film Info\n");
        printf("[0] Back\n");
        printf("============================================\n");
        printf("Choose : ");
        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = '\0'; // Hapus newline

        // Konversi ke lowercase
        for (int i = 0; input[i] != '\0'; i++) {
            lower_input[i] = tolower(input[i]);
        }
        lower_input[strlen(input)] = '\0';

        valid = 1;

        if (strcmp(lower_input, "1") == 0 || strcmp(lower_input, "view all films") == 0) {
            choice = 1;
        } else if (strcmp(lower_input, "2") == 0 || strcmp(lower_input, "search film") == 0) {
            choice = 2;
        } else if (strcmp(lower_input, "3") == 0 || strcmp(lower_input, "add new film") == 0) {
            choice = 3;
        } else if (strcmp(lower_input, "4") == 0 || strcmp(lower_input, "delete film") == 0) {
            choice = 4;
        } else if (strcmp(lower_input, "5") == 0 || strcmp(lower_input, "edit film info") == 0) {
            choice = 5;
        } else if (strcmp(lower_input, "0") == 0 || strcmp(lower_input, "back") == 0) {
            choice = 0;
        } else {
            printf("\n[ERROR] Invalid Input!\n");
            printf("Please enter the number (0-5) or the exact option text.\n");
            system("pause");
            valid = 0;
        }
    } while (!valid);

    switch (choice) {
        case 1: view_film();   break;
        case 2: search_film(); break;
        case 3: add_film();    break;
        case 4: del_film();    break;
        case 5: edit_film();   break;
        case 0: menu_admin();  break;
    }
}

void add_film() {
    system("cls");
    Film film;
    int valid;
    char input[300];

    printf("============================================\n");
    printf("                 ADD FILM                   \n");
    printf(" (Enter '0' at any prompt to go back)\n");
    printf("--------------------------------------------\n");

    do {
        printf("Title           : ");
        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = '\0';

        if (strcmp(input, "0") == 0) { film_manage(); return; }
        valid = 1;
         if (strlen(input) == 0) { printf("Title cannot be empty!\n"); valid = 0; }
        else { strcpy(film.title, input); }
    } while (!valid);

    do {
        printf("Genre           : ");
        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = '\0';

        if (strcmp(input, "0") == 0) { film_manage(); return; }        
        valid = 1;
        if (strlen(input) == 0) { printf("Genre cannot be empty!\n"); valid = 0; }
        else { strcpy(film.genre, input); }
    } while (!valid);

    do {
        printf("Duration (min)  : ");
        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = '\0';

        if (strcmp(input, "0") == 0) { film_manage(); return; }
        
        if (strlen(input) == 0) { printf("Duration cannot be empty!\n"); valid = 0; continue; }

        valid = 1;
        for (int i = 0; input[i] != '\0'; i++) {
            if (!isdigit(input[i])) { valid = 0; break; }
        }

        if (valid) {
            film.duration = atoi(input);
            if (film.duration <= 0) { printf("Duration must be greater than 0!\n"); valid = 0; }
        } else {
            printf("Duration must be a number!\n");
        }
    } while (!valid);

    do {
        printf("Age Rating (n+) : ");
        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = '\0';

        if (strcmp(input, "0") == 0) { film_manage(); return; }
        
        if (strlen(input) == 0) { printf("Age Rating cannot be empty!\n"); valid = 0; continue; }

         valid = 1;
        for (int i = 0; input[i] != '\0'; i++) {
            if (!isdigit(input[i])) { valid = 0; break; }
        }

        if (valid) {
            film.age_rating = atoi(input);
            // Validasi khusus untuk 4 opsi umur
            if (film.age_rating != 3 && film.age_rating != 13 && 
                film.age_rating != 17 && film.age_rating != 21) {
                printf("Invalid Age Rating! Please input exactly 3, 13, 17, or 21.\n");
                valid = 0;
            }
        } else {
            printf("Age rating must be a number!\n");
        }
    } while (!valid);

    do {
        printf("Synopsis        : ");
        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = '\0';

        if (strcmp(input, "0") == 0) { film_manage(); return; }
        valid = 1;
        if (strlen(input) == 0) { printf("Detail cannot be empty!\n"); valid = 0; }
        else { strcpy(film.detail, input); }
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

// ============================================================
// !! DELETE FILM !!
// ============================================================
void del_film() {
    Film result[200];
    int count = 0;
    btree_inorder(film_tree, result, &count);

    if (count == 0) {
        system("cls");
        printf("============================================\n");
        printf("                 DELETE FILM                \n");
        printf("============================================\n");
        printf("No films available.\n");
        printf("============================================\n");
        system("pause");
        film_manage();
        return;
    }

    // [Pass 1] Mencari panjang string maksimal di setiap kolom
    int w_id = 2;       // Lebar minimal "ID"
    int w_title = 5;    // Lebar minimal "Title"
    
    for (int i = 0; i < count; i++) {
        char temp_id[20];
        sprintf(temp_id, "%d", result[i].id);
        if ((int)strlen(temp_id) > w_id) w_id = strlen(temp_id);
        if ((int)strlen(result[i].title) > w_title) w_title = strlen(result[i].title);
    }

    char input[100];
    char lower_input[100];
    int target_id = -1;
    Film matched[200];
    int match_count = 0;

    // ==========================================
    // PAGE 1 : PILIH FILM YANG INGIN DIHAPUS
    // ==========================================
    do {
        match_count = 0;
        
        system("cls");
        printf("============================================\n");
        printf("                 DELETE FILM                \n");
        printf("============================================\n");

        // [Pass 2] Menampilkan tabel dinamis 2 kolom
        printf("+"); for(int i = 0; i < w_id + 2; i++) printf("-"); 
        printf("+"); for(int i = 0; i < w_title + 2; i++) printf("-"); printf("+\n");
        
        printf("| %-*s | %-*s |\n", w_id, "ID", w_title, "Title");
        
        printf("+"); for(int i = 0; i < w_id + 2; i++) printf("-"); 
        printf("+"); for(int i = 0; i < w_title + 2; i++) printf("-"); printf("+\n");

        for (int i = 0; i < count; i++) {
            printf("| %-*d | %-*s |\n", w_id, result[i].id, w_title, result[i].title);
        }

        printf("+"); for(int i = 0; i < w_id + 2; i++) printf("-"); 
        printf("+"); for(int i = 0; i < w_title + 2; i++) printf("-"); printf("+\n");

        printf("--------------------------------------------\n");
        printf(" (Enter '0' to go back)\n");
        printf("--------------------------------------------\n");

        printf("Enter Film ID or Title to delete : ");
        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = '\0';

        if (strcmp(input, "0") == 0) { film_manage(); return; }
        
        if (strlen(input) == 0) { 
            printf("[ERROR] Input cannot be empty!\n"); 
            system("pause"); 
            continue; 
        }

        // Konversi ke lowercase untuk pencarian teks
        for (int i = 0; input[i] != '\0'; i++) {
            lower_input[i] = tolower(input[i]);
        }
        lower_input[strlen(input)] = '\0';

        // Cek apakah murni angka
        int is_num = 1;
        for (int i = 0; input[i] != '\0'; i++) {
            if (!isdigit(input[i])) { is_num = 0; break; }
        }

        if (is_num) {
            int input_id = atoi(input);
            for (int i = 0; i < count; i++) {
                if (result[i].id == input_id) {
                    matched[match_count] = result[i];
                    match_count++;
                    break; 
                }
            }
        } else {
            for (int i = 0; i < count; i++) {
                char lower_title[100];
                strcpy(lower_title, result[i].title);
                for (int j = 0; lower_title[j] != '\0'; j++) {
                    lower_title[j] = tolower(lower_title[j]);
                }
                if (strstr(lower_title, lower_input) != NULL) {
                    matched[match_count] = result[i];
                    match_count++;
                }
            }
        }

        if (match_count == 0) {
            printf("[ERROR] Film \"%s\" not found! Please try again.\n", input);
            system("pause"); 
        } else if (match_count == 1) {
            target_id = matched[0].id;
        } else {
            int resolved = 0;
            do {
                system("cls");
                printf("============================================\n");
                printf("       DELETE FILM - MULTIPLE MATCHES       \n");
                printf("============================================\n");
                printf("Multiple films found for \"%s\":\n", input);
                printf("--------------------------------------------\n");
                for (int i = 0; i < match_count; i++) {
                    printf("ID: %-4d | Title: %s\n", matched[i].id, matched[i].title);
                }
                printf("--------------------------------------------\n");
                printf("Enter the EXACT ID from the list above\n");
                printf(" (or '0' to go back) : ");
                
                char id_input[100];
                fgets(id_input, sizeof(id_input), stdin);
                id_input[strcspn(id_input, "\n")] = '\0';

                if (strcmp(id_input, "0") == 0) { film_manage(); return; }
                
                if (strlen(id_input) == 0) { 
                    printf("[ERROR] Input cannot be empty!\n"); 
                    system("pause"); 
                    continue; 
                }

                int id_is_num = 1;
                for (int i = 0; id_input[i] != '\0'; i++) {
                    if (!isdigit(id_input[i])) { id_is_num = 0; break; }
                }

                if (!id_is_num) {
                    printf("[ERROR] Invalid input! Please enter a numeric ID.\n");
                    system("pause");
                    continue;
                }

                int selected_id = atoi(id_input);
                for (int i = 0; i < match_count; i++) {
                    if (matched[i].id == selected_id) {
                        target_id = selected_id;
                        resolved = 1;
                        break;
                    }
                }

                if (!resolved) {
                    printf("[ERROR] ID %d is not on the list! Please type the exact ID shown above.\n", selected_id);
                    system("pause");
                }
            } while (!resolved);
        }
    } while (target_id == -1);

    Film* target = btree_search(film_tree, target_id);
    if (target == NULL) {
        printf("Unexpected Error: Film not found!\n");
        system("pause");
        film_manage();
        return;
    }

    // ==========================================
    // PAGE 2 : KONFIRMASI Y/N
    // ==========================================
    char confirm_str[10];
    char confirm_char;
    int conf_valid = 0;

    do {
        system("cls");
        printf("============================================\n");
        printf("            CONFIRM DELETION                \n");
        printf("============================================\n");
        printf("  Detail Film:\n");
        printf("  ID       : %d\n", target->id);
        printf("  Title    : %s\n", target->title);
        printf("  Genre    : %s\n", target->genre);
        printf("  Duration : %d min\n", target->duration);
        printf("  Age      : %d+\n", target->age_rating);
        printf("  Synopsis : %s\n", target->detail);
        printf("--------------------------------------------\n");
        printf("Are you sure you want to delete this film?\n");
        printf("Type (Y/N) or '0' to go back : ");

        fgets(confirm_str, sizeof(confirm_str), stdin);
        confirm_str[strcspn(confirm_str, "\n")] = '\0';

        if (strcmp(confirm_str, "0") == 0) { film_manage(); return; }

        if (strlen(confirm_str) == 0) {
            printf("[ERROR] Input cannot be empty! Enter Y or N.\n");
            system("pause");
            continue;
        }

        if (strcmp(confirm_str, "Y") == 0 || strcmp(confirm_str, "y") == 0) {
            confirm_char = 'y';
            conf_valid = 1;
        } else if (strcmp(confirm_str, "N") == 0 || strcmp(confirm_str, "n") == 0) {
            confirm_char = 'n';
            conf_valid = 1;
        } else {
            printf("[ERROR] Invalid input! Enter Y or N.\n");
            system("pause");
        }
    } while (!conf_valid);

    if (confirm_char == 'n') {
        printf("Deletion cancelled.\n");
        system("pause");
        film_manage();
        return;
    }

    // ==========================================
    // PAGE 3 : KONFIRMASI PASSWORD ADMIN
    // ==========================================
    char input_pass[100];
    char confirm_pass_input[100]; 
    int pass_match = 0;

    do {
        system("cls");
        printf("============================================\n");
        printf("         ADMIN PASSWORD CONFIRMATION        \n");
        printf("============================================\n");
        printf(" [WARNING] This action cannot be undone!    \n");
        printf("--------------------------------------------\n");
        printf(" (Enter '0' at any prompt to go back)\n");
        printf("--------------------------------------------\n");

        printf("Enter Admin password : ");
        fgets(input_pass, sizeof(input_pass), stdin);
        input_pass[strcspn(input_pass, "\n")] = '\0';

        if (strcmp(input_pass, "0") == 0) { film_manage(); return; }

        if (strlen(input_pass) == 0) {
            printf("[ERROR] Password cannot be empty!\n");
            system("pause");
            continue;
        }

        printf("Confirm Admin password : ");
        fgets(confirm_pass_input, sizeof(confirm_pass_input), stdin);
        confirm_pass_input[strcspn(confirm_pass_input, "\n")] = '\0';

        if (strcmp(confirm_pass_input, "0") == 0) { film_manage(); return; }

        if (strcmp(input_pass, confirm_pass_input) != 0) {
            printf("\n[ERROR] Passwords do not match! Please try again.\n");
            system("pause");
            continue;
        }

        // Hardcoded admin password validation (seperti login)
        if (strcmp(input_pass, "admin123@") != 0) {
            printf("\n[ERROR] Incorrect password! Please try again.\n");
            system("pause");
            continue;
        }

        pass_match = 1; 
    } while (!pass_match);

    // ==========================================
    // PAGE 4 : EKSEKUSI PENGHAPUSAN
    // ==========================================
    char deleted_title[100];
    strcpy(deleted_title, target->title); // Simpan nama sebelum memory di-free

    btree_delete(&film_tree, target_id);
    btree_save_to_file();

    system("cls");
    printf("============================================\n");
    printf("               FILM DELETED                 \n");
    printf("============================================\n");
    printf("\n");
    printf("   Film \"%s\" (ID: %d)       \n", deleted_title, target_id);
    printf("       deleted successfully!                \n");
    printf("\n");
    printf("============================================\n");
    system("pause");
    film_manage();
}

// ============================================================
// !! EDIT FILM !!
// ============================================================
void edit_film() {
    Film result[200];
    int count = 0;
    btree_inorder(film_tree, result, &count);

    if (count == 0) {
        system("cls");
        printf("============================================\n");
        printf("                 EDIT FILM                  \n");
        printf("============================================\n");
        printf("No films available.\n");
        printf("============================================\n");
        system("pause");
        film_manage();
        return;
    }

    // [Pass 1] Mencari panjang string maksimal di setiap kolom
    int w_id = 2;       // Lebar minimal "ID"
    int w_title = 5;    // Lebar minimal "Title"
    
    for (int i = 0; i < count; i++) {
        char temp_id[20];
        sprintf(temp_id, "%d", result[i].id);
        if ((int)strlen(temp_id) > w_id) w_id = strlen(temp_id);
        if ((int)strlen(result[i].title) > w_title) w_title = strlen(result[i].title);
    }

    char input[100];
    char lower_input[100];
    int target_id = -1;
    Film matched[200];
    int match_count = 0;

    // ==========================================
    // PAGE 1 : PILIH FILM YANG INGIN DIEDIT
    // ==========================================
    do {
        match_count = 0;
        
        // Bersihkan layar setiap kali ada input salah/kosong
        system("cls");
        printf("============================================\n");
        printf("                 EDIT FILM                  \n");
        printf("============================================\n");

        // [Pass 2] Menampilkan tabel dinamis 2 kolom
        printf("+"); for(int i = 0; i < w_id + 2; i++) printf("-"); 
        printf("+"); for(int i = 0; i < w_title + 2; i++) printf("-"); printf("+\n");
        
        printf("| %-*s | %-*s |\n", w_id, "ID", w_title, "Title");
        
        printf("+"); for(int i = 0; i < w_id + 2; i++) printf("-"); 
        printf("+"); for(int i = 0; i < w_title + 2; i++) printf("-"); printf("+\n");

        for (int i = 0; i < count; i++) {
            printf("| %-*d | %-*s |\n", w_id, result[i].id, w_title, result[i].title);
        }

        printf("+"); for(int i = 0; i < w_id + 2; i++) printf("-"); 
        printf("+"); for(int i = 0; i < w_title + 2; i++) printf("-"); printf("+\n");

        printf("--------------------------------------------\n");
        printf(" (Enter '0' to go back)\n");
        printf("--------------------------------------------\n");

        printf("Enter Film ID or Title to edit : ");
        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = '\0';

        if (strcmp(input, "0") == 0) { film_manage(); return; }
        
        if (strlen(input) == 0) { 
            printf("[ERROR] Input cannot be empty!\n"); 
            system("pause"); // Jeda sebelum layar dibersihkan dan tabel dimuat ulang
            continue; 
        }

        // Konversi ke lowercase untuk pencarian teks
        for (int i = 0; input[i] != '\0'; i++) {
            lower_input[i] = tolower(input[i]);
        }
        lower_input[strlen(input)] = '\0';

        // Cek apakah murni angka
        int is_num = 1;
        for (int i = 0; input[i] != '\0'; i++) {
            if (!isdigit(input[i])) { is_num = 0; break; }
        }

        if (is_num) {
            // Pencarian berdasarkan ID
            int input_id = atoi(input);
            for (int i = 0; i < count; i++) {
                if (result[i].id == input_id) {
                    matched[match_count] = result[i];
                    match_count++;
                    break; // ID pasti unik
                }
            }
        } else {
            // Pencarian berdasarkan Judul
            for (int i = 0; i < count; i++) {
                char lower_title[100];
                strcpy(lower_title, result[i].title);
                for (int j = 0; lower_title[j] != '\0'; j++) {
                    lower_title[j] = tolower(lower_title[j]);
                }
                if (strstr(lower_title, lower_input) != NULL) {
                    matched[match_count] = result[i];
                    match_count++;
                }
            }
        }

        // Logika Eksekusi Hasil Pencarian
        if (match_count == 0) {
            printf("[ERROR] Film \"%s\" not found! Please try again.\n", input);
            system("pause"); // Jeda sebelum layar dibersihkan
        } else if (match_count == 1) {
            target_id = matched[0].id; // Langsung masuk mode Edit
        } else {
            // Ambiguity Resolution (Jika ada 2 atau lebih film yang cocok)
            int resolved = 0;
            do {
                system("cls");
                printf("============================================\n");
                printf("        EDIT FILM - MULTIPLE MATCHES        \n");
                printf("============================================\n");
                printf("Multiple films found for \"%s\":\n", input);
                printf("--------------------------------------------\n");
                for (int i = 0; i < match_count; i++) {
                    printf("ID: %-4d | Title: %s\n", matched[i].id, matched[i].title);
                }
                printf("--------------------------------------------\n");
                printf("Enter the EXACT ID from the list above\n");
                printf(" (or '0' to go back) : ");
                
                char id_input[100];
                fgets(id_input, sizeof(id_input), stdin);
                id_input[strcspn(id_input, "\n")] = '\0';

                if (strcmp(id_input, "0") == 0) { film_manage(); return; }
                
                if (strlen(id_input) == 0) { 
                    printf("[ERROR] Input cannot be empty!\n"); 
                    system("pause"); 
                    continue; 
                }

                int id_is_num = 1;
                for (int i = 0; id_input[i] != '\0'; i++) {
                    if (!isdigit(id_input[i])) { id_is_num = 0; break; }
                }

                if (!id_is_num) {
                    printf("[ERROR] Invalid input! Please enter a numeric ID.\n");
                    system("pause");
                    continue;
                }

                int selected_id = atoi(id_input);
                for (int i = 0; i < match_count; i++) {
                    if (matched[i].id == selected_id) {
                        target_id = selected_id;
                        resolved = 1;
                        break;
                    }
                }

                if (!resolved) {
                    printf("[ERROR] ID %d is not on the list! Please type the exact ID shown above.\n", selected_id);
                    system("pause");
                }
            } while (!resolved);
        }
    } while (target_id == -1);


    // ==========================================
    // PAGE 2 : TAHAP EDIT FILM
    // ==========================================
    Film* target = btree_search(film_tree, target_id);
    if (target == NULL) {
        printf("Unexpected Error: Film not found!\n");
        system("pause");
        film_manage();
        return;
    }

    Film temp_film = *target;

    system("cls");
    printf("============================================\n");
    printf("              EDITING FILM ID: %d           \n", temp_film.id);
    printf("============================================\n");
    printf("Leave blank (press Enter) to keep current value\n");
    printf(" (Enter '0' at any prompt to cancel & go back)\n");
    printf("--------------------------------------------\n");

    char new_val[300];
    int valid;

    /* --- EDIT TITLE --- */
    printf("Title [%s] : ", temp_film.title);
    fgets(new_val, sizeof(new_val), stdin);
    new_val[strcspn(new_val, "\n")] = '\0';
    if (strcmp(new_val, "0") == 0) { film_manage(); return; } 
    if (strlen(new_val) != 0) strcpy(temp_film.title, new_val);

    /* --- EDIT GENRE --- */
    printf("Genre [%s] : ", temp_film.genre);
    fgets(new_val, sizeof(new_val), stdin);
    new_val[strcspn(new_val, "\n")] = '\0';
    if (strcmp(new_val, "0") == 0) { film_manage(); return; }
    if (strlen(new_val) != 0) strcpy(temp_film.genre, new_val);

    /* --- EDIT DURATION --- */
    do {
        valid = 1;
        printf("Duration [%d] : ", temp_film.duration);
        fgets(new_val, sizeof(new_val), stdin);
        new_val[strcspn(new_val, "\n")] = '\0';
        
        if (strcmp(new_val, "0") == 0) { film_manage(); return; }
        
        if (strlen(new_val) != 0) {
            for (int i = 0; new_val[i] != '\0'; i++) {
                if (!isdigit(new_val[i])) { valid = 0; break; }
            }
            if (valid && atoi(new_val) > 0) {
                temp_film.duration = atoi(new_val);
            } else {
                printf(" > Invalid duration! Must be a number greater than 0. Try again or press Enter to skip.\n");
                valid = 0;
            }
        }
    } while (!valid);

    /* --- EDIT AGE RATING --- */
    do {
        valid = 1;
        printf("Age Rating [%d] : ", temp_film.age_rating);
        fgets(new_val, sizeof(new_val), stdin);
        new_val[strcspn(new_val, "\n")] = '\0';
        
        if (strcmp(new_val, "0") == 0) { film_manage(); return; } 
        
        if (strlen(new_val) != 0) {
            for (int i = 0; new_val[i] != '\0'; i++) {
                if (!isdigit(new_val[i])) { valid = 0; break; }
            }
            if (valid) {
                int new_age = atoi(new_val);
                if (new_age == 3 || new_age == 13 || new_age == 17 || new_age == 21) {
                    temp_film.age_rating = new_age;
                } else {
                    printf(" > Invalid Age Rating! Must be 3, 13, 17, or 21. Try again or press Enter to skip.\n");
                    valid = 0;
                }
            } else {
                printf(" > Invalid age rating format! Must be a number. Try again or press Enter to skip.\n");
                valid = 0;
            }
        }
    } while (!valid);

    /* --- EDIT DETAIL (SYNOPSIS) --- */
    printf("Detail [%s] : ", temp_film.detail);
    fgets(new_val, sizeof(new_val), stdin);
    new_val[strcspn(new_val, "\n")] = '\0';
    if (strcmp(new_val, "0") == 0) { film_manage(); return; } 
    if (strlen(new_val) != 0) strcpy(temp_film.detail, new_val);

    // ==========================================
    // MENERAPKAN PERUBAHAN SECARA PERMANEN
    // ==========================================
    *target = temp_film;  // Timpa data di B-Tree dengan salinan yang sudah diedit
    btree_save_to_file(); // Simpan ke .txt

    printf("--------------------------------------------\n");
    printf("Film \"%s\" updated successfully!\n", target->title);
    printf("============================================\n");
    system("pause");
    film_manage();
}

// ============================================================
// !! SEARCH FILM !!
// ============================================================
void search_film() {
    Film result[200];
    int count = 0;
    btree_inorder(film_tree, result, &count);

    if (count == 0) {
        system("cls");
        printf("============================================\n");
        printf("                 ALL FILMS                  \n");
        printf("    (Displayed via B-Tree In-Order)         \n");
        printf("                SEARCH FILM                 \n");
        printf("============================================\n");
        printf("No films available in the database.\n");
        printf("============================================\n");
        system("pause");
        film_manage();
        return;
    }

    // [Pass 1] Mencari panjang string maksimal (2 kolom) untuk tabel referensi
    int w_id_ref = 2;       // Lebar minimal "ID"
    int w_title_ref = 5;    // Lebar minimal "Title"
    
    for (int i = 0; i < count; i++) {
        char temp_id[20];
        sprintf(temp_id, "%d", result[i].id);

        if ((int)strlen(temp_id) > w_id_ref) w_id_ref = strlen(temp_id);
        if ((int)strlen(result[i].title) > w_title_ref) w_title_ref = strlen(result[i].title);
    }

    char input[100];
    char lower_input[100];
    Film matched[200]; 
    int match_count = 0;
    int first_try = 1; // Untuk mengatur kapan pesan error muncul

    // ==========================================
    // PAGE 1 : TAMPILKAN TABEL REFERENSI & INPUT
    // ==========================================
    do {
        match_count = 0; // Reset setiap kali loop
        system("cls");
        
        printf("============================================\n");
        printf("                 ALL FILMS                  \n");
        printf("    (Displayed via B-Tree In-Order)         \n");
        printf("                SEARCH FILM                 \n");
        printf("============================================\n");

        // Cetak Garis Atas
        printf("+"); for(int i = 0; i < w_id_ref + 2; i++) printf("-"); 
        printf("+"); for(int i = 0; i < w_title_ref + 2; i++) printf("-"); printf("+\n");
        
        // Cetak Header
        printf("| %-*s | %-*s |\n", w_id_ref, "ID", w_title_ref, "Title");
        
        // Cetak Garis Tengah
        printf("+"); for(int i = 0; i < w_id_ref + 2; i++) printf("-"); 
        printf("+"); for(int i = 0; i < w_title_ref + 2; i++) printf("-"); printf("+\n");

        // Cetak Isi Data Film (Bagian ini yang sebelumnya hilang)
        for (int i = 0; i < count; i++) {
            printf("| %-*d | %-*s |\n", w_id_ref, result[i].id, w_title_ref, result[i].title);
        }

        // Cetak Garis Bawah
        printf("+"); for(int i = 0; i < w_id_ref + 2; i++) printf("-"); 
        printf("+"); for(int i = 0; i < w_title_ref + 2; i++) printf("-"); printf("+\n");

        printf(" (Enter '0' to go back)\n");
        printf("--------------------------------------------\n");

        // Jika input sebelumnya salah, tampilkan pesan error tanpa system pause
        if (!first_try) {
            printf("[ERROR] No film found matching your input. Please try again.\n");
            printf("--------------------------------------------\n");
        }

        printf("Enter Film ID or Title to search : ");
        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = '\0'; // Hapus newline

        // Fitur Back
        if (strcmp(input, "0") == 0) { 
            film_manage(); 
            return; 
        }

        if (strlen(input) == 0) { 
            first_try = 0; 
            continue; 
        }

        // Konversi input ke lowercase untuk komparasi teks
        for (int i = 0; input[i] != '\0'; i++) {
            lower_input[i] = tolower(input[i]);
        }
        lower_input[strlen(input)] = '\0';

        // Cek apakah input murni angka (untuk pencarian ID)
        int is_num = 1;
        for (int i = 0; input[i] != '\0'; i++) {
            if (!isdigit(input[i])) { is_num = 0; break; }
        }
        int input_id = is_num ? atoi(input) : -1;

        // Pencarian melalui array (Partial Match Judul ATAU Exact Match ID)
        for (int i = 0; i < count; i++) {
            char lower_title[100];
            strcpy(lower_title, result[i].title);
            
            // Konversi judul di array ke lowercase
            for (int j = 0; lower_title[j] != '\0'; j++) {
                lower_title[j] = tolower(lower_title[j]);
            }

            // Jika ID cocok ATAU input string ditemukan di dalam judul film
            if ((is_num && result[i].id == input_id) || strstr(lower_title, lower_input) != NULL) {
                matched[match_count] = result[i];
                match_count++;
            }
        }

        first_try = 0; // Set ke 0 agar iterasi berikutnya menampilkan error jika match_count == 0

    } while (match_count == 0);


    // ==========================================
    // PAGE 2 : TAMPILKAN HASIL PENCARIAN
    // ==========================================
    system("cls"); // Membersihkan layar untuk page baru

    // Inisialisasi lebar minimal tabel hasil (6 kolom)
    int w_id = 2, w_title = 5, w_genre = 5, w_dur = 8, w_age = 3, w_detail = 6;
    
    for (int i = 0; i < match_count; i++) {
        char temp_id[20], temp_dur[20], temp_age[20];
        sprintf(temp_id, "%d", matched[i].id);
        sprintf(temp_dur, "%d", matched[i].duration);
        sprintf(temp_age, "%d", matched[i].age_rating);

        if ((int)strlen(temp_id) > w_id) w_id = strlen(temp_id);
        if ((int)strlen(matched[i].title) > w_title) w_title = strlen(matched[i].title);
        if ((int)strlen(matched[i].genre) > w_genre) w_genre = strlen(matched[i].genre);
        if ((int)strlen(temp_dur) > w_dur) w_dur = strlen(temp_dur);
        if ((int)strlen(temp_age) > w_age) w_age = strlen(temp_age);
        if ((int)strlen(matched[i].detail) > w_detail) w_detail = strlen(matched[i].detail);
    }

    int total_width = w_id + w_title + w_genre + w_dur + w_age + w_detail + 19;
    char header_text[150];
    sprintf(header_text, "SEARCH RESULTS FOR: \"%s\"", input);
    
    printf("\n");
    for(int i = 0; i < total_width; i++) printf("="); printf("\n");
    
    // Teks di tengah (Center alignment)
    int padding = (total_width - strlen(header_text)) / 2; 
    if (padding < 0) padding = 0;
    for(int i = 0; i < padding; i++) printf(" ");
    printf("%s\n", header_text);
    
    for(int i = 0; i < total_width; i++) printf("="); printf("\n");

    // Cetak Header Tabel
    print_table_line(w_id, w_title, w_genre, w_dur, w_age, w_detail);
    printf("| %-*s | %-*s | %-*s | %-*s | %-*s | %-*s |\n",
           w_id, "ID", w_title, "Title", w_genre, "Genre", 
           w_dur, "Duration", w_age, "Age", w_detail, "Detail");
    print_table_line(w_id, w_title, w_genre, w_dur, w_age, w_detail);

    // Cetak Isi Data
    for (int i = 0; i < match_count; i++) {
        char temp_dur[20], temp_age[20];
        sprintf(temp_dur, "%d", matched[i].duration);
        sprintf(temp_age, "%d", matched[i].age_rating);

        printf("| %-*d | %-*s | %-*s | %-*s | %-*s | %-*s |\n",
               w_id, matched[i].id, w_title, matched[i].title, w_genre, matched[i].genre,
               w_dur, temp_dur, w_age, temp_age, w_detail, matched[i].detail);
    }
    
    print_table_line(w_id, w_title, w_genre, w_dur, w_age, w_detail);
    printf("Found %d matching film(s).\n", match_count);
    for(int i = 0; i < total_width; i++) printf("="); printf("\n\n");
    system("pause");
    film_manage();
}

// ============================================================
// !! VIEW ALL FILMS !!
// ============================================================
void view_film() {
    system("cls");
    Film result[200];
    int count = 0;
    
    // Ambil data film menggunakan B-Tree In-Order Traversal
    btree_inorder(film_tree, result, &count);

    int w_id = 2;       
    int w_title = 5;    
    int w_genre = 5;    
    int w_dur = 8;      
    int w_age = 3;      
    int w_detail = 6;   

    // [Pass 1] Mencari panjang string maksimal di setiap kolom
    for (int i = 0; i < count; i++) {
        char temp_id[20], temp_dur[20], temp_age[20];
        
        sprintf(temp_id, "%d", result[i].id);
        sprintf(temp_dur, "%d", result[i].duration);
        sprintf(temp_age, "%d", result[i].age_rating);
        if ((int)strlen(temp_id) > w_id) w_id = strlen(temp_id);
        if ((int)strlen(result[i].title) > w_title) w_title = strlen(result[i].title);
        if ((int)strlen(result[i].genre) > w_genre) w_genre = strlen(result[i].genre);
        if ((int)strlen(temp_dur) > w_dur) w_dur = strlen(temp_dur);
        if ((int)strlen(temp_age) > w_age) w_age = strlen(temp_age);
        if ((int)strlen(result[i].detail) > w_detail) w_detail = strlen(result[i].detail);
    }

    int total_width = w_id + w_title + w_genre + w_dur + w_age + w_detail + 19;

    printf("\n");
    for(int i = 0; i < total_width; i++) printf("="); printf("\n");
    
    int padding = (total_width - 9) / 2; 
    for(int i = 0; i < padding; i++) printf(" ");
    printf("ALL FILMS\n");

    padding = (total_width - 31) / 2; 
    for(int i = 0; i < padding; i++) printf(" ");
    printf("(Displayed via B-Tree In-Order)\n");

    for(int i = 0; i < total_width; i++) printf("="); printf("\n");

    // [Pass 2] Tampilkan tabel
    print_table_line(w_id, w_title, w_genre, w_dur, w_age, w_detail);
    
    printf("| %-*s | %-*s | %-*s | %-*s | %-*s | %-*s |\n",
           w_id, "ID", w_title, "Title", w_genre, "Genre", 
           w_dur, "Duration", w_age, "Age", w_detail, "Detail");
           
    print_table_line(w_id, w_title, w_genre, w_dur, w_age, w_detail);

    if (count == 0) {
        printf("| %-*s |\n", total_width - 4, "No films available at the moment.");
    } else {
        for (int i = 0; i < count; i++) {
            char temp_dur[20], temp_age[20];
            sprintf(temp_dur, "%d", result[i].duration);
            sprintf(temp_age, "%d", result[i].age_rating);

            printf("| %-*d | %-*s | %-*s | %-*s | %-*s | %-*s |\n",
                   w_id, result[i].id, w_title, result[i].title, w_genre, result[i].genre,
                   w_dur, temp_dur, w_age, temp_age, w_detail, result[i].detail);
        }
    }
    print_table_line(w_id, w_title, w_genre, w_dur, w_age, w_detail);
    printf("Total: %d film(s)\n", count);
    for(int i = 0; i < total_width; i++) printf("="); printf("\n\n");

    // ==========================================
    // KEMBALI KE MENU (PRESS ANY KEY)
    // ==========================================
    system("pause");
    film_manage();
}

// ============================================================
// !! STUDIO & SCHEDULE MANAGEMENT !!
// ============================================================
void schedule_manage() {
    char input[100];
    char lower_input[100];
    int choice;
    int valid;

    do {
        system("cls");
        printf("============================================\n");
        printf("       STUDIO & SCHEDULE MANAGEMENT         \n");
        printf("--------------------------------------------\n");
        printf("[1] View All Studios\n");
        printf("[2] View All Schedules\n");
        printf("[3] Add New Studio\n");
        printf("[4] Edit Studio\n");
        printf("[5] Delete Studio\n");
        printf("[6] Add Schedule\n");
        printf("[7] Edit Schedule\n");
        printf("[8] Delete Schedule\n");
        printf("[0] Back\n");
        printf("============================================\n");
        printf("Choose : ");
        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = '\0';

        // Mengonversi input menjadi lowercase agar case-insensitive
        for (int i = 0; input[i] != '\0'; i++) {
            lower_input[i] = tolower(input[i]);
        }
        lower_input[strlen(input)] = '\0';

        valid = 1;

        if (strcmp(lower_input, "1") == 0 || strcmp(lower_input, "view all studios") == 0) {
            choice = 1;
        } else if (strcmp(lower_input, "2") == 0 || strcmp(lower_input, "view all schedules") == 0) {
            choice = 2;
        } else if (strcmp(lower_input, "3") == 0 || strcmp(lower_input, "add new studio") == 0) {
            choice = 3;
        } else if (strcmp(lower_input, "4") == 0 || strcmp(lower_input, "edit studio") == 0) {
            choice = 4;
        } else if (strcmp(lower_input, "5") == 0 || strcmp(lower_input, "delete studio") == 0) {
            choice = 5;
        } else if (strcmp(lower_input, "6") == 0 || strcmp(lower_input, "add schedule") == 0) {
            choice = 6;
        } else if (strcmp(lower_input, "7") == 0 || strcmp(lower_input, "edit schedule") == 0) {
            choice = 7;
        } else if (strcmp(lower_input, "8") == 0 || strcmp(lower_input, "delete schedule") == 0) {
            choice = 8;
        } else if (strcmp(lower_input, "0") == 0 || strcmp(lower_input, "back") == 0) {
            choice = 0;
        } else {
            printf("\n[ERROR] Invalid Input!\n");
            printf("Please enter the number (0-8) or the exact option text.\n");
            system("pause");
            valid = 0;
        }
    } while (!valid);

    switch (choice) {
        case 1: view_studios();  break;
        case 2: view_schedule(); break;
        case 3: add_studio();    break;
        case 4: edit_studio();   break;
        case 5: del_studio();    break;
        case 6: add_schedule();  break;
        case 7: edit_schedule(); break;
        case 8: del_schedule();  break;
        case 0: menu_admin();    break;
    }
}

// ============================================================
// !! EDIT STUDIO !!
// ============================================================
void edit_studio() {
    Studio studios[100];
    char buffer[1024];
    int count = 0;

    // ==========================================
    // TAHAP 1: BACA DATA KE DALAM ARRAY
    // ==========================================
    FILE* file = fopen(studio_file, "r");
    if (file != NULL) {
        while (fgets(buffer, sizeof(buffer), file)) {
            buffer[strcspn(buffer, "\n")] = 0;
            if (strlen(buffer) == 0) continue;
            if (sscanf(buffer, "%d=%[^=]=%d=%d=%d",
                   &studios[count].id, studios[count].name,
                   &studios[count].capacity, &studios[count].rows, &studios[count].cols) == 5) {
                count++;
            }
        }
        fclose(file);
    }

    if (count == 0) {
        system("cls");
        printf("============================================\n");
        printf("                EDIT STUDIO                 \n");
        printf("============================================\n");
        printf("No studios available.\n");
        system("pause");
        schedule_manage();
        return;
    }

    // ==========================================
    // HITUNG LEBAR KOLOM DINAMIS
    // ==========================================
    int w_id = 2, w_name = 4, w_cap = 8, w_rows = 4, w_cols = 4;
    for (int i = 0; i < count; i++) {
        char t_id[20], t_cap[20], t_rows[20], t_cols[20];
        sprintf(t_id, "%d", studios[i].id);
        sprintf(t_cap, "%d", studios[i].capacity);
        sprintf(t_rows, "%d", studios[i].rows);
        sprintf(t_cols, "%d", studios[i].cols);

        if ((int)strlen(t_id) > w_id) w_id = strlen(t_id);
        if ((int)strlen(studios[i].name) > w_name) w_name = strlen(studios[i].name);
        if ((int)strlen(t_cap) > w_cap) w_cap = strlen(t_cap);
        if ((int)strlen(t_rows) > w_rows) w_rows = strlen(t_rows);
        if ((int)strlen(t_cols) > w_cols) w_cols = strlen(t_cols);
    }

    #define PRINT_STUDIO_LINE() \
        do { \
            printf("+"); \
            for(int k = 0; k < w_id + 2; k++) printf("-"); printf("+"); \
            for(int k = 0; k < w_name + 2; k++) printf("-"); printf("+"); \
            for(int k = 0; k < w_cap + 2; k++) printf("-"); printf("+"); \
            for(int k = 0; k < w_rows + 2; k++) printf("-"); printf("+"); \
            for(int k = 0; k < w_cols + 2; k++) printf("-"); printf("+\n"); \
        } while(0)

    char input[100];
    char lower_input[100];
    int edit_id = -1;
    Studio target_studio;
    int target_found = 0;

    // ==========================================
    // PAGE 1: PILIH STUDIO YANG INGIN DIEDIT
    // ==========================================
    do {
        system("cls");
        printf("=================================================================\n");
        printf("                           EDIT STUDIO                           \n");
        printf("=================================================================\n");

        PRINT_STUDIO_LINE();
        printf("| %-*s | %-*s | %-*s | %-*s | %-*s |\n",
               w_id, "ID", w_name, "Name", w_cap, "Capacity", w_rows, "Rows", w_cols, "Cols");
        PRINT_STUDIO_LINE();

        for (int i = 0; i < count; i++) {
            char t_cap[20], t_rows[20], t_cols[20];
            sprintf(t_cap, "%d", studios[i].capacity);
            sprintf(t_rows, "%d", studios[i].rows);
            sprintf(t_cols, "%d", studios[i].cols);

            printf("| %-*d | %-*s | %-*s | %-*s | %-*s |\n",
                   w_id, studios[i].id, w_name, studios[i].name,
                   w_cap, t_cap, w_rows, t_rows, w_cols, t_cols);
        }
        PRINT_STUDIO_LINE();
        
        printf(" (Enter '0' to go back)\n");
        printf("-----------------------------------------------------------------\n");
        printf("Enter Studio ID or Name to edit : ");

        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = '\0';

        if (strcmp(input, "0") == 0) { schedule_manage(); return; }

        if (strlen(input) == 0) {
            printf("[ERROR] Input cannot be empty!\n");
            system("pause");
            continue;
        }

        // Konversi ke lowercase untuk pencarian string
        for (int i = 0; input[i] != '\0'; i++) lower_input[i] = tolower(input[i]);
        lower_input[strlen(input)] = '\0';

        int is_num = 1;
        for (int i = 0; input[i] != '\0'; i++) {
            if (!isdigit(input[i])) { is_num = 0; break; }
        }

        Studio matched[100];
        int match_count = 0;

        if (is_num) {
            int input_id = atoi(input);
            for (int i = 0; i < count; i++) {
                if (studios[i].id == input_id) { matched[match_count++] = studios[i]; break; }
            }
        } else {
            for (int i = 0; i < count; i++) {
                char lower_name[100];
                strcpy(lower_name, studios[i].name);
                for (int j = 0; lower_name[j] != '\0'; j++) lower_name[j] = tolower(lower_name[j]);
                if (strstr(lower_name, lower_input) != NULL) matched[match_count++] = studios[i];
            }
        }

        if (match_count == 0) {
            printf("[ERROR] Studio \"%s\" not found!\n", input);
            system("pause");
        } else if (match_count == 1) {
            edit_id = matched[0].id;
            target_studio = matched[0];
            target_found = 1;
        } else {
            int resolved = 0;
            do {
                system("cls");
                printf("============================================\n");
                printf("      EDIT STUDIO - MULTIPLE MATCHES        \n");
                printf("============================================\n");
                for (int i = 0; i < match_count; i++) printf("ID: %-4d | Name: %s\n", matched[i].id, matched[i].name);
                printf("--------------------------------------------\n");
                printf("Enter EXACT ID (or '0' to go back) : ");
                
                char id_input[100];
                fgets(id_input, sizeof(id_input), stdin);
                id_input[strcspn(id_input, "\n")] = '\0';

                if (strcmp(id_input, "0") == 0) { schedule_manage(); return; }
                if (strlen(id_input) == 0) { printf("[ERROR] Empty input!\n"); system("pause"); continue; }

                int id_is_num = 1;
                for (int i = 0; id_input[i] != '\0'; i++) if (!isdigit(id_input[i])) id_is_num = 0;
                if (!id_is_num) { printf("[ERROR] Enter a numeric ID.\n"); system("pause"); continue; }

                int selected_id = atoi(id_input);
                for (int i = 0; i < match_count; i++) {
                    if (matched[i].id == selected_id) {
                        edit_id = selected_id;
                        target_studio = matched[i];
                        resolved = 1; target_found = 1; break;
                    }
                }
                if (!resolved) { printf("[ERROR] ID %d not in list!\n", selected_id); system("pause"); }
            } while (!resolved);
        }
    } while (!target_found);


    // ==========================================
    // PAGE 2: EDIT NAMA STUDIO
    // ==========================================
    int name_done = 0;
    do {
        system("cls");
        printf("============================================\n");
        printf("          EDIT STUDIO ID: %d (NAME)         \n", edit_id);
        printf("============================================\n");
        printf("Leave blank (Press Enter) to keep current Name [%s]\n", target_studio.name);
        printf("(Enter '0' to Cancel & Go Back)\n");
        printf("--------------------------------------------\n");
        printf("Enter Studio Name : ");

        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = '\0';

        if (strcmp(input, "0") == 0) { schedule_manage(); return; }

        if (strlen(input) == 0) { 
            name_done = 1; break; // Keep current
        }

        // Format Title Case
        int capitalize_next = 1;
        for (int i = 0; input[i] != '\0'; i++) {
            if (isspace(input[i])) { capitalize_next = 1; } 
            else if (capitalize_next) { input[i] = toupper(input[i]); capitalize_next = 0; } 
            else { input[i] = tolower(input[i]); }
        }

        // Cek Duplikat (kecuali studio ini sendiri)
        int is_duplicate = 0;
        for (int i = 0; i < count; i++) {
            if (studios[i].id != edit_id) {
                char temp_in[100], temp_ext[100];
                strcpy(temp_in, input); strcpy(temp_ext, studios[i].name);
                for(int j=0; temp_in[j]; j++) temp_in[j] = tolower(temp_in[j]);
                for(int j=0; temp_ext[j]; j++) temp_ext[j] = tolower(temp_ext[j]);
                
                if (strcmp(temp_in, temp_ext) == 0) {
                    is_duplicate = 1; break;
                }
            }
        }

        if (is_duplicate) {
            printf("[ERROR] Studio name \"%s\" already exists!\n", input);
            system("pause");
        } else {
            strcpy(target_studio.name, input);
            name_done = 1;
        }
    } while (!name_done);


    // ==========================================
    // PAGE 3: EDIT ROWS (BARIS)
    // ==========================================
    int rows_done = 0;
    do {
        system("cls");
        printf("============================================\n");
        printf("          EDIT STUDIO ID: %d (ROWS)         \n", edit_id);
        printf("============================================\n");
        printf("Studio Name : %s\n", target_studio.name);
        printf("--------------------------------------------\n");
        printf("Leave blank (Press Enter) to keep current Rows [%d]\n", target_studio.rows);
        printf("(Enter '0' to Cancel & Go Back)\n");
        printf("--------------------------------------------\n");
        printf("Enter Rows (Max 25) : ");

        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = '\0';

        if (strcmp(input, "0") == 0) { schedule_manage(); return; }
        if (strlen(input) == 0) { rows_done = 1; break; }

        int is_num = 1;
        for (int i = 0; input[i] != '\0'; i++) if (!isdigit(input[i])) { is_num = 0; break; }

        if (is_num) {
            int r = atoi(input);
            if (r > 0 && r <= 25) { target_studio.rows = r; rows_done = 1; }
            else { printf("[ERROR] Rows must be between 1 and 25!\n"); system("pause"); }
        } else {
            printf("[ERROR] Rows must be a valid number!\n");
            system("pause");
        }
    } while (!rows_done);


    // ==========================================
    // PAGE 4: EDIT COLS (KOLOM)
    // ==========================================
    int cols_done = 0;
    do {
        system("cls");
        printf("============================================\n");
        printf("          EDIT STUDIO ID: %d (COLS)         \n", edit_id);
        printf("============================================\n");
        printf("Studio Name : %s\n", target_studio.name);
        printf("Rows        : %d\n", target_studio.rows);
        printf("--------------------------------------------\n");
        printf("Leave blank (Press Enter) to keep current Cols [%d]\n", target_studio.cols);
        printf("(Enter '0' to Cancel & Go Back)\n");
        printf("--------------------------------------------\n");
        printf("Enter Cols (Max 25) : ");

        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = '\0';

        if (strcmp(input, "0") == 0) { schedule_manage(); return; }
        if (strlen(input) == 0) { cols_done = 1; break; }

        int is_num = 1;
        for (int i = 0; input[i] != '\0'; i++) if (!isdigit(input[i])) { is_num = 0; break; }

        if (is_num) {
            int c = atoi(input);
            if (c > 0 && c <= 25) { target_studio.cols = c; cols_done = 1; }
            else { printf("[ERROR] Cols must be between 1 and 25!\n"); system("pause"); }
        } else {
            printf("[ERROR] Cols must be a valid number!\n");
            system("pause");
        }
    } while (!cols_done);


    // ==========================================
    // TAHAP 5: KALKULASI & SIMPAN PERUBAHAN
    // ==========================================
    target_studio.capacity = target_studio.rows * target_studio.cols;

    FILE* in  = fopen(studio_file, "r");
    FILE* tmp = fopen("temp_studio.txt", "w");
    if (in == NULL || tmp == NULL) { invalid_file(); return; }
    
    char sbuf[300];
    while (fgets(sbuf, sizeof(sbuf), in)) {
        sbuf[strcspn(sbuf, "\n")] = 0;
        int id; sscanf(sbuf, "%d=", &id);
        if (id == edit_id)
            fprintf(tmp, "%d=%s=%d=%d=%d\n",
                    target_studio.id, target_studio.name, target_studio.capacity,
                    target_studio.rows, target_studio.cols);
        else
            fprintf(tmp, "%s\n", sbuf);
    }
    fclose(in); fclose(tmp);
    remove(studio_file);
    rename("temp_studio.txt", studio_file);


    // ==========================================
    // PAGE 6: TAMPILAN INVOICE / SUCCESS DETAIL
    // ==========================================
    system("cls");
    printf("============================================\n");
    printf("         Studio ID %d updated!              \n", edit_id);
    printf("--------------------------------------------\n");
    printf("  Detail Studio:\n");
    printf("  ID       : %d\n", target_studio.id);
    printf("  Name     : %s\n", target_studio.name);
    printf("  Rows     : %d\n", target_studio.rows);
    printf("  Cols     : %d\n", target_studio.cols);
    printf("  Capacity : %d (Auto: %d x %d)\n", target_studio.capacity, target_studio.rows, target_studio.cols);
    printf("============================================\n");
    system("pause");
    schedule_manage();
}

// ============================================================
// !! ADD STUDIO !!
// ============================================================
void add_studio() {
    Studio s;
    int valid;
    char input[300];

    /* --- INPUT NAMA STUDIO --- */
    do {
        system("cls");
        printf("============================================\n");
        printf("               ADD NEW STUDIO               \n");
        printf(" (Enter '0' at any prompt to go back)\n");
        printf("--------------------------------------------\n");

        printf("Studio Name : ");
        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = '\0';

        if (strcmp(input, "0") == 0) { schedule_manage(); return; }
        
        if (strlen(input) == 0) { 
            printf("[ERROR] Name cannot be empty!\n");
            system("pause");
            valid = 0; 
            continue;
        }

        // 1. Format ke Title Case (Kapital di awal setiap kata)
        int capitalize_next = 1;
        for (int i = 0; input[i] != '\0'; i++) {
            if (isspace(input[i])) {
                capitalize_next = 1; 
            } else if (capitalize_next) {
                input[i] = toupper(input[i]);
                capitalize_next = 0;
            } else {
                input[i] = tolower(input[i]);
            }
        }

        // 2. Cek Duplikat di Database (Case-Insensitive)
        int is_duplicate = 0;
        FILE* check_fp = fopen(studio_file, "r");
        if (check_fp != NULL) {
            char s_buf[300];
            while (fgets(s_buf, sizeof(s_buf), check_fp)) {
                s_buf[strcspn(s_buf, "\n")] = 0;
                if (strlen(s_buf) == 0) continue;
                
                Studio ext_s;
                if (sscanf(s_buf, "%d=%[^=]=%d=%d=%d", 
                           &ext_s.id, ext_s.name, 
                           &ext_s.capacity, &ext_s.rows, &ext_s.cols) == 5) {
                    
                    char temp_in[100], temp_ext[100];
                    strcpy(temp_in, input);
                    strcpy(temp_ext, ext_s.name);
                    
                    for(int j = 0; temp_in[j]; j++) temp_in[j] = tolower(temp_in[j]);
                    for(int j = 0; temp_ext[j]; j++) temp_ext[j] = tolower(temp_ext[j]);
                    
                    if (strcmp(temp_in, temp_ext) == 0) {
                        is_duplicate = 1;
                        break;
                    }
                }
            }
            fclose(check_fp);
        }

        if (is_duplicate) {
            printf("[ERROR] Studio name \"%s\" already exists! Please try another.\n", input);
            system("pause");
            valid = 0;
        } else {
            strcpy(s.name, input); 
            valid = 1;
        }
    } while (!valid);


    /* --- INPUT ROWS (BARIS) --- */
    do {
        system("cls");
        printf("============================================\n");
        printf("               ADD NEW STUDIO               \n");
        printf(" (Enter '0' at any prompt to go back)\n");
        printf("--------------------------------------------\n");
        printf("Studio Name : %s\n", s.name);
        printf("--------------------------------------------\n");

        printf("Rows        : ");
        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = '\0';

        if (strcmp(input, "0") == 0) { schedule_manage(); return; }
        
        if (strlen(input) == 0) { 
            printf("[ERROR] Rows cannot be empty!\n");
            system("pause");
            valid = 0; 
            continue; 
        }

        valid = 1;
        for (int i = 0; input[i] != '\0'; i++) {
            if (!isdigit(input[i])) { valid = 0; break; }
        }

        if (valid) {
            s.rows = atoi(input);
            if (s.rows <= 0 || s.rows > 25) { 
                printf("[ERROR] Rows must be between 1 and 25!\n");
                system("pause");
                valid = 0; 
            }
        } else {
            printf("[ERROR] Rows must be a number!\n");
            system("pause");
        }
    } while (!valid);


    /* --- INPUT COLS (KOLOM) --- */
    do {
        system("cls");
        printf("============================================\n");
        printf("               ADD NEW STUDIO               \n");
        printf(" (Enter '0' at any prompt to go back)\n");
        printf("--------------------------------------------\n");
        printf("Studio Name : %s\n", s.name);
        printf("Rows        : %d\n", s.rows);
        printf("--------------------------------------------\n");

        printf("Cols        : ");
        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = '\0';

        if (strcmp(input, "0") == 0) { schedule_manage(); return; }
        
        if (strlen(input) == 0) { 
            printf("[ERROR] Cols cannot be empty!\n");
            system("pause");
            valid = 0; 
            continue; 
        }

        valid = 1;
        for (int i = 0; input[i] != '\0'; i++) {
            if (!isdigit(input[i])) { valid = 0; break; }
        }

        if (valid) {
            s.cols = atoi(input);
            if (s.cols <= 0 || s.cols > 25) { 
                printf("[ERROR] Cols must be between 1 and 25!\n");
                system("pause");
                valid = 0; 
            }
        } else {
            printf("[ERROR] Cols must be a number!\n");
            system("pause");
        }
    } while (!valid);


    /* --- PROSES PENYIMPANAN --- */
    s.capacity = s.rows * s.cols;
    s.id = auto_id_studio();

    FILE* fp = fopen(studio_file, "a");
    if (fp == NULL) { invalid_file(); return; }
    fprintf(fp, "%d=%s=%d=%d=%d\n", s.id, s.name, s.capacity, s.rows, s.cols);
    fclose(fp);

    /* --- PESAN SUKSES --- */
    system("cls");
    printf("============================================\n");
    printf("        Studio added successfully!          \n");
    printf("--------------------------------------------\n");
    printf("  ID       : %d\n", s.id);
    printf("  Name     : %s\n", s.name);
    printf("  Rows     : %d\n", s.rows);
    printf("  Cols     : %d\n", s.cols);
    printf("  Capacity : %d (Auto: %d x %d)\n", s.capacity, s.rows, s.cols);
    printf("============================================\n");
    system("pause");
    schedule_manage();
}

// ============================================================
// !! VIEW ALL STUDIOS !!
// ============================================================
void view_studios() {
    system("cls");
    Studio result[100];
    char buffer[1024];
    int count = 0;
    
    // ==========================================
    // TAHAP 1: BACA DATA KE DALAM ARRAY
    // ==========================================
    FILE* file = fopen(studio_file, "r"); 
    if (file != NULL) {
        while (fgets(buffer, sizeof(buffer), file)) {
            buffer[strcspn(buffer, "\n")] = 0; // Hapus newline
            
            // Lewati baris kosong agar tidak error
            if (strlen(buffer) == 0) continue;

            // BUG FIXED: Format disesuaikan dengan separator '='
            // %[^=] memastikan nama dibaca sampai ketemu karakter '='
            if (sscanf(buffer, "%d=%[^=]=%d=%d=%d", 
                   &result[count].id, 
                   result[count].name, 
                   &result[count].capacity, 
                   &result[count].rows, 
                   &result[count].cols) == 5) {
                count++; // Hitung hanya jika ke-5 data berhasil terbaca
            }
        }
        fclose(file);
    }

    int w_id = 2;       // Lebar minimal "ID"
    int w_name = 4;     // Lebar minimal "Name"
    int w_cap = 8;      // Lebar minimal "Capacity"
    int w_rows = 4;     // Lebar minimal "Rows"
    int w_cols = 4;     // Lebar minimal "Cols"

    for (int i = 0; i < count; i++) {
        char t_id[20], t_cap[20], t_rows[20], t_cols[20];
        sprintf(t_id, "%d", result[i].id);
        sprintf(t_cap, "%d", result[i].capacity);
        sprintf(t_rows, "%d", result[i].rows);
        sprintf(t_cols, "%d", result[i].cols);

        if ((int)strlen(t_id) > w_id) w_id = strlen(t_id);
        if ((int)strlen(result[i].name) > w_name) w_name = strlen(result[i].name);
        if ((int)strlen(t_cap) > w_cap) w_cap = strlen(t_cap);
        if ((int)strlen(t_rows) > w_rows) w_rows = strlen(t_rows);
        if ((int)strlen(t_cols) > w_cols) w_cols = strlen(t_cols);
    }

    int total_width = w_id + w_name + w_cap + w_rows + w_cols + 16;

    // Macro untuk mencetak garis tabel
    #define PRINT_STUDIO_LINE() \
        do { \
            printf("+"); \
            for(int k = 0; k < w_id + 2; k++) printf("-"); printf("+"); \
            for(int k = 0; k < w_name + 2; k++) printf("-"); printf("+"); \
            for(int k = 0; k < w_cap + 2; k++) printf("-"); printf("+"); \
            for(int k = 0; k < w_rows + 2; k++) printf("-"); printf("+"); \
            for(int k = 0; k < w_cols + 2; k++) printf("-"); printf("+\n"); \
        } while(0)

    // ==========================================
    // TAHAP 3: CETAK TABEL
    // ==========================================
    printf("\n");
    for(int i = 0; i < total_width; i++) printf("="); printf("\n");
    
    int padding = (total_width - 11) / 2; // 11 adalah panjang "ALL STUDIOS"
    if(padding < 0) padding = 0;
    for(int i = 0; i < padding; i++) printf(" ");
    printf("ALL STUDIOS\n");
    
    for(int i = 0; i < total_width; i++) printf("="); printf("\n");

    if (count == 0) {
        printf("| %-*s |\n", total_width - 4, "No studios available at the moment.");
        for(int i = 0; i < total_width; i++) printf("="); printf("\n");
    } else {
        PRINT_STUDIO_LINE();
        
        printf("| %-*s | %-*s | %-*s | %-*s | %-*s |\n",
               w_id, "ID", w_name, "Name", w_cap, "Capacity", w_rows, "Rows", w_cols, "Cols");
               
        PRINT_STUDIO_LINE();

        for (int i = 0; i < count; i++) {
            char t_cap[20], t_rows[20], t_cols[20];
            sprintf(t_cap, "%d", result[i].capacity);
            sprintf(t_rows, "%d", result[i].rows);
            sprintf(t_cols, "%d", result[i].cols);

            printf("| %-*d | %-*s | %-*s | %-*s | %-*s |\n",
                   w_id, result[i].id, 
                   w_name, result[i].name, 
                   w_cap, t_cap, 
                   w_rows, t_rows, 
                   w_cols, t_cols);
        }
        
        PRINT_STUDIO_LINE();
        printf("Total: %d studio(s)\n", count);
        for(int i = 0; i < total_width; i++) printf("="); printf("\n");
    }
    printf("\n");

    // ==========================================
    // TAHAP 4: KEMBALI KE MENU (PRESS ANY KEY)
    // ==========================================
    system("pause");
    schedule_manage();
}

// ============================================================
// !! DELETE STUDIO !!
// ============================================================
void del_studio() {
    Studio studios[100];
    char buffer[1024];
    int count = 0;

    // ==========================================
    // TAHAP 1: BACA DATA & CEK KETERSEDIAAN
    // ==========================================
    FILE* file = fopen(studio_file, "r");
    if (file != NULL) {
        while (fgets(buffer, sizeof(buffer), file)) {
            buffer[strcspn(buffer, "\n")] = 0;
            if (strlen(buffer) == 0) continue;
            if (sscanf(buffer, "%d=%[^=]=%d=%d=%d",
                   &studios[count].id, studios[count].name,
                   &studios[count].capacity, &studios[count].rows, &studios[count].cols) == 5) {
                count++;
            }
        }
        fclose(file);
    }

    if (count == 0) {
        system("cls");
        printf("============================================\n");
        printf("               DELETE STUDIO                \n");
        printf("============================================\n");
        printf("No studios available.\n");
        printf("============================================\n");
        system("pause");
        schedule_manage();
        return;
    }

    // [Hitung Lebar Kolom Dinamis]
    int w_id = 2, w_name = 4, w_cap = 8, w_rows = 4, w_cols = 4;
    for (int i = 0; i < count; i++) {
        char t_id[20], t_cap[20], t_rows[20], t_cols[20];
        sprintf(t_id, "%d", studios[i].id);
        sprintf(t_cap, "%d", studios[i].capacity);
        sprintf(t_rows, "%d", studios[i].rows);
        sprintf(t_cols, "%d", studios[i].cols);

        if ((int)strlen(t_id) > w_id) w_id = strlen(t_id);
        if ((int)strlen(studios[i].name) > w_name) w_name = strlen(studios[i].name);
        if ((int)strlen(t_cap) > w_cap) w_cap = strlen(t_cap);
        if ((int)strlen(t_rows) > w_rows) w_rows = strlen(t_rows);
        if ((int)strlen(t_cols) > w_cols) w_cols = strlen(t_cols);
    }

    #define PRINT_STUDIO_LINE() \
        do { \
            printf("+"); \
            for(int k = 0; k < w_id + 2; k++) printf("-"); printf("+"); \
            for(int k = 0; k < w_name + 2; k++) printf("-"); printf("+"); \
            for(int k = 0; k < w_cap + 2; k++) printf("-"); printf("+"); \
            for(int k = 0; k < w_rows + 2; k++) printf("-"); printf("+"); \
            for(int k = 0; k < w_cols + 2; k++) printf("-"); printf("+\n"); \
        } while(0)

    char input[100];
    char lower_input[100];
    int target_id = -1;
    Studio matched[100];
    int match_count = 0;

    // ==========================================
    // PAGE 1 : PILIH STUDIO
    // ==========================================
    do {
        match_count = 0;
        system("cls");
        printf("=================================================================\n");
        printf("                          DELETE STUDIO                          \n");
        printf("=================================================================\n");

        PRINT_STUDIO_LINE();
        printf("| %-*s | %-*s | %-*s | %-*s | %-*s |\n",
               w_id, "ID", w_name, "Name", w_cap, "Capacity", w_rows, "Rows", w_cols, "Cols");
        PRINT_STUDIO_LINE();

        for (int i = 0; i < count; i++) {
            char t_cap[20], t_rows[20], t_cols[20];
            sprintf(t_cap, "%d", studios[i].capacity);
            sprintf(t_rows, "%d", studios[i].rows);
            sprintf(t_cols, "%d", studios[i].cols);

            printf("| %-*d | %-*s | %-*s | %-*s | %-*s |\n",
                   w_id, studios[i].id,
                   w_name, studios[i].name,
                   w_cap, t_cap,
                   w_rows, t_rows,
                   w_cols, t_cols);
        }
        PRINT_STUDIO_LINE();
        
        printf(" (Enter '0' to go back)\n");
        printf("-----------------------------------------------------------------\n");
        printf("Enter Studio ID or Name to delete : ");

        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = '\0';

        if (strcmp(input, "0") == 0) { schedule_manage(); return; }

        if (strlen(input) == 0) {
            printf("[ERROR] Input cannot be empty!\n");
            system("pause");
            continue;
        }

        // Konversi ke lowercase untuk pencarian string
        for (int i = 0; input[i] != '\0'; i++) {
            lower_input[i] = tolower(input[i]);
        }
        lower_input[strlen(input)] = '\0';

        // Deteksi apakah input berupa angka murni
        int is_num = 1;
        for (int i = 0; input[i] != '\0'; i++) {
            if (!isdigit(input[i])) { is_num = 0; break; }
        }

        if (is_num) {
            int input_id = atoi(input);
            for (int i = 0; i < count; i++) {
                if (studios[i].id == input_id) {
                    matched[match_count] = studios[i];
                    match_count++;
                    break;
                }
            }
        } else {
            for (int i = 0; i < count; i++) {
                char lower_name[100];
                strcpy(lower_name, studios[i].name);
                for (int j = 0; lower_name[j] != '\0'; j++) {
                    lower_name[j] = tolower(lower_name[j]);
                }
                if (strstr(lower_name, lower_input) != NULL) {
                    matched[match_count] = studios[i];
                    match_count++;
                }
            }
        }

        if (match_count == 0) {
            printf("[ERROR] Studio \"%s\" not found! Please try again.\n", input);
            system("pause");
        } else if (match_count == 1) {
            target_id = matched[0].id;
        } else {
            // ==========================================
            // RESOLUSI AMBIGUITAS (Jika lebih dari 1 mirip)
            // ==========================================
            int resolved = 0;
            do {
                system("cls");
                printf("============================================\n");
                printf("     DELETE STUDIO - MULTIPLE MATCHES       \n");
                printf("============================================\n");
                printf("Multiple studios found for \"%s\":\n", input);
                printf("--------------------------------------------\n");
                for (int i = 0; i < match_count; i++) {
                    printf("ID: %-4d | Name: %s\n", matched[i].id, matched[i].name);
                }
                printf("--------------------------------------------\n");
                printf("Enter the EXACT ID from the list above\n");
                printf(" (or '0' to go back) : ");

                char id_input[100];
                fgets(id_input, sizeof(id_input), stdin);
                id_input[strcspn(id_input, "\n")] = '\0';

                if (strcmp(id_input, "0") == 0) { schedule_manage(); return; }

                if (strlen(id_input) == 0) {
                    printf("[ERROR] Input cannot be empty!\n");
                    system("pause");
                    continue;
                }

                int id_is_num = 1;
                for (int i = 0; id_input[i] != '\0'; i++) {
                    if (!isdigit(id_input[i])) { id_is_num = 0; break; }
                }

                if (!id_is_num) {
                    printf("[ERROR] Invalid input! Please enter a numeric ID.\n");
                    system("pause");
                    continue;
                }

                int selected_id = atoi(id_input);
                for (int i = 0; i < match_count; i++) {
                    if (matched[i].id == selected_id) {
                        target_id = selected_id;
                        resolved = 1;
                        break;
                    }
                }

                if (!resolved) {
                    printf("[ERROR] ID %d is not on the list! Please type the exact ID.\n", selected_id);
                    system("pause");
                }
            } while (!resolved);
        }
    } while (target_id == -1);


    // ==========================================
    // TAHAP 2: CEK DEPENDENCY (Jadwal Aktif)
    // ==========================================
    Studio target_studio;
    find_studio(target_id, &target_studio);

    if (studio_in_use(target_id)) {
        printf("-----------------------------------------------------------------\n");
        printf("[ERROR] Cannot delete studio \"%s\"!\n", target_studio.name);
        printf("This studio is currently being used by active schedules.\n");
        printf("Please delete the related schedules before deleting this studio.\n");
        printf("-----------------------------------------------------------------\n");
        system("pause");
        schedule_manage();
        return;
    }


    // ==========================================
    // PAGE 2 : KONFIRMASI Y/N
    // ==========================================
    char confirm_str[10];
    char confirm_char;
    int conf_valid = 0;

    do {
        system("cls");
        printf("============================================\n");
        printf("            CONFIRM DELETION                \n");
        printf("============================================\n");
        printf("  Detail Studio:\n");
        printf("  ID       : %d\n", target_studio.id);
        printf("  Name     : %s\n", target_studio.name);
        printf("  Capacity : %d\n", target_studio.capacity);
        printf("  Rows     : %d\n", target_studio.rows);
        printf("  Cols     : %d\n", target_studio.cols);
        printf("--------------------------------------------\n");
        printf("Are you sure you want to delete this studio?\n");
        printf("Type (Y/N) or '0' to go back : ");

        fgets(confirm_str, sizeof(confirm_str), stdin);
        confirm_str[strcspn(confirm_str, "\n")] = '\0';

        if (strcmp(confirm_str, "0") == 0) { schedule_manage(); return; }

        if (strlen(confirm_str) == 0) {
            printf("[ERROR] Input cannot be empty! Enter Y or N.\n");
            system("pause");
            continue;
        }

        if (strcmp(confirm_str, "Y") == 0 || strcmp(confirm_str, "y") == 0) {
            confirm_char = 'y';
            conf_valid = 1;
        } else if (strcmp(confirm_str, "N") == 0 || strcmp(confirm_str, "n") == 0) {
            confirm_char = 'n';
            conf_valid = 1;
        } else {
            printf("[ERROR] Invalid input! Enter Y or N.\n");
            system("pause");
        }
    } while (!conf_valid);

    if (confirm_char == 'n') {
        printf("Deletion cancelled.\n");
        system("pause");
        schedule_manage();
        return;
    }


    // ==========================================
    // PAGE 3 : KONFIRMASI PASSWORD ADMIN
    // ==========================================
    char input_pass[100];
    char confirm_pass_input[100]; 
    int pass_match = 0;

    do {
        system("cls");
        printf("============================================\n");
        printf("         ADMIN PASSWORD CONFIRMATION        \n");
        printf("============================================\n");
        printf(" [WARNING] This action cannot be undone!    \n");
        printf("--------------------------------------------\n");
        printf(" (Enter '0' at any prompt to go back)\n");
        printf("--------------------------------------------\n");

        printf("Enter Admin password : ");
        fgets(input_pass, sizeof(input_pass), stdin);
        input_pass[strcspn(input_pass, "\n")] = '\0';

        if (strcmp(input_pass, "0") == 0) { schedule_manage(); return; }

        if (strlen(input_pass) == 0) {
            printf("[ERROR] Password cannot be empty!\n");
            system("pause");
            continue;
        }

        printf("Confirm Admin password : ");
        fgets(confirm_pass_input, sizeof(confirm_pass_input), stdin);
        confirm_pass_input[strcspn(confirm_pass_input, "\n")] = '\0';

        if (strcmp(confirm_pass_input, "0") == 0) { schedule_manage(); return; }

        if (strcmp(input_pass, confirm_pass_input) != 0) {
            printf("\n[ERROR] Passwords do not match! Please try again.\n");
            system("pause");
            continue;
        }

        // Hardcoded admin password validation
        if (strcmp(input_pass, "admin123@") != 0) {
            printf("\n[ERROR] Incorrect password! Please try again.\n");
            system("pause");
            continue;
        }

        pass_match = 1; 
    } while (!pass_match);


    // ==========================================
    // PAGE 4 : PROSES PENGHAPUSAN FILE
    // ==========================================
    FILE* in  = fopen(studio_file, "r");
    FILE* tmp = fopen("temp_studio.txt", "w");
    if (in == NULL || tmp == NULL) { invalid_file(); return; }

    char sbuf[300];
    while (fgets(sbuf, sizeof(sbuf), in)) {
        sbuf[strcspn(sbuf, "\n")] = 0;
        if(strlen(sbuf) == 0) continue;
        
        int sid; 
        sscanf(sbuf, "%d=", &sid);
        if (sid != target_id) {
            fprintf(tmp, "%s\n", sbuf);
        }
    }
    fclose(in); 
    fclose(tmp);
    remove(studio_file);
    rename("temp_studio.txt", studio_file);


    // ==========================================
    // PAGE 5 : SUCCESS INVOICE
    // ==========================================
    system("cls");
    printf("============================================\n");
    printf("               STUDIO DELETED               \n");
    printf("============================================\n");
    printf("\n");
    printf("   Studio \"%s\" (ID: %d)     \n", target_studio.name, target_id);
    printf("       deleted successfully!                \n");
    printf("\n");
    printf("============================================\n");
    system("pause");
    schedule_manage();
}

// ============================================================
// !! ADD SCHEDULE !!
// ============================================================
void add_schedule() {
    char input[100];
    char lower_input[100];
    int valid;

    /* ==========================================
       STEP 1: PILIH FILM (Dynamic Table & Search)
       ========================================== */
    Film all_films[200];
    int film_count = 0;
    btree_inorder(film_tree, all_films, &film_count);

    if (film_count == 0) {
        system("cls");
        printf("============================================\n");
        printf("               ADD SCHEDULE                 \n");
        printf("============================================\n");
        printf("No films available. Please add a film first.\n");
        system("pause");
        schedule_manage();
        return;
    }

    int w_id = 2, w_title = 5, w_genre = 5, w_dur = 8, w_age = 3;
    for (int i = 0; i < film_count; i++) {
        char temp_id[20], temp_dur[20], temp_age[20];
        sprintf(temp_id, "%d", all_films[i].id);
        sprintf(temp_dur, "%d", all_films[i].duration);
        sprintf(temp_age, "%d+", all_films[i].age_rating);

        if ((int)strlen(temp_id) > w_id) w_id = strlen(temp_id);
        if ((int)strlen(all_films[i].title) > w_title) w_title = strlen(all_films[i].title);
        if ((int)strlen(all_films[i].genre) > w_genre) w_genre = strlen(all_films[i].genre);
        if ((int)strlen(temp_dur) > w_dur) w_dur = strlen(temp_dur);
        if ((int)strlen(temp_age) > w_age) w_age = strlen(temp_age);
    }

    int film_id = -1;
    Film* selected_film = NULL;

    do {
        system("cls");
        printf("======================================================================================================\n");
        printf("                                              ADD SCHEDULE                                            \n");
        printf("======================================================================================================\n");
        printf("\n[ STEP 1 ] Select Film\n");

        printf("+"); for(int k=0; k<w_id+2; k++) printf("-");
        printf("+"); for(int k=0; k<w_title+2; k++) printf("-");
        printf("+"); for(int k=0; k<w_genre+2; k++) printf("-");
        printf("+"); for(int k=0; k<w_dur+2; k++) printf("-");
        printf("+"); for(int k=0; k<w_age+2; k++) printf("-"); printf("+\n");

        printf("| %-*s | %-*s | %-*s | %-*s | %-*s |\n", w_id, "ID", w_title, "Title", w_genre, "Genre", w_dur, "Duration", w_age, "Age");

        printf("+"); for(int k=0; k<w_id+2; k++) printf("-");
        printf("+"); for(int k=0; k<w_title+2; k++) printf("-");
        printf("+"); for(int k=0; k<w_genre+2; k++) printf("-");
        printf("+"); for(int k=0; k<w_dur+2; k++) printf("-");
        printf("+"); for(int k=0; k<w_age+2; k++) printf("-"); printf("+\n");

        for (int i = 0; i < film_count; i++) {
            char temp_dur[20], temp_age[20];
            sprintf(temp_dur, "%d", all_films[i].duration);
            sprintf(temp_age, "%d+", all_films[i].age_rating);
            printf("| %-*d | %-*s | %-*s | %-*s | %-*s |\n", 
                   w_id, all_films[i].id, w_title, all_films[i].title, w_genre, all_films[i].genre, w_dur, temp_dur, w_age, temp_age);
        }

        printf("+"); for(int k=0; k<w_id+2; k++) printf("-");
        printf("+"); for(int k=0; k<w_title+2; k++) printf("-");
        printf("+"); for(int k=0; k<w_genre+2; k++) printf("-");
        printf("+"); for(int k=0; k<w_dur+2; k++) printf("-");
        printf("+"); for(int k=0; k<w_age+2; k++) printf("-"); printf("+\n");

        printf("Enter Film ID or Title (Type '0' to Cancel) : ");
        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = '\0';

        if (strcmp(input, "0") == 0) { schedule_manage(); return; }
        if (strlen(input) == 0) { printf("[ERROR] Input cannot be empty!\n"); system("pause"); continue; }

        for (int i = 0; input[i] != '\0'; i++) lower_input[i] = tolower(input[i]);
        lower_input[strlen(input)] = '\0';

        int is_num = 1;
        for (int i = 0; input[i] != '\0'; i++) {
            if (!isdigit(input[i])) { is_num = 0; break; }
        }

        Film matched[200];
        int match_count = 0;

        if (is_num) {
            int input_id = atoi(input);
            for (int i = 0; i < film_count; i++) {
                if (all_films[i].id == input_id) {
                    matched[match_count++] = all_films[i];
                    break;
                }
            }
        } else {
            for (int i = 0; i < film_count; i++) {
                char lower_title[100];
                strcpy(lower_title, all_films[i].title);
                for (int j = 0; lower_title[j] != '\0'; j++) lower_title[j] = tolower(lower_title[j]);
                if (strstr(lower_title, lower_input) != NULL) {
                    matched[match_count++] = all_films[i];
                }
            }
        }

        if (match_count == 0) {
            printf("[ERROR] Film \"%s\" not found!\n", input);
            system("pause");
        } else if (match_count == 1) {
            film_id = matched[0].id;
        } else {
            int resolved = 0;
            do {
                system("cls");
                printf("============================================\n");
                printf("        SELECT FILM - MULTIPLE MATCHES      \n");
                printf("============================================\n");
                for (int i = 0; i < match_count; i++) {
                    printf("ID: %-4d | Title: %s\n", matched[i].id, matched[i].title);
                }
                printf("--------------------------------------------\n");
                printf("Enter EXACT ID (or '0' to go back) : ");
                
                char id_input[100];
                fgets(id_input, sizeof(id_input), stdin);
                id_input[strcspn(id_input, "\n")] = '\0';

                if (strcmp(id_input, "0") == 0) { schedule_manage(); return; }
                if (strlen(id_input) == 0) { printf("[ERROR] Empty input!\n"); system("pause"); continue; }

                int id_is_num = 1;
                for (int i = 0; id_input[i] != '\0'; i++) if (!isdigit(id_input[i])) id_is_num = 0;

                if (!id_is_num) { printf("[ERROR] Enter a numeric ID.\n"); system("pause"); continue; }

                int selected_id = atoi(id_input);
                for (int i = 0; i < match_count; i++) {
                    if (matched[i].id == selected_id) {
                        film_id = selected_id;
                        resolved = 1;
                        break;
                    }
                }
                if (!resolved) { printf("[ERROR] ID %d not in list!\n", selected_id); system("pause"); }
            } while (!resolved);
        }

        if (film_id != -1) {
            selected_film = btree_search(film_tree, film_id);
            if (selected_film == NULL) film_id = -1;
        }
    } while (film_id == -1);


    /* ==========================================
       STEP 2: PILIH STUDIO (Dynamic Table & Search)
       ========================================== */
    Studio all_studios[100];
    int studio_count = 0;
    FILE* sfp = fopen(studio_file, "r");
    if (sfp != NULL) {
        char sbuf[300];
        while (fgets(sbuf, sizeof(sbuf), sfp)) {
            sbuf[strcspn(sbuf, "\n")] = 0;
            if (strlen(sbuf) == 0) continue;
            if (sscanf(sbuf, "%d=%[^=]=%d=%d=%d", &all_studios[studio_count].id, all_studios[studio_count].name,
                       &all_studios[studio_count].capacity, &all_studios[studio_count].rows, &all_studios[studio_count].cols) == 5) {
                studio_count++;
            }
        }
        fclose(sfp);
    }

    if (studio_count == 0) {
        system("cls");
        printf("No studios available. Please add a studio first.\n");
        system("pause");
        schedule_manage();
        return;
    }

    int w_sid = 2, w_sname = 4, w_cap = 8;
    for (int i = 0; i < studio_count; i++) {
        char t_id[20], t_cap[20];
        sprintf(t_id, "%d", all_studios[i].id);
        sprintf(t_cap, "%d", all_studios[i].capacity);
        if ((int)strlen(t_id) > w_sid) w_sid = strlen(t_id);
        if ((int)strlen(all_studios[i].name) > w_sname) w_sname = strlen(all_studios[i].name);
        if ((int)strlen(t_cap) > w_cap) w_cap = strlen(t_cap);
    }

    int studio_id = -1;
    Studio selected_studio;

    do {
        system("cls");
        printf("======================================================================================================\n");
        printf("                                              ADD SCHEDULE                                            \n");
        printf("======================================================================================================\n");
        printf("\n[ STEP 2 ] Select Studio for \"%s\"\n", selected_film->title);

        printf("+"); for(int k=0; k<w_sid+2; k++) printf("-");
        printf("+"); for(int k=0; k<w_sname+2; k++) printf("-");
        printf("+"); for(int k=0; k<w_cap+2; k++) printf("-"); printf("+\n");

        printf("| %-*s | %-*s | %-*s |\n", w_sid, "ID", w_sname, "Name", w_cap, "Capacity");

        printf("+"); for(int k=0; k<w_sid+2; k++) printf("-");
        printf("+"); for(int k=0; k<w_sname+2; k++) printf("-");
        printf("+"); for(int k=0; k<w_cap+2; k++) printf("-"); printf("+\n");

        for (int i = 0; i < studio_count; i++) {
            char t_cap[20]; sprintf(t_cap, "%d", all_studios[i].capacity);
            printf("| %-*d | %-*s | %-*s |\n", w_sid, all_studios[i].id, w_sname, all_studios[i].name, w_cap, t_cap);
        }

        printf("+"); for(int k=0; k<w_sid+2; k++) printf("-");
        printf("+"); for(int k=0; k<w_sname+2; k++) printf("-");
        printf("+"); for(int k=0; k<w_cap+2; k++) printf("-"); printf("+\n");

        printf("Enter Studio ID or Name (Type '0' to Cancel) : ");
        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = '\0';

        if (strcmp(input, "0") == 0) { schedule_manage(); return; }
        if (strlen(input) == 0) { printf("[ERROR] Input cannot be empty!\n"); system("pause"); continue; }

        for (int i = 0; input[i] != '\0'; i++) lower_input[i] = tolower(input[i]);
        lower_input[strlen(input)] = '\0';

        int is_num = 1;
        for (int i = 0; input[i] != '\0'; i++) if (!isdigit(input[i])) { is_num = 0; break; }

        Studio matched[100];
        int match_count = 0;

        if (is_num) {
            int input_id = atoi(input);
            for (int i = 0; i < studio_count; i++) {
                if (all_studios[i].id == input_id) {
                    matched[match_count++] = all_studios[i];
                    break;
                }
            }
        } else {
            for (int i = 0; i < studio_count; i++) {
                char lower_name[100]; strcpy(lower_name, all_studios[i].name);
                for (int j = 0; lower_name[j] != '\0'; j++) lower_name[j] = tolower(lower_name[j]);
                if (strstr(lower_name, lower_input) != NULL) matched[match_count++] = all_studios[i];
            }
        }

        if (match_count == 0) {
            printf("[ERROR] Studio \"%s\" not found!\n", input);
            system("pause");
        } else if (match_count == 1) {
            studio_id = matched[0].id;
            selected_studio = matched[0];
        } else {
            int resolved = 0;
            do {
                system("cls");
                printf("============================================\n");
                printf("       SELECT STUDIO - MULTIPLE MATCHES     \n");
                printf("============================================\n");
                for (int i = 0; i < match_count; i++) printf("ID: %-4d | Name: %s\n", matched[i].id, matched[i].name);
                printf("--------------------------------------------\n");
                printf("Enter EXACT ID (or '0' to go back) : ");
                
                char id_input[100];
                fgets(id_input, sizeof(id_input), stdin);
                id_input[strcspn(id_input, "\n")] = '\0';

                if (strcmp(id_input, "0") == 0) { schedule_manage(); return; }
                if (strlen(id_input) == 0) { printf("[ERROR] Empty input!\n"); system("pause"); continue; }

                int id_is_num = 1;
                for (int i = 0; id_input[i] != '\0'; i++) if (!isdigit(id_input[i])) id_is_num = 0;
                if (!id_is_num) { printf("[ERROR] Enter a numeric ID.\n"); system("pause"); continue; }

                int selected_id = atoi(id_input);
                for (int i = 0; i < match_count; i++) {
                    if (matched[i].id == selected_id) {
                        studio_id = selected_id;
                        selected_studio = matched[i];
                        resolved = 1;
                        break;
                    }
                }
                if (!resolved) { printf("[ERROR] ID %d not in list!\n", selected_id); system("pause"); }
            } while (!resolved);
        }
    } while (studio_id == -1);


    /* ==========================================
       STEP 3: TANGGAL & JAM (Dengan Validasi Fgets)
       ========================================== */
    Schedule sch;
    sch.film_id = film_id;
    sch.studio_id = studio_id;

    do {
        system("cls");
        printf("============================================\n");
        printf("            ADD SCHEDULE - DATE             \n");
        printf("============================================\n");
        printf("  Film   : %s\n", selected_film->title);
        printf("  Studio : %s\n", selected_studio.name);
        printf("--------------------------------------------\n");
        printf("Date (YYYY-MM-DD) ['0' to cancel] : ");
        
        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = '\0';

        if (strcmp(input, "0") == 0) { schedule_manage(); return; }
        
        if (!validate_date(input)) {
            printf("[ERROR] Invalid date! Format must be YYYY-MM-DD.\n");
            system("pause");
        } else {
            strcpy(sch.date, input);
            break;
        }
    } while (1);

    int conflict_id;
    do {
        system("cls");
        printf("============================================\n");
        printf("            ADD SCHEDULE - TIME             \n");
        printf("============================================\n");
        printf("  Film   : %s\n", selected_film->title);
        printf("  Studio : %s\n", selected_studio.name);
        printf("  Date   : %s\n", sch.date);
        printf("--------------------------------------------\n");
        printf("Time (HH:MM) ['0' to cancel] : ");
        
        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = '\0';

        if (strcmp(input, "0") == 0) { schedule_manage(); return; }

        if (!validate_time(input)) {
            printf("[ERROR] Invalid time! Format must be HH:MM (00:00-23:59).\n");
            system("pause");
            continue;
        }

        conflict_id = check_schedule_conflict(sch.studio_id, sch.date, input, selected_film->duration, -1);

        if (conflict_id != -1) {
            Schedule conflict_sch;
            find_schedule(conflict_id, &conflict_sch);
            Film* cf = btree_search(film_tree, conflict_sch.film_id);
            char end_time_str[10];
            int end_min = time_to_minutes(conflict_sch.time) + (cf ? cf->duration : 0);
            minutes_to_time_str(end_min, end_time_str);
            
            printf("\n[ERROR] SCHEDULE CONFLICT DETECTED!\n");
            printf("Studio is occupied by: [ID %d] %s (%s - %s)\n", conflict_id, cf ? cf->title : "?", conflict_sch.time, end_time_str);
            printf("Please enter a different time.\n");
            system("pause");
        } else {
            strcpy(sch.time, input);
        }
    } while (conflict_id != -1);


    /* ==========================================
       STEP 4: HARGA
       ========================================== */
    do {
        system("cls");
        printf("============================================\n");
        printf("            ADD SCHEDULE - PRICE            \n");
        printf("============================================\n");
        printf("  Film   : %s\n", selected_film->title);
        printf("  Studio : %s\n", selected_studio.name);
        printf("  Date   : %s\n", sch.date);
        printf("  Time   : %s\n", sch.time);
        printf("--------------------------------------------\n");
        printf("Price (Rp) ['0' to cancel] : ");
        
        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = '\0';

        if (strcmp(input, "0") == 0) { schedule_manage(); return; }

        int is_float = 1;
        for (int i = 0; input[i] != '\0'; i++) {
            if (!isdigit(input[i]) && input[i] != '.') { is_float = 0; break; }
        }

        if (is_float && strlen(input) > 0) {
            sch.price = atof(input);
            if (sch.price <= 0) {
                printf("[ERROR] Price must be greater than 0!\n");
                system("pause");
            } else {
                break; // Harga valid
            }
        } else {
            printf("[ERROR] Price must be a valid number!\n");
            system("pause");
        }
    } while (1);


    /* ==========================================
       STEP 5: SIMPAN DATA
       ========================================== */
    sch.id = auto_id_schedule();

    // Anti-Inline Append Fix
    FILE* check_newline = fopen(schedule_file, "r");
    int needs_newline = 0;
    if (check_newline != NULL) {
        fseek(check_newline, -1, SEEK_END);
        int last_char = fgetc(check_newline);
        if (last_char != '\n' && last_char != EOF) needs_newline = 1;
        fclose(check_newline);
    }

    FILE* fp = fopen(schedule_file, "a");
    if (fp == NULL) { invalid_file(); return; }
    
    if (needs_newline) fprintf(fp, "\n");
    fprintf(fp, "%d=%d=%d=%s=%s=%.0f\n", sch.id, sch.film_id, sch.studio_id, sch.date, sch.time, sch.price);
    fclose(fp);

    char end_time_str[10];
    minutes_to_time_str(time_to_minutes(sch.time) + selected_film->duration, end_time_str);

    system("cls");
    printf("============================================\n");
    printf("        Schedule added successfully!        \n");
    printf("--------------------------------------------\n");
    printf("  ID      : %d\n", sch.id);
    printf("  Film    : %s\n", selected_film->title);
    printf("  Studio  : %s\n", selected_studio.name);
    printf("  Date    : %s\n", sch.date);
    printf("  Time    : %s - %s\n", sch.time, end_time_str);
    printf("  Price   : Rp %.0f\n", sch.price);
    printf("============================================\n");
    system("pause");
    schedule_manage();
}

// ============================================================
// !! EDIT SCHEDULE !!
// ============================================================
void edit_schedule() {
    Schedule schedules[500];
    int count = 0;

    // ==========================================
    // TAHAP 1: BACA DATA KE DALAM ARRAY
    // ==========================================
    FILE* fp = fopen(schedule_file, "r");
    if (fp != NULL) {
        char buffer[300];
        while (fgets(buffer, sizeof(buffer), fp)) {
            buffer[strcspn(buffer, "\n")] = 0;
            if (strlen(buffer) == 0) continue;

            if (sscanf(buffer, "%d=%d=%d=%[^=]=%[^=]=%f",
                       &schedules[count].id, &schedules[count].film_id, &schedules[count].studio_id,
                       schedules[count].date, schedules[count].time, &schedules[count].price) == 6) {
                count++;
            }
        }
        fclose(fp);
    }

    if (count == 0) {
        system("cls");
        printf("============================================\n");
        printf("               EDIT SCHEDULE                \n");
        printf("============================================\n");
        printf("No schedules available.\n");
        system("pause");
        schedule_manage();
        return;
    }

    // ==========================================
    // HITUNG LEBAR KOLOM DINAMIS
    // ==========================================
    int w_id = 2, w_film = 4, w_studio = 6, w_date = 4, w_start = 5, w_end = 3, w_price = 5;

    for (int i = 0; i < count; i++) {
        char t_id[20], t_price[30], t_end[20];
        sprintf(t_id, "%d", schedules[i].id);
        sprintf(t_price, "Rp %.0f", schedules[i].price);

        Film* f = btree_search(film_tree, schedules[i].film_id);
        char film_title[100] = "Unknown";
        if (f != NULL) {
            strcpy(film_title, f->title);
            minutes_to_time_str(time_to_minutes(schedules[i].time) + f->duration, t_end);
        } else {
            strcpy(t_end, "?");
        }

        Studio st;
        char st_name[50] = "Unknown";
        if (find_studio(schedules[i].studio_id, &st)) strcpy(st_name, st.name);

        if ((int)strlen(t_id) > w_id) w_id = strlen(t_id);
        if ((int)strlen(film_title) > w_film) w_film = strlen(film_title);
        if ((int)strlen(st_name) > w_studio) w_studio = strlen(st_name);
        if ((int)strlen(schedules[i].date) > w_date) w_date = strlen(schedules[i].date);
        if ((int)strlen(schedules[i].time) > w_start) w_start = strlen(schedules[i].time);
        if ((int)strlen(t_end) > w_end) w_end = strlen(t_end);
        if ((int)strlen(t_price) > w_price) w_price = strlen(t_price);
    }

    #define PRINT_SCH_LINE() \
        do { \
            printf("+"); \
            for(int k=0; k<w_id+2; k++) printf("-"); printf("+"); \
            for(int k=0; k<w_film+2; k++) printf("-"); printf("+"); \
            for(int k=0; k<w_studio+2; k++) printf("-"); printf("+"); \
            for(int k=0; k<w_date+2; k++) printf("-"); printf("+"); \
            for(int k=0; k<w_start+2; k++) printf("-"); printf("+"); \
            for(int k=0; k<w_end+2; k++) printf("-"); printf("+"); \
            for(int k=0; k<w_price+2; k++) printf("-"); printf("+\n"); \
        } while(0)

    char input[100];
    char lower_input[100];
    int edit_id = -1;
    Schedule target_sch;
    int target_found = 0;

    // ==========================================
    // PAGE 1: PILIH JADWAL YANG INGIN DIEDIT
    // ==========================================
    do {
        system("cls");
        printf("=================================================================================================\n");
        printf("                                          EDIT SCHEDULE                                          \n");
        printf("=================================================================================================\n");
        
        PRINT_SCH_LINE();
        printf("| %-*s | %-*s | %-*s | %-*s | %-*s | %-*s | %-*s |\n",
               w_id, "ID", w_film, "Film", w_studio, "Studio", 
               w_date, "Date", w_start, "Start", w_end, "End", w_price, "Price");
        PRINT_SCH_LINE();

        for (int i = 0; i < count; i++) {
            char t_price[30], t_end[20];
            sprintf(t_price, "Rp %.0f", schedules[i].price);

            Film* f = btree_search(film_tree, schedules[i].film_id);
            char film_title[100] = "Unknown";
            if (f != NULL) {
                strcpy(film_title, f->title);
                minutes_to_time_str(time_to_minutes(schedules[i].time) + f->duration, t_end);
            } else {
                strcpy(t_end, "?");
            }

            Studio st;
            char st_name[50] = "Unknown";
            if (find_studio(schedules[i].studio_id, &st)) strcpy(st_name, st.name);

            printf("| %-*d | %-*s | %-*s | %-*s | %-*s | %-*s | %-*s |\n",
                   w_id, schedules[i].id, w_film, film_title, w_studio, st_name,
                   w_date, schedules[i].date, w_start, schedules[i].time, w_end, t_end, w_price, t_price);
        }
        PRINT_SCH_LINE();
        printf(" (Enter '0' to go back)\n");
        printf("-------------------------------------------------------------------------------------------------\n");
        
        printf("Enter Schedule ID to edit : ");
        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = '\0';

        if (strcmp(input, "0") == 0) { schedule_manage(); return; }

        if (strlen(input) == 0) {
            printf("[ERROR] Input cannot be empty!\n");
            system("pause");
            continue;
        }

        int is_num = 1;
        for (int i = 0; input[i] != '\0'; i++) {
            if (!isdigit(input[i])) { is_num = 0; break; }
        }

        if (!is_num) {
            printf("[ERROR] ID must be a number!\n");
            system("pause");
            continue;
        }

        edit_id = atoi(input);
        if (!find_schedule(edit_id, &target_sch)) {
            printf("[ERROR] Schedule with ID %d not found!\n", edit_id);
            system("pause");
            edit_id = -1;
        } else {
            target_found = 1;
        }
    } while (!target_found);


    // ==========================================
    // PAGE 2: EDIT STUDIO (Dynamic Search)
    // ==========================================
    Studio all_studios[100];
    int studio_count = 0;
    FILE* sfp = fopen(studio_file, "r");
    if (sfp != NULL) {
        char sbuf[300];
        while (fgets(sbuf, sizeof(sbuf), sfp)) {
            sbuf[strcspn(sbuf, "\n")] = 0;
            if (strlen(sbuf) == 0) continue;
            if (sscanf(sbuf, "%d=%[^=]=%d=%d=%d", &all_studios[studio_count].id, all_studios[studio_count].name,
                       &all_studios[studio_count].capacity, &all_studios[studio_count].rows, &all_studios[studio_count].cols) == 5) {
                studio_count++;
            }
        }
        fclose(sfp);
    }

    int w_sid = 2, w_sname = 4, w_cap = 8;
    for (int i = 0; i < studio_count; i++) {
        char t_id[20], t_cap[20];
        sprintf(t_id, "%d", all_studios[i].id);
        sprintf(t_cap, "%d", all_studios[i].capacity);
        if ((int)strlen(t_id) > w_sid) w_sid = strlen(t_id);
        if ((int)strlen(all_studios[i].name) > w_sname) w_sname = strlen(all_studios[i].name);
        if ((int)strlen(t_cap) > w_cap) w_cap = strlen(t_cap);
    }

    int studio_done = 0;
    do {
        system("cls");
        printf("=================================================================\n");
        printf("                 EDIT SCHEDULE ID: %d (STUDIO)                   \n", edit_id);
        printf("=================================================================\n");
        
        printf("+"); for(int k=0; k<w_sid+2; k++) printf("-");
        printf("+"); for(int k=0; k<w_sname+2; k++) printf("-");
        printf("+"); for(int k=0; k<w_cap+2; k++) printf("-"); printf("+\n");

        printf("| %-*s | %-*s | %-*s |\n", w_sid, "ID", w_sname, "Name", w_cap, "Capacity");

        printf("+"); for(int k=0; k<w_sid+2; k++) printf("-");
        printf("+"); for(int k=0; k<w_sname+2; k++) printf("-");
        printf("+"); for(int k=0; k<w_cap+2; k++) printf("-"); printf("+\n");

        for (int i = 0; i < studio_count; i++) {
            char t_cap[20]; sprintf(t_cap, "%d", all_studios[i].capacity);
            printf("| %-*d | %-*s | %-*s |\n", w_sid, all_studios[i].id, w_sname, all_studios[i].name, w_cap, t_cap);
        }
        printf("+"); for(int k=0; k<w_sid+2; k++) printf("-");
        printf("+"); for(int k=0; k<w_sname+2; k++) printf("-");
        printf("+"); for(int k=0; k<w_cap+2; k++) printf("-"); printf("+\n");
        
        printf("Leave blank (Press Enter) to keep current Studio ID [%d]\n", target_sch.studio_id);
        printf("(Enter '0' to Cancel & Go Back)\n");
        printf("-----------------------------------------------------------------\n");
        printf("Enter Studio ID or Name : ");
        
        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = '\0';

        if (strcmp(input, "0") == 0) { schedule_manage(); return; }
        
        // Skip jika blank
        if (strlen(input) == 0) {
            studio_done = 1;
            break;
        }

        for (int i = 0; input[i] != '\0'; i++) lower_input[i] = tolower(input[i]);
        lower_input[strlen(input)] = '\0';

        int is_num = 1;
        for (int i = 0; input[i] != '\0'; i++) if (!isdigit(input[i])) { is_num = 0; break; }

        Studio matched[100];
        int match_count = 0;

        if (is_num) {
            int input_id = atoi(input);
            for (int i = 0; i < studio_count; i++) {
                if (all_studios[i].id == input_id) { matched[match_count++] = all_studios[i]; break; }
            }
        } else {
            for (int i = 0; i < studio_count; i++) {
                char lower_name[100]; strcpy(lower_name, all_studios[i].name);
                for (int j = 0; lower_name[j] != '\0'; j++) lower_name[j] = tolower(lower_name[j]);
                if (strstr(lower_name, lower_input) != NULL) matched[match_count++] = all_studios[i];
            }
        }

        if (match_count == 0) {
            printf("[ERROR] Studio \"%s\" not found!\n", input);
            system("pause");
        } else if (match_count == 1) {
            target_sch.studio_id = matched[0].id;
            studio_done = 1;
        } else {
            int resolved = 0;
            do {
                system("cls");
                printf("============================================\n");
                printf("       SELECT STUDIO - MULTIPLE MATCHES     \n");
                printf("============================================\n");
                for (int i = 0; i < match_count; i++) printf("ID: %-4d | Name: %s\n", matched[i].id, matched[i].name);
                printf("--------------------------------------------\n");
                printf("Enter EXACT ID (or '0' to go back) : ");
                
                char id_input[100];
                fgets(id_input, sizeof(id_input), stdin);
                id_input[strcspn(id_input, "\n")] = '\0';

                if (strcmp(id_input, "0") == 0) { schedule_manage(); return; }
                if (strlen(id_input) == 0) { printf("[ERROR] Empty input!\n"); system("pause"); continue; }

                int id_is_num = 1;
                for (int i = 0; id_input[i] != '\0'; i++) if (!isdigit(id_input[i])) id_is_num = 0;
                if (!id_is_num) { printf("[ERROR] Enter a numeric ID.\n"); system("pause"); continue; }

                int selected_id = atoi(id_input);
                for (int i = 0; i < match_count; i++) {
                    if (matched[i].id == selected_id) {
                        target_sch.studio_id = selected_id;
                        resolved = 1; studio_done = 1; break;
                    }
                }
                if (!resolved) { printf("[ERROR] ID %d not in list!\n", selected_id); system("pause"); }
            } while (!resolved);
        }
    } while (!studio_done);


    // ==========================================
    // PAGE 3: EDIT FILM (Dynamic Search)
    // ==========================================
    Film all_films[200];
    int film_count = 0;
    btree_inorder(film_tree, all_films, &film_count);

    int w_fid = 2, w_ftitle = 5, w_fgenre = 5, w_fdur = 8;
    for (int i = 0; i < film_count; i++) {
        char temp_id[20], temp_dur[20];
        sprintf(temp_id, "%d", all_films[i].id);
        sprintf(temp_dur, "%d", all_films[i].duration);
        if ((int)strlen(temp_id) > w_fid) w_fid = strlen(temp_id);
        if ((int)strlen(all_films[i].title) > w_ftitle) w_ftitle = strlen(all_films[i].title);
        if ((int)strlen(all_films[i].genre) > w_fgenre) w_fgenre = strlen(all_films[i].genre);
        if ((int)strlen(temp_dur) > w_fdur) w_fdur = strlen(temp_dur);
    }

    int film_done = 0;
    do {
        system("cls");
        printf("===========================================================================\n");
        printf("                   EDIT SCHEDULE ID: %d (FILM)                             \n", edit_id);
        printf("===========================================================================\n");
        
        printf("+"); for(int k=0; k<w_fid+2; k++) printf("-");
        printf("+"); for(int k=0; k<w_ftitle+2; k++) printf("-");
        printf("+"); for(int k=0; k<w_fgenre+2; k++) printf("-");
        printf("+"); for(int k=0; k<w_fdur+2; k++) printf("-"); printf("+\n");

        printf("| %-*s | %-*s | %-*s | %-*s |\n", w_fid, "ID", w_ftitle, "Title", w_fgenre, "Genre", w_fdur, "Duration");

        printf("+"); for(int k=0; k<w_fid+2; k++) printf("-");
        printf("+"); for(int k=0; k<w_ftitle+2; k++) printf("-");
        printf("+"); for(int k=0; k<w_fgenre+2; k++) printf("-");
        printf("+"); for(int k=0; k<w_fdur+2; k++) printf("-"); printf("+\n");

        for (int i = 0; i < film_count; i++) {
            char temp_dur[20]; sprintf(temp_dur, "%d", all_films[i].duration);
            printf("| %-*d | %-*s | %-*s | %-*s |\n", w_fid, all_films[i].id, w_ftitle, all_films[i].title, w_fgenre, all_films[i].genre, w_fdur, temp_dur);
        }
        printf("+"); for(int k=0; k<w_fid+2; k++) printf("-");
        printf("+"); for(int k=0; k<w_ftitle+2; k++) printf("-");
        printf("+"); for(int k=0; k<w_fgenre+2; k++) printf("-");
        printf("+"); for(int k=0; k<w_fdur+2; k++) printf("-"); printf("+\n");
        
        printf("Leave blank (Press Enter) to keep current Film ID [%d]\n", target_sch.film_id);
        printf("(Enter '0' to Cancel & Go Back)\n");
        printf("---------------------------------------------------------------------------\n");
        printf("Enter Film ID or Title : ");
        
        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = '\0';

        if (strcmp(input, "0") == 0) { schedule_manage(); return; }
        
        if (strlen(input) == 0) { film_done = 1; break; } // Keep current

        for (int i = 0; input[i] != '\0'; i++) lower_input[i] = tolower(input[i]);
        lower_input[strlen(input)] = '\0';

        int is_num = 1;
        for (int i = 0; input[i] != '\0'; i++) if (!isdigit(input[i])) { is_num = 0; break; }

        Film matched[200];
        int match_count = 0;

        if (is_num) {
            int input_id = atoi(input);
            for (int i = 0; i < film_count; i++) {
                if (all_films[i].id == input_id) { matched[match_count++] = all_films[i]; break; }
            }
        } else {
            for (int i = 0; i < film_count; i++) {
                char lower_title[100]; strcpy(lower_title, all_films[i].title);
                for (int j = 0; lower_title[j] != '\0'; j++) lower_title[j] = tolower(lower_title[j]);
                if (strstr(lower_title, lower_input) != NULL) matched[match_count++] = all_films[i];
            }
        }

        if (match_count == 0) {
            printf("[ERROR] Film \"%s\" not found!\n", input);
            system("pause");
        } else if (match_count == 1) {
            target_sch.film_id = matched[0].id;
            film_done = 1;
        } else {
            int resolved = 0;
            do {
                system("cls");
                printf("============================================\n");
                printf("        SELECT FILM - MULTIPLE MATCHES      \n");
                printf("============================================\n");
                for (int i = 0; i < match_count; i++) printf("ID: %-4d | Title: %s\n", matched[i].id, matched[i].title);
                printf("--------------------------------------------\n");
                printf("Enter EXACT ID (or '0' to go back) : ");
                
                char id_input[100];
                fgets(id_input, sizeof(id_input), stdin);
                id_input[strcspn(id_input, "\n")] = '\0';

                if (strcmp(id_input, "0") == 0) { schedule_manage(); return; }
                if (strlen(id_input) == 0) { printf("[ERROR] Empty input!\n"); system("pause"); continue; }

                int id_is_num = 1;
                for (int i = 0; id_input[i] != '\0'; i++) if (!isdigit(id_input[i])) id_is_num = 0;
                if (!id_is_num) { printf("[ERROR] Enter a numeric ID.\n"); system("pause"); continue; }

                int selected_id = atoi(id_input);
                for (int i = 0; i < match_count; i++) {
                    if (matched[i].id == selected_id) {
                        target_sch.film_id = selected_id;
                        resolved = 1; film_done = 1; break;
                    }
                }
                if (!resolved) { printf("[ERROR] ID %d not in list!\n", selected_id); system("pause"); }
            } while (!resolved);
        }
    } while (!film_done);

    Film* selected_film = btree_search(film_tree, target_sch.film_id);


    // ==========================================
    // PAGE 4: EDIT DATE & TIME
    // ==========================================
    int date_done = 0;
    do {
        system("cls");
        printf("============================================\n");
        printf("        EDIT SCHEDULE ID: %d (DATE)         \n", edit_id);
        printf("============================================\n");
        printf("Leave blank (Press Enter) to keep current Date [%s]\n", target_sch.date);
        printf("(Enter '0' to Cancel & Go Back)\n");
        printf("--------------------------------------------\n");
        printf("Enter Date (YYYY-MM-DD) : ");

        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = '\0';

        if (strcmp(input, "0") == 0) { schedule_manage(); return; }

        if (strlen(input) == 0) { 
            date_done = 1; break; // Keep current
        }

        if (validate_date(input)) {
            strcpy(target_sch.date, input);
            date_done = 1;
        } else {
            printf("[ERROR] Invalid date format! Must be YYYY-MM-DD.\n");
            system("pause");
        }
    } while (!date_done);


    int conflict_id;
    char temp_time[20];
    strcpy(temp_time, target_sch.time); // Backup time awal
    do {
        system("cls");
        printf("============================================\n");
        printf("        EDIT SCHEDULE ID: %d (TIME)         \n", edit_id);
        printf("============================================\n");
        printf("Leave blank (Press Enter) to keep current Time [%s]\n", temp_time);
        printf("(Enter '0' to Cancel & Go Back)\n");
        printf("--------------------------------------------\n");
        printf("Enter Time (HH:MM) : ");

        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = '\0';

        if (strcmp(input, "0") == 0) { schedule_manage(); return; }

        if (strlen(input) > 0) {
            if (!validate_time(input)) {
                printf("[ERROR] Invalid time format (HH:MM).\n");
                system("pause");
                conflict_id = 0; // Trigger ulangan loop
                continue;
            }
            strcpy(temp_time, input);
        }

        conflict_id = check_schedule_conflict(
            target_sch.studio_id, target_sch.date, temp_time,
            selected_film->duration, edit_id); // Skip edit_id ini sendiri

        if (conflict_id != -1) {
            Schedule conflict_sch;
            find_schedule(conflict_id, &conflict_sch);
            Film* cf = btree_search(film_tree, conflict_sch.film_id);
            char end_time_str[10];
            int end_min = time_to_minutes(conflict_sch.time) + (cf ? cf->duration : 0);
            minutes_to_time_str(end_min, end_time_str);
            
            printf("\n[ERROR] SCHEDULE CONFLICT DETECTED!\n");
            printf("Studio is occupied by: [ID %d] %s (%s - %s)\n", conflict_id, cf ? cf->title : "?", conflict_sch.time, end_time_str);
            printf("Please enter a different time.\n");
            system("pause");
            strcpy(temp_time, target_sch.time); // Reset ke original kalau error
        } else {
            strcpy(target_sch.time, temp_time);
        }
    } while (conflict_id != -1);


    // ==========================================
    // PAGE 5: EDIT PRICE
    // ==========================================
    int price_done = 0;
    do {
        system("cls");
        printf("============================================\n");
        printf("        EDIT SCHEDULE ID: %d (PRICE)        \n", edit_id);
        printf("============================================\n");
        printf("Leave blank (Press Enter) to keep current Price [Rp %.0f]\n", target_sch.price);
        printf("(Enter '0' to Cancel & Go Back)\n");
        printf("--------------------------------------------\n");
        printf("Enter Price (Rp) : ");

        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = '\0';

        if (strcmp(input, "0") == 0) { schedule_manage(); return; }

        if (strlen(input) == 0) { price_done = 1; break; }

        int is_float = 1;
        for (int i = 0; input[i] != '\0'; i++) {
            if (!isdigit(input[i]) && input[i] != '.') { is_float = 0; break; }
        }

        if (is_float) {
            float np = atof(input);
            if (np > 0) {
                target_sch.price = np;
                price_done = 1;
            } else {
                printf("[ERROR] Price must be greater than 0!\n");
                system("pause");
            }
        } else {
            printf("[ERROR] Price must be a valid number!\n");
            system("pause");
        }
    } while (!price_done);


    // ==========================================
    // TAHAP 6: SIMPAN PERUBAHAN
    // ==========================================
    FILE* in  = fopen(schedule_file, "r");
    FILE* tmp = fopen("temp_schedule.txt", "w");
    if (in == NULL || tmp == NULL) { invalid_file(); return; }
    
    char buf[300];
    while (fgets(buf, sizeof(buf), in)) {
        buf[strcspn(buf, "\n")] = 0;
        int id; sscanf(buf, "%d=", &id);
        if (id == edit_id)
            fprintf(tmp, "%d=%d=%d=%s=%s=%.0f\n",
                    target_sch.id, target_sch.film_id, target_sch.studio_id,
                    target_sch.date, target_sch.time, target_sch.price);
        else
            fprintf(tmp, "%s\n", buf);
    }
    fclose(in); fclose(tmp);
    remove(schedule_file);
    rename("temp_schedule.txt", schedule_file);


    // ==========================================
    // PAGE 7: TAMPILAN INVOICE / SUCCESS DETAIL
    // ==========================================
    Film* final_film = btree_search(film_tree, target_sch.film_id);
    char final_film_title[100] = "Unknown";
    int final_duration = 0;
    if (final_film != NULL) {
        strcpy(final_film_title, final_film->title);
        final_duration = final_film->duration;
    }

    Studio final_st;
    char final_studio_name[50] = "Unknown";
    if (find_studio(target_sch.studio_id, &final_st)) {
        strcpy(final_studio_name, final_st.name);
    }

    char final_end_time[20] = "?";
    if (final_film != NULL) {
        minutes_to_time_str(time_to_minutes(target_sch.time) + final_duration, final_end_time);
    }

    system("cls");
    printf("============================================\n");
    printf("        Schedule ID %d updated!             \n", edit_id);
    printf("--------------------------------------------\n");
    printf("  Detail Schedule:\n");
    printf("  ID       : %d\n", target_sch.id);
    printf("  Film     : %s\n", final_film_title);
    printf("  Studio   : %s\n", final_studio_name);
    printf("  Date     : %s\n", target_sch.date);
    printf("  Time     : %s - %s\n", target_sch.time, final_end_time);
    printf("  Price    : Rp %.0f\n", target_sch.price);
    printf("============================================\n");
    system("pause");
    schedule_manage();
}

// ============================================================
// !! DELETE SCHEDULE !!
// ============================================================
void del_schedule() {
    Schedule schedules[500];
    int count = 0;

    // ==========================================
    // TAHAP 1: BACA DATA KE DALAM ARRAY
    // ==========================================
    FILE* fp = fopen(schedule_file, "r");
    if (fp != NULL) {
        char buffer[300];
        while (fgets(buffer, sizeof(buffer), fp)) {
            buffer[strcspn(buffer, "\n")] = 0;
            if (strlen(buffer) == 0) continue;

            if (sscanf(buffer, "%d=%d=%d=%[^=]=%[^=]=%f",
                       &schedules[count].id, &schedules[count].film_id, &schedules[count].studio_id,
                       schedules[count].date, schedules[count].time, &schedules[count].price) == 6) {
                count++;
            }
        }
        fclose(fp);
    }

    if (count == 0) {
        system("cls");
        printf("============================================\n");
        printf("              DELETE SCHEDULE               \n");
        printf("============================================\n");
        printf("No schedules available.\n");
        system("pause");
        schedule_manage();
        return;
    }

    // ==========================================
    // HITUNG LEBAR KOLOM DINAMIS
    // ==========================================
    int w_id = 2, w_film = 4, w_studio = 6, w_date = 4, w_start = 5, w_end = 3, w_price = 5;

    for (int i = 0; i < count; i++) {
        char t_id[20], t_price[30], t_end[20];
        sprintf(t_id, "%d", schedules[i].id);
        sprintf(t_price, "Rp %.0f", schedules[i].price);

        Film* f = btree_search(film_tree, schedules[i].film_id);
        char film_title[100] = "Unknown";
        if (f != NULL) {
            strcpy(film_title, f->title);
            minutes_to_time_str(time_to_minutes(schedules[i].time) + f->duration, t_end);
        } else {
            strcpy(t_end, "?");
        }

        Studio st;
        char st_name[50] = "Unknown";
        if (find_studio(schedules[i].studio_id, &st)) strcpy(st_name, st.name);

        if ((int)strlen(t_id) > w_id) w_id = strlen(t_id);
        if ((int)strlen(film_title) > w_film) w_film = strlen(film_title);
        if ((int)strlen(st_name) > w_studio) w_studio = strlen(st_name);
        if ((int)strlen(schedules[i].date) > w_date) w_date = strlen(schedules[i].date);
        if ((int)strlen(schedules[i].time) > w_start) w_start = strlen(schedules[i].time);
        if ((int)strlen(t_end) > w_end) w_end = strlen(t_end);
        if ((int)strlen(t_price) > w_price) w_price = strlen(t_price);
    }

    #define PRINT_SCH_LINE() \
        do { \
            printf("+"); \
            for(int k=0; k<w_id+2; k++) printf("-"); printf("+"); \
            for(int k=0; k<w_film+2; k++) printf("-"); printf("+"); \
            for(int k=0; k<w_studio+2; k++) printf("-"); printf("+"); \
            for(int k=0; k<w_date+2; k++) printf("-"); printf("+"); \
            for(int k=0; k<w_start+2; k++) printf("-"); printf("+"); \
            for(int k=0; k<w_end+2; k++) printf("-"); printf("+"); \
            for(int k=0; k<w_price+2; k++) printf("-"); printf("+\n"); \
        } while(0)

    char input[100];
    int target_id = -1;
    Schedule target_sch;
    int target_found = 0;

    // ==========================================
    // PAGE 1: PILIH JADWAL YANG INGIN DIHAPUS
    // ==========================================
    do {
        system("cls");
        printf("=================================================================================================\n");
        printf("                                         DELETE SCHEDULE                                         \n");
        printf("=================================================================================================\n");
        
        PRINT_SCH_LINE();
        printf("| %-*s | %-*s | %-*s | %-*s | %-*s | %-*s | %-*s |\n",
               w_id, "ID", w_film, "Film", w_studio, "Studio", 
               w_date, "Date", w_start, "Start", w_end, "End", w_price, "Price");
        PRINT_SCH_LINE();

        for (int i = 0; i < count; i++) {
            char t_price[30], t_end[20];
            sprintf(t_price, "Rp %.0f", schedules[i].price);

            Film* f = btree_search(film_tree, schedules[i].film_id);
            char film_title[100] = "Unknown";
            if (f != NULL) {
                strcpy(film_title, f->title);
                minutes_to_time_str(time_to_minutes(schedules[i].time) + f->duration, t_end);
            } else {
                strcpy(t_end, "?");
            }

            Studio st;
            char st_name[50] = "Unknown";
            if (find_studio(schedules[i].studio_id, &st)) strcpy(st_name, st.name);

            printf("| %-*d | %-*s | %-*s | %-*s | %-*s | %-*s | %-*s |\n",
                   w_id, schedules[i].id, w_film, film_title, w_studio, st_name,
                   w_date, schedules[i].date, w_start, schedules[i].time, w_end, t_end, w_price, t_price);
        }
        PRINT_SCH_LINE();
        printf(" (Enter '0' to go back)\n");
        printf("-------------------------------------------------------------------------------------------------\n");
        
        printf("Enter Schedule ID to delete : ");
        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = '\0';

        if (strcmp(input, "0") == 0) { schedule_manage(); return; }

        if (strlen(input) == 0) {
            printf("[ERROR] Input cannot be empty!\n");
            system("pause");
            continue;
        }

        int is_num = 1;
        for (int i = 0; input[i] != '\0'; i++) {
            if (!isdigit(input[i])) { is_num = 0; break; }
        }

        if (!is_num) {
            printf("[ERROR] ID must be a number!\n");
            system("pause");
            continue;
        }

        target_id = atoi(input);
        if (!find_schedule(target_id, &target_sch)) {
            printf("[ERROR] Schedule with ID %d not found!\n", target_id);
            system("pause");
            target_id = -1;
        } else {
            target_found = 1;
        }
    } while (!target_found);


    // ==========================================
    // PERSIAPKAN DATA DETAIL UNTUK DITAMPILKAN
    // ==========================================
    Film* del_film_ptr = btree_search(film_tree, target_sch.film_id);
    char del_film_title[100] = "Unknown";
    int del_film_dur = 0;
    if (del_film_ptr != NULL) {
        strcpy(del_film_title, del_film_ptr->title);
        del_film_dur = del_film_ptr->duration;
    }

    Studio del_st;
    char del_studio_name[50] = "Unknown";
    if (find_studio(target_sch.studio_id, &del_st)) {
        strcpy(del_studio_name, del_st.name);
    }

    char del_end_time[20] = "?";
    if (del_film_ptr != NULL) {
        minutes_to_time_str(time_to_minutes(target_sch.time) + del_film_dur, del_end_time);
    }


    // ==========================================
    // PAGE 2 : KONFIRMASI Y/N
    // ==========================================
    char confirm_str[10];
    char confirm_char;
    int conf_valid = 0;

    do {
        system("cls");
        printf("============================================\n");
        printf("            CONFIRM DELETION                \n");
        printf("============================================\n");
        printf("  Detail Schedule:\n");
        printf("  ID       : %d\n", target_sch.id);
        printf("  Film     : %s\n", del_film_title);
        printf("  Studio   : %s\n", del_studio_name);
        printf("  Date     : %s\n", target_sch.date);
        printf("  Time     : %s - %s\n", target_sch.time, del_end_time);
        printf("  Price    : Rp %.0f\n", target_sch.price);
        printf("--------------------------------------------\n");
        printf("Are you sure you want to delete this schedule?\n");
        printf("Type (Y/N) or '0' to go back : ");

        fgets(confirm_str, sizeof(confirm_str), stdin);
        confirm_str[strcspn(confirm_str, "\n")] = '\0';

        if (strcmp(confirm_str, "0") == 0) { schedule_manage(); return; }

        if (strlen(confirm_str) == 0) {
            printf("[ERROR] Input cannot be empty! Enter Y or N.\n");
            system("pause");
            continue;
        }

        if (strcmp(confirm_str, "Y") == 0 || strcmp(confirm_str, "y") == 0) {
            confirm_char = 'y';
            conf_valid = 1;
        } else if (strcmp(confirm_str, "N") == 0 || strcmp(confirm_str, "n") == 0) {
            confirm_char = 'n';
            conf_valid = 1;
        } else {
            printf("[ERROR] Invalid input! Enter Y or N.\n");
            system("pause");
        }
    } while (!conf_valid);

    if (confirm_char == 'n') {
        printf("Deletion cancelled.\n");
        system("pause");
        schedule_manage();
        return;
    }


    // ==========================================
    // PAGE 3 : KONFIRMASI PASSWORD ADMIN
    // ==========================================
    char input_pass[100];
    char confirm_pass_input[100]; 
    int pass_match = 0;

    do {
        system("cls");
        printf("============================================\n");
        printf("         ADMIN PASSWORD CONFIRMATION        \n");
        printf("============================================\n");
        printf(" [WARNING] This action cannot be undone!    \n");
        printf("--------------------------------------------\n");
        printf(" (Enter '0' at any prompt to go back)\n");
        printf("--------------------------------------------\n");

        printf("Enter Admin password : ");
        fgets(input_pass, sizeof(input_pass), stdin);
        input_pass[strcspn(input_pass, "\n")] = '\0';

        if (strcmp(input_pass, "0") == 0) { schedule_manage(); return; }

        if (strlen(input_pass) == 0) {
            printf("[ERROR] Password cannot be empty!\n");
            system("pause");
            continue;
        }

        printf("Confirm Admin password : ");
        fgets(confirm_pass_input, sizeof(confirm_pass_input), stdin);
        confirm_pass_input[strcspn(confirm_pass_input, "\n")] = '\0';

        if (strcmp(confirm_pass_input, "0") == 0) { schedule_manage(); return; }

        if (strcmp(input_pass, confirm_pass_input) != 0) {
            printf("\n[ERROR] Passwords do not match! Please try again.\n");
            system("pause");
            continue;
        }

        // Hardcoded admin password validation
        if (strcmp(input_pass, "admin123@") != 0) {
            printf("\n[ERROR] Incorrect password! Please try again.\n");
            system("pause");
            continue;
        }

        pass_match = 1; 
    } while (!pass_match);


    // ==========================================
    // PAGE 4 : EKSEKUSI PENGHAPUSAN FILE
    // ==========================================
    FILE* in  = fopen(schedule_file, "r");
    FILE* tmp = fopen("temp_schedule.txt", "w");
    if (in == NULL || tmp == NULL) { invalid_file(); return; }

    char sbuf[300];
    while (fgets(sbuf, sizeof(sbuf), in)) {
        sbuf[strcspn(sbuf, "\n")] = 0;
        if(strlen(sbuf) == 0) continue;
        
        int sid; 
        sscanf(sbuf, "%d=", &sid);
        if (sid != target_id) {
            fprintf(tmp, "%s\n", sbuf);
        }
    }
    fclose(in); 
    fclose(tmp);
    remove(schedule_file);
    rename("temp_schedule.txt", schedule_file);


    // ==========================================
    // PAGE 5 : SUCCESS INVOICE
    // ==========================================
    system("cls");
    printf("============================================\n");
    printf("              SCHEDULE DELETED              \n");
    printf("============================================\n");
    printf("\n");
    printf("   Schedule ID %d (%s) \n", target_id, del_film_title);
    printf("       deleted successfully!                \n");
    printf("\n");
    printf("============================================\n");
    system("pause");
    schedule_manage();
}

// ============================================================
// !! VIEW ALL SCHEDULES !!
// ============================================================
void view_schedule() {
    system("cls");

    Schedule schedules[500];
    int count = 0;

    // ==========================================
    // TAHAP 1: BACA DATA KE DALAM ARRAY
    // ==========================================
    FILE* fp = fopen(schedule_file, "r");
    if (fp != NULL) {
        char buffer[300];
        while (fgets(buffer, sizeof(buffer), fp)) {
            buffer[strcspn(buffer, "\n")] = 0;
            if (strlen(buffer) == 0) continue;

            if (sscanf(buffer, "%d=%d=%d=%[^=]=%[^=]=%f",
                       &schedules[count].id, &schedules[count].film_id, &schedules[count].studio_id,
                       schedules[count].date, schedules[count].time, &schedules[count].price) == 6) {
                count++;
            }
        }
        fclose(fp);
    }

    // ==========================================
    // TAHAP 2: HITUNG LEBAR KOLOM DINAMIS
    // ==========================================
    int w_id = 2;       // "ID"
    int w_film = 4;     // "Film"
    int w_studio = 6;   // "Studio"
    int w_date = 4;     // "Date"
    int w_start = 5;    // "Start"
    int w_end = 3;      // "End"
    int w_price = 5;    // "Price"

    for (int i = 0; i < count; i++) {
        char t_id[20], t_price[30], t_end[20];
        sprintf(t_id, "%d", schedules[i].id);
        sprintf(t_price, "Rp %.0f", schedules[i].price);

        Film* f = btree_search(film_tree, schedules[i].film_id);
        char film_title[100] = "Unknown";
        if (f != NULL) {
            strcpy(film_title, f->title);
            minutes_to_time_str(time_to_minutes(schedules[i].time) + f->duration, t_end);
        } else {
            strcpy(t_end, "?");
        }

        Studio st;
        char st_name[50] = "Unknown";
        if (find_studio(schedules[i].studio_id, &st)) {
            strcpy(st_name, st.name);
        }

        if ((int)strlen(t_id) > w_id) w_id = strlen(t_id);
        if ((int)strlen(film_title) > w_film) w_film = strlen(film_title);
        if ((int)strlen(st_name) > w_studio) w_studio = strlen(st_name);
        if ((int)strlen(schedules[i].date) > w_date) w_date = strlen(schedules[i].date);
        if ((int)strlen(schedules[i].time) > w_start) w_start = strlen(schedules[i].time);
        if ((int)strlen(t_end) > w_end) w_end = strlen(t_end);
        if ((int)strlen(t_price) > w_price) w_price = strlen(t_price);
    }

    // Hitung total lebar tabel (7 kolom)
    int total_width = w_id + w_film + w_studio + w_date + w_start + w_end + w_price + 22;

    #define PRINT_SCH_LINE() \
        do { \
            printf("+"); \
            for(int k=0; k<w_id+2; k++) printf("-"); printf("+"); \
            for(int k=0; k<w_film+2; k++) printf("-"); printf("+"); \
            for(int k=0; k<w_studio+2; k++) printf("-"); printf("+"); \
            for(int k=0; k<w_date+2; k++) printf("-"); printf("+"); \
            for(int k=0; k<w_start+2; k++) printf("-"); printf("+"); \
            for(int k=0; k<w_end+2; k++) printf("-"); printf("+"); \
            for(int k=0; k<w_price+2; k++) printf("-"); printf("+\n"); \
        } while(0)

    // ==========================================
    // TAHAP 3: CETAK TABEL
    // ==========================================
    printf("\n");
    for(int i = 0; i < total_width; i++) printf("="); printf("\n");
    
    // Teks tengah (Center alignment)
    int padding = (total_width - 13) / 2; // "ALL SCHEDULES" = 13 char
    if (padding < 0) padding = 0;
    for(int i = 0; i < padding; i++) printf(" ");
    printf("ALL SCHEDULES\n");
    
    for(int i = 0; i < total_width; i++) printf("="); printf("\n");

    if (count == 0) {
        printf("| %-*s |\n", total_width - 4, "No schedules available at the moment.");
        for(int i = 0; i < total_width; i++) printf("="); printf("\n");
    } else {
        PRINT_SCH_LINE();
        
        // Cetak Header
        printf("| %-*s | %-*s | %-*s | %-*s | %-*s | %-*s | %-*s |\n",
               w_id, "ID", w_film, "Film", w_studio, "Studio", 
               w_date, "Date", w_start, "Start", w_end, "End", w_price, "Price");
               
        PRINT_SCH_LINE();

        // Cetak Konten
        for (int i = 0; i < count; i++) {
            char t_price[30], t_end[20];
            sprintf(t_price, "Rp %.0f", schedules[i].price);

            Film* f = btree_search(film_tree, schedules[i].film_id);
            char film_title[100] = "Unknown";
            if (f != NULL) {
                strcpy(film_title, f->title);
                minutes_to_time_str(time_to_minutes(schedules[i].time) + f->duration, t_end);
            } else {
                strcpy(t_end, "?");
            }

            Studio st;
            char st_name[50] = "Unknown";
            if (find_studio(schedules[i].studio_id, &st)) {
                strcpy(st_name, st.name);
            }

            printf("| %-*d | %-*s | %-*s | %-*s | %-*s | %-*s | %-*s |\n",
                   w_id, schedules[i].id, 
                   w_film, film_title, 
                   w_studio, st_name,
                   w_date, schedules[i].date, 
                   w_start, schedules[i].time, 
                   w_end, t_end, 
                   w_price, t_price);
        }
        
        PRINT_SCH_LINE();
        printf("Total: %d schedule(s)\n", count);
        for(int i = 0; i < total_width; i++) printf("="); printf("\n");
    }
    printf("\n");

    // ==========================================
    // TAHAP 4: KEMBALI KE MENU (PRESS ANY KEY)
    // ==========================================
    system("pause");
    schedule_manage();
}

// ============================================================
// !! USER ACCOUNT MANAGEMENT !!
// ============================================================
void acc_manage() {
    char input[100];
    char lower_input[100];
    int choice;
    int valid;

    do {
        system("cls");
        printf("============================================\n");
        printf("           USER ACCOUNT MANAGEMENT          \n");
        printf("--------------------------------------------\n");
        printf("[1] View All Users\n");
        printf("[2] Search User\n");
        printf("[3] Delete User Account\n");
        printf("[0] Back\n");
        printf("============================================\n");
        printf("Choose : ");
        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = '\0';

        for (int i = 0; input[i] != '\0'; i++) {
            lower_input[i] = tolower(input[i]);
        }
        lower_input[strlen(input)] = '\0';

        valid = 1;

        if (strcmp(lower_input, "1") == 0 || strcmp(lower_input, "view all users") == 0) {
            choice = 1;
        } else if (strcmp(lower_input, "2") == 0 || strcmp(lower_input, "search user") == 0) {
            choice = 2;
        } else if (strcmp(lower_input, "3") == 0 || strcmp(lower_input, "delete user account") == 0) {
            choice = 3;
        } else if (strcmp(lower_input, "0") == 0 || strcmp(lower_input, "back") == 0) {
            choice = 0;
        } else {
            printf("\n[ERROR] Invalid Input!\n");
            printf("Please enter the number (0-3) or the exact option text.\n");
            system("pause");
            valid = 0;
        }
    } while (!valid);

    switch (choice) {
        case 1: view_users();   break;
        case 2: search_user();  break;
        case 3: delete_user();  break;
        case 0: menu_admin();   break;
    }
}

// ============================================================
// !! VIEW ALL USERS !!
// ============================================================
void view_users() { // Sesuaikan nama fungsi ini jika berbeda
    system("cls");
    
    // Asumsi struktur data sementara untuk membaca file
    struct {
        char username[100];
        char fullname[100];
        char email[100];
    } users[500];
    
    int count = 0;
    char buffer[300];

    // ==========================================
    // TAHAP 1: BACA DATA KE DALAM ARRAY
    // ==========================================
    // Ganti 'account_file' dengan variabel file Anda (misal: "users.txt" atau "account.txt")
    FILE* fp = fopen(account_file, "r"); 
    if (fp != NULL) {
        while (fgets(buffer, sizeof(buffer), fp)) {
            buffer[strcspn(buffer, "\n")] = 0;
            if (strlen(buffer) == 0) continue;

            char temp_pass[100];
            
            // ASUMSI FORMAT FILE: username=password=fullname=email
            // Jika separator Anda '#' (bukan '='), ubah "%[^=]=" menjadi "%[^#]#"
            // Jika tidak ada password, hapus variabel temp_pass dan sesuaikan format sscanf
            if (sscanf(buffer, "%[^,],%[^,],%[^,],%[^,]", 
                       users[count].username, 
                       temp_pass, 
                       users[count].fullname, 
                       users[count].email) >= 3) {
                count++;
            }
        }
        fclose(fp);
    }

    // ==========================================
    // TAHAP 2: HITUNG LEBAR KOLOM DINAMIS
    // ==========================================
    int w_uname = 8;     // Lebar minimal "Username"
    int w_fname = 9;     // Lebar minimal "Full Name"
    int w_email = 5;     // Lebar minimal "Email"

    for (int i = 0; i < count; i++) {
        if ((int)strlen(users[i].username) > w_uname) w_uname = strlen(users[i].username);
        if ((int)strlen(users[i].fullname) > w_fname) w_fname = strlen(users[i].fullname);
        if ((int)strlen(users[i].email) > w_email) w_email = strlen(users[i].email);
    }

    int total_width = w_uname + w_fname + w_email + 10;

    #define PRINT_USER_LINE() \
        do { \
            printf("+"); \
            for(int k=0; k<w_uname+2; k++) printf("-"); printf("+"); \
            for(int k=0; k<w_fname+2; k++) printf("-"); printf("+"); \
            for(int k=0; k<w_email+2; k++) printf("-"); printf("+\n"); \
        } while(0)

    // ==========================================
    // TAHAP 3: CETAK TABEL
    // ==========================================
    printf("\n");
    for (int i = 0; i < total_width; i++) printf("="); printf("\n");
    
    // Render text ke tengah (Center alignment)
    int padding = (total_width - 9) / 2; // 9 adalah panjang teks "ALL USERS"
    if (padding < 0) padding = 0;
    for (int i = 0; i < padding; i++) printf(" ");
    printf("ALL USERS\n");
    
    for (int i = 0; i < total_width; i++) printf("="); printf("\n");

    if (count == 0) {
        printf("| %-*s |\n", total_width - 4, "No users available at the moment.");
        for (int i = 0; i < total_width; i++) printf("="); printf("\n");
    } else {
        PRINT_USER_LINE();
        
        printf("| %-*s | %-*s | %-*s |\n", w_uname, "Username", w_fname, "Full Name", w_email, "Email");
               
        PRINT_USER_LINE();

        for (int i = 0; i < count; i++) {
            printf("| %-*s | %-*s | %-*s |\n", 
                   w_uname, users[i].username, 
                   w_fname, users[i].fullname, 
                   w_email, users[i].email);
        }
        
        PRINT_USER_LINE();
        printf("Total: %d user(s)\n", count);
        for (int i = 0; i < total_width; i++) printf("="); printf("\n");
    }
    printf("\n");

    // ==========================================
    // TAHAP 4: KEMBALI KE MENU (PRESS ANY KEY)
    // ==========================================
    system("pause");
    acc_manage();
}

// ============================================================
// !! SEARCH USER (BY USERNAME OR FULL NAME) !!
// ============================================================
void search_user() {
    // Struktur data sementara untuk membaca file
    struct {
        char username[100];
        char password[100];
        char fullname[100];
        char email[100];
    } users[500];
    
    int count = 0;
    char buffer[512];

    // ==========================================
    // TAHAP 1: BACA DATA KE DALAM ARRAY
    // ==========================================
    FILE* fp = fopen("users.txt", "r"); 
    if (fp != NULL) {
        while (fgets(buffer, sizeof(buffer), fp)) {
            buffer[strcspn(buffer, "\n")] = 0;
            if (strlen(buffer) == 0) continue;

            if (sscanf(buffer, "%[^,],%[^,],%[^,],%[^,]", 
                       users[count].username, 
                       users[count].password, 
                       users[count].fullname, 
                       users[count].email) >= 3) {
                count++;
            }
        }
        fclose(fp);
    }

    if (count == 0) {
        system("cls");
        printf("============================================\n");
        printf("                SEARCH USER                 \n");
        printf("============================================\n");
        printf("No users available in the database.\n");
        printf("============================================\n");
        system("pause");
        acc_manage(); // <-- PENTING: Ganti dengan nama fungsi menu User Anda jika berbeda
        return; 
    }

    // ==========================================
    // TAHAP 2: HITUNG LEBAR KOLOM DINAMIS
    // ==========================================
    int w_uname = 8;     
    int w_fname = 9;     
    int w_email = 5;     

    for (int i = 0; i < count; i++) {
        if ((int)strlen(users[i].username) > w_uname) w_uname = strlen(users[i].username);
        if ((int)strlen(users[i].fullname) > w_fname) w_fname = strlen(users[i].fullname);
        if ((int)strlen(users[i].email) > w_email) w_email = strlen(users[i].email);
    }

    #define PRINT_ALL_USER_LINE() \
        do { \
            printf("+"); \
            for(int k=0; k<w_uname+2; k++) printf("-"); printf("+"); \
            for(int k=0; k<w_fname+2; k++) printf("-"); printf("+"); \
            for(int k=0; k<w_email+2; k++) printf("-"); printf("+\n"); \
        } while(0)

    char input[100];
    char lower_input[100];
    int target_index = -1;

    // ==========================================
    // PAGE 1: TAMPILKAN TABEL & INPUT PENCARIAN
    // ==========================================
    do {
        system("cls");
        
        int total_width = w_uname + w_fname + w_email + 10;
        for (int i = 0; i < total_width; i++) printf("="); printf("\n");
        
        int padding = (total_width - 11) / 2;
        if (padding < 0) padding = 0;
        for (int i = 0; i < padding; i++) printf(" ");
        printf("SEARCH USER\n");
        
        for (int i = 0; i < total_width; i++) printf("="); printf("\n");

        PRINT_ALL_USER_LINE();
        printf("| %-*s | %-*s | %-*s |\n", w_uname, "Username", w_fname, "Full Name", w_email, "Email");
        PRINT_ALL_USER_LINE();

        for (int i = 0; i < count; i++) {
            printf("| %-*s | %-*s | %-*s |\n", 
                   w_uname, users[i].username, 
                   w_fname, users[i].fullname, 
                   w_email, users[i].email);
        }
        
        PRINT_ALL_USER_LINE();
        
        printf(" (Enter '0' to go back)\n");
        for (int i = 0; i < total_width; i++) printf("-"); printf("\n");
        
        printf("Enter Username or Full Name to search : ");

        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = '\0';

        // --- FITUR BACK BERFUNGSI DI SINI ---
        if (strcmp(input, "0") == 0) {
            acc_manage(); // <-- PENTING: Ganti dengan nama fungsi menu User Anda jika berbeda
            return; 
        }

        if (strlen(input) == 0) {
            printf("[ERROR] Input cannot be empty!\n");
            system("pause");
            continue;
        }

        for (int i = 0; input[i] != '\0'; i++) lower_input[i] = tolower(input[i]);
        lower_input[strlen(input)] = '\0';

        int match_indices[500];
        int match_count = 0;

        for (int i = 0; i < count; i++) {
            char lower_uname[100];
            char lower_fname[100];
            strcpy(lower_uname, users[i].username);
            strcpy(lower_fname, users[i].fullname);
            
            for (int j = 0; lower_uname[j] != '\0'; j++) lower_uname[j] = tolower(lower_uname[j]);
            for (int j = 0; lower_fname[j] != '\0'; j++) lower_fname[j] = tolower(lower_fname[j]);
            
            if (strstr(lower_uname, lower_input) != NULL || strstr(lower_fname, lower_input) != NULL) {
                match_indices[match_count++] = i;
            }
        }

        if (match_count == 0) {
            printf("[ERROR] User \"%s\" not found! Please try again.\n", input);
            system("pause");
        } else if (match_count == 1) {
            target_index = match_indices[0]; 
        } else {
            // ==========================================
            // RESOLUSI AMBIGUITAS
            // ==========================================
            int resolved = 0;
            
            int w_uname_match = 8;
            int w_fname_match = 9;
            for (int i = 0; i < match_count; i++) {
                int idx = match_indices[i];
                if ((int)strlen(users[idx].username) > w_uname_match) w_uname_match = strlen(users[idx].username);
                if ((int)strlen(users[idx].fullname) > w_fname_match) w_fname_match = strlen(users[idx].fullname);
            }

            #define PRINT_MATCH_LINE() \
                do { \
                    printf("+"); \
                    for(int k=0; k<w_uname_match+2; k++) printf("-"); printf("+"); \
                    for(int k=0; k<w_fname_match+2; k++) printf("-"); printf("+\n"); \
                } while(0)

            do {
                system("cls");
                printf("============================================\n");
                printf("       SEARCH USER - MULTIPLE MATCHES       \n");
                printf("============================================\n");
                printf("Multiple users found for \"%s\":\n", input);
                
                PRINT_MATCH_LINE();
                printf("| %-*s | %-*s |\n", w_uname_match, "Username", w_fname_match, "Full Name");
                PRINT_MATCH_LINE();
                
                for (int i = 0; i < match_count; i++) {
                    int idx = match_indices[i];
                    printf("| %-*s | %-*s |\n", w_uname_match, users[idx].username, w_fname_match, users[idx].fullname);
                }
                PRINT_MATCH_LINE();
                
                printf("\nEnter EXACT Username from the list above\n");
                printf(" (or '0' to go back) : ");
                
                char exact_input[100];
                fgets(exact_input, sizeof(exact_input), stdin);
                exact_input[strcspn(exact_input, "\n")] = '\0';

                // --- FITUR BACK BERFUNGSI DI SINI ---
                if (strcmp(exact_input, "0") == 0) {
                    acc_manage(); // <-- PENTING: Ganti dengan nama fungsi menu User Anda jika berbeda
                    return; 
                }

                if (strlen(exact_input) == 0) { 
                    printf("[ERROR] Input cannot be empty!\n"); 
                    system("pause"); 
                    continue; 
                }

                for (int i = 0; i < match_count; i++) {
                    int idx = match_indices[i];
                    char temp_exact[100], temp_user[100];
                    strcpy(temp_exact, exact_input);
                    strcpy(temp_user, users[idx].username);
                    
                    for(int k=0; temp_exact[k]; k++) temp_exact[k] = tolower(temp_exact[k]);
                    for(int k=0; temp_user[k]; k++) temp_user[k] = tolower(temp_user[k]);

                    if (strcmp(temp_exact, temp_user) == 0) {
                        target_index = idx;
                        resolved = 1;
                        break;
                    }
                }

                if (!resolved) {
                    printf("[ERROR] Username \"%s\" is not on the list! Please type exactly as shown.\n", exact_input);
                    system("pause");
                }
            } while (!resolved);
        }
    } while (target_index == -1);


    // ==========================================
    // PAGE 2: TAMPILAN PROFIL USER
    // ==========================================
    system("cls");
    printf("============================================\n");
    printf("                USER PROFILE                \n");
    printf("============================================\n");
    printf("  Username   : %s\n", users[target_index].username);
    printf("  Full Name  : %s\n", users[target_index].fullname);
    printf("  Email      : %s\n", users[target_index].email);
    printf("============================================\n");
    system("pause");
    
    acc_manage();
}

// ============================================================
// !! DELETE USER ACCOUNT !!
// ============================================================
void delete_user() {
    struct {
        char username[100];
        char password[100];
        char fullname[100];
        char email[100];
    } users[500];
    
    int count = 0;
    char buffer[512];

    // ==========================================
    // TAHAP 1: BACA DATA KE DALAM ARRAY
    // ==========================================
    FILE* fp = fopen("users.txt", "r"); 
    if (fp != NULL) {
        while (fgets(buffer, sizeof(buffer), fp)) {
            buffer[strcspn(buffer, "\n")] = 0;
            if (strlen(buffer) == 0) continue;

            if (sscanf(buffer, "%[^,],%[^,],%[^,],%[^,]", 
                       users[count].username, 
                       users[count].password, 
                       users[count].fullname, 
                       users[count].email) >= 3) {
                count++;
            }
        }
        fclose(fp);
    }

    if (count == 0) {
        system("cls");
        printf("============================================\n");
        printf("               DELETE USER                  \n");
        printf("============================================\n");
        printf("No users available in the database.\n");
        printf("============================================\n");
        system("pause");
        acc_manage(); 
        return; 
    }

    // ==========================================
    // TAHAP 2: HITUNG LEBAR KOLOM DINAMIS
    // ==========================================
    int w_uname = 8;     
    int w_fname = 9;     
    int w_email = 5;     

    for (int i = 0; i < count; i++) {
        if ((int)strlen(users[i].username) > w_uname) w_uname = strlen(users[i].username);
        if ((int)strlen(users[i].fullname) > w_fname) w_fname = strlen(users[i].fullname);
        if ((int)strlen(users[i].email) > w_email) w_email = strlen(users[i].email);
    }

    #define PRINT_ALL_USER_LINE() \
        do { \
            printf("+"); \
            for(int k=0; k<w_uname+2; k++) printf("-"); printf("+"); \
            for(int k=0; k<w_fname+2; k++) printf("-"); printf("+"); \
            for(int k=0; k<w_email+2; k++) printf("-"); printf("+\n"); \
        } while(0)

    char input[100];
    char lower_input[100];
    int target_index = -1;

    // ==========================================
    // PAGE 1: TAMPILKAN TABEL & INPUT PENCARIAN
    // ==========================================
    do {
        system("cls");
        
        int total_width = w_uname + w_fname + w_email + 10;
        for (int i = 0; i < total_width; i++) printf("="); printf("\n");
        
        int padding = (total_width - 11) / 2;
        if (padding < 0) padding = 0;
        for (int i = 0; i < padding; i++) printf(" ");
        printf("DELETE USER\n");
        
        for (int i = 0; i < total_width; i++) printf("="); printf("\n");

        PRINT_ALL_USER_LINE();
        printf("| %-*s | %-*s | %-*s |\n", w_uname, "Username", w_fname, "Full Name", w_email, "Email");
        PRINT_ALL_USER_LINE();

        for (int i = 0; i < count; i++) {
            printf("| %-*s | %-*s | %-*s |\n", 
                   w_uname, users[i].username, 
                   w_fname, users[i].fullname, 
                   w_email, users[i].email);
        }
        
        PRINT_ALL_USER_LINE();
        
        printf(" (Enter '0' to go back)\n");
        for (int i = 0; i < total_width; i++) printf("-"); printf("\n");
        
        printf("Enter Username or Full Name to delete : ");

        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = '\0';

        if (strcmp(input, "0") == 0) {
            acc_manage(); 
            return; 
        }

        if (strlen(input) == 0) {
            printf("[ERROR] Input cannot be empty!\n");
            system("pause");
            continue;
        }

        for (int i = 0; input[i] != '\0'; i++) lower_input[i] = tolower(input[i]);
        lower_input[strlen(input)] = '\0';

        int match_indices[500];
        int match_count = 0;

        for (int i = 0; i < count; i++) {
            char lower_uname[100];
            char lower_fname[100];
            strcpy(lower_uname, users[i].username);
            strcpy(lower_fname, users[i].fullname);
            
            for (int j = 0; lower_uname[j] != '\0'; j++) lower_uname[j] = tolower(lower_uname[j]);
            for (int j = 0; lower_fname[j] != '\0'; j++) lower_fname[j] = tolower(lower_fname[j]);
            
            if (strstr(lower_uname, lower_input) != NULL || strstr(lower_fname, lower_input) != NULL) {
                match_indices[match_count++] = i;
            }
        }

        if (match_count == 0) {
            printf("[ERROR] User \"%s\" not found! Please try again.\n", input);
            system("pause");
        } else if (match_count == 1) {
            target_index = match_indices[0]; 
        } else {
            // ==========================================
            // RESOLUSI AMBIGUITAS
            // ==========================================
            int resolved = 0;
            
            int w_uname_match = 8;
            int w_fname_match = 9;
            for (int i = 0; i < match_count; i++) {
                int idx = match_indices[i];
                if ((int)strlen(users[idx].username) > w_uname_match) w_uname_match = strlen(users[idx].username);
                if ((int)strlen(users[idx].fullname) > w_fname_match) w_fname_match = strlen(users[idx].fullname);
            }

            #define PRINT_MATCH_LINE() \
                do { \
                    printf("+"); \
                    for(int k=0; k<w_uname_match+2; k++) printf("-"); printf("+"); \
                    for(int k=0; k<w_fname_match+2; k++) printf("-"); printf("+\n"); \
                } while(0)

            do {
                system("cls");
                printf("============================================\n");
                printf("       DELETE USER - MULTIPLE MATCHES       \n");
                printf("============================================\n");
                printf("Multiple users found for \"%s\":\n", input);
                
                PRINT_MATCH_LINE();
                printf("| %-*s | %-*s |\n", w_uname_match, "Username", w_fname_match, "Full Name");
                PRINT_MATCH_LINE();
                
                for (int i = 0; i < match_count; i++) {
                    int idx = match_indices[i];
                    printf("| %-*s | %-*s |\n", w_uname_match, users[idx].username, w_fname_match, users[idx].fullname);
                }
                PRINT_MATCH_LINE();
                
                printf("\nEnter EXACT Username to delete\n");
                printf(" (or '0' to go back) : ");
                
                char exact_input[100];
                fgets(exact_input, sizeof(exact_input), stdin);
                exact_input[strcspn(exact_input, "\n")] = '\0';

                if (strcmp(exact_input, "0") == 0) {
                    acc_manage(); 
                    return; 
                }

                if (strlen(exact_input) == 0) { 
                    printf("[ERROR] Input cannot be empty!\n"); 
                    system("pause"); 
                    continue; 
                }

                for (int i = 0; i < match_count; i++) {
                    int idx = match_indices[i];
                    char temp_exact[100], temp_user[100];
                    strcpy(temp_exact, exact_input);
                    strcpy(temp_user, users[idx].username);
                    
                    for(int k=0; temp_exact[k]; k++) temp_exact[k] = tolower(temp_exact[k]);
                    for(int k=0; temp_user[k]; k++) temp_user[k] = tolower(temp_user[k]);

                    if (strcmp(temp_exact, temp_user) == 0) {
                        target_index = idx;
                        resolved = 1;
                        break;
                    }
                }

                if (!resolved) {
                    printf("[ERROR] Username \"%s\" is not on the list! Please type exactly as shown.\n", exact_input);
                    system("pause");
                }
            } while (!resolved);
        }
    } while (target_index == -1);


    // ==========================================
    // PAGE 2 : KONFIRMASI Y/N
    // ==========================================
    char confirm_str[10];
    char confirm_char;
    int conf_valid = 0;

    do {
        system("cls");
        printf("============================================\n");
        printf("            CONFIRM DELETION                \n");
        printf("============================================\n");
        printf("  Account Detail:\n");
        printf("  Username  : %s\n", users[target_index].username);
        printf("  Full Name : %s\n", users[target_index].fullname);
        printf("  Email     : %s\n", users[target_index].email);
        printf("--------------------------------------------\n");
        printf("Are you sure you want to delete this user?\n");
        printf("Type (Y/N) or '0' to go back : ");

        fgets(confirm_str, sizeof(confirm_str), stdin);
        confirm_str[strcspn(confirm_str, "\n")] = '\0';

        if (strcmp(confirm_str, "0") == 0) { acc_manage(); return; }

        if (strlen(confirm_str) == 0) {
            printf("[ERROR] Input cannot be empty! Enter Y or N.\n");
            system("pause");
            continue;
        }

        if (strcmp(confirm_str, "Y") == 0 || strcmp(confirm_str, "y") == 0) {
            confirm_char = 'y';
            conf_valid = 1;
        } else if (strcmp(confirm_str, "N") == 0 || strcmp(confirm_str, "n") == 0) {
            confirm_char = 'n';
            conf_valid = 1;
        } else {
            printf("[ERROR] Invalid input! Enter Y or N.\n");
            system("pause");
        }
    } while (!conf_valid);

    if (confirm_char == 'n') {
        printf("Deletion cancelled.\n");
        system("pause");
        acc_manage();
        return;
    }


    // ==========================================
    // PAGE 3 : KONFIRMASI PASSWORD ADMIN
    // ==========================================
    char input_pass[100];
    char confirm_pass_input[100]; 
    int pass_match = 0;

    do {
        system("cls");
        printf("============================================\n");
        printf("         ADMIN PASSWORD CONFIRMATION        \n");
        printf("============================================\n");
        printf(" [WARNING] This action cannot be undone!    \n");
        printf("--------------------------------------------\n");
        printf(" (Enter '0' at any prompt to go back)\n");
        printf("--------------------------------------------\n");

        printf("Enter Admin password : ");
        fgets(input_pass, sizeof(input_pass), stdin);
        input_pass[strcspn(input_pass, "\n")] = '\0';

        if (strcmp(input_pass, "0") == 0) { acc_manage(); return; }

        if (strlen(input_pass) == 0) {
            printf("[ERROR] Password cannot be empty!\n");
            system("pause");
            continue;
        }

        printf("Confirm Admin password : ");
        fgets(confirm_pass_input, sizeof(confirm_pass_input), stdin);
        confirm_pass_input[strcspn(confirm_pass_input, "\n")] = '\0';

        if (strcmp(confirm_pass_input, "0") == 0) { acc_manage(); return; }

        if (strcmp(input_pass, confirm_pass_input) != 0) {
            printf("\n[ERROR] Passwords do not match! Please try again.\n");
            system("pause");
            continue;
        }

        // Hardcoded admin password validation
        if (strcmp(input_pass, "admin123@") != 0) {
            printf("\n[ERROR] Incorrect password! Please try again.\n");
            system("pause");
            continue;
        }

        pass_match = 1; 
    } while (!pass_match);


    // ==========================================
    // PAGE 4 : EKSEKUSI PENGHAPUSAN FILE
    // ==========================================
    FILE* in  = fopen("users.txt", "r");
    FILE* tmp = fopen("temp_users.txt", "w");
    if (in == NULL || tmp == NULL) { 
        printf("[ERROR] Could not open database files!\n");
        system("pause");
        acc_manage();
        return; 
    }

    char sbuf[512];
    while (fgets(sbuf, sizeof(sbuf), in)) {
        char current_line[512];
        strcpy(current_line, sbuf); // Backup baris original (termasuk \n)
        
        sbuf[strcspn(sbuf, "\n")] = 0;
        if(strlen(sbuf) == 0) continue;
        
        char parsed_uname[100];
        // Baca username sebelum koma pertama
        if (sscanf(sbuf, "%[^,]", parsed_uname) == 1) {
            // Jika username tidak sama dengan target, tulis kembali ke file sementara
            if (strcmp(parsed_uname, users[target_index].username) != 0) {
                fprintf(tmp, "%s", current_line); 
            }
        }
    }
    fclose(in); 
    fclose(tmp);
    remove("users.txt");
    rename("temp_users.txt", "users.txt");


    // ==========================================
    // PAGE 5 : SUCCESS INVOICE
    // ==========================================
    system("cls");
    printf("============================================\n");
    printf("               ACCOUNT DELETED              \n");
    printf("============================================\n");
    printf("\n");
    printf("   User \"%s\" (%s) \n", users[target_index].username, users[target_index].fullname);
    printf("       deleted successfully!                \n");
    printf("\n");
    printf("============================================\n");
    system("pause");
    
    acc_manage();
}

// ============================================================
// !! CUSTOMER MENU !!
// ============================================================
void menu_cust() {

    int choice;
    char buffer[1024];
    char input[100];
    char lower_input[100];
    int valid;

    Account acc;
    char current_fullname[100] = "";

    btree_load_from_file();

    FILE* check = fopen(account_file, "r");

    if (check != NULL) {

        while (fgets(buffer, sizeof(buffer), check)) {

            buffer[strcspn(buffer, "\n")] = 0;

            sscanf(buffer,
                   "%[^,],%[^,],%[^,],%[^\n]",
                   acc.username,
                   acc.password,
                   acc.name,
                   acc.email);

            if (strcmp(acc.username, current_user) == 0) {

                strcpy(current_fullname, acc.name);
                break;
            }
        }

        fclose(check);
    }

    if (strlen(current_fullname) == 0)
        strcpy(current_fullname, current_user);

    do {

        system("cls");

        view_film_cust();

        
        printf("\t\t\t\t\t\t\t----------------------------------------------\n");
        printf(GOLD);
        printf("\t\t\t\t\t\t\t                 CUSTOMER MENU                \n");
        printf(RESET);
        printf("\t\t\t\t\t\t\t----------------------------------------------\n");

        printf("\t\t\t\t\t\t\t| Welcome Back, " GOLD "%-27s" RESET "  |\n",
               current_fullname);

        printf("\t\t\t\t\t\t\t----------------------------------------------\n");
        
        printf("\t\t\t\t\t\t\t| " GOLD "[1]" RESET " Book Ticket                            |\n");
        printf("\t\t\t\t\t\t\t| " GOLD "[2]" RESET " My Booking History                     |\n");
        printf("\t\t\t\t\t\t\t| " GOLD "[3]" RESET " Cancel Booking                         |\n");
        printf("\t\t\t\t\t\t\t| " GOLD "[4]" RESET " Edit Profile                           |\n");
        printf("\t\t\t\t\t\t\t| " RED  "[0]" RESET " Logout                                 |\n");

        printf("\t\t\t\t\t\t\t----------------------------------------------\n");

        printf(GOLD "\n\t\t\t\t\t\t\tChoose Menu : " RESET);

        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = '\0';

        for (int i = 0; input[i] != '\0'; i++) {
            lower_input[i] = tolower(input[i]);
        }

        lower_input[strlen(input)] = '\0';

        valid = 1;

        if (strcmp(lower_input, "1") == 0 ||
            strcmp(lower_input, "book ticket") == 0) {

            choice = 1;

        }
        else if (strcmp(lower_input, "2") == 0 ||
                 strcmp(lower_input, "history") == 0 ||
                 strcmp(lower_input, "my booking history") == 0) {

            choice = 2;

        }
        else if (strcmp(lower_input, "3") == 0 ||
                 strcmp(lower_input, "cancel") == 0 ||
                 strcmp(lower_input, "cancel booking") == 0) {

            choice = 3;

        }
        else if (strcmp(lower_input, "4") == 0 ||
                 strcmp(lower_input, "edit profile") == 0) {

            choice = 4;

        }
        else if (strcmp(lower_input, "0") == 0 ||
                 strcmp(lower_input, "logout") == 0) {

            choice = 0;

        }
        else {

            printf(RED);
            printf("\n");
            printf("\t\t\t\t\t\t\t      Invalid Input!\n");
            printf(RESET);

            printf(DIM_GOLD);
            printf("\t\t\t\t\t\t\tPress ENTER to continue...");
            printf(RESET);

            getchar();

            valid = 0;
        }

    } while (!valid);

    switch (choice) {

        case 1:
            book_ticket();
            break;

        case 2:
            history();
            break;

        case 3:
            cancel();
            break;

        case 4:
            edit_profile();
            break;

        case 0:
            main_menu();
            break;
    }
}

void print_table_line(int w_id, int w_title, int w_genre, int w_dur, int w_age, int w_detail) {
    printf("+");
    for(int i = 0; i < w_id + 2; i++) printf("-"); printf("+");
    for(int i = 0; i < w_title + 2; i++) printf("-"); printf("+");
    for(int i = 0; i < w_genre + 2; i++) printf("-"); printf("+");
    for(int i = 0; i < w_dur + 2; i++) printf("-"); printf("+");
    for(int i = 0; i < w_age + 2; i++) printf("-"); printf("+");
    for(int i = 0; i < w_detail + 2; i++) printf("-"); printf("+\n");
}

void view_film_cust() {

    tampilkan_banner_train();

    FILE *data = fopen(film_file, "r");

    if (data == NULL) {
        printf(RED "\n\t\t\t\tCannot open film database!\n" RESET);
        return;
    }

    Film film;
    char buffer[1024];
    int count = 0;

    /* Header Tabel */
    printf(GOLD);
    printf("\t\t\t\t----------------------------------------------------------------------------------------------\n");
    printf("\t\t\t\t| %-4s | %-30s | %-24s | %-10s | %-10s |\n",
           "ID",
           "TITLE",
           "GENRE",
           "DURATION",
           "AGE");
    printf("\t\t\t\t----------------------------------------------------------------------------------------------\n");
    printf(RESET);

    /* Data Film */
    while (fgets(buffer, sizeof(buffer), data)) {

        buffer[strcspn(buffer, "\n")] = '\0';

        if (sscanf(buffer,
                   "%d=%[^=]=%[^=]=%d=%d=%[^\n]",
                   &film.id,
                   film.title,
                   film.genre,
                   &film.duration,
                   &film.age_rating,
                   film.detail) == 6) {

            count++;

            printf("\t\t\t\t| %-4d | %-30.30s | %-24.24s | %-10d | %-10d |\n",
                   film.id,
                   film.title,
                   film.genre,
                   film.duration,
                   film.age_rating);
        }
    }

    /* Jika Tidak Ada Film */
    if (count == 0) {

        printf("\t\t\t\t| %-69s |\n",
               "No movies are currently showing.");
    }

    /* Footer Tabel */
    printf(GOLD);
    printf("\t\t\t\t----------------------------------------------------------------------------------------------\n");
    printf(RESET);

    printf("\n");

    fclose(data);
}

// ============================================================
// !! BOOK TICKET !!
// ============================================================
void book_ticket() {
    char input[100];
    char lower_input[100];
    int valid;

    /* -- LANGKAH 1: Ambil Data Film -- */
    Film all_films[200];
    int film_count = 0;
    btree_inorder(film_tree, all_films, &film_count);

    if (film_count == 0) {
        printf("Tidak ada film tersedia.\n");
        system("pause");
        menu_cust();
        return;
    }

    /* Hitung Lebar Kolom Dinamis Untuk Tabel Film */
    int w_id = 2, w_title = 5, w_genre = 5, w_dur = 8, w_age = 3;
    for (int i = 0; i < film_count; i++) {
        char temp_id[20], temp_dur[20], temp_age[20];
        sprintf(temp_id, "%d", all_films[i].id);
        sprintf(temp_dur, "%d", all_films[i].duration);
        sprintf(temp_age, "%d+", all_films[i].age_rating);

        if ((int)strlen(temp_id) > w_id) w_id = strlen(temp_id);
        if ((int)strlen(all_films[i].title) > w_title) w_title = strlen(all_films[i].title);
        if ((int)strlen(all_films[i].genre) > w_genre) w_genre = strlen(all_films[i].genre);
        if ((int)strlen(temp_dur) > w_dur) w_dur = strlen(temp_dur);
        if ((int)strlen(temp_age) > w_age) w_age = strlen(temp_age);
    }

    int film_id = -1;
    Film* chosen_film = NULL;

    /* Loop Halaman PAGE 1: Pemilihan Film */
    do {
        system("cls");
        printf("======================================================================================================\n");
        printf("                                              BOOK TICKET                                             \n");
        printf("======================================================================================================\n");
        printf("\n[ STEP 1 ] Pilih Film\n");

        /* Cetak Garis Pembatas Atas Tabel Film */
        printf("+"); for(int k=0; k<w_id+2; k++) printf("-");
        printf("+"); for(int k=0; k<w_title+2; k++) printf("-");
        printf("+"); for(int k=0; k<w_genre+2; k++) printf("-");
        printf("+"); for(int k=0; k<w_dur+2; k++) printf("-");
        printf("+"); for(int k=0; k<w_age+2; k++) printf("-"); printf("+\n");

        /* Cetak Header */
        printf("| %-*s | %-*s | %-*s | %-*s | %-*s |\n", w_id, "ID", w_title, "Title", w_genre, "Genre", w_dur, "Duration", w_age, "Age");

        /* Cetak Garis Tengah */
        printf("+"); for(int k=0; k<w_id+2; k++) printf("-");
        printf("+"); for(int k=0; k<w_title+2; k++) printf("-");
        printf("+"); for(int k=0; k<w_genre+2; k++) printf("-");
        printf("+"); for(int k=0; k<w_dur+2; k++) printf("-");
        printf("+"); for(int k=0; k<w_age+2; k++) printf("-"); printf("+\n");

        /* Cetak Konten Film */
        for (int i = 0; i < film_count; i++) {
            char temp_dur[20], temp_age[20];
            sprintf(temp_dur, "%d", all_films[i].duration);
            sprintf(temp_age, "%d+", all_films[i].age_rating);
            printf("| %-*d | %-*s | %-*s | %-*s | %-*s |\n", 
                   w_id, all_films[i].id, 
                   w_title, all_films[i].title, 
                   w_genre, all_films[i].genre, 
                   w_dur, temp_dur, 
                   w_age, temp_age);
        }

        /* Cetak Garis Bawah */
        printf("+"); for(int k=0; k<w_id+2; k++) printf("-");
        printf("+"); for(int k=0; k<w_title+2; k++) printf("-");
        printf("+"); for(int k=0; k<w_genre+2; k++) printf("-");
        printf("+"); for(int k=0; k<w_dur+2; k++) printf("-");
        printf("+"); for(int k=0; k<w_age+2; k++) printf("-"); printf("+\n");

        printf("Masukkan Film ID atau Judul (Ketik '0' untuk Batal) : ");
        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = '\0';

        // Fitur Back
        if (strcmp(input, "0") == 0) {
            menu_cust();
            return;
        }

        if (strlen(input) == 0) {
            printf("[ERROR] Input tidak boleh kosong!\n");
            system("pause");
            continue;
        }

        // Konversi ke lowercase untuk pencarian teks
        for (int i = 0; input[i] != '\0'; i++) {
            lower_input[i] = tolower(input[i]);
        }
        lower_input[strlen(input)] = '\0';

        // Cek apakah input murni angka
        int is_num = 1;
        for (int i = 0; input[i] != '\0'; i++) {
            if (!isdigit(input[i])) { is_num = 0; break; }
        }

        Film matched[200];
        int match_count = 0;

        if (is_num) {
            // Pencarian berdasarkan ID
            int input_id = atoi(input);
            for (int i = 0; i < film_count; i++) {
                if (all_films[i].id == input_id) {
                    matched[match_count] = all_films[i];
                    match_count++;
                    break;
                }
            }
        } else {
            // Pencarian berdasarkan Judul
            for (int i = 0; i < film_count; i++) {
                char lower_title[100];
                strcpy(lower_title, all_films[i].title);
                for (int j = 0; lower_title[j] != '\0'; j++) {
                    lower_title[j] = tolower(lower_title[j]);
                }
                if (strstr(lower_title, lower_input) != NULL) {
                    matched[match_count] = all_films[i];
                    match_count++;
                }
            }
        }

        // Eksekusi Hasil Pencarian
        if (match_count == 0) {
            printf("[ERROR] Film \"%s\" tidak ditemukan! Silakan coba lagi.\n", input);
            system("pause");
        } else if (match_count == 1) {
            film_id = matched[0].id;
        } else {
            // Jika ada banyak kecocokan
            int resolved = 0;
            do {
                system("cls");
                printf("======================================================================================================\n");
                printf("                                      BOOK TICKET - MULTIPLE MATCHES                                  \n");
                printf("======================================================================================================\n");
                printf("Beberapa film ditemukan untuk pencarian \"%s\":\n", input);
                printf("--------------------------------------------\n");
                for (int i = 0; i < match_count; i++) {
                    printf("ID: %-4d | Title: %s\n", matched[i].id, matched[i].title);
                }
                printf("--------------------------------------------\n");
                printf("Masukkan EXACT ID dari daftar di atas\n");
                printf(" (atau '0' untuk kembali) : ");
                
                char id_input[100];
                fgets(id_input, sizeof(id_input), stdin);
                id_input[strcspn(id_input, "\n")] = '\0';

                if (strcmp(id_input, "0") == 0) { menu_cust(); return; }
                
                if (strlen(id_input) == 0) { 
                    printf("[ERROR] Input tidak boleh kosong!\n"); 
                    system("pause"); 
                    continue; 
                }

                int id_is_num = 1;
                for (int i = 0; id_input[i] != '\0'; i++) {
                    if (!isdigit(id_input[i])) { id_is_num = 0; break; }
                }

                if (!id_is_num) {
                    printf("[ERROR] Invalid input! Harus berupa ID angka.\n");
                    system("pause");
                    continue;
                }

                int selected_id = atoi(id_input);
                for (int i = 0; i < match_count; i++) {
                    if (matched[i].id == selected_id) {
                        film_id = selected_id;
                        resolved = 1;
                        break;
                    }
                }

                if (!resolved) {
                    printf("[ERROR] ID %d tidak ada dalam daftar di atas!\n", selected_id);
                    system("pause");
                }
            } while (!resolved);
        }

        if (film_id != -1) {
            chosen_film = btree_search(film_tree, film_id);
            if (chosen_film == NULL) {
                printf("[ERROR] Terjadi kesalahan. Film tidak ditemukan!\n");
                system("pause");
                film_id = -1; // Reset agar kembali ke pemilihan
            }
        }
    } while (film_id == -1);


    /* -- LANGKAH 2: Ambil Data Jadwal Sesuai Film ID -- */
    Schedule match_schedules[200];
    int sch_count = 0;

    FILE* sch_fp = fopen(schedule_file, "r");
    if (sch_fp != NULL) {
        char sch_buffer[300];
        while (fgets(sch_buffer, sizeof(sch_buffer), sch_fp)) {
            sch_buffer[strcspn(sch_buffer, "\n")] = 0;
            Schedule sch;
            if (sscanf(sch_buffer, "%d=%d=%d=%[^=]=%[^=]=%f",
                       &sch.id, &sch.film_id, &sch.studio_id,
                       sch.date, sch.time, &sch.price) == 6) {
                if (sch.film_id == film_id) {
                    match_schedules[sch_count++] = sch;
                }
            }
        }
        fclose(sch_fp);
    }

    if (sch_count == 0) {
        printf("\nTidak ada jadwal tersedia untuk film \"%s\".\n", chosen_film->title);
        system("pause");
        menu_cust();
        return;
    }

    /* Hitung Lebar Kolom Dinamis Untuk Tabel Jadwal */
    int w_sch_id = 2, w_studio = 6, w_date = 4, w_time = 4, w_price = 5;
    for (int i = 0; i < sch_count; i++) {
        char temp_id[20], temp_price[30];
        sprintf(temp_id, "%d", match_schedules[i].id);
        sprintf(temp_price, "Rp %.0f", match_schedules[i].price);

        Studio st;
        char st_name[50] = "Unknown";
        if (find_studio(match_schedules[i].studio_id, &st)) {
            strcpy(st_name, st.name);
        }

        if ((int)strlen(temp_id) > w_sch_id) w_sch_id = strlen(temp_id);
        if ((int)strlen(st_name) > w_studio) w_studio = strlen(st_name);
        if ((int)strlen(match_schedules[i].date) > w_date) w_date = strlen(match_schedules[i].date);
        if ((int)strlen(match_schedules[i].time) > w_time) w_time = strlen(match_schedules[i].time);
        if ((int)strlen(temp_price) > w_price) w_price = strlen(temp_price);
    }

    int schedule_id;
    Schedule chosen_sch;

    /* Loop Halaman PAGE 2: Pemilihan Jadwal */
    do {
        system("cls");
        printf("======================================================================================================\n");
        printf("                                              BOOK TICKET                                             \n");
        printf("======================================================================================================\n");
        printf("\n[ STEP 2 ] Pilih Jadwal untuk \"%s\"\n", chosen_film->title);

        /* Cetak Garis Atas Tabel Jadwal */
        printf("+"); for(int k=0; k<w_sch_id+2; k++) printf("-");
        printf("+"); for(int k=0; k<w_studio+2; k++) printf("-");
        printf("+"); for(int k=0; k<w_date+2; k++) printf("-");
        printf("+"); for(int k=0; k<w_time+2; k++) printf("-");
        printf("+"); for(int k=0; k<w_price+2; k++) printf("-"); printf("+\n");

        /* Cetak Header */
        printf("| %-*s | %-*s | %-*s | %-*s | %-*s |\n", w_sch_id, "ID", w_studio, "Studio", w_date, "Date", w_time, "Time", w_price, "Price");

        /* Cetak Garis Tengah */
        printf("+"); for(int k=0; k<w_sch_id+2; k++) printf("-");
        printf("+"); for(int k=0; k<w_studio+2; k++) printf("-");
        printf("+"); for(int k=0; k<w_date+2; k++) printf("-");
        printf("+"); for(int k=0; k<w_time+2; k++) printf("-");
        printf("+"); for(int k=0; k<w_price+2; k++) printf("-"); printf("+\n");

        /* Cetak Konten Jadwal */
        for (int i = 0; i < sch_count; i++) {
            Studio st;
            char st_name[50] = "Unknown";
            if (find_studio(match_schedules[i].studio_id, &st)) {
                strcpy(st_name, st.name);
            }
            char temp_price[30];
            sprintf(temp_price, "Rp %.0f", match_schedules[i].price);

            printf("| %-*d | %-*s | %-*s | %-*s | %-*s |\n",
                   w_sch_id, match_schedules[i].id,
                   w_studio, st_name,
                   w_date, match_schedules[i].date,
                   w_time, match_schedules[i].time,
                   w_price, temp_price);
        }

        /* Cetak Garis Bawah */
        printf("+"); for(int k=0; k<w_sch_id+2; k++) printf("-");
        printf("+"); for(int k=0; k<w_studio+2; k++) printf("-");
        printf("+"); for(int k=0; k<w_date+2; k++) printf("-");
        printf("+"); for(int k=0; k<w_time+2; k++) printf("-");
        printf("+"); for(int k=0; k<w_price+2; k++) printf("-"); printf("+\n");

        printf("Masukkan Schedule ID (Ketik '0' untuk Batal) : ");
        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = '\0';

        // Fitur Back
        if (strcmp(input, "0") == 0) {
            menu_cust();
            return;
        }

        if (strlen(input) == 0) {
            printf("[ERROR] ID tidak boleh kosong!\n");
            system("pause");
            valid = 0;
            continue;
        }

        valid = 1;
        for (int i = 0; input[i] != '\0'; i++) {
            if (!isdigit(input[i])) { valid = 0; break; }
        }
        if (!valid) {
            printf("[ERROR] ID harus berupa angka!\n");
            system("pause");
            continue;
        }

        schedule_id = atoi(input);
        if (!find_schedule(schedule_id, &chosen_sch)) {
            printf("[ERROR] Jadwal dengan ID %d tidak ditemukan!\n", schedule_id);
            system("pause");
            valid = 0;
            continue;
        }
        if (chosen_sch.film_id != film_id) {
            printf("[ERROR] Jadwal ID %d bukan untuk film \"%s\"!\n", schedule_id, chosen_film->title);
            system("pause");
            valid = 0;
            continue;
        }
    } while (!valid);

    Studio chosen_studio;
    find_studio(chosen_sch.studio_id, &chosen_studio);


    /* -- LANGKAH 3: Pilih Kursi (Multi-seat) -- */
    char  selected_seats[MAX_SEATS_PER_ORDER][10];
    int   seat_count = 0;

    /* Loop Halaman PAGE 3: Pemilihan Kursi */
    while (seat_count < MAX_SEATS_PER_ORDER) {
        system("cls");
        printf("======================================================================================================\n");
        printf("                                              BOOK TICKET                                             \n");
        printf("======================================================================================================\n");
        printf("\n[ STEP 3 ] Pilih Kursi\n");
        printf("  Anda bisa memilih hingga %d kursi sekaligus.\n", MAX_SEATS_PER_ORDER);
        printf("  Ketik \"DONE\" jika sudah selesai memilih.\n");
        printf("  Ketik \"0\" untuk membatalkan pesanan dan kembali ke menu.\n");

        /* Tampilkan Denah Kursi Secara Update */
        display_seat_map(schedule_id, &chosen_studio);

        /* Info Kursi Yang Sudah Dipilih Sebelumnya */
        if (seat_count > 0) {
            printf("\n  Kursi dipilih (%d): ", seat_count);
            for (int i = 0; i < seat_count; i++)
                printf("%s%s", selected_seats[i], (i < seat_count - 1) ? ", " : "\n");
        } else {
            printf("\n");
        }

        char seat_input[20];
        printf("Masukkan kursi ke-%d (atau \"DONE\" / \"0\") : ", seat_count + 1);
        
        fgets(seat_input, sizeof(seat_input), stdin);
        seat_input[strcspn(seat_input, "\n")] = '\0';

        // Fitur Back
        if (strcmp(seat_input, "0") == 0) {
            menu_cust();
            return;
        }

        if (strlen(seat_input) == 0) {
            printf("[ERROR] Input kursi tidak boleh kosong!\n");
            system("pause");
            continue;
        }

        /* Konversi input ke huruf kapital */
        for (int i = 0; seat_input[i]; i++)
            seat_input[i] = toupper(seat_input[i]);

        if (strcmp(seat_input, "DONE") == 0) {
            if (seat_count == 0) {
                printf("[ERROR] Pilih minimal 1 kursi!\n");
                system("pause");
                continue;
            }
            break; // Lanjut ke Step 4
        }

        if (!validate_seat_format(seat_input, &chosen_studio)) {
            printf("[ERROR] Kursi \"%s\" tidak valid! Baris A-%c, Kolom 1-%d.\n",
                   seat_input, 'A' + chosen_studio.rows - 1, chosen_studio.cols);
            system("pause");
            continue;
        }

        if (is_seat_booked(schedule_id, seat_input)) {
            printf("[ERROR] Kursi %s sudah dibooking orang lain!\n", seat_input);
            system("pause");
            continue;
        }

        int duplicate = 0;
        for (int i = 0; i < seat_count; i++) {
            if (strcmp(selected_seats[i], seat_input) == 0) {
                duplicate = 1; break;
            }
        }
        if (duplicate) {
            printf("[ERROR] Kursi %s sudah Anda pilih sebelumnya!\n", seat_input);
            system("pause");
            continue;
        }

        strcpy(selected_seats[seat_count], seat_input);
        seat_count++;
        
        /* Optional: Memberikan jeda notifikasi bahwa kursi berhasil ditambahkan */
        printf("Kursi %s berhasil ditambahkan!\n", seat_input);
        system("pause");
    }


    /* -- LANGKAH 4: Konfirmasi Pembayaran -- */
    float total_harga = chosen_sch.price * seat_count;
    char confirm_str[10];
    char confirm_char;
    int conf_valid = 0;

    /* Loop Halaman PAGE 4: Konfirmasi Pembayaran */
    do {
        system("cls");
        printf("============================================\n");
        printf("           KONFIRMASI BOOKING               \n");
        printf("============================================\n");
        printf("  Film    : %s\n", chosen_film->title);
        printf("  Studio  : %s\n", chosen_studio.name);
        printf("  Tanggal : %s\n", chosen_sch.date);
        printf("  Jam     : %s\n", chosen_sch.time);
        printf("  Kursi   : ");
        for (int i = 0; i < seat_count; i++)
            printf("%s%s", selected_seats[i], (i < seat_count - 1) ? ", " : "\n");
        printf("  Jumlah  : %d kursi\n", seat_count);
        printf("  Harga   : Rp %.0f x %d = Rp %.0f\n", chosen_sch.price, seat_count, total_harga);
        printf("--------------------------------------------\n");
        printf("Konfirmasi booking? (Y/N) : ");

        fgets(confirm_str, sizeof(confirm_str), stdin);
        confirm_str[strcspn(confirm_str, "\n")] = '\0';

        if (strlen(confirm_str) == 0) {
            printf("[ERROR] Input tidak boleh kosong! Masukkan huruf Y atau N.\n");
            system("pause");
            continue;
        }

        // Pengecekan ketat (Strict Checking) huruf Y atau N
        if (strcmp(confirm_str, "Y") == 0 || strcmp(confirm_str, "y") == 0) {
            confirm_char = 'y';
            conf_valid = 1;
        } else if (strcmp(confirm_str, "N") == 0 || strcmp(confirm_str, "n") == 0) {
            confirm_char = 'n';
            conf_valid = 1;
        } else {
            printf("[ERROR] Input tidak valid! Masukkan hanya huruf Y atau N.\n");
            system("pause");
        }
    } while (!conf_valid);

    // Mengeksekusi aksi setelah loop validasi
    if (confirm_char == 'n') {
        printf("Booking dibatalkan.\n");
        system("pause");
        menu_cust();
        return;
    }


    /* -- LANGKAH 5: Simpan Data ke File -- */
    int base_id = auto_id_booking();
    char saved_codes[MAX_SEATS_PER_ORDER][20];

    FILE* bk_fp = fopen(booking_file, "a");
    if (bk_fp == NULL) { invalid_file(); return; }

    for (int i = 0; i < seat_count; i++) {
        generate_booking_code(base_id + i, saved_codes[i]);
        fprintf(bk_fp, "%s=%s=%d=%s=%.0f=1\n",
                saved_codes[i], current_user,
                schedule_id, selected_seats[i], chosen_sch.price);
    }
    fclose(bk_fp);


    /* -- TAMPILKAN STRUK/INVOICE BERHASIL -- */
    system("cls");
    printf("============================================\n");
    printf("          BOOKING BERHASIL!                 \n");
    printf("============================================\n");
    printf("  Film         : %s\n", chosen_film->title);
    printf("  Studio       : %s\n", chosen_studio.name);
    printf("  Tanggal      : %s\n", chosen_sch.date);
    printf("  Jam          : %s\n", chosen_sch.time);
    printf("  Total Harga  : Rp %.0f\n", total_harga);
    printf("--------------------------------------------\n");
    printf("  %-6s  %s\n", "Kursi", "Booking Code");
    printf("  ------  ----------------\n");
    for (int i = 0; i < seat_count; i++)
        printf("  %-6s  %s\n", selected_seats[i], saved_codes[i]);
    printf("============================================\n");
    printf("  Simpan setiap kode booking Anda!\n");
    printf("============================================\n");
    system("pause");
    menu_cust();
}

// ============================================================
// !! HISTORY !!
// ============================================================
void history() {
    system("cls");
    printf("============================================\n");
    printf("            MY BOOKING HISTORY              \n");
    printf("============================================\n");

    Booking bookings[MAX_BOOKINGS];
    int total = load_bookings(bookings, MAX_BOOKINGS);

    int found = 0;
    for (int i = 0; i < total; i++) {
        /* Tampilkan hanya booking milik current_user */
        if (strcmp(bookings[i].username, current_user) != 0) continue;

        /* Ambil detail jadwal, film, dan studio */
        Schedule sch;
        Film*  film   = NULL;
        Studio studio;
        char   studio_name[50]  = "Unknown";
        char   film_title[100]  = "Unknown";
        char   date_str[20]     = "-";
        char   time_str[20]     = "-";

        if (find_schedule(bookings[i].schedule_id, &sch)) {
            film = btree_search(film_tree, sch.film_id);
            if (film != NULL) strcpy(film_title, film->title);
            if (find_studio(sch.studio_id, &studio))
                strcpy(studio_name, studio.name);
            strcpy(date_str, sch.date);
            strcpy(time_str, sch.time);
        }

        /* Tentukan label status */
        const char* status_label = (bookings[i].status == 1) ? "ACTIVE" : "CANCELLED";

        printf("--------------------------------------------\n");
        printf("  Booking Code : %s\n", bookings[i].booking_code);
        printf("  Film         : %s\n", film_title);
        printf("  Studio       : %s\n", studio_name);
        printf("  Tanggal      : %s\n", date_str);
        printf("  Jam          : %s\n", time_str);
        printf("  Kursi        : %s\n", bookings[i].seat);
        printf("  Harga        : Rp %.0f\n", bookings[i].total_price);
        printf("  Status       : %s\n", status_label);
        found++;
    }

    if (found == 0) {
        printf("  Anda belum memiliki riwayat booking.\n");
    }

    printf("============================================\n");
    printf("  Total: %d booking(s)\n", found);
    printf("============================================\n");
    system("pause");
    menu_cust();
}

// ============================================================
// !! CANCEL BOOKING !!
// ============================================================
void cancel() {
    Booking bookings[MAX_BOOKINGS];
    int total = load_bookings(bookings, MAX_BOOKINGS);

    int active_idx[MAX_BOOKINGS];
    int active_count = 0;

    /* Hitung Lebar Kolom Dinamis Untuk Tabel */
    int w_code = 4, w_film = 4, w_date = 4, w_seat = 4, w_price = 5;

    for (int i = 0; i < total; i++) {
        if (strcmp(bookings[i].username, current_user) == 0 && bookings[i].status == 1) {
            active_idx[active_count] = i;
            active_count++;

            if ((int)strlen(bookings[i].booking_code) > w_code) w_code = strlen(bookings[i].booking_code);
            if ((int)strlen(bookings[i].seat) > w_seat) w_seat = strlen(bookings[i].seat);

            char price_str[30];
            sprintf(price_str, "Rp %.0f", bookings[i].total_price);
            if ((int)strlen(price_str) > w_price) w_price = strlen(price_str);

            Schedule sch;
            if (find_schedule(bookings[i].schedule_id, &sch)) {
                if ((int)strlen(sch.date) > w_date) w_date = strlen(sch.date);
                Film* f = btree_search(film_tree, sch.film_id);
                if (f != NULL) {
                    if ((int)strlen(f->title) > w_film) w_film = strlen(f->title);
                } else {
                    if (7 > w_film) w_film = 7; // Panjang kata "Unknown"
                }
            }
        }
    }

    char code_input[30];
    Booking target;
    int target_found = 0;

    /* ==========================================
       PAGE 1: TAMPILAN TABEL BOOKING
       ========================================== */
    do {
        system("cls");
        printf("==========================================================================\n");
        printf("                              CANCEL BOOKING                              \n");
        printf("==========================================================================\n");

        if (active_count == 0) {
            printf("\n  Tidak ada booking aktif untuk dibatalkan.\n");
            printf("==========================================================================\n");
            system("pause");
            menu_cust();
            return;
        }

        printf("\n[ Booking aktif Anda ]\n");

        /* Cetak Garis Atas Tabel */
        printf("+"); for(int k=0; k<w_code+2; k++) printf("-");
        printf("+"); for(int k=0; k<w_film+2; k++) printf("-");
        printf("+"); for(int k=0; k<w_date+2; k++) printf("-");
        printf("+"); for(int k=0; k<w_seat+2; k++) printf("-");
        printf("+"); for(int k=0; k<w_price+2; k++) printf("-"); printf("+\n");

        /* Cetak Header */
        printf("| %-*s | %-*s | %-*s | %-*s | %-*s |\n", w_code, "Code", w_film, "Film", w_date, "Date", w_seat, "Seat", w_price, "Price");

        /* Cetak Garis Tengah */
        printf("+"); for(int k=0; k<w_code+2; k++) printf("-");
        printf("+"); for(int k=0; k<w_film+2; k++) printf("-");
        printf("+"); for(int k=0; k<w_date+2; k++) printf("-");
        printf("+"); for(int k=0; k<w_seat+2; k++) printf("-");
        printf("+"); for(int k=0; k<w_price+2; k++) printf("-"); printf("+\n");

        /* Cetak Konten Tabel */
        for (int i = 0; i < active_count; i++) {
            int idx = active_idx[i];
            Schedule sch;
            char film_title[100] = "Unknown";
            char date_str[20]    = "-";

            if (find_schedule(bookings[idx].schedule_id, &sch)) {
                Film* f = btree_search(film_tree, sch.film_id);
                if (f != NULL) strcpy(film_title, f->title);
                strcpy(date_str, sch.date);
            }

            char price_str[30];
            sprintf(price_str, "Rp %.0f", bookings[idx].total_price);

            printf("| %-*s | %-*s | %-*s | %-*s | %-*s |\n",
                   w_code, bookings[idx].booking_code,
                   w_film, film_title,
                   w_date, date_str,
                   w_seat, bookings[idx].seat,
                   w_price, price_str);
        }

        /* Cetak Garis Bawah */
        printf("+"); for(int k=0; k<w_code+2; k++) printf("-");
        printf("+"); for(int k=0; k<w_film+2; k++) printf("-");
        printf("+"); for(int k=0; k<w_date+2; k++) printf("-");
        printf("+"); for(int k=0; k<w_seat+2; k++) printf("-");
        printf("+"); for(int k=0; k<w_price+2; k++) printf("-"); printf("+\n\n");

        printf("Masukkan Booking Code yang ingin dibatalkan\n");
        printf("(Ketik '0' untuk membatalkan dan kembali) : ");
        
        fgets(code_input, sizeof(code_input), stdin);
        code_input[strcspn(code_input, "\n")] = '\0';

        /* Fitur Back */
        if (strcmp(code_input, "0") == 0) {
            menu_cust();
            return;
        }

        if (strlen(code_input) == 0) {
            printf("[ERROR] Booking code tidak boleh kosong!\n");
            system("pause");
            continue;
        }

        /* Konversi input ke huruf kapital */
        for (int i = 0; code_input[i]; i++)
            code_input[i] = toupper(code_input[i]);

        if (!find_booking(code_input, &target)) {
            printf("[ERROR] Booking code \"%s\" tidak ditemukan!\n", code_input);
            system("pause");
            continue;
        }

        if (strcmp(target.username, current_user) != 0) {
            printf("[ERROR] Booking ini bukan milik Anda!\n");
            system("pause");
            continue;
        }

        if (target.status != 1) {
            printf("[ERROR] Booking %s sudah dibatalkan sebelumnya.\n", code_input);
            system("pause");
            continue;
        }

        target_found = 1;
    } while (!target_found);


    /* ==========================================
       PAGE 2: KONFIRMASI PEMBATALAN
       ========================================== */
    Schedule sch;
    char film_title[100]  = "Unknown";
    char studio_name[50]  = "Unknown";

    if (find_schedule(target.schedule_id, &sch)) {
        Film* f = btree_search(film_tree, sch.film_id);
        if (f != NULL) strcpy(film_title, f->title);
        Studio st;
        if (find_studio(sch.studio_id, &st)) strcpy(studio_name, st.name);
    }

    int conf_valid = 0;
    char confirm_char;
    char confirm_str[10];

    do {
        system("cls");
        printf("============================================\n");
        printf("           KONFIRMASI PEMBATALAN            \n");
        printf("============================================\n");
        printf("  Detail Booking:\n");
        printf("  Code    : %s\n", target.booking_code);
        printf("  Film    : %s\n", film_title);
        printf("  Studio  : %s\n", studio_name);
        printf("  Tanggal : %s\n", (find_schedule(target.schedule_id, &sch)) ? sch.date : "-");
        printf("  Kursi   : %s\n", target.seat);
        printf("  Harga   : Rp %.0f\n", target.total_price);
        printf("--------------------------------------------\n");
        printf("Yakin ingin membatalkan booking %s? (Y/N) : ", target.booking_code);

        fgets(confirm_str, sizeof(confirm_str), stdin);
        confirm_str[strcspn(confirm_str, "\n")] = '\0';

        if (strlen(confirm_str) == 0) {
            printf("[ERROR] Input tidak boleh kosong! Masukkan huruf Y atau N.\n");
            system("pause");
            continue;
        }

        if (strcmp(confirm_str, "Y") == 0 || strcmp(confirm_str, "y") == 0) {
            confirm_char = 'y';
            conf_valid = 1;
        } else if (strcmp(confirm_str, "N") == 0 || strcmp(confirm_str, "n") == 0) {
            confirm_char = 'n';
            conf_valid = 1;
        } else {
            printf("[ERROR] Input tidak valid! Masukkan hanya huruf Y atau N.\n");
            system("pause");
        }
    } while (!conf_valid);

    if (confirm_char == 'n') {
        printf("Pembatalan diurungkan.\n");
        system("pause");
        menu_cust();
        return;
    }

    /* Proses Tulis Ulang File (Ubah status = 0) */
    FILE* in  = fopen(booking_file, "r");
    FILE* tmp = fopen("temp_booking.txt", "w");
    if (in == NULL || tmp == NULL) { invalid_file(); return; }

    char buf[300];
    while (fgets(buf, sizeof(buf), in)) {
        buf[strcspn(buf, "\n")] = 0;
        Booking bk;
        sscanf(buf, "%[^=]=%[^=]=%d=%[^=]=%f=%d",
               bk.booking_code, bk.username,
               &bk.schedule_id, bk.seat,
               &bk.total_price, &bk.status);

        if (strcmp(bk.booking_code, code_input) == 0) {
            fprintf(tmp, "%s=%s=%d=%s=%.0f=0\n",
                    bk.booking_code, bk.username,
                    bk.schedule_id, bk.seat, bk.total_price);
        } else {
            fprintf(tmp, "%s\n", buf);
        }
    }
    fclose(in);
    fclose(tmp);
    remove(booking_file);
    rename("temp_booking.txt", booking_file);

    /* ==========================================
       PAGE 3: SUCCESS INVOICE
       ========================================== */
    system("cls");
    printf("============================================\n");
    printf("          PEMBATALAN BERHASIL!              \n");
    printf("============================================\n");
    printf("\n");
    printf("  Booking %s berhasil dibatalkan.\n", code_input);
    printf("  Kursi %s sekarang tersedia kembali.\n", target.seat);
    printf("\n");
    printf("============================================\n");
    system("pause");
    menu_cust();
}

// ============================================================
// !! EDIT PROFILE !!
// ============================================================
void edit_profile() {
    char input[100];
    char lower_input[100];
    int choice;
    int valid;

    do {
        system("cls");
        printf("============================================\n");
        printf("                EDIT PROFILE                \n");
        printf("--------------------------------------------\n");
        printf("[1] View Profile\n");
        printf("[2] Change Full Name\n");
        printf("[3] Change Username\n");
        printf("[4] Change Email\n");
        printf("[5] Change Password\n");
        printf("[6] Delete Account\n");
        printf("[0] Back\n");
        printf("============================================\n");
        printf("Choose : ");
        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = '\0';

        // Mengonversi input menjadi lowercase
        for (int i = 0; input[i] != '\0'; i++) {
            lower_input[i] = tolower(input[i]);
        }
        lower_input[strlen(input)] = '\0';

        valid = 1;

        // Pengecekan kondisi berdasarkan angka atau teks
        if (strcmp(lower_input, "1") == 0 || strcmp(lower_input, "view profile") == 0) {
            choice = 1;
        } else if (strcmp(lower_input, "2") == 0 || strcmp(lower_input, "change full name") == 0) {
            choice = 2;
        } else if (strcmp(lower_input, "3") == 0 || strcmp(lower_input, "change username") == 0) {
            choice = 3;
        } else if (strcmp(lower_input, "4") == 0 || strcmp(lower_input, "change email") == 0) {
            choice = 4;
        } else if (strcmp(lower_input, "5") == 0 || strcmp(lower_input, "change password") == 0) {
            choice = 5;
        } else if (strcmp(lower_input, "6") == 0 || strcmp(lower_input, "delete account") == 0) {
            choice = 6;
        } else if (strcmp(lower_input, "0") == 0 || strcmp(lower_input, "back") == 0) {
            choice = 0;
        } else {
            // Jika input kosong atau di luar opsi
            printf("\n[ERROR] Invalid Input!\n");
            printf("Please enter the number (0-6) or the exact option text.\n");
            system("pause");
            valid = 0;
        }
    } while (!valid);

    switch (choice) {
        case 1: view_profile(); break;
        case 2: change_name();  break;
        case 3: change_usn();   break;
        case 4: change_email(); break;
        case 5: change_pass();  break;
        case 6: delete_account_cust(); break;
        case 0: menu_cust();    break;
    }
}

// ============================================================
// !! EDIT PROFILE FUNCTIONS !!
// ============================================================

void change_name() {
    char buffer[1024];
    Account acc;
    char current_name[100] = "";
    char new_name[100];
    int valid = 0;

    FILE* check = fopen(account_file, "r");
    if (check != NULL) {
        while (fgets(buffer, sizeof(buffer), check)) {
            buffer[strcspn(buffer, "\n")] = 0;
            sscanf(buffer, "%[^,],%[^,],%[^,],%[^\n]",
                   acc.username, acc.password, acc.name, acc.email);
            if (strcmp(acc.username, current_user) == 0) {
                strcpy(current_name, acc.name); break;
            }
        }
        fclose(check);
    }

    // Pindahkan UI dan cls ke dalam do-while loop
    do {
        system("cls");
        printf("============================================\n");
        printf("              CHANGE FULL NAME              \n");
        printf("============================================\n");
        printf("Current Full Name : %s\n", current_name);
        printf("--------------------------------------------\n");
        printf(" (Enter '0' to go back)\n");
        printf("--------------------------------------------\n");
        printf("Enter new Full Name : ");
        
        fgets(new_name, sizeof(new_name), stdin);
        new_name[strcspn(new_name, "\n")] = '\0';

        if (strcmp(new_name, "0") == 0) {
            edit_profile();
            return;
        }
        if (strlen(new_name) == 0) {
            printf("Full Name cannot be empty!\n");
            system("pause"); 
            continue; // Ulangi loop tanpa kembali ke menu
        }
        if (strcmp(current_name, new_name) == 0) {
            printf("New Full Name cannot be the same as current!\n");
            system("pause"); 
            continue;
        }
        
        valid = 1; // Asumsi valid
        for (int i = 0; new_name[i] != '\0'; i++) {
            if (isdigit(new_name[i])) {
                printf("Full name cannot contain numbers!\n");
                system("pause"); 
                valid = 0; 
                break;
            }
        }
    } while (!valid);

    FILE* data = fopen(account_file, "r");
    FILE* temp = fopen("temp.txt", "w");
    if (!data || !temp) { invalid_file(); return; }
    while (fgets(buffer, sizeof(buffer), data)) {
        buffer[strcspn(buffer, "\n")] = 0;
        sscanf(buffer, "%[^,],%[^,],%[^,],%[^\n]",
               acc.username, acc.password, acc.name, acc.email);
        if (strcmp(acc.username, current_user) == 0) strcpy(acc.name, new_name);
        fprintf(temp, "%s,%s,%s,%s\n",
                acc.username, acc.password, acc.name, acc.email);
    }
    fclose(data); fclose(temp);
    remove(account_file); rename("temp.txt", account_file);

    printf("Full Name successfully updated!\n");
    system("pause");
    edit_profile();
}


void change_usn() {
    char buffer[1024];
    Account acc;
    char new_usn[100];
    int valid = 0;

    do {
        system("cls");
        printf("============================================\n");
        printf("              CHANGE USERNAME               \n");
        printf("============================================\n");
        printf("Current Username : %s\n", current_user);
        printf("--------------------------------------------\n");
        printf(" (Enter '0' to go back)\n");
        printf("--------------------------------------------\n");
        printf("Enter new Username : ");
        
        fgets(new_usn, sizeof(new_usn), stdin);
        new_usn[strcspn(new_usn, "\n")] = '\0';
        
        if (strcmp(new_usn, "0") == 0) {
            edit_profile();
            return;
        }
        if (strlen(new_usn) == 0) {
            printf("Username cannot be empty!\n");
            system("pause"); 
            continue;
        }
        if (strcmp(current_user, new_usn) == 0) {
            printf("New Username cannot be the same as current!\n");
            system("pause"); 
            continue;
        }
        
        int has_space = 0;
        for (int i = 0; new_usn[i] != '\0'; i++) {
            if (isspace(new_usn[i])) { has_space = 1; break; }
        }
        if (has_space) { 
            printf("Username cannot contain spaces!\n"); 
            system("pause"); 
            continue; 
        }

        int found = 0;
        FILE* chk = fopen(account_file, "r");
        if (chk != NULL) {
            char tb[1024], cu[100];
            while (fgets(tb, sizeof(tb), chk)) {
                sscanf(tb, "%[^,]", cu);
                if (strcmp(cu, new_usn) == 0) {
                    found = 1; 
                    break;
                }
            }
            fclose(chk);
        }
        
        if (found) {
            printf("Username already exists!\n");
            system("pause");
            continue;
        }
        
        valid = 1;
    } while (!valid);

    FILE* data = fopen(account_file, "r");
    FILE* temp = fopen("temp.txt", "w");
    if (!data || !temp) { invalid_file(); return; }
    while (fgets(buffer, sizeof(buffer), data)) {
        buffer[strcspn(buffer, "\n")] = 0;
        sscanf(buffer, "%[^,],%[^,],%[^,],%[^\n]",
               acc.username, acc.password, acc.name, acc.email);
        if (strcmp(acc.username, current_user) == 0) strcpy(acc.username, new_usn);
        fprintf(temp, "%s,%s,%s,%s\n",
                acc.username, acc.password, acc.name, acc.email);
    }
    fclose(data); fclose(temp);
    remove(account_file); rename("temp.txt", account_file);
    strcpy(current_user, new_usn);

    printf("Username successfully updated!\n");
    system("pause");
    edit_profile();
}


void change_email() {
    char buffer[1024];
    Account acc;
    char current_email[100] = "";
    char new_email[100];
    char confirm_email[100];
    int valid = 0;

    FILE* check = fopen(account_file, "r");
    if (check != NULL) {
        while (fgets(buffer, sizeof(buffer), check)) {
            buffer[strcspn(buffer, "\n")] = 0;
            sscanf(buffer, "%[^,],%[^,],%[^,],%[^\n]",
                   acc.username, acc.password, acc.name, acc.email);
            if (strcmp(acc.username, current_user) == 0) {
                strcpy(current_email, acc.email); break;
            }
        }
        fclose(check);
    }

    do {
        system("cls");
        printf("============================================\n");
        printf("                CHANGE EMAIL                \n");
        printf("============================================\n");
        printf("Current Email : %s\n", current_email);
        printf("--------------------------------------------\n");
        printf(" (Enter '0' to go back)\n");
        printf("--------------------------------------------\n");
        printf("Enter new Email : ");
        
        fgets(new_email, sizeof(new_email), stdin);
        new_email[strcspn(new_email, "\n")] = '\0';
        
        if (strcmp(new_email, "0") == 0) {
            edit_profile();
            return;
        }
        if (strlen(new_email) == 0) {
            printf("Email cannot be empty!\n");
            system("pause"); 
            continue;
        }
        if (strcmp(current_email, new_email) == 0) {
            printf("New Email cannot be the same as current!\n");
            system("pause"); 
            continue;
        }
        
        int validasi_at = 0;
        for (int i = 0; new_email[i] != '\0'; i++) {
            if (new_email[i] == '@') { validasi_at = 1; break; }
        }
        if (!validasi_at) { 
            printf("Email must contain '@'!\n"); 
            system("pause"); 
            continue; 
        }

        printf("Confirm new Email : ");
        fgets(confirm_email, sizeof(confirm_email), stdin);
        confirm_email[strcspn(confirm_email, "\n")] = '\0';
        
        if (strcmp(confirm_email, "0") == 0) {
            edit_profile();
            return;
        }
        
        if (strcmp(new_email, confirm_email) != 0) {
            printf("Email does not match! Try again!\n");
            system("pause");
            continue;
        }
        
        valid = 1;
    } while (!valid);

    FILE* data = fopen(account_file, "r");
    FILE* temp = fopen("temp.txt", "w");
    if (!data || !temp) { invalid_file(); return; }
    while (fgets(buffer, sizeof(buffer), data)) {
        buffer[strcspn(buffer, "\n")] = 0;
        sscanf(buffer, "%[^,],%[^,],%[^,],%[^\n]",
               acc.username, acc.password, acc.name, acc.email);
        if (strcmp(acc.username, current_user) == 0) strcpy(acc.email, new_email);
        fprintf(temp, "%s,%s,%s,%s\n",
                acc.username, acc.password, acc.name, acc.email);
    }
    fclose(data); fclose(temp);
    remove(account_file); rename("temp.txt", account_file);

    printf("Email successfully updated!\n");
    system("pause");
    edit_profile();
}


void change_pass() {
    char buffer[1024];
    Account acc;
    char current_pass[100] = "";
    char new_pass[100];
    char confirm_pass[100];
    int valid = 0;

    FILE* check = fopen(account_file, "r");
    if (check != NULL) {
        while (fgets(buffer, sizeof(buffer), check)) {
            buffer[strcspn(buffer, "\n")] = 0;
            sscanf(buffer, "%[^,],%[^,],%[^,],%[^\n]",
                   acc.username, acc.password, acc.name, acc.email);
            if (strcmp(acc.username, current_user) == 0) {
                strcpy(current_pass, acc.password); break;
            }
        }
        fclose(check);
    }

    do {
        system("cls");
        printf("============================================\n");
        printf("              CHANGE PASSWORD               \n");
        printf("============================================\n");
        printf("Current Password : ");
        for (int i = 0; i < (int)strlen(current_pass); i++) printf("*");
        printf("\n--------------------------------------------\n");
        printf(" (Enter '0' to go back)\n");
        printf("--------------------------------------------\n");
        printf("Enter new Password : ");
        
        fgets(new_pass, sizeof(new_pass), stdin);
        new_pass[strcspn(new_pass, "\n")] = '\0';
        
        if (strcmp(new_pass, "0") == 0) {
            edit_profile();
            return;
        }
        if (strlen(new_pass) == 0) {
            printf("Password cannot be empty!\n");
            system("pause"); 
            continue;
        }
        if (strcmp(current_pass, new_pass) == 0) {
            printf("New Password cannot be the same as current!\n");
            system("pause"); 
            continue;
        }
        if (strlen(new_pass) < 5) {
            printf("Password must be at least 5 characters long!\n"); 
            system("pause"); 
            continue;
        }
        
        printf("Confirm new Password : ");
        fgets(confirm_pass, sizeof(confirm_pass), stdin);
        confirm_pass[strcspn(confirm_pass, "\n")] = '\0';
        
        if (strcmp(confirm_pass, "0") == 0) {
            edit_profile();
            return;
        }
        
        if (strcmp(new_pass, confirm_pass) != 0) {
            printf("Password does not match! Try again!\n");
            system("pause"); 
            continue;
        }
        
        valid = 1;
    } while (!valid);

    FILE* data = fopen(account_file, "r");
    FILE* temp = fopen("temp.txt", "w");
    if (!data || !temp) { invalid_file(); return; }
    while (fgets(buffer, sizeof(buffer), data)) {
        buffer[strcspn(buffer, "\n")] = 0;
        sscanf(buffer, "%[^,],%[^,],%[^,],%[^\n]",
               acc.username, acc.password, acc.name, acc.email);
        if (strcmp(acc.username, current_user) == 0) strcpy(acc.password, new_pass);
        fprintf(temp, "%s,%s,%s,%s\n",
                acc.username, acc.password, acc.name, acc.email);
    }
    fclose(data); fclose(temp);
    remove(account_file); rename("temp.txt", account_file);

    printf("Password successfully updated!\n");
    system("pause");
    edit_profile();
}


void delete_account_cust() {
    char buffer[1024];
    Account acc;
    char current_pass[100] = "";
    char input_pass[100];
    char confirm_pass_input[100]; 
    char confirm[100];
    char lower_confirm[100];
    int pass_match = 0;
    int confirmed = 0;

    FILE* check = fopen(account_file, "r");
    if (check != NULL) {
        while (fgets(buffer, sizeof(buffer), check)) {
            buffer[strcspn(buffer, "\n")] = 0;
            sscanf(buffer, "%[^,],%[^,],%[^,],%[^\n]", 
                   acc.username, acc.password, acc.name, acc.email);
            if (strcmp(acc.username, current_user) == 0) {
                strcpy(current_pass, acc.password);
                break;
            }
        }
        fclose(check);
    }

    do {
        system("cls");
        printf("============================================\n");
        printf("               DELETE ACCOUNT               \n");
        printf("============================================\n");
        printf(" [WARNING] This action cannot be undone!    \n");
        printf("--------------------------------------------\n");
        printf(" (Enter '0' at any prompt to go back)\n");
        printf("--------------------------------------------\n");

        printf("Enter your password : ");
        fgets(input_pass, sizeof(input_pass), stdin);
        input_pass[strcspn(input_pass, "\n")] = '\0';

        if (strcmp(input_pass, "0") == 0) {
            edit_profile();
            return;
        }

        if (strlen(input_pass) == 0) {
            printf("Password cannot be empty!\n");
            system("pause");
            continue;
        }

        printf("Confirm password    : ");
        fgets(confirm_pass_input, sizeof(confirm_pass_input), stdin);
        confirm_pass_input[strcspn(confirm_pass_input, "\n")] = '\0';

        if (strcmp(confirm_pass_input, "0") == 0) {
            edit_profile();
            return;
        }

        if (strcmp(input_pass, confirm_pass_input) != 0) {
            printf("\nPasswords do not match! Please try again.\n");
            system("pause");
            continue;
        }

        if (strcmp(input_pass, current_pass) != 0) {
            printf("\nIncorrect password! Please try again.\n");
            system("pause");
            continue;
        }

        pass_match = 1; 
    } while (!pass_match);


    do {
        system("cls");
        printf("============================================\n");
        printf("             FINAL CONFIRMATION             \n");
        printf("============================================\n");
        printf(" Are you absolutely sure you want to delete \n");
        printf(" your account? All data will be lost.       \n");
        printf("--------------------------------------------\n");
        printf(" Type 'Yes' to permanently delete.\n");
        printf(" Type 'No' or '0' to cancel and go back.\n");
        printf("--------------------------------------------\n");
        printf("Your Choice : ");

        fgets(confirm, sizeof(confirm), stdin);
        confirm[strcspn(confirm, "\n")] = '\0';

        if (strlen(confirm) == 0) {
            printf("\nInput cannot be empty!\n");
            system("pause");
            continue;
        }

        for (int i = 0; confirm[i] != '\0'; i++) {
            lower_confirm[i] = tolower(confirm[i]);
        }
        lower_confirm[strlen(confirm)] = '\0';

        if (strcmp(lower_confirm, "yes") == 0) {
            confirmed = 1;
        } else if (strcmp(lower_confirm, "no") == 0 || strcmp(lower_confirm, "0") == 0) {
            printf("\nAccount deletion cancelled.\n");
            system("pause");
            edit_profile();
            return;
        } else {
            printf("\nInvalid input. Please type 'Yes', 'No', or '0'.\n");
            system("pause");
        }
    } while (!confirmed);

    FILE* data = fopen(account_file, "r");
    FILE* temp = fopen("temp.txt", "w");
    if (!data || !temp) { invalid_file(); return; }

    while (fgets(buffer, sizeof(buffer), data)) {
        buffer[strcspn(buffer, "\n")] = 0;
        sscanf(buffer, "%[^,],%[^,],%[^,],%[^\n]", 
               acc.username, acc.password, acc.name, acc.email);
        
        if (strcmp(acc.username, current_user) != 0) {
            fprintf(temp, "%s,%s,%s,%s\n", 
                    acc.username, acc.password, acc.name, acc.email);
        }
    }

    fclose(data);
    fclose(temp);
    remove(account_file);
    rename("temp.txt", account_file);

    memset(current_user, 0, sizeof(current_user));

    system("cls");
    printf("============================================\n");
    printf("               ACCOUNT DELETED              \n");
    printf("============================================\n");
    printf("\n");
    printf("       Account deleted successfully!        \n");
    printf("\n");
    printf("============================================\n");
    system("pause");
    
    main_menu();
}

void view_profile() {
    system("cls");
    char buffer[1024];
    Account acc;
    int found = 0;

    FILE* check = fopen(account_file, "r");
    if (check != NULL) {
        while (fgets(buffer, sizeof(buffer), check)) {
            buffer[strcspn(buffer, "\n")] = 0;
            sscanf(buffer, "%[^,],%[^,],%[^,],%[^\n]",
                   acc.username, acc.password, acc.name, acc.email);
            if (strcmp(acc.username, current_user) == 0) { found = 1; break; }
        }
        fclose(check);
    }

    printf("============================================\n");
    printf("                MY PROFILE                  \n");
    printf("============================================\n");
    if (found) {
        printf("Full Name : %s\n", acc.name);
        printf("Username  : %s\n", acc.username);
        printf("Email     : %s\n", acc.email);
        printf("Password  : ");
        for (int i = 0; i < (int)strlen(acc.password); i++) printf("*");
        printf("\n");
    } else {
        printf("Error: Profile data not found!\n");
    }
    printf("============================================\n");
    system("pause");
    edit_profile();
}

// ============================================================
// !! CASHIER MENU !!
// ============================================================
void menu_cashier() {
    int choice;
    btree_load_from_file();
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

// ============================================================
// !! VALIDATE TICKET (Cashier) !!
// !! Cashier memasukkan booking code untuk check-in         !!
// ============================================================
void validate_ticket() {
    system("cls");
    printf("============================================\n");
    printf("          VALIDATE TICKET (CHECK-IN)        \n");
    printf("============================================\n");

    char code_input[20];
    printf("Masukkan Booking Code : ");
    scanf("%s", code_input);
    for (int i = 0; code_input[i]; i++)
        code_input[i] = toupper(code_input[i]);

    Booking bk;
    if (!find_booking(code_input, &bk)) {
        printf("--------------------------------------------\n");
        printf("Booking code \"%s\" tidak ditemukan!\n", code_input);
        printf("============================================\n");
        system("pause");
        menu_cashier();
        return;
    }

    /* Ambil detail */
    Schedule sch;
    char film_title[100]  = "Unknown";
    char studio_name[50]  = "Unknown";
    if (find_schedule(bk.schedule_id, &sch)) {
        Film* f = btree_search(film_tree, sch.film_id);
        if (f != NULL) strcpy(film_title, f->title);
        Studio st;
        if (find_studio(sch.studio_id, &st)) strcpy(studio_name, st.name);
    }

    printf("--------------------------------------------\n");
    printf("  Booking Code : %s\n", bk.booking_code);
    printf("  Username     : %s\n", bk.username);
    printf("  Film         : %s\n", film_title);
    printf("  Studio       : %s\n", studio_name);
    printf("  Tanggal      : %s\n", sch.date);
    printf("  Jam          : %s\n", sch.time);
    printf("  Kursi        : %s\n", bk.seat);
    printf("  Harga        : Rp %.0f\n", bk.total_price);
    printf("  Status       : %s\n", bk.status == 1 ? "ACTIVE" : "CANCELLED");
    printf("--------------------------------------------\n");

    if (bk.status == 1) {
        printf("  [OK] Tiket VALID. Persilakan masuk.\n");
    } else {
        printf("  [TOLAK] Tiket sudah DIBATALKAN!\n");
    }
    printf("============================================\n");
    system("pause");
    menu_cashier();
}

// ============================================================
// !! SELL TICKET (Cashier) !!
// !! Cashier bisa memesan tiket atas nama customer offline  !!
// ============================================================
void sell() {
    system("cls");
    printf("============================================\n");
    printf("               SELL TICKET                  \n");
    printf("============================================\n");

    /* -- STEP 1: Pilih Film -- */
    Film all_films[200];
    int film_count = 0;
    btree_inorder(film_tree, all_films, &film_count);

    if (film_count == 0) {
        printf("Tidak ada film tersedia.\n");
        system("pause");
        menu_cashier();
        return;
    }

    printf("\n[ STEP 1 ] Pilih Film\n");
    printf("%-5s %-25s %-12s %-6s\n", "ID", "Title", "Genre", "Min");
    printf("--------------------------------------------\n");
    for (int i = 0; i < film_count; i++)
        printf("%-5d %-25s %-12s %d min\n",
               all_films[i].id, all_films[i].title,
               all_films[i].genre, all_films[i].duration);
    printf("--------------------------------------------\n");

    char input[20];
    int film_id, valid;

    do {
        printf("Masukkan Film ID : ");
        scanf("%s", input);
        valid = 1;
        for (int i = 0; input[i]; i++)
            if (!isdigit(input[i])) { valid = 0; break; }
        if (!valid) { printf("ID harus angka!\n"); continue; }
        film_id = atoi(input);
        if (btree_search(film_tree, film_id) == NULL) {
            printf("Film ID %d tidak ditemukan!\n", film_id);
            valid = 0;
        }
    } while (!valid);

    Film* chosen_film = btree_search(film_tree, film_id);

    /* -- STEP 2: Pilih Jadwal -- */
    printf("\n[ STEP 2 ] Pilih Jadwal untuk \"%s\"\n", chosen_film->title);
    printf("%-4s %-15s %-12s %-8s %s\n",
           "ID", "Studio", "Date", "Time", "Price");
    printf("--------------------------------------------\n");

    FILE* sfp = fopen(schedule_file, "r");
    if (sfp == NULL) {
        printf("Tidak ada jadwal.\n");
        system("pause"); menu_cashier(); return;
    }
    int sch_count = 0;
    char sch_buf[300];
    while (fgets(sch_buf, sizeof(sch_buf), sfp)) {
        sch_buf[strcspn(sch_buf, "\n")] = 0;
        Schedule sch;
        sscanf(sch_buf, "%d=%d=%d=%[^=]=%[^=]=%f",
               &sch.id, &sch.film_id, &sch.studio_id,
               sch.date, sch.time, &sch.price);
        if (sch.film_id != film_id) continue;
        Studio st; find_studio(sch.studio_id, &st);
        printf("%-4d %-15s %-12s %-8s Rp %.0f\n",
               sch.id, st.name, sch.date, sch.time, sch.price);
        sch_count++;
    }
    fclose(sfp);

    if (sch_count == 0) {
        printf("Tidak ada jadwal untuk film ini.\n");
        system("pause"); menu_cashier(); return;
    }
    printf("--------------------------------------------\n");

    int schedule_id;
    Schedule chosen_sch;

    do {
        printf("Masukkan Schedule ID : ");
        scanf("%s", input);
        valid = 1;
        for (int i = 0; input[i]; i++)
            if (!isdigit(input[i])) { valid = 0; break; }
        if (!valid) { printf("ID harus angka!\n"); continue; }
        schedule_id = atoi(input);
        if (!find_schedule(schedule_id, &chosen_sch)) {
            printf("Jadwal ID %d tidak ditemukan!\n", schedule_id);
            valid = 0; continue;
        }
        if (chosen_sch.film_id != film_id) {
            printf("Jadwal ini bukan untuk film yang dipilih!\n");
            valid = 0;
        }
    } while (!valid);

    Studio chosen_studio;
    find_studio(chosen_sch.studio_id, &chosen_studio);

    /* -- STEP 3: Pilih Kursi (multi-seat) -- */
    char selected_seats[MAX_SEATS_PER_ORDER][10];
    int  seat_count = 0;

    printf("\n[ STEP 3 ] Pilih Kursi\n");
    printf("  Cashier bisa memilih hingga %d kursi sekaligus.\n", MAX_SEATS_PER_ORDER);
    printf("  Ketik \"DONE\" jika sudah selesai.\n\n");

    while (seat_count < MAX_SEATS_PER_ORDER) {
        display_seat_map(schedule_id, &chosen_studio);

        if (seat_count > 0) {
            printf("  Kursi dipilih (%d): ", seat_count);
            for (int i = 0; i < seat_count; i++)
                printf("%s%s", selected_seats[i],
                               (i < seat_count - 1) ? ", " : "\n");
        }

        char seat_input[10];
        printf("Masukkan kursi ke-%d (atau \"DONE\") : ", seat_count + 1);
        scanf("%s", seat_input);

        for (int i = 0; seat_input[i]; i++)
            seat_input[i] = toupper(seat_input[i]);

        if (strcmp(seat_input, "DONE") == 0) {
            if (seat_count == 0) {
                printf("  Pilih minimal 1 kursi!\n\n");
                continue;
            }
            break;
        }

        if (!validate_seat_format(seat_input, &chosen_studio)) {
            printf("  Kursi \"%s\" tidak valid! Baris A-%c, Kolom 1-%d.\n\n",
                   seat_input,
                   'A' + chosen_studio.rows - 1,
                   chosen_studio.cols);
            continue;
        }

        if (is_seat_booked(schedule_id, seat_input)) {
            printf("  Kursi %s sudah dipesan!\n\n", seat_input);
            continue;
        }

        /* Cek duplikat dalam sesi ini */
        int duplicate = 0;
        for (int i = 0; i < seat_count; i++) {
            if (strcmp(selected_seats[i], seat_input) == 0) {
                duplicate = 1; break;
            }
        }
        if (duplicate) {
            printf("  Kursi %s sudah Anda pilih sebelumnya!\n\n", seat_input);
            continue;
        }

        strcpy(selected_seats[seat_count], seat_input);
        seat_count++;
        printf("  Kursi %s ditambahkan.\n\n", seat_input);
    }

    /* -- STEP 4: Username Customer -- */
    char cust_username[100];
    printf("\n[ STEP 4 ] Username Customer\n");
    printf("Masukkan username customer : ");
    scanf("%s", cust_username);

    /* -- STEP 5: Konfirmasi -- */
    float total_harga = chosen_sch.price * seat_count;

    system("cls");
    printf("============================================\n");
    printf("           KONFIRMASI PENJUALAN             \n");
    printf("============================================\n");
    printf("  Film     : %s\n", chosen_film->title);
    printf("  Studio   : %s\n", chosen_studio.name);
    printf("  Tanggal  : %s\n", chosen_sch.date);
    printf("  Jam      : %s\n", chosen_sch.time);
    printf("  Kursi    : ");
    for (int i = 0; i < seat_count; i++)
        printf("%s%s", selected_seats[i],
                       (i < seat_count - 1) ? ", " : "\n");
    printf("  Jumlah   : %d kursi\n", seat_count);
    printf("  Customer : %s\n", cust_username);
    printf("  Harga    : Rp %.0f x %d = Rp %.0f\n",
           chosen_sch.price, seat_count, total_harga);
    printf("--------------------------------------------\n");
    printf("  Konfirmasi penjualan? (Y/N) : ");

    char confirm;
    scanf(" %c", &confirm);
    if (confirm != 'Y' && confirm != 'y') {
        printf("  Penjualan dibatalkan.\n");
        system("pause"); menu_cashier(); return;
    }

    /* -- STEP 6: Simpan Booking -- */
    // Ambil base_id SEBELUM fopen agar tidak ada race condition
    int  base_id = auto_id_booking();
    char saved_codes[MAX_SEATS_PER_ORDER][20];

    FILE* bk_fp = fopen(booking_file, "a");
    if (bk_fp == NULL) { invalid_file(); return; }

    for (int i = 0; i < seat_count; i++) {
        generate_booking_code(base_id + i, saved_codes[i]);
        fprintf(bk_fp, "%s=%s=%d=%s=%.0f=1\n",
                saved_codes[i], cust_username,
                schedule_id, selected_seats[i], chosen_sch.price);
    }
    fclose(bk_fp);

    /* -- Tampilkan Hasil -- */
    system("cls");
    printf("============================================\n");
    printf("          TIKET BERHASIL DIJUAL!            \n");
    printf("============================================\n");
    printf("  Film         : %s\n", chosen_film->title);
    printf("  Studio       : %s\n", chosen_studio.name);
    printf("  Tanggal      : %s\n", chosen_sch.date);
    printf("  Jam          : %s\n", chosen_sch.time);
    printf("  Customer     : %s\n", cust_username);
    printf("  Total Harga  : Rp %.0f\n", total_harga);
    printf("--------------------------------------------\n");
    printf("  %-6s  %s\n", "Kursi", "Booking Code");
    printf("  ------  ----------------\n");
    for (int i = 0; i < seat_count; i++)
        printf("  %-6s  %s\n", selected_seats[i], saved_codes[i]);
    printf("============================================\n");
    system("pause");
    menu_cashier();
}

// ============================================================
// !! SEAT STATUS (Cashier) !!
// !! Tampilkan status kursi untuk jadwal tertentu           !!
// ============================================================
void seat_status() {
    system("cls");
    printf("============================================\n");
    printf("           SEAT STATUS PER SCHEDULE         \n");
    printf("============================================\n");

    /* Tampilkan daftar jadwal */
    FILE* fp = fopen(schedule_file, "r");
    if (fp == NULL) {
        printf("Tidak ada jadwal tersedia.\n");
        system("pause"); menu_cashier(); return;
    }

    char buffer[300];
    int count = 0;
    printf("%-4s %-22s %-12s %-12s %-8s %s\n",
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
        printf("%-4d %-22s %-12s %-12s %-8s Rp %.0f\n",
               sch.id, f ? f->title : "Unknown", st.name,
               sch.date, sch.time, sch.price);
        count++;
    }
    fclose(fp);

    if (count == 0) {
        printf("Tidak ada jadwal.\n");
        system("pause"); menu_cashier(); return;
    }
    printf("--------------------------------------------\n");

    char input[20];
    int sch_id, valid;

    do {
        printf("Masukkan Schedule ID : ");
        scanf("%s", input);
        valid = 1;
        for (int i = 0; input[i]; i++)
            if (!isdigit(input[i])) { valid = 0; break; }
        if (!valid) { printf("ID harus angka!\n"); continue; }
        sch_id = atoi(input);
        Schedule tmp;
        if (!find_schedule(sch_id, &tmp)) {
            printf("Schedule ID %d tidak ditemukan!\n", sch_id);
            valid = 0;
        }
    } while (!valid);

    Schedule chosen_sch;
    find_schedule(sch_id, &chosen_sch);
    Studio chosen_studio;
    find_studio(chosen_sch.studio_id, &chosen_studio);
    Film* f = btree_search(film_tree, chosen_sch.film_id);

    printf("\n  Film    : %s\n", f ? f->title : "Unknown");
    printf("  Studio  : %s\n", chosen_studio.name);
    printf("  Tanggal : %s  Jam: %s\n", chosen_sch.date, chosen_sch.time);

    display_seat_map(sch_id, &chosen_studio);

    /* Hitung ringkasan */
    int total_seats  = chosen_studio.rows * chosen_studio.cols;
    int booked_seats = 0;
    for (int r = 0; r < chosen_studio.rows; r++) {
        char row_char = 'A' + r;
        for (int c = 1; c <= chosen_studio.cols; c++) {
            char seat[10];
            sprintf(seat, "%c%d", row_char, c);
            if (is_seat_booked(sch_id, seat)) booked_seats++;
        }
    }

    printf("  Total Kursi   : %d\n", total_seats);
    printf("  Terisi        : %d\n", booked_seats);
    printf("  Tersedia      : %d\n", total_seats - booked_seats);
    printf("============================================\n");
    system("pause");
    menu_cashier();
}


// ============================================================
// MAIN
// ============================================================
int main() {
    XXI_Banner();

    printf("\n" CENTER_PAD "\033[2;33m");
    for (int i = 0; i < 58; i++)
        printf("-");
    printf("\033[0m\n\n");

    /* Loading Bar */
    loading_bar(100, "Loading...");

    /* Pesan sukses */
    printf("\n" CENTER_PAD "\033[1;32m? Welcome. System fully loaded.\033[0m\n\n");

    getchar(); // Tunggu user menekan Enter

    /* Bersihkan layar */
    printf("\033[2J\033[H");

    main_menu();

    return 0;
}

