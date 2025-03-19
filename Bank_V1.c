#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char username[50];
    char password[50];
    gdouble balance;
} User;

User current_user;

typedef struct {
    GtkWidget *username;
    GtkWidget *password;
    GtkWidget *amount;
    GtkWidget *status_label;
    GtkWidget *balance_label;
    GtkWidget *receipt_label;
} BankWidgets;

void update_balance_label(BankWidgets *widgets) {
    char balance_str[50];
    snprintf(balance_str, sizeof(balance_str), "Balance: %.2f", current_user.balance);
    gtk_label_set_text(GTK_LABEL(widgets->balance_label), balance_str);
}

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
    update_balance_label(widgets);
    gtk_label_set_text(GTK_LABEL(widgets->status_label), "Deposit Successful!");
    
    char receipt[100];
    snprintf(receipt, sizeof(receipt), "Receipt:\nDeposited Amount: %.2f", amount);
    gtk_label_set_text(GTK_LABEL(widgets->receipt_label), receipt);
}

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
    update_balance_label(widgets);
    gtk_label_set_text(GTK_LABEL(widgets->status_label), "Withdrawal Successful!");
    
    char receipt[100];
    snprintf(receipt, sizeof(receipt), "Receipt:\nWithdrawn Amount: %.2f", amount);
    gtk_label_set_text(GTK_LABEL(widgets->receipt_label), receipt);
}

void on_change_password(GtkWidget *widget, gpointer data) {
    BankWidgets *widgets = (BankWidgets *)data;
    const gchar *new_password = gtk_entry_get_text(GTK_ENTRY(widgets->password));
    
    if (strlen(new_password) < 4) {
        gtk_label_set_text(GTK_LABEL(widgets->status_label), "Password too short!");
        return;
    }
    
    strcpy(current_user.password, new_password);
    gtk_label_set_text(GTK_LABEL(widgets->status_label), "Password changed successfully!");
}

void activate(GtkApplication *app, gpointer user_data) {
    GtkWidget *window, *grid, *deposit_button, *withdraw_button, *password_button;
    
    BankWidgets *widgets = g_malloc(sizeof(BankWidgets));
    strcpy(current_user.username, "User");
    strcpy(current_user.password, "1234");
    current_user.balance = 0.0;

    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Bank Management System");
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 400);
    
    grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(window), grid);

    widgets->username = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(widgets->username), current_user.username);
    gtk_editable_set_editable(GTK_EDITABLE(widgets->username), FALSE);
    gtk_grid_attach(GTK_GRID(grid), widgets->username, 0, 0, 2, 1);
    
    widgets->password = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(widgets->password), "Enter New Password");
    gtk_entry_set_visibility(GTK_ENTRY(widgets->password), FALSE);
    gtk_grid_attach(GTK_GRID(grid), widgets->password, 0, 1, 2, 1);

    widgets->amount = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(widgets->amount), "Enter Amount");
    gtk_grid_attach(GTK_GRID(grid), widgets->amount, 0, 2, 2, 1);
    
    widgets->status_label = gtk_label_new("Welcome to the Bank!");
    gtk_grid_attach(GTK_GRID(grid), widgets->status_label, 0, 3, 2, 1);
    
    widgets->balance_label = gtk_label_new("Balance: 0.00");
    gtk_grid_attach(GTK_GRID(grid), widgets->balance_label, 0, 4, 2, 1);
    
    widgets->receipt_label = gtk_label_new("");
    gtk_grid_attach(GTK_GRID(grid), widgets->receipt_label, 0, 5, 2, 1);
    
    deposit_button = gtk_button_new_with_label("Deposit");
    withdraw_button = gtk_button_new_with_label("Withdraw");
    password_button = gtk_button_new_with_label("Change Password");
    gtk_grid_attach(GTK_GRID(grid), deposit_button, 0, 6, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), withdraw_button, 1, 6, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), password_button, 0, 7, 2, 1);
    
    g_signal_connect(deposit_button, "clicked", G_CALLBACK(on_deposit), widgets);
    g_signal_connect(withdraw_button, "clicked", G_CALLBACK(on_withdraw), widgets);
    g_signal_connect(password_button, "clicked", G_CALLBACK(on_change_password), widgets);
    
    gtk_widget_show_all(window);
}

int main(int argc, char **argv) {
    GtkApplication *app;
    int status;

    app = gtk_application_new("com.bank.system", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    return status;
}
