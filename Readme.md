# GTK Bank System

🚀 **A Simple Yet Powerful Banking System Using GTK and C**

Welcome to the **GTK Bank System**, a secure and user-friendly desktop banking application built using **C** and **GTK**. This project showcases advanced **file handling**, **GUI programming**, and **secure user authentication**. It serves as an example of how C can be taken to the next level by integrating it with modern UI frameworks.

---

## 🌟 Why I Built This

I embarked on this project to explore the **power of C beyond the console**. Many believe C is only for low-level programming, but with **GTK**, I aimed to push its capabilities into modern, interactive desktop applications. Through this project, I strengthened my expertise in **GUI development, data persistence, and user authentication**.

This project reflects my **growth as a C developer**, from writing simple command-line programs to building a **full-fledged, aesthetically pleasing desktop application**. My journey has been fueled by a passion for **efficiency, simplicity, and user experience**.

---

## 🎯 Features

✅ **User Authentication:** Secure login system with password-based authentication.  
✅ **Deposit & Withdraw:** Real-time balance updates with transaction management.  
✅ **Change Password:** Allows users to securely update their passwords.  
✅ **Transaction Receipt:** Users can view a detailed receipt of their transactions with a click.  
✅ **Switch Users:** Easily log out and switch between different accounts.  
✅ **Data Persistence:** All user data is stored in a secure file-based system.  
✅ **Beautiful UI:** Designed with clarity and simplicity using GTK.  

---

## 🌟 Installation

### Prerequisites
#### For Linux:
Ensure you have GTK installed on your system:
```bash
sudo apt update && sudo apt install libgtk-3-dev
```

#### For Windows:
1. **Install GTK**  
   - Download and install **MSYS2** from [https://www.msys2.org/](https://www.msys2.org/).  
   - Open **MSYS2 MinGW 64-bit** and run the following commands to install GTK:
   ```sh
   pacman -S mingw-w64-x86_64-gtk3
   pacman -S mingw-w64-x86_64-gcc
   ```
2. **Add GTK to Path**  
   - Find your MSYS2 installation directory (e.g., `C:\msys64\mingw64\bin`) and add it to your system **Environment Variables** under `Path`.

### Clone the Repository
```bash
git clone https://github.com/Deba-Codex/gtk-bank-system.git
cd gtk-bank-system
```

### Compile and Run
#### On Linux:
```bash
gcc bank_system.c -o bank_system `pkg-config --cflags --libs gtk+-3.0`
./bank_system
```

#### On Windows:
```sh
gcc bank_system.c -o bank_system.exe `pkg-config --cflags --libs gtk+-3.0`
./bank_system.exe
```


---

## 🛠 Usage

1. Run the program and enter your **username and password**.
2. If new, a **new account** will be created for you.
3. After login, you can **deposit, withdraw, change password, or view transaction history**.
4. Click **Logout** to switch users securely.
5. The **receipt button** lets you view past transactions.

---

## 🎨 UI & Aesthetics

I've designed the interface to be **clean, minimalistic, and easy to use**. The application is centered on the screen for a **modern feel**, and fonts are optimized for clarity.

> **Enhancing UI/UX in C? YES, IT'S POSSIBLE!**

Icons, colors, and spacing are carefully arranged to **improve usability** while keeping performance optimal.

---

## 🤝 Contribution

Want to make this project even better? Contributions are welcome! Feel free to **fork**, **star**, and **submit pull requests**.

1. **Fork the repo** and make your changes.
2. **Submit a pull request** explaining your modifications.

---

## 🐝 License

MIT License. Feel free to use and modify this project as you like.

---

## ❤️ Connect with Me

📧 Email: debasishdash2616@gmail.com  
<!-- 🐦 Twitter: [@yourhandle](https://twitter.com/yourhandle)   -->
🌚 LinkedIn: [Debasish Dash](https://www.linkedin.com/in/debasish-dash-276638310/)  

⭐ If you like this project, give it a star on GitHub! ⭐

