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
        printf("============================================\n");
        printf("          CINEMA BOOKING SYSTEM             \n");
        printf("--------------------------------------------\n");
        printf("[1] Login\n");
        printf("[2] Register New Account\n");
        printf("[0] Exit\n");
        printf("============================================\n");
        printf("Choose : ");
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
            printf("\n[ERROR] Invalid Input!\n");
            printf("Please enter the number (1, 2, 0) or the exact option text.\n");
            system("pause");
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
// ============================================================
void login() {
    system("cls");
    char username[100], password[100];
    char fileUsername[100], filePassword[100];
    char buffer[200];
    int  success;

    while (1) {
        success = 0; // Reset status success
        system("cls");
        printf("============================================\n");
        printf("                   LOGIN                    \n");
        printf("--------------------------------------------\n");
        printf(" (Enter '0' on Username to go back)\n");
        printf("--------------------------------------------\n");

        do {
            printf("Username : ");
            fgets(username, sizeof(username), stdin);
            username[strcspn(username, "\n")] = 0; // Hapus karakter newline
            
            // Fitur Back ke Main Menu
            if (strcmp(username, "0") == 0) {
                main_menu();
                return;
            }
            if (strlen(username) == 0) {
                printf("Username cannot be empty!\n");
            }
        } while (strlen(username) == 0);

        do {
            printf("Password : ");
            fgets(password, sizeof(password), stdin);
            password[strcspn(password, "\n")] = 0; // Hapus karakter newline

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
    int  found, valid, validasi_at, has_space;

    FILE* reg = fopen(account_file, "a");
    if (reg == NULL) { invalid_file(); return; }

    Account customer;

    printf("==========================================\n");
    printf("                 REGISTER                 \n");
    printf("==========================================\n");
    printf("Complete the registration form to continue\n");
    printf(" (Enter '0' at any prompt to go back)\n");
    printf("------------------------------------------\n");

    // full name
    do {
        printf("Full Name     : ");
        fgets(customer.name, sizeof(customer.name), stdin);
        customer.name[strcspn(customer.name, "\n")] = '\0'; // Hapus newline

        if (strcmp(customer.name, "0") == 0) { main_menu(); return; }
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
    } while (!valid);

    // email
    do {
        printf("Email         : ");
        fgets(customer.email, sizeof(customer.email), stdin);
        customer.email[strcspn(customer.email, "\n")] = '\0';

        if (strcmp(customer.email, "0") == 0) { main_menu(); return; }
        validasi_at = 0;
        for (int i = 0; customer.email[i] != '\0'; i++) {
            if (customer.email[i] == '@') { validasi_at = 1; break; }
        }
        if (strlen(customer.email) == 0) printf("Email cannot be empty!\n");
        else if (!validasi_at) printf("Email must contain '@'!\n");
    } while (strlen(customer.email) == 0 || !validasi_at);

    // username
    do {
        printf("Username      : ");
        fgets(customer.username, sizeof(customer.username), stdin);
        customer.username[strcspn(customer.username, "\n")] = '\0';

        if (strcmp(customer.username, "0") == 0) { main_menu(); return; }

        if (strlen(customer.username) == 0) {
            printf("Username cannot be empty!\n");
            found = 1; 
            continue;
        }

        // Cek spasi pada username
        has_space = 0;
        for (int i = 0; customer.username[i] != '\0'; i++) {
            if (isspace(customer.username[i])) { has_space = 1; break; }
        }
        if (has_space) {
            printf("Username cannot contain spaces!\n");
            found = 1;
            continue;
        }
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
    } while (found || strlen(customer.username) == 0);

    // password
    do {
        printf("Password      : ");
        fgets(customer.password, sizeof(customer.password), stdin);
        customer.password[strcspn(customer.password, "\n")] = '\0';

        if (strcmp(customer.password, "0") == 0) { main_menu(); return; }

        if (strlen(customer.password) == 0) printf("Password cannot be empty!\n");
        else if (strlen(customer.password) < 5)
            printf("Password must be at least 5 characters!\n");
    } while (strlen(customer.password) < 5);

    // confirm password
    do {
        printf("Confirm Password : ");
        fgets(confirm_pass, sizeof(confirm_pass), stdin);
        confirm_pass[strcspn(confirm_pass, "\n")] = '\0';

        if (strcmp(confirm_pass, "0") == 0) { main_menu(); return; }

        if (strlen(confirm_pass) == 0) printf("Confirm password cannot be empty!\n");
        else if (strcmp(customer.password, confirm_pass) != 0)
            printf("Password does not match! Try again!\n");
    } while (strcmp(customer.password, confirm_pass) != 0 || strlen(confirm_pass) == 0);

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
        printf("[2] Search Film by ID\n");
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
        } else if (strcmp(lower_input, "2") == 0 || strcmp(lower_input, "search film by id") == 0) {
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

    if (count == 0) {
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

        if (strlen(temp_id) > w_id) w_id = strlen(temp_id);
        if (strlen(result[i].title) > w_title) w_title = strlen(result[i].title);
    }

    // [Pass 2] Menampilkan tabel dinamis 2 kolom
    printf("+"); 
    for(int i = 0; i < w_id + 2; i++) printf("-"); 
    printf("+"); 
    for(int i = 0; i < w_title + 2; i++) printf("-"); 
    printf("+\n");
    
    printf("| %-*s | %-*s |\n", w_id, "ID", w_title, "Title");
    
    printf("+"); 
    for(int i = 0; i < w_id + 2; i++) printf("-"); 
    printf("+"); 
    for(int i = 0; i < w_title + 2; i++) printf("-"); 
    printf("+\n");

    for (int i = 0; i < count; i++) {
        printf("| %-*d | %-*s |\n", w_id, result[i].id, w_title, result[i].title);
    }

    printf("+"); 
    for(int i = 0; i < w_id + 2; i++) printf("-"); 
    printf("+"); 
    for(int i = 0; i < w_title + 2; i++) printf("-"); 
    printf("+\n");

    printf("--------------------------------------------\n");
    printf(" (Enter '0' to go back)\n");
    printf("--------------------------------------------\n");

     char input[100];
    char lower_input[100];
    int target_id = -1;
    char target_title[100] = "";

    // Input & Validation
    do {
        printf("Enter Film ID or Title to delete : ");
        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = '\0'; // Hapus newline

        // Fitur Back
        if (strcmp(input, "0") == 0) { film_manage(); return; }
        
        if (strlen(input) == 0) { 
            printf("Input cannot be empty!\n"); 
            continue; 
        }

        // Konversi input ke lowercase untuk komparasi teks
        for (int i = 0; input[i] != '\0'; i++) {
            lower_input[i] = tolower(input[i]);
        }
        lower_input[strlen(input)] = '\0';

        // Cek apakah input murni angka (untuk pencarian by ID)
        int is_num = 1;
        for (int i = 0; input[i] != '\0'; i++) {
            if (!isdigit(input[i])) { is_num = 0; break; }
        }
        int input_id = is_num ? atoi(input) : -1;

        // Pencarian melalui array (bisa cocok dengan ID ATAU Judul)
        target_id = -1; // Reset target
        for (int i = 0; i < count; i++) {
            char lower_title[100];
            strcpy(lower_title, result[i].title);
            // Konversi judul di array ke lowercase
            for (int j = 0; lower_title[j] != '\0'; j++) {
                lower_title[j] = tolower(lower_title[j]);
            }

            // Jika input cocok dengan judul (lower_title) ATAU cocok dengan ID
            if (strcmp(lower_input, lower_title) == 0 || result[i].id == input_id) {
                target_id = result[i].id;
                strcpy(target_title, result[i].title);
                break;
            }
        }

        if (target_id == -1) {
            printf("Film with ID or Title \"%s\" not found! Please try again.\n", input);
        }
    } while (target_id == -1);

    btree_delete(&film_tree, target_id);
    btree_save_to_file();
    printf("--------------------------------------------\n");
    printf("Film \"%s\" (ID: %d) deleted successfully!\n", target_title, target_id);
    printf("============================================\n");
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

    if (count == 0) {
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
        if (strlen(temp_id) > w_id) w_id = strlen(temp_id);
        if (strlen(result[i].title) > w_title) w_title = strlen(result[i].title);
    }

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

    char input[100];
    char lower_input[100];
    int target_id = -1;
    Film matched[200];
    int match_count = 0;

    do {
        match_count = 0;
        printf("Enter Film ID or Title to edit : ");
        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = '\0';

        if (strcmp(input, "0") == 0) { film_manage(); return; }
        if (strlen(input) == 0) { printf("Input cannot be empty!\n"); continue; }

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
            printf("Film \"%s\" not found! Please try again.\n", input);
        } else if (match_count == 1) {
            target_id = matched[0].id; // Langsung masuk mode Edit jika hanya 1 yang cocok
        } else {
            // Ambiguity Resolution (Jika ada 2 atau lebih film yang cocok)
            int resolved = 0;
            do {
                // PAGE BARU: Tampilkan daftar film yang cocok
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
                    printf("Input cannot be empty!\n"); 
                    system("pause"); // Beri jeda agar user bisa baca error
                    continue; 
                }

                int id_is_num = 1;
                for (int i = 0; id_input[i] != '\0'; i++) {
                    if (!isdigit(id_input[i])) { id_is_num = 0; break; }
                }

                if (!id_is_num) {
                    printf("Invalid input! Please enter a numeric ID.\n");
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
                    printf("ID %d is not on the list! Please type the exact ID shown above.\n", selected_id);
                    system("pause");
                }
            } while (!resolved);
        }
    } while (target_id == -1);


    // ==========================================
    // TAHAP EDIT FILM (Menggunakan Struct Sementara)
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

    printf("Title [%s] : ", temp_film.title);
    fgets(new_val, sizeof(new_val), stdin);
    new_val[strcspn(new_val, "\n")] = '\0';
    if (strcmp(new_val, "0") == 0) { film_manage(); return; } 
    if (strlen(new_val) != 0) strcpy(temp_film.title, new_val);

    do {
        valid = 1;
    printf("Genre [%s] : ", temp_film.genre);
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

    printf("Duration [%d] : ", temp_film.duration);
    fgets(new_val, sizeof(new_val), stdin);
    new_val[strcspn(new_val, "\n")] = '\0';
    if (strlen(new_val) != 0) {
        valid = 1;
        for (int i = 0; new_val[i] != '\0'; i++)
            if (!isdigit(new_val[i])) { valid = 0; break; }
        if (valid && atoi(new_val) > 0) temp_film.duration = atoi(new_val);
        else printf("Invalid duration, keeping current value.\n");
    }

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

void search_film() {
    system("cls");
    Film result[200];
    int count = 0;
    btree_inorder(film_tree, result, &count);

    printf("============================================\n");
    printf("                 ALL FILMS                  \n");
    printf("    (Displayed via B-Tree In-Order)         \n");
    printf("                SEARCH FILM                 \n");
    printf("============================================\n");
     if (count == 0) {
        printf("No films available in the database.\n");
        printf("============================================\n");
        system("pause");
        film_manage();
        return;
    }

    // ==========================================
    // PAGE 1 : TAMPILKAN TABEL REFERENSI
    // ==========================================
    
    // [Pass 1] Mencari panjang string maksimal (2 kolom)
    int w_id_ref = 2;       // Lebar minimal "ID"
    int w_title_ref = 5;    // Lebar minimal "Title"
    
    for (int i = 0; i < count; i++) {
        char temp_id[20];
        sprintf(temp_id, "%d", result[i].id);

        if (strlen(temp_id) > w_id_ref) w_id_ref = strlen(temp_id);
        if (strlen(result[i].title) > w_title_ref) w_title_ref = strlen(result[i].title);
    }

    // [Pass 2] Menampilkan tabel referensi 2 kolom
    printf("+"); for(int i = 0; i < w_id_ref + 2; i++) printf("-"); 
    printf("+"); for(int i = 0; i < w_title_ref + 2; i++) printf("-"); printf("+\n");
    
    printf("| %-*s | %-*s |\n", w_id_ref, "ID", w_title_ref, "Title");
    
    printf("+"); for(int i = 0; i < w_id_ref + 2; i++) printf("-"); 
    printf("+"); for(int i = 0; i < w_title_ref + 2; i++) printf("-"); printf("+\n");

    printf(" (Enter '0' to go back)\n");
    printf("--------------------------------------------\n");

    printf("+"); for(int i = 0; i < w_id_ref + 2; i++) printf("-"); 
    printf("+"); for(int i = 0; i < w_title_ref + 2; i++) printf("-"); printf("+\n");

    printf("--------------------------------------------\n");

    char input[100];
    char lower_input[100];
    Film matched[200]; // Array untuk menyimpan film yang cocok
    int match_count = 0;

    // Input & Validation
    do {
        match_count = 0; // Reset jumlah kecocokan setiap kali looping
        
        printf("Enter Film ID or Title to search : ");
        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = '\0'; // Hapus newline

        // Fitur Back
        if (strcmp(input, "0") == 0) { film_manage(); return; }

        if (strlen(input) == 0) { 
            printf("Input cannot be empty!\n"); 
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

        // Pencarian melalui array (Partial Match untuk Judul ATAU Exact Match untuk ID)
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

        if (match_count == 0) {
            printf("No film found matching \"%s\". Please try again.\n", input);
        }

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

        if (strlen(temp_id) > w_id) w_id = strlen(temp_id);
        if (strlen(matched[i].title) > w_title) w_title = strlen(matched[i].title);
        if (strlen(matched[i].genre) > w_genre) w_genre = strlen(matched[i].genre);
        if (strlen(temp_dur) > w_dur) w_dur = strlen(temp_dur);
        if (strlen(temp_age) > w_age) w_age = strlen(temp_age);
        if (strlen(matched[i].detail) > w_detail) w_detail = strlen(matched[i].detail);
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
        if (strlen(temp_id) > w_id) w_id = strlen(temp_id);
        if (strlen(result[i].title) > w_title) w_title = strlen(result[i].title);
        if (strlen(result[i].genre) > w_genre) w_genre = strlen(result[i].genre);
        if (strlen(temp_dur) > w_dur) w_dur = strlen(temp_dur);
        if (strlen(temp_age) > w_age) w_age = strlen(temp_age);
        if (strlen(result[i].detail) > w_detail) w_detail = strlen(result[i].detail);
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

    // Mengganti system("pause") dengan prompt Back (0)
    char input[50];
    do {
        printf("Type '0' to go back : ");
        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = '\0';
        
        if (strlen(input) == 0) {
            printf("Input cannot be empty!\n");
        } else if (strcmp(input, "0") != 0) {
            printf("Invalid input! Please type '0' to go back.\n");
        }
    } while (strcmp(input, "0") != 0);

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

// EDIT STUDIO
void edit_studio() {
    system("cls");
    printf("============================================\n");
    printf("                EDIT STUDIO                 \n");
    printf("============================================\n");
    printf(" This feature is currently under development.\n");
    printf("--------------------------------------------\n");
    system("pause");
    schedule_manage();
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

        if (strlen(t_id) > w_id) w_id = strlen(t_id);
        if (strlen(result[i].name) > w_name) w_name = strlen(result[i].name);
        if (strlen(t_cap) > w_cap) w_cap = strlen(t_cap);
        if (strlen(t_rows) > w_rows) w_rows = strlen(t_rows);
        if (strlen(t_cols) > w_cols) w_cols = strlen(t_cols);
    }

    int total_width = w_id + w_name + w_cap + w_rows + w_cols + 16;

    // Macro untuk mencetak garis tabel
    #define PRINT_STUDIO_LINE() \
        printf("+"); \
        for(int k = 0; k < w_id + 2; k++) printf("-"); printf("+"); \
        for(int k = 0; k < w_name + 2; k++) printf("-"); printf("+"); \
        for(int k = 0; k < w_cap + 2; k++) printf("-"); printf("+"); \
        for(int k = 0; k < w_rows + 2; k++) printf("-"); printf("+"); \
        for(int k = 0; k < w_cols + 2; k++) printf("-"); printf("+\n")

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
    // TAHAP 4: FITUR BACK (0)
    // ==========================================
    char input[50];
    do {
        printf("Type '0' to go back : ");
        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = '\0';
        
        if (strlen(input) == 0) {
            printf("Input cannot be empty!\n");
        } else if (strcmp(input, "0") != 0) {
            printf("Invalid input! Please type '0' to go back.\n");
        }
    } while (strcmp(input, "0") != 0);
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
        }
    } while (!validate_date(sch.date));

    // Input Jam + Cek Bentrok
    int conflict_id;
    do {
        do {
            printf("Time (HH:MM)      : ");
            scanf("%s", sch.time);
            if (!validate_time(sch.time)) {
                printf("Invalid time! Format must be HH:MM (00:00-23:59).\n");
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
            printf("SCHEDULE CONFLICT detected! (ID %d: %s, %s-%s)\n",
                   conflict_id, cf ? cf->title : "?",
                   conflict_sch.time, end_time_str);
            printf("Please enter a different time.\n");
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

    printf("\n=== DATE ===\n");
    printf("Current Date [%s] : ", sch.date);
    fgets(new_val, sizeof(new_val), stdin);
    new_val[strcspn(new_val, "\n")] = '\0';
    if (strlen(new_val) != 0) {
        if (validate_date(new_val)) strcpy(sch.date, new_val);
        else printf("Invalid date format. Keeping current date.\n");
    }

    printf("\n=== TIME ===\n");
    char temp_time[20];
    printf("Current Time [%s] (leave blank to keep) : ", sch.time);
    fgets(new_val, sizeof(new_val), stdin);
    new_val[strcspn(new_val, "\n")] = '\0';
    strcpy(temp_time, (strlen(new_val) != 0) ? new_val : sch.time);

    int conflict_id;
    while (1) {
        if (!validate_time(temp_time)) {
            printf("Invalid time format (HH:MM).\n");
            printf("Re-enter Time : ");
            scanf("%s", temp_time);
            while (getchar() != '\n');
            continue;
        }
        conflict_id = check_schedule_conflict(
            sch.studio_id, sch.date, temp_time,
            selected_film->duration, edit_id);
        if (conflict_id != -1) {
            printf("SCHEDULE CONFLICT with ID %d. Re-enter Time (HH:MM) : ", conflict_id);
            scanf("%s", temp_time);
            while (getchar() != '\n');
        } else break;
    }
    strcpy(sch.time, temp_time);

    printf("\n=== PRICE ===\n");
    printf("Current Price [%.0f] : ", sch.price);
    fgets(new_val, sizeof(new_val), stdin);
    new_val[strcspn(new_val, "\n")] = '\0';
    if (strlen(new_val) != 0) {
        float np = atof(new_val);
        if (np > 0) sch.price = np;
        else printf("Invalid price. Keeping current.\n");
    }

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
        printf("[2] Search User by Username\n");
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
        } else if (strcmp(lower_input, "2") == 0 || strcmp(lower_input, "search user by username") == 0) {
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
    char buffer[1024];
    char input[100];
    char lower_input[100];
    int valid;
    Account acc;
    char current_fullname[100] = "";

    btree_load_from_file(); /* pastikan B-Tree termuat untuk customer */

    FILE* check = fopen(account_file, "r");
    if (check != NULL) {
        while (fgets(buffer, sizeof(buffer), check)) {
            buffer[strcspn(buffer, "\n")] = 0;
            sscanf(buffer, "%[^,],%[^,],%[^,],%[^\n]",
                   acc.username, acc.password, acc.name, acc.email);
            if (strcmp(acc.username, current_user) == 0) {
                strcpy(current_fullname, acc.name);
                break;
            }
        }
        fclose(check);
    }
    if (strlen(current_fullname) == 0) strcpy(current_fullname, current_user);

    do {
        system("cls");
        view_film_cust();
        printf("============================================\n");
        printf("               CUSTOMER MENU                \n");
        printf("--------------------------------------------\n");
        printf("Welcome Back, %s!\n", current_fullname);
        printf("[1] Book Ticket\n");
        printf("[2] My Booking History\n");
        printf("[3] Cancel Booking\n");
        printf("[4] Edit Profile\n");
        printf("[0] Logout\n");
        printf("============================================\n");
        printf("Choose : ");
        // Menggunakan fgets untuk membaca string, termasuk spasi/enter kosong
        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = '\0'; // Menghapus karakter newline (\n)

        // Mengonversi input menjadi lowercase (huruf kecil) agar case-insensitive
        for (int i = 0; input[i] != '\0'; i++) {
            lower_input[i] = tolower(input[i]);
        }
        lower_input[strlen(input)] = '\0';

        valid = 1; // Asumsikan input valid di awal

        // Pengecekan kondisi berdasarkan angka atau teks
        if (strcmp(lower_input, "1") == 0 || strcmp(lower_input, "book ticket") == 0) {
            choice = 1;
        } else if (strcmp(lower_input, "2") == 0 || strcmp(lower_input, "history") == 0) {
            choice = 2;
        } else if (strcmp(lower_input, "3") == 0 || strcmp(lower_input, "cancel ticket") == 0) {
            choice = 3;
        } else if (strcmp(lower_input, "4") == 0 || strcmp(lower_input, "edit profile") == 0) {
            choice = 4;
        } else if (strcmp(lower_input, "0") == 0 || strcmp(lower_input, "logout") == 0) {
            choice = 0;
        } else {
            // Jika input kosong atau di luar opsi
            printf("\n[ERROR] Invalid Input!\n");
            printf("Please enter the number (0-4) or the exact option text.\n");
            system("pause");
            valid = 0; // Input tidak valid, loop akan diulang
        }
    } while (!valid);

    switch (choice) {
        case 1: book_ticket();  break;
        case 2: history();      break;
        case 3: cancel();       break;
        case 4: edit_profile(); break;
        case 0: main_menu();    break;
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
    FILE* data = fopen(film_file, "r");
    if (data == NULL) return;

    Film film;
    char buffer[1024];
    int count = 0;

    int w_id = 2;       // "ID"
    int w_title = 5;    // "Title"
    int w_genre = 5;    // "Genre"
    int w_dur = 8;      // "Duration"
    int w_age = 3;      // "Age"
    int w_detail = 6;   // "Detail"

    while (fgets(buffer, sizeof(buffer), data)) {
         char temp_id[20], temp_dur[20], temp_age[20];
        
        buffer[strcspn(buffer, "\n")] = 0; // Hapus newline
        
        if (sscanf(buffer, "%d=%[^=]=%[^=]=%d=%d=%[^\n]", 
                   &film.id, film.title, film.genre, 
                   &film.duration, &film.age_rating, film.detail) == 6) {
            
            sprintf(temp_id, "%d", film.id);
            sprintf(temp_dur, "%d", film.duration);
            sprintf(temp_age, "%d", film.age_rating);

            if (strlen(temp_id) > w_id) w_id = strlen(temp_id);
            if (strlen(film.title) > w_title) w_title = strlen(film.title);
            if (strlen(film.genre) > w_genre) w_genre = strlen(film.genre);
            if (strlen(temp_dur) > w_dur) w_dur = strlen(temp_dur);
            if (strlen(temp_age) > w_age) w_age = strlen(temp_age);
            if (strlen(film.detail) > w_detail) w_detail = strlen(film.detail);
            
            count++;
        }
    }

    // Menghitung total lebar tabel untuk garis judul "NOW SHOWING"
    // 19 didapat dari padding sisi kiri-kanan dan batas '|' tiap kolom
    int total_width = w_id + w_title + w_genre + w_dur + w_age + w_detail + 19;

    printf("\n");
    for(int i = 0; i < total_width; i++) printf("="); printf("\n");
    
    // Teks di tengah (Center alignment) untuk "NOW SHOWING"
    int padding = (total_width - 11) / 2; // 11 adalah panjang string "NOW SHOWING"
    for(int i = 0; i < padding; i++) printf(" ");
    printf("NOW SHOWING\n");
    
    for(int i = 0; i < total_width; i++) printf("="); printf("\n");

    // Kembalikan pointer file ke awal untuk mulai membaca data kembali
    rewind(data);

    // [Pass 2] Tampilkan data dengan kolom dinamis
    print_table_line(w_id, w_title, w_genre, w_dur, w_age, w_detail);
    
    printf("| %-*s | %-*s | %-*s | %-*s | %-*s | %-*s |\n",
           w_id, "ID", w_title, "Title", w_genre, "Genre", 
           w_dur, "Duration", w_age, "Age", w_detail, "Detail");
           
    print_table_line(w_id, w_title, w_genre, w_dur, w_age, w_detail);

    if (count == 0) {
        // Jika belum ada film sama sekali
        printf("| %-*s |\n", total_width - 4, "No films available at the moment.");
        print_table_line(w_id, w_title, w_genre, w_dur, w_age, w_detail);
    } else {
        // Jika ada film, print isinya
        while (fgets(buffer, sizeof(buffer), data)) {
            Film film;
            char temp_dur[20], temp_age[20];
            
            buffer[strcspn(buffer, "\n")] = 0;
            if (sscanf(buffer, "%d=%[^=]=%[^=]=%d=%d=%[^\n]", 
                       &film.id, film.title, film.genre, 
                       &film.duration, &film.age_rating, film.detail) == 6) {
                
                sprintf(temp_dur, "%d", film.duration);
                sprintf(temp_age, "%d", film.age_rating);

                printf("| %-*d | %-*s | %-*s | %-*s | %-*s | %-*s |\n",
                       w_id, film.id, w_title, film.title, w_genre, film.genre,
                       w_dur, temp_dur, w_age, temp_age, w_detail, film.detail);
            }
        }
        print_table_line(w_id, w_title, w_genre, w_dur, w_age, w_detail);
    }
    printf("\n");
    fclose(data);
}

// ============================================================
// !! BOOK TICKET !!
// ============================================================
void book_ticket() {
    system("cls");
    printf("============================================\n");
    printf("               BOOK TICKET                  \n");
    printf("============================================\n");

    /* -- LANGKAH 1: Pilih Film -- */
    Film all_films[200];
    int film_count = 0;
    btree_inorder(film_tree, all_films, &film_count);

    if (film_count == 0) {
        printf("Tidak ada film tersedia.\n");
        system("pause");
        menu_cust();
        return;
    }

    printf("\n[ STEP 1 ] Pilih Film\n");
    printf("%-5s %-25s %-12s %-6s %s\n", "ID", "Title", "Genre", "Min", "Age");
    printf("--------------------------------------------\n");
    for (int i = 0; i < film_count; i++) {
        printf("%-5d %-25s %-12s %-6d %d+\n",
               all_films[i].id, all_films[i].title,
               all_films[i].genre, all_films[i].duration,
               all_films[i].age_rating);
    }
    printf("--------------------------------------------\n");

    char input[20];
    int film_id, valid;

    do {
        printf("Masukkan Film ID : ");
        scanf("%s", input);
        valid = 1;
        for (int i = 0; input[i] != '\0'; i++)
            if (!isdigit(input[i])) { valid = 0; break; }
        if (!valid) { printf("ID harus berupa angka!\n"); continue; }
        film_id = atoi(input);
        if (btree_search(film_tree, film_id) == NULL) {
            printf("Film dengan ID %d tidak ditemukan!\n", film_id);
            valid = 0;
        }
    } while (!valid);

    Film* chosen_film = btree_search(film_tree, film_id);

    /* -- LANGKAH 2: Pilih Jadwal -- */
    printf("\n[ STEP 2 ] Pilih Jadwal untuk \"%s\"\n", chosen_film->title);
    printf("%-4s %-15s %-12s %-8s %s\n",
           "ID", "Studio", "Date", "Time", "Price");
    printf("--------------------------------------------\n");

    FILE* sch_fp = fopen(schedule_file, "r");
    if (sch_fp == NULL) {
        printf("Tidak ada jadwal tersedia.\n");
        system("pause");
        menu_cust();
        return;
    }

    int sch_count = 0;
    char sch_buffer[300];
    while (fgets(sch_buffer, sizeof(sch_buffer), sch_fp)) {
        sch_buffer[strcspn(sch_buffer, "\n")] = 0;
        Schedule sch;
        sscanf(sch_buffer, "%d=%d=%d=%[^=]=%[^=]=%f",
               &sch.id, &sch.film_id, &sch.studio_id,
               sch.date, sch.time, &sch.price);
        if (sch.film_id != film_id) continue;
        Studio st; find_studio(sch.studio_id, &st);
        printf("%-4d %-15s %-12s %-8s Rp %.0f\n",
               sch.id, st.name, sch.date, sch.time, sch.price);
        sch_count++;
    }
    fclose(sch_fp);

    if (sch_count == 0) {
        printf("Tidak ada jadwal untuk film ini.\n");
        system("pause");
        menu_cust();
        return;
    }
    printf("--------------------------------------------\n");

    int schedule_id;
    Schedule chosen_sch;

    do {
        printf("Masukkan Schedule ID : ");
        scanf("%s", input);
        valid = 1;
        for (int i = 0; input[i] != '\0'; i++)
            if (!isdigit(input[i])) { valid = 0; break; }
        if (!valid) { printf("ID harus berupa angka!\n"); continue; }
        schedule_id = atoi(input);
        if (!find_schedule(schedule_id, &chosen_sch)) {
            printf("Jadwal dengan ID %d tidak ditemukan!\n", schedule_id);
            valid = 0;
            continue;
        }
        if (chosen_sch.film_id != film_id) {
            printf("Jadwal ID %d bukan untuk film \"%s\"!\n",
                   schedule_id, chosen_film->title);
            valid = 0;
        }
    } while (!valid);

    Studio chosen_studio;
    find_studio(chosen_sch.studio_id, &chosen_studio);

    /* -- LANGKAH 3: Pilih Kursi (bisa lebih dari satu) -- */
    // -------------------------------------------------------
    // Tampung kursi yang dipilih sementara di array lokal.
    // MAX_SEATS_PER_ORDER = batas kursi per pesanan (misal 8).
    // -------------------------------------------------------

    char  selected_seats[MAX_SEATS_PER_ORDER][10];
    int   seat_count = 0;

    printf("\n[ STEP 3 ] Pilih Kursi\n");
    printf("  Anda bisa memilih hingga %d kursi sekaligus.\n", MAX_SEATS_PER_ORDER);
    printf("  Ketik \"DONE\" jika sudah selesai memilih.\n\n");

    while (seat_count < MAX_SEATS_PER_ORDER) {
        /* Tampilkan denah kursi setiap iterasi agar up-to-date */
        display_seat_map(schedule_id, &chosen_studio);

        /* Tampilkan kursi yang sudah dipilih sesi ini */
        if (seat_count > 0) {
            printf("  Kursi dipilih (%d): ", seat_count);
            for (int i = 0; i < seat_count; i++)
                printf("%s%s", selected_seats[i],
                               (i < seat_count - 1) ? ", " : "\n");
        }

        char seat_input[10];
        printf("Masukkan kursi ke-%d (atau \"DONE\") : ", seat_count + 1);
        scanf("%s", seat_input);

        /* Konversi ke huruf besar */
        for (int i = 0; seat_input[i]; i++)
            seat_input[i] = toupper(seat_input[i]);

        /* User selesai memilih */
        if (strcmp(seat_input, "DONE") == 0) {
            if (seat_count == 0) {
                printf("  Pilih minimal 1 kursi!\n\n");
                continue;
            }
            break;
        }

        /* Validasi format */
        if (!validate_seat_format(seat_input, &chosen_studio)) {
            printf("  Kursi \"%s\" tidak valid! Baris A-%c, Kolom 1-%d.\n\n",
                   seat_input,
                   'A' + chosen_studio.rows - 1,
                   chosen_studio.cols);
            continue;
        }

        /* Cek apakah kursi sudah dibooking di database */
        if (is_seat_booked(schedule_id, seat_input)) {
            printf("  Kursi %s sudah dibooking orang lain!\n\n", seat_input);
            continue;
        }

        /* Cek duplikat dalam sesi pemilihan ini */
        int duplicate = 0;
        for (int i = 0; i < seat_count; i++) {
            if (strcmp(selected_seats[i], seat_input) == 0) {
                duplicate = 1;
                break;
            }
        }
        if (duplicate) {
            printf("  Kursi %s sudah Anda pilih sebelumnya!\n\n", seat_input);
            continue;
        }

        /* Simpan ke daftar sementara */
        strcpy(selected_seats[seat_count], seat_input);
        seat_count++;
        printf("  Kursi %s ditambahkan.\n\n", seat_input);
    }

    /* -- LANGKAH 4: Konfirmasi -- */
    float total_harga = chosen_sch.price * seat_count;

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
        printf("%s%s", selected_seats[i],
                       (i < seat_count - 1) ? ", " : "\n");
    printf("  Jumlah  : %d kursi\n", seat_count);
    printf("  Harga   : Rp %.0f x %d = Rp %.0f\n",
           chosen_sch.price, seat_count, total_harga);
    printf("--------------------------------------------\n");
    printf("Konfirmasi booking? (Y/N) : ");

    char confirm;
    scanf(" %c", &confirm);

    if (confirm != 'Y' && confirm != 'y') {
        printf("Booking dibatalkan.\n");
        system("pause");
        menu_cust();
        return;
    }

    /* -- LANGKAH 5: Simpan Booking (satu record per kursi, ID berbeda) -- */
    // -----------------------------------------------------------------------
    // Setiap kursi mendapat booking_code unik tersendiri.
    // auto_id_booking() harus dipanggil ulang setiap iterasi agar ID
    // selalu naik dan tidak bentrok.
    // -----------------------------------------------------------------------
    /* -- LANGKAH 5: Simpan Booking (satu record per kursi, ID unik) -- */
    
    // Ambil base ID SEBELUM fopen agar file sudah dalam kondisi final
    int base_id = auto_id_booking();  // misal: 11
    
    char saved_codes[MAX_SEATS_PER_ORDER][20];

    FILE* bk_fp = fopen(booking_file, "a");
    if (bk_fp == NULL) { invalid_file(); return; }

    for (int i = 0; i < seat_count; i++) {
        // Increment manual: 11, 12, 13, dst — tidak perlu baca file lagi
        generate_booking_code(base_id + i, saved_codes[i]);

        fprintf(bk_fp, "%s=%s=%d=%s=%.0f=1\n",
                saved_codes[i], current_user,
                schedule_id, selected_seats[i], chosen_sch.price);
    }
    fclose(bk_fp);  // flush sekali di sini

    /* -- Tampilkan Hasil -- */
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
    system("cls");
    printf("============================================\n");
    printf("              CANCEL BOOKING                \n");
    printf("============================================\n");

    /* Tampilkan booking aktif milik user */
    Booking bookings[MAX_BOOKINGS];
    int total = load_bookings(bookings, MAX_BOOKINGS);

    int active_count = 0;
    printf("  Booking aktif Anda:\n");
    printf("--------------------------------------------\n");
    printf("  %-12s %-20s %-12s %-8s %s\n",
           "Code", "Film", "Date", "Seat", "Price");
    printf("--------------------------------------------\n");

    for (int i = 0; i < total; i++) {
        if (strcmp(bookings[i].username, current_user) != 0) continue;
        if (bookings[i].status != 1) continue; /* hanya yang aktif */

        Schedule sch;
        char film_title[100] = "Unknown";
        char date_str[20]    = "-";

        if (find_schedule(bookings[i].schedule_id, &sch)) {
            Film* f = btree_search(film_tree, sch.film_id);
            if (f != NULL) strcpy(film_title, f->title);
            strcpy(date_str, sch.date);
        }

        printf("  %-12s %-20s %-12s %-8s Rp %.0f\n",
               bookings[i].booking_code, film_title,
               date_str, bookings[i].seat, bookings[i].total_price);
        active_count++;
    }

    if (active_count == 0) {
        printf("  Tidak ada booking aktif untuk dibatalkan.\n");
        printf("============================================\n");
        system("pause");
        menu_cust();
        return;
    }

    printf("--------------------------------------------\n");

    /* Input booking code yang ingin dibatalkan */
    char code_input[20];
    printf("  Masukkan Booking Code yang ingin dibatalkan : ");
    scanf("%s", code_input);

    /* Konversi ke huruf besar (BK-xxxxx) */
    for (int i = 0; code_input[i]; i++)
        code_input[i] = toupper(code_input[i]);

    /* Cari booking */
    Booking target;
    if (!find_booking(code_input, &target)) {
        printf("  Booking code \"%s\" tidak ditemukan!\n", code_input);
        system("pause");
        menu_cust();
        return;
    }

    /* Pastikan booking milik user yang login */
    if (strcmp(target.username, current_user) != 0) {
        printf("  Booking ini bukan milik Anda!\n");
        system("pause");
        menu_cust();
        return;
    }

    /* Pastikan booking masih aktif */
    if (target.status != 1) {
        printf("  Booking %s sudah dibatalkan sebelumnya.\n", code_input);
        system("pause");
        menu_cust();
        return;
    }

    /* Tampilkan detail sebelum konfirmasi */
    Schedule sch;
    char film_title[100]  = "Unknown";
    char studio_name[50]  = "Unknown";

    if (find_schedule(target.schedule_id, &sch)) {
        Film* f = btree_search(film_tree, sch.film_id);
        if (f != NULL) strcpy(film_title, f->title);
        Studio st;
        if (find_studio(sch.studio_id, &st)) strcpy(studio_name, st.name);
    }

    printf("--------------------------------------------\n");
    printf("  Detail Booking:\n");
    printf("  Code    : %s\n", target.booking_code);
    printf("  Film    : %s\n", film_title);
    printf("  Studio  : %s\n", studio_name);
    printf("  Tanggal : %s\n", (find_schedule(target.schedule_id, &sch)) ? sch.date : "-");
    printf("  Kursi   : %s\n", target.seat);
    printf("  Harga   : Rp %.0f\n", target.total_price);
    printf("--------------------------------------------\n");
    printf("  Yakin ingin membatalkan booking %s? (Y/N) : ",
           target.booking_code);

    char confirm;
    scanf(" %c", &confirm);

    if (confirm != 'Y' && confirm != 'y') {
        printf("  Pembatalan dibatalkan.\n");
        system("pause");
        menu_cust();
        return;
    }

    /* Tulis ulang file: ubah status booking menjadi 0 */
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
            /* Ubah status menjadi 0 (dibatalkan) */
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

    printf("--------------------------------------------\n");
    printf("  Booking %s berhasil dibatalkan.\n", code_input);
    printf("  Kursi %s sekarang tersedia kembali.\n", target.seat);
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

void delete_account_cust() {
    char buffer[1024];
    Account acc;
    char current_pass[100] = "";
    char input_pass[100];
    char confirm_pass_input[100]; // Untuk konfirmasi input password
    char confirm[100];
    char lower_confirm[100];
    int pass_match = 0;
    int confirmed = 0;

    // Ambil password saat ini dari file
    FILE* check = fopen(account_file, "r");
    if (check != NULL) {
        while (fgets(buffer, sizeof(buffer), check)) {
            buffer[strcspn(buffer, "\n")] = 0;
            sscanf(buffer, "%[^,],%[^,],%[^,],%[^\n]", acc.username, acc.password, acc.name, acc.email);
            if (strcmp(acc.username, current_user) == 0) {
                strcpy(current_pass, acc.password);
                break;
            }
        }
        fclose(check);
    }

    // ==========================================
    // PAGE 1 : PASSWORD VERIFICATION
    // ==========================================
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

        // Fitur Back
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

        // Fitur Back
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

        pass_match = 1; // Jika semua validasi lolos
    } while (!pass_match);


    // ==========================================
    // PAGE 2 : FINAL CONFIRMATION
    // ==========================================
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

        // Konversi input konfirmasi ke huruf kecil (lowercase)
        for (int i = 0; confirm[i] != '\0'; i++) {
            lower_confirm[i] = tolower(confirm[i]);
        }
        lower_confirm[strlen(confirm)] = '\0';

        // Logika Pilihan
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


    // ==========================================
    // DATA DELETION PROCESS (File Handling)
    // ==========================================
    FILE* data = fopen(account_file, "r");
    FILE* temp = fopen("temp.txt", "w");
    if (!data || !temp) { invalid_file(); return; }

    while (fgets(buffer, sizeof(buffer), data)) {
        buffer[strcspn(buffer, "\n")] = 0;
        sscanf(buffer, "%[^,],%[^,],%[^,],%[^\n]", acc.username, acc.password, acc.name, acc.email);

        // Salin semua akun KECUALI akun user yang sedang login
        if (strcmp(acc.username, current_user) != 0) {
            fprintf(temp, "%s,%s,%s,%s\n", acc.username, acc.password, acc.name, acc.email);
        }
    }

    fclose(data);
    fclose(temp);
    remove(account_file);
    rename("temp.txt", account_file);

    // Kosongkan variabel current_user karena akun sudah tidak ada
    memset(current_user, 0, sizeof(current_user));


    // ==========================================
    // PAGE 3 : SUCCESS MESSAGE
    // ==========================================
    system("cls");
    printf("============================================\n");
    printf("              ACCOUNT DELETED               \n");
    printf("============================================\n");
    printf("\n");
    printf("       Account deleted successfully!        \n");
    printf("\n");
    printf("============================================\n");
    system("pause");
    
    // Langsung arahkan kembali ke menu utama (Main Menu)
    main_menu();
}

void change_name() {
    system("cls");
    char buffer[1024];
    Account acc;
    char current_name[100] = "";
    char new_name[100];
    int valid;

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

    printf("============================================\n");
    printf("              CHANGE FULL NAME              \n");
    printf("============================================\n");
    printf("Current Full Name : %s\n", current_name);
    printf("--------------------------------------------\n");
    printf(" (Enter '0' to go back)\n");
    printf("--------------------------------------------\n");


    do {
        printf("Enter new Full Name : ");
        fgets(new_name, sizeof(new_name), stdin);
        new_name[strcspn(new_name, "\n")] = '\0';

        if (strcmp(new_name, "0") == 0) {
            edit_profile();
            return;
        }
        if (strlen(new_name) == 0) {
            printf("Full Name cannot be empty!\n");
            system("pause"); edit_profile(); return;
        }
        if (strcmp(current_name, new_name) == 0) {
            printf("New Full Name cannot be the same as current!\n");
            system("pause"); edit_profile(); return;
        }
        valid = 1;
        for (int i = 0; new_name[i] != '\0'; i++) {
            if (isdigit(new_name[i])) {
                printf("Full name cannot contain numbers!\n");
                valid = 0; break;
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
    system("cls");
    char buffer[1024];
    Account acc;
    char new_usn[100];
    int found, has_space;

    printf("============================================\n");
    printf("              CHANGE USERNAME               \n");
    printf("============================================\n");
    printf("Current Username : %s\n", current_user);
    printf("--------------------------------------------\n");
    printf(" (Enter '0' to go back)\n");
    printf("--------------------------------------------\n");

    do {
        printf("Enter new Username : ");
        fgets(new_usn, sizeof(new_usn), stdin);
        new_usn[strcspn(new_usn, "\n")] = '\0';
        // Fitur Back
        if (strcmp(new_usn, "0") == 0) {
            edit_profile();
            return;
        }
        if (strlen(new_usn) == 0) {
            printf("Username cannot be empty!\n");
            system("pause"); edit_profile(); return;
        }
        if (strcmp(current_user, new_usn) == 0) {
            printf("New Username cannot be the same as current!\n");
            system("pause"); edit_profile(); return;
        }
        has_space = 0;
        for (int i = 0; new_usn[i] != '\0'; i++)
            if (isspace(new_usn[i])) { has_space = 1; break; }
        if (has_space) { printf("Username cannot contain spaces!\n"); found = 1; continue; }

        found = 0;
        FILE* chk = fopen(account_file, "r");
        if (chk != NULL) {
            char tb[1024], cu[100];
            while (fgets(tb, sizeof(tb), chk)) {
                sscanf(tb, "%[^,]", cu);
                if (strcmp(cu, new_usn) == 0) {
                    found = 1;
                    printf("Username already exists!\n"); break;
                }
            }
            fclose(chk);
        }
    } while (found);

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
    system("cls");
    char buffer[1024];
    Account acc;
    char current_email[100] = "";
    char new_email[100];
    char confirm_email[100];
    int validasi_at;

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

    printf("============================================\n");
    printf("                CHANGE EMAIL                \n");
    printf("============================================\n");
    printf("Current Email : %s\n", current_email);
    printf("--------------------------------------------\n");
    printf(" (Enter '0' to go back)\n");
    printf("--------------------------------------------\n");

    do {
        printf("Enter new Email : ");
        fgets(new_email, sizeof(new_email), stdin);
        new_email[strcspn(new_email, "\n")] = '\0';
         // Fitur Back
        if (strcmp(new_email, "0") == 0) {
            edit_profile();
            return;
        }
        if (strlen(new_email) == 0) {
            printf("Email cannot be empty!\n");
            system("pause"); edit_profile(); return;
        }
        if (strcmp(current_email, new_email) == 0) {
            printf("New Email cannot be the same as current!\n");
            system("pause"); edit_profile(); return;
        }
        validasi_at = 0;
        for (int i = 0; new_email[i] != '\0'; i++)
            if (new_email[i] == '@') { validasi_at = 1; break; }
        if (!validasi_at) { printf("Email must contain '@'!\n"); continue; }

        printf("Confirm new Email : ");
        fgets(confirm_email, sizeof(confirm_email), stdin);
        confirm_email[strcspn(confirm_email, "\n")] = '\0';
        if (strcmp(confirm_email, "0") == 0) {
            edit_profile();
            return;
        }
        if (strcmp(new_email, confirm_email) != 0)
            printf("Email does not match! Try again!\n");
    } while (!validasi_at || strcmp(new_email, confirm_email) != 0);

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
    system("cls");
    char buffer[1024];
    Account acc;
    char current_pass[100] = "";
    char new_pass[100];
    char confirm_pass[100];

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

    printf("============================================\n");
    printf("              CHANGE PASSWORD               \n");
    printf("============================================\n");
    printf("Current Password : ");
    for (int i = 0; i < (int)strlen(current_pass); i++) printf("*");
    printf("\n--------------------------------------------\n");
    printf(" (Enter '0' to go back)\n");
    printf("--------------------------------------------\n");

    do {
        printf("Enter new Password : ");
        fgets(new_pass, sizeof(new_pass), stdin);
        new_pass[strcspn(new_pass, "\n")] = '\0';
        if (strcmp(new_pass, "0") == 0) {
            edit_profile();
            return;
        }
        if (strlen(new_pass) == 0) {
            printf("Password cannot be empty!\n");
            system("pause"); edit_profile(); return;
        }
        if (strcmp(current_pass, new_pass) == 0) {
            printf("New Password cannot be the same as current!\n");
            system("pause"); edit_profile(); return;
        }
        if (strlen(new_pass) < 5) {
            printf("Password must be at least 5 characters long!\n"); continue;
        }
        printf("Confirm new Password : ");
        fgets(confirm_pass, sizeof(confirm_pass), stdin);
        confirm_pass[strcspn(confirm_pass, "\n")] = '\0';
        if (strcmp(confirm_pass, "0") == 0) {
            edit_profile();
            return;
        }
        if (strcmp(new_pass, confirm_pass) != 0)
            printf("Password does not match! Try again!\n");
    } while (strlen(new_pass) < 5 || strcmp(new_pass, confirm_pass) != 0);

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