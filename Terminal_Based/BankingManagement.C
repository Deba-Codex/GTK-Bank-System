#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Account {
    int accountNumber;
    char name[50];
    float balance;
};

void createAccount();
void deposit();
void withdraw();
void checkBalance();
void viewAccounts();

int main() {
    int choice;
    while (1) {
        printf("\nBanking Management System\n");
        printf("1. Create Account\n");
        printf("2. Deposit\n");
        printf("3. Withdraw\n");
        printf("4. Check Balance\n");
        printf("5. View All Accounts\n");
        printf("6. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                createAccount();
                break;
            case 2:
                deposit();
                break;
            case 3:
                withdraw();
                break;
            case 4:
                checkBalance();
                break;
            case 5:
                viewAccounts();
                break;
            case 6:
                exit(0);
            default:
                printf("Invalid choice! Try again.\n");
        }
    }
    return 0;
}

void createAccount() {
    struct Account acc;
    FILE *fp = fopen("accounts.dat", "ab");
    if (fp == NULL) {
        printf("Error opening file!\n");
        return;
    }
    printf("Enter account number: ");
    scanf("%d", &acc.accountNumber);
    printf("Enter name: ");
    scanf("%s", acc.name);
    acc.balance = 0;
    fwrite(&acc, sizeof(acc), 1, fp);
    fclose(fp);
    printf("Account created successfully!\n");
}

void deposit() {
    int accNum, found = 0;
    float amount;
    struct Account acc;
    FILE *fp = fopen("accounts.dat", "rb+"), *temp;
    if (fp == NULL) {
        printf("Error opening file!\n");
        return;
    }
    printf("Enter account number: ");
    scanf("%d", &accNum);
    while (fread(&acc, sizeof(acc), 1, fp)) {
        if (acc.accountNumber == accNum) {
            printf("Enter amount to deposit: ");
            scanf("%f", &amount);
            acc.balance += amount;
            fseek(fp, -sizeof(acc), SEEK_CUR);
            fwrite(&acc, sizeof(acc), 1, fp);
            printf("Amount deposited successfully!\n");
            found = 1;
            break;
        }
    }
    fclose(fp);
    if (!found) printf("Account not found!\n");
}

void withdraw() {
    int accNum, found = 0;
    float amount;
    struct Account acc;
    FILE *fp = fopen("accounts.dat", "rb+");
    if (fp == NULL) {
        printf("Error opening file!\n");
        return;
    }
    printf("Enter account number: ");
    scanf("%d", &accNum);
    while (fread(&acc, sizeof(acc), 1, fp)) {
        if (acc.accountNumber == accNum) {
            printf("Enter amount to withdraw: ");
            scanf("%f", &amount);
            if (amount > acc.balance) {
                printf("Insufficient balance!\n");
            } else {
                acc.balance -= amount;
                fseek(fp, -sizeof(acc), SEEK_CUR);
                fwrite(&acc, sizeof(acc), 1, fp);
                printf("Amount withdrawn successfully!\n");
            }
            found = 1;
            break;
        }
    }
    fclose(fp);
    if (!found) printf("Account not found!\n");
}

void checkBalance() {
    int accNum, found = 0;
    struct Account acc;
    FILE *fp = fopen("accounts.dat", "rb");
    if (fp == NULL) {
        printf("Error opening file!\n");
        return;
    }
    printf("Enter account number: ");
    scanf("%d", &accNum);
    while (fread(&acc, sizeof(acc), 1, fp)) {
        if (acc.accountNumber == accNum) {
            printf("Account Balance: %.2f\n", acc.balance);
            found = 1;
            break;
        }
    }
    fclose(fp);
    if (!found) printf("Account not found!\n");
}

void viewAccounts() {
    struct Account acc;
    FILE *fp = fopen("accounts.dat", "rb");
    if (fp == NULL) {
        printf("Error opening file!\n");
        return;
    }
    printf("\nAccount List:\n");
    while (fread(&acc, sizeof(acc), 1, fp)) {
        printf("Account No: %d, Name: %s, Balance: %.2f\n", acc.accountNumber, acc.name, acc.balance);
    }
    fclose(fp);
}
