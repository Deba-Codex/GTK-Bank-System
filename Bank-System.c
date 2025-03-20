#include <gtk/gtk.h>
#include <cairo.h>
#include <cairo-pdf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#define USER_FILE "users.txt"
#define TRANSACTION_FILE "transactions.txt"
#define RECEIPT_PDF "receipt.pdf"
#define MAX_USERS 100
#define MAX_TRANSACTIONS 100

typedef struct {
    char username[50];
    char password[50];
    double balance;
} User;

typedef struct {
    char username[50];
    char type[20];
    double amount;
    char datetime[64];
} Transaction;

User current_user = {"", "", 0.0};

typedef struct {
    GtkWidget *login_username;
    GtkWidget *login_password;
    GtkWidget *amount_entry;
    GtkWidget *status_label;
    GtkWidget *balance_label;
    GtkWidget *login_window;
    GtkWidget *main_window;
} BankWidgets;

// Helper functions
int is_valid_number(const char *str) {
    int dot_count = 0;
    for (; *str; str++) {
        if (*str == '.') {
            if (dot_count++ > 0) return 0;
            continue;
        }
        if (!isdigit(*str)) return 0;
    }
    return 1;
}

void get_current_datetime(char *buffer, size_t size) {
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    strftime(buffer, size, "%Y-%m-%d %H:%M:%S", tm_info);
}

// File operations
int user_exists(const char *username) {
    FILE *file = fopen(USER_FILE, "r");
    if (!file) return 0;
    
    User user;
    while (fscanf(file, "%49s %49s %lf", user.username, user.password, &user.balance) == 3) {
        if (strcmp(username, user.username) == 0) {
            fclose(file);
            return 1;
        }
    }
    fclose(file);
    return 0;
}

int load_user(const char *username, const char *password) {
    FILE *file = fopen(USER_FILE, "r");
    if (!file) return 0;
    
    User user;
    while (fscanf(file, "%49s %49s %lf", user.username, user.password, &user.balance) == 3) {
        if (strcmp(username, user.username) == 0 && strcmp(password, user.password) == 0) {
            current_user = user;
            fclose(file);
            return 1;
        }
    }
    fclose(file);
    return 0;
}

void update_user_file() {
    User users[MAX_USERS];
    int count = 0;
    FILE *file = fopen(USER_FILE, "r");
    
    if (file) {
        while (count < MAX_USERS && 
              fscanf(file, "%49s %49s %lf", users[count].username, 
                     users[count].password, &users[count].balance) == 3) {
            count++;
        }
        fclose(file);
    }

    int found = 0;
    for (int i = 0; i < count; i++) {
        if (strcmp(users[i].username, current_user.username) == 0) {
            users[i] = current_user;
            found = 1;
            break;
        }
    }
    
    if (!found && count < MAX_USERS) {
        users[count++] = current_user;
    }

    file = fopen(USER_FILE, "w");
    if (file) {
        for (int i = 0; i < count; i++) {
            fprintf(file, "%s %s %.2f\n", users[i].username, 
                    users[i].password, users[i].balance);
        }
        fclose(file);
    }
}

void save_transaction(const char *type, double amount) {
    FILE *file = fopen(TRANSACTION_FILE, "a");
    if (file) {
        char datetime[64];
        get_current_datetime(datetime, sizeof(datetime));
        fprintf(file, "%s %s %.2f %s\n", current_user.username, type, amount, datetime);
        fclose(file);
    }
}

int load_user_transactions(Transaction transactions[], int max_count) {
    FILE *file = fopen(TRANSACTION_FILE, "r");
    if (!file) return 0;
    
    int count = 0;
    char line[256];
    while (count < max_count && fgets(line, sizeof(line), file)) {
        Transaction t;
        if (sscanf(line, "%49s %19s %lf %63[^\n]", 
                  t.username, t.type, &t.amount, t.datetime) == 4) {
            if (strcmp(t.username, current_user.username) == 0) {
                transactions[count++] = t;
            }
        }
    }
    fclose(file);
    return count;
}

// PDF Generation
int generate_pdf_receipt() {
    cairo_surface_t *surface = cairo_pdf_surface_create(RECEIPT_PDF, 595, 842);
    if (cairo_surface_status(surface)) {
        return 0;
    }

    cairo_t *cr = cairo_create(surface);
    if (cairo_status(cr)) {
        cairo_surface_destroy(surface);
        return 0;
    }

    // Header
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 24);
    cairo_move_to(cr, 50, 50);
    cairo_show_text(cr, "Bank Receipt");

    // Account Info
    cairo_set_font_size(cr, 14);
    cairo_move_to(cr, 50, 80);
    cairo_show_text(cr, current_user.username);

    char datetime[64];
    get_current_datetime(datetime, sizeof(datetime));
    cairo_move_to(cr, 50, 100);
    cairo_show_text(cr, datetime);

    // Transactions
    Transaction transactions[MAX_TRANSACTIONS];
    int count = load_user_transactions(transactions, MAX_TRANSACTIONS);
    cairo_move_to(cr, 50, 130);
    cairo_show_text(cr, "Transactions:");

    double y = 160;
    for (int i = 0; i < count; i++) {
        char line[128];
        snprintf(line, sizeof(line), "%s: ₹%.2f - %s", 
                transactions[i].type, transactions[i].amount, transactions[i].datetime);
        cairo_move_to(cr, 60, y);
        cairo_show_text(cr, line);
        y += 20;
    }

    // Balance
    cairo_move_to(cr, 50, y + 30);
    char balance_str[64];
    snprintf(balance_str, sizeof(balance_str), "Current Balance: ₹%.2f", current_user.balance);
    cairo_show_text(cr, balance_str);

    cairo_show_page(cr);
    cairo_destroy(cr);
    cairo_surface_destroy(surface);
    return 1;
}

// Callbacks
void on_show_receipt(GtkWidget *widget, BankWidgets *widgets) {
    if (generate_pdf_receipt()) {
        gtk_label_set_text(GTK_LABEL(widgets->status_label), "Receipt generated!");
        
        // Platform-specific command to open the PDF
        #ifdef _WIN32
            system("start " RECEIPT_PDF);
        #elif __APPLE__
            system("open " RECEIPT_PDF);
        #else
            system("xdg-open " RECEIPT_PDF);
        #endif
    } else {
        gtk_label_set_text(GTK_LABEL(widgets->status_label), "Failed to generate receipt!");
    }
}

void on_deposit(GtkWidget *widget, BankWidgets *widgets) {
    const char *amount_str = gtk_entry_get_text(GTK_ENTRY(widgets->amount_entry));
    
    if (!is_valid_number(amount_str)) {
        gtk_label_set_text(GTK_LABEL(widgets->status_label), "Invalid amount!");
        return;
    }
    
    double amount = atof(amount_str);
    if (amount <= 0) {
        gtk_label_set_text(GTK_LABEL(widgets->status_label), "Amount must be positive!");
        return;
    }

    current_user.balance += amount;
    update_user_file();
    save_transaction("DEPOSIT", amount);
    
    char balance[64];
    snprintf(balance, sizeof(balance), "Balance: ₹%.2f", current_user.balance);
    gtk_label_set_text(GTK_LABEL(widgets->balance_label), balance);
    gtk_entry_set_text(GTK_ENTRY(widgets->amount_entry), "");
    gtk_label_set_text(GTK_LABEL(widgets->status_label), "Deposit successful!");
}

void on_withdraw(GtkWidget *widget, BankWidgets *widgets) {
    const char *amount_str = gtk_entry_get_text(GTK_ENTRY(widgets->amount_entry));
    
    if (!is_valid_number(amount_str)) {
        gtk_label_set_text(GTK_LABEL(widgets->status_label), "Invalid amount!");
        return;
    }
    
    double amount = atof(amount_str);
    if (amount <= 0) {
        gtk_label_set_text(GTK_LABEL(widgets->status_label), "Amount must be positive!");
        return;
    }
    
    if (amount > current_user.balance) {
        // Show a pop-up dialog for insufficient balance
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(widgets->main_window),
                                                  GTK_DIALOG_MODAL,
                                                  GTK_MESSAGE_ERROR,
                                                  GTK_BUTTONS_OK,
                                                  "Insufficient balance!");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return;
    }

    current_user.balance -= amount;
    update_user_file();
    save_transaction("WITHDRAW", amount);
    
    char balance[64];
    snprintf(balance, sizeof(balance), "Balance: ₹%.2f", current_user.balance);
    gtk_label_set_text(GTK_LABEL(widgets->balance_label), balance);
    gtk_entry_set_text(GTK_ENTRY(widgets->amount_entry), "");
    gtk_label_set_text(GTK_LABEL(widgets->status_label), "Withdrawal successful!");
}

void on_login(GtkWidget *widget, BankWidgets *widgets) {
    const char *username = gtk_entry_get_text(GTK_ENTRY(widgets->login_username));
    const char *password = gtk_entry_get_text(GTK_ENTRY(widgets->login_password));
    
    if (strlen(username) == 0 || strlen(password) == 0) {
        gtk_label_set_text(GTK_LABEL(widgets->status_label), "Enter username/password!");
        return;
    }
    
    if (load_user(username, password)) {
        gtk_widget_hide(widgets->login_window);
        gtk_widget_show_all(widgets->main_window);
        
        char balance_str[64];
        snprintf(balance_str, sizeof(balance_str), "Balance: ₹%.2f", current_user.balance);
        gtk_label_set_text(GTK_LABEL(widgets->balance_label), balance_str);
    } else {
        if (user_exists(username)) {
            gtk_label_set_text(GTK_LABEL(widgets->status_label), "Invalid password!");
        } else {
            // Register new user
            strcpy(current_user.username, username);
            strcpy(current_user.password, password);
            current_user.balance = 0.0;
            update_user_file();
            gtk_widget_hide(widgets->login_window);
            gtk_widget_show_all(widgets->main_window);
            gtk_label_set_text(GTK_LABEL(widgets->balance_label), "Balance: ₹0.00");
        }
    }
}

void on_change_password(GtkWidget *widget, BankWidgets *widgets) {
    GtkWidget *dialog = gtk_dialog_new_with_buttons("Change Password",
                                                  GTK_WINDOW(widgets->main_window),
                                                  GTK_DIALOG_MODAL,
                                                  "OK", GTK_RESPONSE_OK,
                                                  "Cancel", GTK_RESPONSE_CANCEL,
                                                  NULL);
    GtkWidget *content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *entry = gtk_entry_new();
    gtk_entry_set_visibility(GTK_ENTRY(entry), FALSE);
    gtk_container_add(GTK_CONTAINER(content), entry);
    gtk_widget_show_all(dialog);
    
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
        const char *new_pass = gtk_entry_get_text(GTK_ENTRY(entry));
        if (strlen(new_pass) >= 3) {
            strcpy(current_user.password, new_pass);
            update_user_file();
            gtk_label_set_text(GTK_LABEL(widgets->status_label), "Password changed!");
        } else {
            gtk_label_set_text(GTK_LABEL(widgets->status_label), "Password too short!");
        }
    }
    gtk_widget_destroy(dialog);
}

void on_logout(GtkWidget *widget, BankWidgets *widgets) {
    memset(&current_user, 0, sizeof(User));
    gtk_entry_set_text(GTK_ENTRY(widgets->login_username), "");
    gtk_entry_set_text(GTK_ENTRY(widgets->login_password), "");
    gtk_label_set_text(GTK_LABEL(widgets->status_label), "");
    gtk_widget_hide(widgets->main_window);
    gtk_widget_show_all(widgets->login_window);
}

void activate(GtkApplication *app, gpointer data) {
    BankWidgets *widgets = g_malloc(sizeof(BankWidgets));

    // Login Window
    widgets->login_window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(widgets->login_window), "Bank Login");
    gtk_window_set_default_size(GTK_WINDOW(widgets->login_window), 300, 200);
    
    GtkWidget *login_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(login_box), 15);
    gtk_container_add(GTK_CONTAINER(widgets->login_window), login_box);
    
    widgets->login_username = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(widgets->login_username), "Username");
    gtk_box_pack_start(GTK_BOX(login_box), widgets->login_username, FALSE, FALSE, 5);
    
    widgets->login_password = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(widgets->login_password), "Password");
    gtk_entry_set_visibility(GTK_ENTRY(widgets->login_password), FALSE);
    gtk_box_pack_start(GTK_BOX(login_box), widgets->login_password, FALSE, FALSE, 5);
    
    GtkWidget *login_btn = gtk_button_new_with_label("Login/Register");
    g_signal_connect(login_btn, "clicked", G_CALLBACK(on_login), widgets);
    gtk_box_pack_start(GTK_BOX(login_box), login_btn, FALSE, FALSE, 5);
    
    widgets->status_label = gtk_label_new("");
    gtk_box_pack_start(GTK_BOX(login_box), widgets->status_label, FALSE, FALSE, 5);

    // Main Window
    widgets->main_window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(widgets->main_window), "Bank Account");
    gtk_window_set_default_size(GTK_WINDOW(widgets->main_window), 400, 300);
    
    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(main_box), 15);
    gtk_container_add(GTK_CONTAINER(widgets->main_window), main_box);
    
    widgets->amount_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(widgets->amount_entry), "Amount in INR");
    gtk_box_pack_start(GTK_BOX(main_box), widgets->amount_entry, FALSE, FALSE, 5);
    
    GtkWidget *btn_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    GtkWidget *deposit_btn = gtk_button_new_with_label("Deposit");
    GtkWidget *withdraw_btn = gtk_button_new_with_label("Withdraw");
    g_signal_connect(deposit_btn, "clicked", G_CALLBACK(on_deposit), widgets);
    g_signal_connect(withdraw_btn, "clicked", G_CALLBACK(on_withdraw), widgets);
    gtk_box_pack_start(GTK_BOX(btn_box), deposit_btn, TRUE, TRUE, 5);
    gtk_box_pack_start(GTK_BOX(btn_box), withdraw_btn, TRUE, TRUE, 5);
    gtk_box_pack_start(GTK_BOX(main_box), btn_box, FALSE, FALSE, 5);
    
    widgets->balance_label = gtk_label_new("Balance: ₹0.00");
    gtk_box_pack_start(GTK_BOX(main_box), widgets->balance_label, FALSE, FALSE, 5);
    
    // Add Generate Receipt button
        // Add Generate Receipt button
        GtkWidget *receipt_btn = gtk_button_new_with_label("Generate Receipt");
        g_signal_connect(receipt_btn, "clicked", G_CALLBACK(on_show_receipt), widgets);
        gtk_box_pack_start(GTK_BOX(main_box), receipt_btn, FALSE, FALSE, 5);
    
        // Add Change Password button
        GtkWidget *pass_btn = gtk_button_new_with_label("Change Password");
        g_signal_connect(pass_btn, "clicked", G_CALLBACK(on_change_password), widgets);
        gtk_box_pack_start(GTK_BOX(main_box), pass_btn, FALSE, FALSE, 5);
    
        // Add Logout button
        GtkWidget *logout_btn = gtk_button_new_with_label("Logout");
        g_signal_connect(logout_btn, "clicked", G_CALLBACK(on_logout), widgets);
        gtk_box_pack_start(GTK_BOX(main_box), logout_btn, FALSE, FALSE, 5);
    
        // Show initial windows
        gtk_widget_show_all(widgets->login_window);
        gtk_widget_hide(widgets->main_window);
    }
    
    int main(int argc, char **argv) {
        GtkApplication *app = gtk_application_new("com.example.bank", G_APPLICATION_DEFAULT_FLAGS);
        g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
        int status = g_application_run(G_APPLICATION(app), argc, argv);
        g_object_unref(app);
        return status;
    }