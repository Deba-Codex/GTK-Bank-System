#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define USER_FILE "users.txt"
#define TRANSACTION_FILE "transactions.txt"
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
    GtkWidget *transaction_view;
    GtkWidget *login_window;
    GtkWidget *main_window;
} BankWidgets;

/* Update the user file with current_user's record */
void update_user_file() {
    User users[MAX_USERS];
    int count = 0;
    FILE *file = fopen(USER_FILE, "r");
    if (file) {
        while (count < MAX_USERS && fscanf(file, "%49s %49s %lf", 
               users[count].username, users[count].password, &users[count].balance) == 3) {
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

/* Save a transaction record */
void save_transaction(const char *type, gdouble amount) {
    FILE *file = fopen(TRANSACTION_FILE, "a");
    if (file) {
        fprintf(file, "%s %s %.2f\n", current_user.username, type, amount);
        fclose(file);
    }
}

/* Load transactions for current user into the text view */
void load_transactions(BankWidgets *widgets) {
    FILE *file = fopen(TRANSACTION_FILE, "r");
    if (!file) {
        GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(widgets->transaction_view));
        gtk_text_buffer_set_text(buffer, "", -1);
        return;
    }
    char line[200];
    char all[10000] = "";
    while (fgets(line, sizeof(line), file)) {
        if (strstr(line, current_user.username) != NULL) {
            strcat(all, line);
        }
    }
    fclose(file);
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(widgets->transaction_view));
    gtk_text_buffer_set_text(buffer, all, -1);
}

/* Update balance label on main window */
void update_balance_label(BankWidgets *widgets) {
    char balance_str[50];
    snprintf(balance_str, sizeof(balance_str), "Balance: %.2f", current_user.balance);
    gtk_label_set_text(GTK_LABEL(widgets->balance_label), balance_str);
}

/* Load a user matching username and password */
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

/* Check if a username exists */
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

/* Login callback: log in existing user or register new one */
void on_login(GtkWidget *widget, gpointer data) {
    BankWidgets *widgets = (BankWidgets *)data;
    const gchar *username = gtk_entry_get_text(GTK_ENTRY(widgets->login_username));
    const gchar *password = gtk_entry_get_text(GTK_ENTRY(widgets->login_password));
    
    if (strlen(username) == 0 || strlen(password) == 0) {
        gtk_label_set_text(GTK_LABEL(widgets->status_label), "Enter username and password!");
        return;
    }
    
    if (load_user(username, password)) {
        gtk_widget_hide(widgets->login_window);
        gtk_widget_show_all(widgets->main_window);
        update_balance_label(widgets);
    } else {
        if (user_exists(username)) {
            gtk_label_set_text(GTK_LABEL(widgets->status_label), "Invalid credentials!");
        } else {
            /* Register new user */
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
    save_transaction("Deposit", amount);
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
    save_transaction("Withdraw", amount);
    update_balance_label(widgets);
    gtk_label_set_text(GTK_LABEL(widgets->status_label), "Withdrawal Successful!");
    gtk_entry_set_text(GTK_ENTRY(widgets->amount), "");
}

/* Logout callback: clear login fields and clear transaction history */
void on_logout(GtkWidget *widget, gpointer data) {
    BankWidgets *widgets = (BankWidgets *)data;
    strcpy(current_user.username, "");
    strcpy(current_user.password, "");
    current_user.balance = 0.0;
    gtk_widget_hide(widgets->main_window);
    gtk_entry_set_text(GTK_ENTRY(widgets->login_username), "");
    gtk_entry_set_text(GTK_ENTRY(widgets->login_password), "");
    gtk_label_set_text(GTK_LABEL(widgets->status_label), "");
    /* Clear transaction view buffer */
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(widgets->transaction_view));
    gtk_text_buffer_set_text(buffer, "", -1);
    gtk_widget_show_all(widgets->login_window);
}

/* Change password callback */
void on_change_password(GtkWidget *widget, gpointer data) {
    BankWidgets *widgets = (BankWidgets *)data;
    
    GtkWidget *dialog = gtk_dialog_new_with_buttons("Change Password",
                         GTK_WINDOW(widgets->main_window),
                         GTK_DIALOG_MODAL,
                         "OK", GTK_RESPONSE_OK,
                         "Cancel", GTK_RESPONSE_CANCEL,
                         NULL);
    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *new_pass_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(new_pass_entry), "Enter New Password");
    gtk_entry_set_visibility(GTK_ENTRY(new_pass_entry), FALSE);
    gtk_container_add(GTK_CONTAINER(content_area), new_pass_entry);
    gtk_widget_show_all(dialog);
    
    int response = gtk_dialog_run(GTK_DIALOG(dialog));
    if (response == GTK_RESPONSE_OK) {
        const gchar *new_pass = gtk_entry_get_text(GTK_ENTRY(new_pass_entry));
        if (strlen(new_pass) < 4) {
            gtk_label_set_text(GTK_LABEL(widgets->status_label), "Password too short!");
        } else {
            strcpy(current_user.password, new_pass);
            update_user_file();
            gtk_label_set_text(GTK_LABEL(widgets->status_label), "Password changed successfully!");
        }
    }
    gtk_widget_destroy(dialog);
}

/* Show Receipt callback: load and display transaction history */
void on_show_receipt(GtkWidget *widget, gpointer data) {
    BankWidgets *widgets = (BankWidgets *)data;
    load_transactions(widgets);
}

/* Setup the UI: create login and main windows with default graphics */
void activate(GtkApplication *app, gpointer user_data) {
    BankWidgets *widgets = g_malloc(sizeof(BankWidgets));
    
    /* --- Create Login Window --- */
    widgets->login_window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(widgets->login_window), "Login");
    gtk_window_set_default_size(GTK_WINDOW(widgets->login_window), 300, 200);
    gtk_window_set_position(GTK_WINDOW(widgets->login_window), GTK_WIN_POS_CENTER);
    
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
    gtk_window_set_default_size(GTK_WINDOW(widgets->main_window), 600, 500);
    gtk_window_set_position(GTK_WINDOW(widgets->main_window), GTK_WIN_POS_CENTER);
    gtk_widget_hide(widgets->main_window);
    
    GtkWidget *main_grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(widgets->main_window), main_grid);
    
    widgets->amount = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(widgets->amount), "Enter Amount");
    gtk_grid_attach(GTK_GRID(main_grid), widgets->amount, 0, 0, 2, 1);
    
    widgets->balance_label = gtk_label_new("Balance: 0.00");
    gtk_grid_attach(GTK_GRID(main_grid), widgets->balance_label, 0, 1, 2, 1);
    
    GtkWidget *button_box = gtk_grid_new();  /* for deposit/withdraw buttons */
    gtk_grid_attach(GTK_GRID(main_grid), button_box, 0, 2, 2, 1);
    
    GtkWidget *deposit_button = gtk_button_new_with_label("Deposit");
    GtkWidget *withdraw_button = gtk_button_new_with_label("Withdraw");
    gtk_grid_attach(GTK_GRID(button_box), deposit_button, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(button_box), withdraw_button, 1, 0, 1, 1);
    g_signal_connect(deposit_button, "clicked", G_CALLBACK(on_deposit), widgets);
    g_signal_connect(withdraw_button, "clicked", G_CALLBACK(on_withdraw), widgets);
    
    GtkWidget *change_pass_button = gtk_button_new_with_label("Change Password");
    gtk_grid_attach(GTK_GRID(main_grid), change_pass_button, 0, 3, 2, 1);
    g_signal_connect(change_pass_button, "clicked", G_CALLBACK(on_change_password), widgets);
    
    GtkWidget *logout_button = gtk_button_new_with_label("Logout");
    gtk_grid_attach(GTK_GRID(main_grid), logout_button, 0, 4, 2, 1);
    g_signal_connect(logout_button, "clicked", G_CALLBACK(on_logout), widgets);
    
    GtkWidget *receipt_button = gtk_button_new_with_label("Show Receipt");
    gtk_grid_attach(GTK_GRID(main_grid), receipt_button, 0, 5, 2, 1);
    g_signal_connect(receipt_button, "clicked", G_CALLBACK(on_show_receipt), widgets);
    
    /* Add a text view for transaction history inside a scrolled window */ 
    widgets->transaction_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(widgets->transaction_view), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(widgets->transaction_view), GTK_WRAP_WORD);
    GtkWidget *scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(scrolled_window), widgets->transaction_view);
    gtk_widget_set_vexpand(scrolled_window, TRUE);
    gtk_grid_attach(GTK_GRID(main_grid), scrolled_window, 0, 6, 2, 1);
    
    /* Show the login window initially */
    gtk_widget_show_all(widgets->login_window);
}

int main(int argc, char **argv) {
    GtkApplication *app = gtk_application_new("com.bank.system", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return status;
}
