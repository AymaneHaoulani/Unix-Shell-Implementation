# Shell Implementation Project

## Project Overview
This project involves implementing a command-line shell to gain hands-on experience with process management. The shell will be a subset of widely used command interpreters such as Bash, Zsh, or PowerShell, with functionality progressively expanded through different stages.

## Features and Variants
The shell implementation follows a core structure, with multiple variants incorporating different functionalities. Each variant introduces new features related to process management, command interpretation, and time constraints.

### Core Features
- **Command Execution**: Execute system commands using `fork`, `exec`, and `wait`.
- **Process Management**: Handle foreground and background process execution.
- **Environment Variables**: Support for variable expansion.
- **Wildcard Expansion**: Basic pattern matching for filenames.

### Variants and Additional Functionalities
| ID  | Features Included |
|-----|------------------|
| 0   | Wildcards & environment variables (Sec. 6.1); Compute time limitation (Sec. 6.6) |
| 1   | Extended wildcards (tilde, brace) (Sec. 6.2); Multiple pipes (Sec. 6.5) |
| 2   | Asynchronous termination (Sec. 6.4); Compute time limitation (Sec. 6.6) |
| 3   | Compute time measurement (Sec. 6.3); Multiple pipes (Sec. 6.5) |
| 4   | Wildcards & environment variables (Sec. 6.1); Multiple pipes (Sec. 6.5) |
| 5   | Extended wildcards (tilde, brace) (Sec. 6.2); Compute time limitation (Sec. 6.6) |
| 6   | Asynchronous termination (Sec. 6.4); Multiple pipes (Sec. 6.5) |
| 7   | Compute time measurement (Sec. 6.3); Compute time limitation (Sec. 6.6) |
| 8   | Wildcards & environment variables (Sec. 6.1); Asynchronous termination (Sec. 6.4) |
| 9   | Extended wildcards (tilde, brace) (Sec. 6.2); Compute time measurement (Sec. 6.3) |
| 10  | Wildcards & environment variables (Sec. 6.1); Compute time measurement (Sec. 6.3) |
| 11  | Extended wildcards (tilde, brace) (Sec. 6.2); Asynchronous termination (Sec. 6.4) |