# B.Tech Object-Oriented Programming (OOP) Project Report
## Project Title: **MiniGit — A Terminal-Based Version Control System in C++**

---

## 1. What is the Problem? (Problem Statement)

When students and software developers work on programming projects without a Version Control System (VCS), they face several critical issues:
1. **Loss of Previous Working Code**: Modifying a working program to add a new feature often introduces bugs. Without version history, reverting back to a previously working state is difficult or impossible.
2. **Manual Duplicate Folders (`project_v1`, `project_final`, `project_final_2`)**: Beginners often copy-paste entire project folders to save backups, which wastes disk space and creates confusion about which file was actually modified.
3. **Lack of Line-by-Line Change Tracking**: It is hard to identify exact lines added or deleted between the current working file and its last saved version.
4. **No Safe Local Backup / Synchronization**: If a developer accidentally deletes a file from their working folder, there is no local central ("Bare") repository from which they can pull or restore their code.
5. **High Complexity of Industrial Git Internals**: While full-scale Git solves these problems, its internal binary packfiles and cryptographic internals are opaque to students learning core Object-Oriented Programming and Data Structures.

### Proposed Solution: **MiniGit**
**MiniGit** solves these problems by providing a lightweight, Linux-shell-inspired Version Control System written entirely in **C++** using core **Object-Oriented Programming (OOP)** principles and **dynamic raw pointer memory management**. It tracks file snapshots using content hashing, maintains a linked list of commits, compares line-by-line differences, and synchronizes with a local **Bare Remote Repository** using plain, human-readable storage inside `.minigit/`.

---

## 2. Core OOP Concepts Used

MiniGit is specifically architected to demonstrate every major pillar of the B.Tech C++ Object-Oriented Programming curriculum:

| # | OOP Concept | Where It Is Used in MiniGit | Purpose & Technical Explanation |
| :- | :--- | :--- | :--- |
| **1** | **Abstraction** | `GitObject` class (`include/GitObjects.hpp`) | Declared as an **abstract base class** containing pure virtual functions `virtual void display() const = 0;` and `virtual string getType() const = 0;`. Prevents direct instantiation of generic Git objects and defines a strict interface contract. |
| **2** | **Inheritance** | `FileBlob` & `Commit` inherit from `GitObject`; `MiniGitException` inherits from `runtime_error` | Enables code reuse of the object hash `id` (`getId()`, `setId()`) from the base class `GitObject` across both file blobs and commit snapshots. |
| **3** | **Runtime Polymorphism** | `Repository::showObjectsPolymorphic()` (`src/Repository.cpp`) | Uses a collection of base-class pointers (`vector<const GitObject*>`) storing both `FileBlob*` and `Commit*` objects. Calling `objects[i]->display()` dynamically resolves to the appropriate derived class method at runtime via the `virtual` table (vtable). Virtual destructor `virtual ~GitObject()` ensures safe polymorphic cleanup. |
| **4** | **Encapsulation** | All classes (`GitObject`, `FileBlob`, `Commit`, `BareRemote`, `Repository`) | Internal states (`fileName`, `content`, `files`, `head`, `stagedFiles`) are hidden using `private` and `protected` access specifiers and manipulated strictly through validated `public` member functions. |
| **5** | **Raw Pointers & Dynamic Memory (`new` / `delete`)** | `Commit` (`FileBlob** files`, `Commit* parent`) and `Repository` (`Commit* head`, `FileBlob* stagedFiles[]`, `BareRemote* remote`) | Demonstrates manual heap allocation without smart pointers. `Commit` dynamically resizes its array of `FileBlob*` pointers on the heap, and `Repository` maintains a **Singly Linked List** of `Commit*` nodes. Destructors (`~Commit()` and `~Repository()`) explicitly free all heap memory using `delete` and `delete[]`. |
| **6** | **Copy Constructor & Deep Copy (Rule of Three)** | `Commit::Commit(const Commit& other)` (`src/GitObjects.cpp`) | Allocates a new `FileBlob*` array and clones each `FileBlob` (`new FileBlob(*other.files[i])`) so that two commits never share or double-free the same memory address. |
| **7** | **Operator Overloading** | `FileBlob::operator==` and `operator<<` in `Commit` (`src/GitObjects.cpp`) | `operator==` compares two `FileBlob` objects to detect whether a working file has been modified compared to `HEAD`. Friend `operator<<` allows printing a `Commit` object directly via `cout << *curr;`. |
| **8** | **Exception Handling** | `MiniGitException` with `try`, `throw`, `catch` | Gracefully catches invalid commands, missing files, uninitialized repositories, or empty commits without crashing the interactive shell. |
| **9** | **File Handling (`<fstream>`)** | `ifstream` and `ofstream` in `Repository` & `BareRemote` | Persists repository metadata (`config.txt`, `index.txt`, `commits.txt`, `remote.txt`) and immutable file snapshots (`.minigit/objects/<hash>.blob`) on disk across sessions. |

---

## 3. How We Implemented It (Architecture & Working)

1. **Content Hashing (`SimpleHash::generate`)**:
   - Uses the **DJB2 polynomial rolling hash algorithm** (`hash = hash * 33 + char`) formatted into an 8-character hexadecimal string (e.g., `4a81f20c`).
   - Whenever a file's text changes by even a single character, its generated hash ID changes.
2. **Staging Area (`minigit add <file>`)**:
   - Reads the file from disk, dynamically allocates a `new FileBlob(fileName, content)`, writes the snapshot to `.minigit/objects/<id>.blob`, and stores the pointer in `stagedFiles[]` (persisted in `.minigit/index.txt`).
3. **Commit Linked List (`minigit commit -m "<msg>"`)**:
   - Dynamically allocates a `new Commit(message, author, head)` whose `parent` pointer points to the previous `head` commit.
   - Deep-copies all staged `FileBlob` objects into the new `Commit` node, carries forward unchanged files from the parent commit, appends the record to `.minigit/commits.txt`, and clears the staging area.
4. **Status & Diff Engine (`minigit status`, `minigit diff <file>`)**:
   - `status()` creates a temporary `FileBlob` from the current file on disk and uses the overloaded `operator==` (`if (!(workingBlob == *committedBlob))`) to check if the file was modified since the last commit.
   - `diff()` splits both the committed snapshot and the working file into lines and prints added (`+`) and removed (`-`) lines.
5. **Time-Travel Checkout (`minigit checkout <id>`)**:
   - Traverses the `Commit*` linked list starting from `head` via `curr = curr->getParent()`. Once the matching 8-character Commit ID is found, it restores every `FileBlob` in that commit back into the working directory.
6. **Local Bare Remote Synchronization (`minigit remote`, `push`, `pull`)**:
   - A **Bare Repository** stores only `commits.txt` and `objects/*.blob` without working files.
   - `push()` copies commit history and `.blob` files from `.minigit/` to the bare folder; `pull()` fetches them back and updates the working directory.

---

## 4. System Modules

The project is organized into **4 modular components** inside `E:\minigit`:

### Module 1: Core Object & Hashing Module (`GitObjects.hpp` / `GitObjects.cpp`)
* **`MiniGitException`**: Custom exception class for error reporting.
* **`SimpleHash`**: Generates deterministic 8-character hexadecimal hashes.
* **`GitObject`**: Abstract base class defining `id`, `display()`, and `getType()`.
* **`FileBlob`**: Concrete derived class encapsulating a file's name, content, and overloaded `operator==`.
* **`Commit`**: Concrete derived class managing commit metadata, a dynamic `FileBlob**` array, a `Commit* parent` pointer, deep copy constructor, destructor, and overloaded `operator<<`.

### Module 2: Local Bare Remote Module (`BareRemote.hpp` / `BareRemote.cpp`)
* **`BareRemote`**: Encapsulates remote directory paths and handles cross-platform folder creation (`_mkdir`/`mkdir`) and binary-safe file transfer (`copySingleFile`) for `initBare()`, `push()`, and `pull()`.

### Module 3: Repository Engine Module (`Repository.hpp` / `Repository.cpp`)
* **`Repository`**: Central controller managing `.minigit/` persistence (`loadState()`, `saveIndex()`, `appendCommitToDisk()`), memory cleanup (`freeMemory()`), and all core VCS operations (`init`, `uninit`, `add`, `removeFile`, `commit`, `status`, `log`, `diff`, `checkout`, `showObjectsPolymorphic`).

### Module 4: Linux Interactive Shell & CLI Module (`main.cpp`)
* Provides an ANSI-colored **Linux REPL shell** (`student@minigit:~/workspace (master | HEAD:...)`) with quote-aware command parsing (`parseCommandLine`).
* Includes built-in Linux file utilities (`touch`, `touch -a` / `append`, `cat`, `rm`, `ls`, `clear`, `help`, `exit`) alongside all `minigit` commands.

---

## 5. Conclusion

The **MiniGit** project successfully demonstrates how real-world systems software like Git can be modeled cleanly using **Object-Oriented Programming in C++**. By combining **Abstraction** (`GitObject`), **Inheritance** (`FileBlob`, `Commit`), **Runtime Polymorphism** (`virtual display()`), **Operator Overloading** (`==`, `<<`), **Exception Handling** (`MiniGitException`), and **Explicit Raw Pointer Memory Management** (`new`, `delete`, Deep Copy Constructor, and Linked List traversal), the project fulfills all academic objectives of the B.Tech OOP curriculum while providing a practical, interactive Linux-style version control tool.
