🚀 TechShell - A Custom Unix Shell

Author: Jack Revelett  
Date: 2/19/2025  

---

## 1️⃣ Description of How the Shell Works
TechShell is a simple Unix-based shell that allows users to execute commands, manage input and output redirection, and navigate directories. 

The shell works as follows:
1. **Command Prompt:**  
   - The shell continuously displays the current working directory followed by a `$` prompt.  
   - The user enters a command which is then parsed and executed.  

2. **Parsing and Execution:**  
   - The shell reads the user input and tokenizes it into individual commands and arguments.  
   - If input or output redirection (`<`, `>`) is detected, the respective files are opened and associated with `stdin` or `stdout`.  
   - The shell forks a child process and executes the command using `execvp()`.  
   - The parent process waits for the child to complete before displaying the next prompt.  

3. **Built-in Commands:**  
   - `cd [directory]` – Changes the current working directory.  
   - `exit` – Terminates the shell.  

4. **Error Handling:**  
   - Invalid commands result in an error message: `Error: Command not found`.  
   - Redirection errors (e.g., missing filenames) are handled gracefully.  
   - Permission issues display appropriate error messages.  

---

## 2️⃣ Features Implemented
✅ **Custom Command Prompt** – Displays the current working directory.  
✅ **Command Execution** – Runs system commands using `execvp()`.  
✅ **Input Redirection (`<`)** – Reads input from specified files.  
✅ **Output Redirection (`>`)** – Redirects command output to files.  
✅ **Handles Errors** – Manages invalid commands, file permissions, and execution failures.  
✅ **Built-in Commands:**  
   - `cd` – Change directories.  
   - `exit` – Exit the shell.  


---

## 3️⃣ Unimplemented / Partially Working Features
⚠ **NOT Implemented:**
- **Piping (`|`)** – Cannot chain commands together like `ls | grep txt`.
- **Background Execution (`&`)** – All commands run in the foreground.
- **No Signal Handling (`Ctrl+C`)** – Does not properly terminate, instead ends application.

⚠ **PARTIALLY Implemented:**
- **Quoted Arguments in `cd`** – Handles `"Directory Name"` but may break with complex cases.
