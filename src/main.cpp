#include "../include/Repository.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cstdlib>

using namespace std;

#ifdef _WIN32
#include <windows.h>
#endif

// ANSI Escape Codes for an Authentic Linux Terminal Look
#define CLR_RESET   "\033[0m"
#define CLR_BOLD    "\033[1m"
#define CLR_GREEN   "\033[1;32m"
#define CLR_CYAN    "\033[1;36m"
#define CLR_YELLOW  "\033[1;33m"
#define CLR_RED     "\033[1;31m"
#define CLR_MAGENTA "\033[1;35m"
#define CLR_BLUE    "\033[1;34m"
#define CLR_GRAY    "\033[0;37m"

// Enable ANSI colors in Windows CMD / PowerShell
void enableAnsiTerminal() {
#ifdef _WIN32
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE) {
        DWORD dwMode = 0;
        if (GetConsoleMode(hOut, &dwMode)) {
            dwMode |= 0x0004; // ENABLE_VIRTUAL_TERMINAL_PROCESSING
            SetConsoleMode(hOut, dwMode);
        }
    }
#endif
}

// Print Linux-Style ASCII Art Banner & Command Dashboard
void printLinuxBanner() {
    cout << CLR_CYAN <<
        "  __  __ _       _  ____ _ _   \n"
        " |  \\/  (_)_ __ (_)/ ___(_) |_ \n"
        " | |\\/| | | '_ \\| | |  _| | __|\n"
        " | |  | | | | | | | |_| | | |_ \n"
        " |_|  |_|_|_| |_|_|\\____|_|\\__|  " << CLR_YELLOW << "v1.0 (Linux Shell Edition)\n" << CLR_RESET;

    cout << CLR_GRAY << " -------------------------------------------------------------------------\n" << CLR_RESET;
    cout << CLR_BOLD << "  CORE MINIGIT COMMANDS                     REMOTE & SHELL UTILITIES\n" << CLR_RESET;
    cout << CLR_GRAY << " -------------------------------------------------------------------------\n" << CLR_RESET;
    cout << "  " << CLR_GREEN << "minigit init" << CLR_RESET << "               Init repo      "
         << CLR_MAGENTA << "minigit remote <dir>" << CLR_RESET << "  Link bare repo\n";
    cout << "  " << CLR_GREEN << "minigit add <file>" << CLR_RESET << "         Stage file     "
         << CLR_MAGENTA << "minigit push" << CLR_RESET << "          Push to remote\n";
    cout << "  " << CLR_GREEN << "minigit commit -m \"msg\"" << CLR_RESET << "    Commit staged  "
         << CLR_MAGENTA << "minigit pull" << CLR_RESET << "          Pull from remote\n";
    cout << "  " << CLR_GREEN << "minigit status" << CLR_RESET << "             Repo status    "
         << CLR_BLUE << "touch <file> \"text\"" << CLR_RESET << "   Create/edit file\n";
    cout << "  " << CLR_GREEN << "minigit log" << CLR_RESET << "                Commit history "
         << CLR_BLUE << "cat <file>" << CLR_RESET << " / " << CLR_BLUE << "rm <file>" << CLR_RESET << " View / Delete file\n";
    cout << "  " << CLR_GREEN << "minigit diff <file>" << CLR_RESET << "        Line diff      "
         << CLR_BLUE << "ls" << CLR_RESET << " / " << CLR_BLUE << "clear" << CLR_RESET << "            List / Clear\n";
    cout << "  " << CLR_GREEN << "minigit checkout <id>" << CLR_RESET << "      Restore commit "
         << CLR_BLUE << "help" << CLR_RESET << " / " << CLR_BLUE << "exit" << CLR_RESET << "           Help / Quit\n";
    cout << "  " << CLR_GREEN << "minigit uninit" << CLR_RESET << "             Remove .minigit"
         << "  " << CLR_GREEN << "minigit objects" << CLR_RESET << "        Polymorphism demo\n";
    cout << CLR_GRAY << " -------------------------------------------------------------------------\n" << CLR_RESET;
}

// Tokenizer that respects quoted strings: e.g., minigit commit -m "first commit"
vector<string> parseCommandLine(const string& input) {
    vector<string> tokens;
    string current;
    bool inQuotes = false;

    for (size_t i = 0; i < input.length(); ++i) {
        char c = input[i];
        if (c == '"') {
            inQuotes = !inQuotes;
        } else if ((c == ' ' || c == '\t') && !inQuotes) {
            if (!current.empty()) {
                tokens.push_back(current);
                current.clear();
            }
        } else {
            current += c;
        }
    }
    if (!current.empty()) {
        tokens.push_back(current);
    }
    return tokens;
}

// Executes a parsed command vector against the Repository
// Returns false if the user typed "exit" or "quit"
bool executeCommandTokens(const vector<string>& args, Repository& repo) {
    if (args.empty()) return true;

    size_t idx = 0;
    // Allow prefix "minigit" or "mingit" (or direct commands like "init", "status")
    if (args[0] == "minigit" || args[0] == "mingit") {
        if (args.size() == 1) {
            printLinuxBanner();
            return true;
        }
        idx = 1;
    }

    string cmd = args[idx];

    if (cmd == "exit" || cmd == "quit") {
        cout << CLR_YELLOW << "logout\n[MiniGit Shell session terminated]\n" << CLR_RESET;
        return false;
    } else if (cmd == "help" || cmd == "--help" || cmd == "-h") {
        printLinuxBanner();
    } else if (cmd == "clear" || cmd == "cls") {
#ifdef _WIN32
        system("cls");
#else
        system("clear");
#endif
        printLinuxBanner();
    } else if (cmd == "ls" || cmd == "dir") {
#ifdef _WIN32
        system("dir /b");
#else
        system("ls -la");
#endif
    } else if (cmd == "cat") {
        if (idx + 1 >= args.size()) {
            throw MiniGitException("Usage: cat <filename>");
        }
        ifstream f(args[idx + 1].c_str());
        if (!f.is_open()) {
            throw MiniGitException("cat: " + args[idx + 1] + ": No such file");
        }
        cout << f.rdbuf() << "\n";
    } else if (cmd == "touch" || cmd == "append") {
        bool appendMode = (cmd == "append");
        size_t fileArgIdx = idx + 1;
        if (fileArgIdx < args.size() && args[fileArgIdx] == "-a") {
            appendMode = true;
            fileArgIdx++;
        }
        if (fileArgIdx >= args.size()) {
            throw MiniGitException("Usage: touch [-a] <filename> [\"line1\\nline2\"]");
        }
        string fileName = args[fileArgIdx];
        ios_base::openmode mode = appendMode ? ios::app : ios::trunc;
        ofstream f(fileName.c_str(), mode);
        if (!f.is_open()) {
            throw MiniGitException("Cannot open file: " + fileName);
        }
        if (fileArgIdx + 1 < args.size()) {
            string rawText = args[fileArgIdx + 1];
            // Convert literal "\n" sequences into actual newline characters
            for (size_t i = 0; i < rawText.length(); ++i) {
                if (rawText[i] == '\\' && i + 1 < rawText.length() && rawText[i + 1] == 'n') {
                    f << '\n';
                    ++i;
                } else {
                    f << rawText[i];
                }
            }
            f << "\n";
        }
        f.close();
        cout << CLR_GREEN << "[Shell] " << (appendMode ? "Appended to '" : "Wrote file '")
             << fileName << "'\n" << CLR_RESET;
    } else if (cmd == "rm" || cmd == "del") {
        if (idx + 1 >= args.size()) {
            throw MiniGitException("Usage: rm <filename>");
        }
        repo.removeFile(args[idx + 1]);
    } else if (cmd == "init") {
        repo.init();
    } else if (cmd == "uninit") {
        repo.uninit();
    } else if (cmd == "init-bare") {
        if (idx + 1 >= args.size()) throw MiniGitException("Usage: minigit init-bare <folder_path>");
        repo.initBare(args[idx + 1]);
    } else if (cmd == "add") {
        if (idx + 1 >= args.size()) throw MiniGitException("Usage: minigit add <filename>");
        repo.add(args[idx + 1]);
    } else if (cmd == "commit") {
        string msg;
        if (idx + 2 < args.size() && args[idx + 1] == "-m") {
            msg = args[idx + 2];
        } else if (idx + 1 < args.size() && args[idx + 1] != "-m") {
            msg = args[idx + 1];
        } else {
            throw MiniGitException("Usage: minigit commit -m \"<message>\"");
        }
        repo.commit(msg);
    } else if (cmd == "status") {
        repo.status();
    } else if (cmd == "log") {
        repo.log();
    } else if (cmd == "diff") {
        if (idx + 1 >= args.size()) throw MiniGitException("Usage: minigit diff <filename>");
        repo.diff(args[idx + 1]);
    } else if (cmd == "checkout") {
        if (idx + 1 >= args.size()) throw MiniGitException("Usage: minigit checkout <commit_id>");
        repo.checkout(args[idx + 1]);
    } else if (cmd == "remote") {
        if (idx + 1 >= args.size()) throw MiniGitException("Usage: minigit remote <bare_folder_path>");
        repo.initBare(args[idx + 1]);
        repo.setRemote(args[idx + 1]);
    } else if (cmd == "push") {
        repo.push();
    } else if (cmd == "pull") {
        repo.pull();
    } else if (cmd == "objects") {
        repo.showObjectsPolymorphic();
    } else {
        cerr << CLR_RED << "minigit: '" << cmd << "' is not a minigit command. Type 'help' for usage.\n" << CLR_RESET;
    }

    return true;
}

// Interactive Linux-Style Terminal REPL
void runLinuxShell() {
    Repository repo;
    printLinuxBanner();

    string line;
    while (true) {
        // Build a Linux Git-Bash / Oh-My-Zsh style prompt:
        // student@minigit:~/workspace (master | HEAD: 7f3a9c12)$
        cout << "\n" << CLR_GREEN << "student@minigit" << CLR_RESET << ":"
             << CLR_BLUE << "~/workspace" << CLR_RESET;

        if (repo.isInitialized()) {
            cout << CLR_YELLOW << " (master | HEAD:" << repo.getHeadId();
            if (repo.getStagedCount() > 0) {
                cout << " +" << repo.getStagedCount() << " staged";
            }
            cout << ")" << CLR_RESET;
        } else {
            cout << CLR_RED << " (uninitialized)" << CLR_RESET;
        }

        cout << "$ ";

        if (!getline(cin, line)) {
            break;
        }

        vector<string> tokens = parseCommandLine(line);
        try {
            if (!executeCommandTokens(tokens, repo)) {
                break;
            }
        } catch (const MiniGitException& ex) {
            cerr << CLR_RED << ex.what() << CLR_RESET << "\n";
        } catch (const exception& ex) {
            cerr << CLR_RED << "[Error] " << ex.what() << CLR_RESET << "\n";
        }
    }
}

int main(int argc, char* argv[]) {
    enableAnsiTerminal();

    // If launched without CLI arguments, enter the interactive Linux-style shell
    if (argc < 2) {
        runLinuxShell();
        return 0;
    }

    // Also support direct one-shot execution from OS terminal: e.g. ./minigit.exe init
    vector<string> cliArgs;
    for (int i = 1; i < argc; ++i) {
        cliArgs.push_back(argv[i]);
    }

    Repository repo;
    try {
        executeCommandTokens(cliArgs, repo);
    } catch (const MiniGitException& ex) {
        cerr << CLR_RED << ex.what() << CLR_RESET << "\n";
        return 1;
    } catch (const exception& ex) {
        cerr << CLR_RED << "[Unexpected Error] " << ex.what() << CLR_RESET << "\n";
        return 1;
    }

    return 0;
}
