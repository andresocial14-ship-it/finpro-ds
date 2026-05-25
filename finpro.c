//Testes git Rafif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

// !! DEFINE FILE !! //
#define account_file "users.txt"
#define film_file "films.txt"
#define studio_file "studios.txt"
#define schedule_file "schedules.txt"
#define booking_file "bookings.txt"

// !! GLOBAL VARIABLE !! //
char current_user[100];

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

// !! FUNCTION STRUCT !! //
Account* find_username(char* username, char* password);

// !! PRINTILAN !! // 
void invalid_choice();  // DONE
void invalid_file();    // DONE
int auto_id();

// !! MENU !! //
void main_menu();       // DONE
void login();           // DONE
void register_acc();    // DONE

// ---------------- //
// !! MENU ADMIN !! //
// ---------------- //
void menu_admin();      // DONE

// ! FILM MANAGEMENT ! //
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

// ! TICKET PRICE MANAGEMENT ! //
void price_manage();
void set_weekday();
void set_weekend();
void set_vip();
void view_curr_prices();

// ! REPORTS & STATISTICS ! //
void reports();
void view_all_transactions();
void revenue_film();
void revenue_day();
void export_reports();

// ! USER ACCOUNT MANAGEMENT ! //
void acc_manage();
void view_users();
void search_user();
void deactivate_user();

// ------------------- //
// !! MENU CUSTOMER !! //
// ------------------- //
void menu_cust();
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
void menu_cashier();    // DONE
void validate_ticket();
void sell();
void seat_status();

int main () {
    main_menu();
    return 0;
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

// AUTO ID FILM //
int auto_id() {
    FILE* fp = fopen(film_file, "r");
    if (fp == NULL) return 0;

    char buffer[500];
    int last_id = 0;
    int id;

    while (fgets(buffer, sizeof(buffer), fp)) {
        sscanf(buffer, "%d,", &id);
        last_id = id;
    }
    fclose(fp);
    return last_id;
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

    do {
        system("cls");
        printf("============================================\n");
        printf("                 ADMIN MENU                 \n");
        printf("--------------------------------------------\n");
        printf("Welcome Back, Admin!\n");
        printf("[1] Film Management\n");
        printf("[2] Studio & Schedule Management\n");
        printf("[3] Ticket Price Management\n");
        printf("[4] Reports & Statistics\n");
        printf("[5] User Account Management\n");
        printf("[0] Logout\n");
        printf("============================================\n");    
        printf("Choose : ");
        scanf("%d", &choice);
        
        if (choice < 0 || choice > 5) {
            invalid_choice();
        }
    } while(choice < 0 || choice > 5); 

    switch (choice) {
        case 1 : 
            film_manage();
            break;
        case 2 : 
            schedule_manage();
            break;
        case 3 : 
            price_manage();
            break;
        case 4 : 
            reports();
            break;
        case 5 : 
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
        printf("Title         : ");
        scanf(" %[^\n]", film.title);

        valid = 1;
        if (strlen(film.title) == 0) {
            printf("Title cannot be empty!\n");
            valid = 0;
        }
    } while(!valid);

    // genre
    do {
        printf("Genre         : ");
        scanf(" %[^\n]", film.genre);

        valid = 1;
        if (strlen(film.genre) == 0) {
            printf("Genre cannot be empty!\n");
            valid = 0;
        }
    } while(!valid);

    // duration
    do {
        printf("Duration (min): ");
        scanf("%d", &film.duration);

        if (film.duration <= 0) {
            printf("Duration must be greater than 0!\n");
        }
    } while(film.duration <= 0);

    // age rating
    do {
        printf("Age Rating (n+)  : ");
        scanf("%d", &film.age_rating);

        if (film.age_rating < 0) {
            printf("Age rating cannot be negative!\n");
        }
    } while(film.age_rating < 0);

    // synopsis
    do {
        printf("Synopsis      : ");
        scanf(" %[^\n]", film.detail);

        valid = 1;
        if (strlen(film.detail) == 0) {
            printf("Detail cannot be empty!\n");
            valid = 0;
        }
    } while(!valid);

    // auto ID
    film.id = auto_id() + 1;

    FILE* fp = fopen(film_file, "a");
    if (fp == NULL) {
        invalid_file();
        return;
    }
    fprintf(fp, "%d=%s=%s=%d=%d=%s\n", film.id, film.title, film.genre, film.duration, film.age_rating, film.detail);
    fclose(fp);

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
void del_film () {

}

// EDIT FILM // 
void edit_film () {

}

// VIEW FILM //
void view_film () {

}

// SEARCH FILM //
void search_film () {

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

// SUB-MENU TICKET PRICE MANAGEMENT // 
void price_manage () {
    int choice;

    do {
        system("cls");
        printf("============================================\n");
        printf("            TICKET PRICE MANAGEMENT         \n");
        printf("--------------------------------------------\n");
        printf("[1] Set Weekday Price\n");
        printf("[2] Set Weekend Price\n");
        printf("[3] Set VIP Price\n");
        printf("[4] View Current Prices\n");
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
            set_weekday();
            break;
        case 2 : 
            set_weekend();
            break;
        case 3 : 
            set_vip();
            break;
        case 4 : 
            view_curr_prices();
            break;
        case 0 : 
            menu_admin();
            break;
    }
}

// SET WEEKDAY //
void set_weekday () {

}

// SET WEEKEND //
void set_weekend () {

}

// SET VIP //
void set_vip () {

}

// VIEW CURRENT PRICES //
void view_curr_prices () {

}

// SUB-MENU REPORTS & STATISTICS // 
void reports () {
    int choice;

    do {
        system("cls");
        printf("============================================\n");
        printf("             REPORTS & STATISTICS           \n");
        printf("--------------------------------------------\n");
        printf("[1] View All Transactions\n");
        printf("[2] Revenue per Film\n");
        printf("[3] Revenue per Day\n");
        printf("[4] Export Reports to .txt\n");
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
            view_all_transactions();
            break;
        case 2 : 
            revenue_film();
            break;
        case 3 : 
            revenue_day();
            break;
        case 4 : 
            export_reports();
            break;
        case 0 : 
            menu_admin();
            break;
    }
}

// VIEW ALL TRANSACTIONS //
void view_all_transactions () {

}

// REVENUE FILM //
void revenue_film () {
    
}

// REVENUE DAY //
void revenue_day () {

}

// EXPORT REPORTS //
void export_reports () {

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
        printf("[3] Deactivate User Account\n");
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
            deactivate_user();
            break;
        case 0 : 
            menu_admin();
            break;
    }
}

// VIEW USERS //
void view_users () {

}

// SEARCH USER //
void search_user () {

}

// DEACTIVATE USER //
void deactivate_user () {

}

// MENU CUSTOMER // 
void menu_cust () {
    int choice;

    do {
        system("cls");
        printf("============================================\n");
        printf("               CUSTOMER MENU                \n");
        printf("--------------------------------------------\n");
        // TAMPILIN NOW SHOWING MOVIE
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

void validate_ticket () {
    // konfirmasi tiket online
}

void sell () {
    // jual tiket offline
}

void seat_status () {
    // ngatur kursi booked, tersedia
}