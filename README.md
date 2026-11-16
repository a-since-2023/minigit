# MiniGit —  C++ OOP Project (Linux Shell Edition)


REport details 
1. what is the problem ?
3. concepts 
4. how we implemented it ?
5. modules 
6. conclusion 


A clean, lightweight, terminal-based Version Control System written in **C++** featuring a **Linux Git-Bash / Zsh-style Interactive Shell** that demonstrates every core concept of **Object-Oriented Programming (OOP)** using **raw pointers** and a **Local Bare Remote Repository**.

---

## 1. Project File Structure (`E:\minigit`)

```
E:\minigit\
├── include/
│   ├── GitObjects.hpp     # Abstract GitObject, FileBlob, Commit, SimpleHash, MiniGitException
│   ├── BareRemote.hpp     # Local Bare Repository class (initBare, push, pull)
│   └── Repository.hpp     # Main Repository controller class (Commit* linked list, FileBlob*   array)
├── src/
│   ├── GitObjects.cpp     # Hash calculation, FileBlob & Commit methods, operator overloading
│   ├── BareRemote.cpp     # Push/Pull file synchronization with Bare Remote
│   ├── Repository.cpp     # Core commands: init, add, commit, status, log, diff, checkout
│   └── main.cpp           # Linux-style Interactive Shell (REPL) + CLI command parser
├── Makefile               # Standard Make build script
├── build.bat              # One-click Windows build script
└── README.md              # Documentation & Viva Guide
```
 
---

## 2. How to Compile & Run

### Compile with `g++`:
```bash
g++ -Wall -std=c++11 -Iinclude src/main.cpp src/GitObjects.cpp src/BareRemote.cpp src/Repository.cpp -o minigit.exe
```
*(Or simply run `build.bat` on Windows).*

### Launch the Linux-Style MiniGit Shell:
Run `./minigit.exe` to open the custom Linux-inspired terminal prompt:
```text
  __  __ _       _  ____ _ _   
 |  \/  (_)_ __ (_)/ ___(_) |_ 
 | |\/| | | '_ \| | |  _| | __|
 | |  | | | | | | | |_| | | |_ 
 |_|  |_|_|_| |_|_|\____|_|\__|  v1.0 (Linux Shell Edition)
 -------------------------------------------------------------------------
  CORE MINIGIT COMMANDS                     REMOTE & SHELL UTILITIES
 -------------------------------------------------------------------------
  minigit init               Init repo      minigit remote <dir>  Link bare repo
  minigit add <file>         Stage file     minigit push          Push to remote
  minigit commit -m "msg"    Commit staged  minigit pull          Pull from remote
  minigit status             Repo status    touch <file> "text"   Create/edit file
  minigit log                Commit history cat <file>            View file text
  minigit diff <file>        Line diff      ls / clear            List / Clear
  minigit checkout <id>      Restore commit help / exit           Help / Quit
  minigit objects            Polymorphism demo (GitObject*)
 -------------------------------------------------------------------------

student@minigit:~/workspace (uninitialized)$ minigit init
[MiniGit] Initialized empty MiniGit repository in .minigit/

student@minigit:~/workspace (master | HEAD:no-commits)$ touch hello.txt "Hello OOP World"
[Shell] Wrote file 'hello.txt'

student@minigit:~/workspace (master | HEAD:no-commits)$ minigit add hello.txt
[MiniGit] Staged file: hello.txt (Blob ID: 6e4a8b12)

student@minigit:~/workspace (master | HEAD:no-commits +1 staged)$ minigit commit -m "First commit"
[MiniGit] Created Commit [9c2d1f44]: First commit

student@minigit:~/workspace (master | HEAD:9c2d1f44)$ minigit log
```

8. **File Handling (`<fstream>`)**:
   - Stores human-readable metadata inside `.minigit/index.txt`, `.minigit/commits.txt`, `.minigit/remote.txt`, and file snapshots in `.minigit/objects/<hash>.blob`.
