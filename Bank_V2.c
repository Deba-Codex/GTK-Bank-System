#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define USER_FILE "users.txt"
#define MAX_USERS 100

typedef struct {
    char username[50];
    char password[50];
    gdouble balance;
} User;

User current_user = {"", "", 0.0};

typedef struct {
    GtkWidget *login_username;
    GtkWidget *login_password;
    GtkWidget *amount;
    GtkWidget *status_label;
    GtkWidget *balance_label;
    GtkWidget *receipt_label;
    GtkWidget *login_window;
    GtkWidget *main_window;
} BankWidgets;

/* Check if a username exists (regardless of password) */
int user_exists(const char *username) {
    FILE *file = fopen(USER_FILE, "r");
    if (!file) return 0;
    User user;
    int exists = 0;
    while (fscanf(file, "%49s %49s %lf", user.username, user.password, &user.balance) == 3) {
        if (strcmp(user.username, username) == 0) {
            exists = 1;
            break;
        }
    }
    fclose(file);
    return exists;
}

/* Load a user matching username and password.
   Returns 1 if found and loads into current_user, else 0. */
int load_user(const char *username, const char *password) {
    FILE *file = fopen(USER_FILE, "r");
    if (!file) return 0;
    User user;
    while (fscanf(file, "%49s %49s %lf", user.username, user.password, &user.balance) == 3) {
        if (strcmp(user.username, username) == 0 && strcmp(user.password, password) == 0) {
            current_user = user;
            fclose(file);
            return 1;
        }
    }
    fclose(file);
    return 0;
}

/* Read all users from file, update current_user's record (or add it if not found),
   then rewrite the file with updated records. */
void update_user_file() {
    User users[MAX_USERS];
    int count = 0;
    FILE *file = fopen(USER_FILE, "r");
    if (file) {
        while (count < MAX_USERS && fscanf(file, "%49s %49s %lf", users[count].username, users[count].password, &users[count].balance) == 3) {
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
            fprintf(file, "%s %s %.2f\n", users[i].username, users[i].password, users[i].balance);
        }
        fclose(file);
    }
}

/* Update the balance label on the main window */
void update_balance_label(BankWidgets *widgets) {
    char balance_str[50];
    snprintf(balance_str, sizeof(balance_str), "Balance: %.2f", current_user.balance);
    gtk_label_set_text(GTK_LABEL(widgets->balance_label), balance_str);
}

/* Login callback: if user exists and password matches, log in;
   if username does not exist, register new user.
   If username exists but password is wrong, show error. */
void on_login(GtkWidget *widget, gpointer data) {
    BankWidgets *widgets = (BankWidgets *)data;
    const gchar *username = gtk_entry_get_text(GTK_ENTRY(widgets->login_username));
    const gchar *password = gtk_entry_get_text(GTK_ENTRY(widgets->login_password));
    
    if (strlen(username) == 0 || strlen(password) == 0) {
        gtk_label_set_text(GTK_LABEL(widgets->status_label), "Enter username and password!");
        return;
    }
    
    if (load_user(username, password)) {
        /* Successful login for existing user */
        gtk_widget_hide(widgets->login_window);
        gtk_widget_show_all(widgets->main_window);
        update_balance_label(widgets);
    } else {
        if (user_exists(username)) {
            gtk_label_set_text(GTK_LABEL(widgets->status_label), "Invalid credentials!");
        } else {
            /* New user registration */
            strcpy(current_user.username, username);
            strcpy(current_user.password, password);
            current_user.balance = 0.0;
            update_user_file();
            gtk_widget_hide(widgets->login_window);
            gtk_widget_show_all(widgets->main_window);
            update_balance_label(widgets);
        }
    }
}

/* Deposit callback */
void on_deposit(GtkWidget *widget, gpointer data) {
    BankWidgets *widgets = (BankWidgets *)data;
    const gchar *amount_text = gtk_entry_get_text(GTK_ENTRY(widgets->amount));
    
    if (strlen(amount_text) == 0) {
        gtk_label_set_text(GTK_LABEL(widgets->status_label), "Please enter an amount!");
        return;
    }
    
    gdouble amount = g_ascii_strtod(amount_text, NULL);
    if (amount <= 0) {
        gtk_label_set_text(GTK_LABEL(widgets->status_label), "Invalid amount!");
        return;
    }
    
    current_user.balance += amount;
    update_user_file();
    update_balance_label(widgets);
    gtk_label_set_text(GTK_LABEL(widgets->status_label), "Deposit Successful!");
    gtk_entry_set_text(GTK_ENTRY(widgets->amount), "");
}

/* Withdraw callback */
void on_withdraw(GtkWidget *widget, gpointer data) {
    BankWidgets *widgets = (BankWidgets *)data;
    const gchar *amount_text = gtk_entry_get_text(GTK_ENTRY(widgets->amount));
    
    if (strlen(amount_text) == 0) {
        gtk_label_set_text(GTK_LABEL(widgets->status_label), "Please enter an amount!");
        return;
    }
    
    gdouble amount = g_ascii_strtod(amount_text, NULL);
    if (amount <= 0 || amount > current_user.balance) {
        gtk_label_set_text(GTK_LABEL(widgets->status_label), "Invalid or insufficient funds!");
        return;
    }
    
    current_user.balance -= amount;
    update_user_file();
    update_balance_label(widgets);
    gtk_label_set_text(GTK_LABEL(widgets->status_label), "Withdrawal Successful!");
    gtk_entry_set_text(GTK_ENTRY(widgets->amount), "");
}

/* Setup the UI: two windows – a login window and a main banking window */
void activate(GtkApplication *app, gpointer user_data) {
    BankWidgets *widgets = g_malloc(sizeof(BankWidgets));
    
    /* --- Create Login Window --- */
    widgets->login_window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(widgets->login_window), "Login");
    gtk_window_set_default_size(GTK_WINDOW(widgets->login_window), 300, 200);
    
    GtkWidget *login_grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(widgets->login_window), login_grid);
    
    widgets->login_username = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(widgets->login_username), "Username");
    gtk_grid_attach(GTK_GRID(login_grid), widgets->login_username, 0, 0, 2, 1);
    
    widgets->login_password = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(widgets->login_password), "Password");
    gtk_entry_set_visibility(GTK_ENTRY(widgets->login_password), FALSE);
    gtk_grid_attach(GTK_GRID(login_grid), widgets->login_password, 0, 1, 2, 1);
    
    widgets->status_label = gtk_label_new("");
    gtk_grid_attach(GTK_GRID(login_grid), widgets->status_label, 0, 2, 2, 1);
    
    GtkWidget *login_button = gtk_button_new_with_label("Login");
    gtk_grid_attach(GTK_GRID(login_grid), login_button, 0, 3, 2, 1);
    g_signal_connect(login_button, "clicked", G_CALLBACK(on_login), widgets);
    
    /* --- Create Main Banking Window --- */
    widgets->main_window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(widgets->main_window), "Bank Management System");
    gtk_window_set_default_size(GTK_WINDOW(widgets->main_window), 400, 400);
    gtk_widget_hide(widgets->main_window);
    
    GtkWidget *main_grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(widgets->main_window), main_grid);
    
    widgets->amount = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(widgets->amount), "Enter Amount");
    gtk_grid_attach(GTK_GRID(main_grid), widgets->amount, 0, 0, 2, 1);
    
    widgets->balance_label = gtk_label_new("Balance: 0.00");
    gtk_grid_attach(GTK_GRID(main_grid), widgets->balance_label, 0, 1, 2, 1);
    
    widgets->receipt_label = gtk_label_new("");
    gtk_grid_attach(GTK_GRID(main_grid), widgets->receipt_label, 0, 2, 2, 1);
    
    GtkWidget *deposit_button = gtk_button_new_with_label("Deposit");
    GtkWidget *withdraw_button = gtk_button_new_with_label("Withdraw");
    gtk_grid_attach(GTK_GRID(main_grid), deposit_button, 0, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(main_grid), withdraw_button, 1, 3, 1, 1);
    
    g_signal_connect(deposit_button, "clicked", G_CALLBACK(on_deposit), widgets);
    g_signal_connect(withdraw_button, "clicked", G_CALLBACK(on_withdraw), widgets);
    
    /* Show the login window to start */
    gtk_widget_show_all(widgets->login_window);
}

int main(int argc, char **argv) {
    GtkApplication *app = gtk_application_new("com.bank.system", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return status;
}
