#include "../include/Repository.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cstdio>

using namespace std;

#ifdef _WIN32
#include <direct.h>
#define MKDIR(path) _mkdir(path)
#else
#include <sys/stat.h>
#define MKDIR(path) mkdir(path, 0777)
#endif

Repository::Repository()
    : gitDir(".minigit"), head(nullptr), stagedCount(0), remote(nullptr) {
    for (int i = 0; i < MAX_STAGED; ++i) {
        stagedFiles[i] = nullptr;
    }
    if (isInitialized()) {
        loadState();
    }
}

Repository::~Repository() {
    freeMemory();
}

void Repository::freeMemory() {
    // 1. Free Linked List of Commit* objects
    Commit* current = head;
    while (current != nullptr) {
        Commit* nextParent = current->getParent();
        delete current;
        current = nextParent;
    }
    head = nullptr;

    // 2. Free array of staged FileBlob* objects
    for (int i = 0; i < stagedCount; ++i) {
        delete stagedFiles[i];
        stagedFiles[i] = nullptr;
    }
    stagedCount = 0;

    // 3. Free BareRemote* pointer
    delete remote;
    remote = nullptr;
}

bool Repository::isInitialized() const {
    ifstream check((gitDir + "/config.txt").c_str());
    return check.is_open();
}

string Repository::getHeadId() const {
    if (head != nullptr) {
        return head->getId();
    }
    return "no-commits";
}

int Repository::getStagedCount() const {
    return stagedCount;
}

void Repository::ensureInitialized() const {
    if (!isInitialized()) {
        throw MiniGitException("Repository not initialized! Run 'minigit init' first.");
    }
}

string Repository::readFileContent(const string& path) {
    ifstream file(path.c_str(), ios::binary);
    if (!file.is_open()) {
        throw MiniGitException("Cannot open file: " + path);
    }
    stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

void Repository::writeFileContent(const string& path, const string& content) {
    ofstream file(path.c_str(), ios::binary | ios::trunc);
    if (!file.is_open()) {
        throw MiniGitException("Cannot write to file: " + path);
    }
    file << content;
}

void Repository::loadState() {
    freeMemory();

    // 1. Load Remote path if configured
    ifstream remIn((gitDir + "/remote.txt").c_str());
    if (remIn.is_open()) {
        string rPath;
        if (getline(remIn, rPath) && !rPath.empty()) {
            remote = new BareRemote(rPath);
        }
        remIn.close();
    }

    // 2. Load Staged Files from .minigit/index.txt
    ifstream idxIn((gitDir + "/index.txt").c_str());
    if (idxIn.is_open()) {
        string line;
        while (getline(idxIn, line) && stagedCount < MAX_STAGED) {
            size_t pipePos = line.find('|');
            if (pipePos != string::npos) {
                string fName = line.substr(0, pipePos);
                string blobId = line.substr(pipePos + 1);
                string blobPath = gitDir + "/objects/" + blobId + ".blob";
                ifstream blobFile(blobPath.c_str(), ios::binary);
                if (blobFile.is_open()) {
                    stringstream ss;
                    ss << blobFile.rdbuf();
                    stagedFiles[stagedCount++] = new FileBlob(fName, ss.str());
                }
            }
        }
        idxIn.close();
    }

    // 3. Load Commit Linked List from .minigit/commits.txt
    ifstream comIn((gitDir + "/commits.txt").c_str());
    if (comIn.is_open()) {
        string line;
        Commit* currentCommit = nullptr;

        while (getline(comIn, line)) {
            if (line.rfind("COMMIT|", 0) == 0) {
                // Format: COMMIT|<id>|<author>|<timestamp>|<message>
                vector<string> parts;
                stringstream ss(line);
                string item;
                while (getline(ss, item, '|')) {
                    parts.push_back(item);
                }
                if (parts.size() >= 5) {
                    currentCommit = new Commit(parts[1], parts[4], parts[2], parts[3], head);
                    head = currentCommit;
                }
            } else if (line.rfind("FILE|", 0) == 0 && currentCommit != nullptr) {
                // Format: FILE|<filename>|<blobId>
                size_t firstPipe = line.find('|');
                size_t secondPipe = line.rfind('|');
                if (firstPipe != string::npos && secondPipe != firstPipe) {
                    string fName = line.substr(firstPipe + 1, secondPipe - firstPipe - 1);
                    string blobId = line.substr(secondPipe + 1);
                    string blobPath = gitDir + "/objects/" + blobId + ".blob";
                    ifstream bFile(blobPath.c_str(), ios::binary);
                    string content = "";
                    if (bFile.is_open()) {
                        stringstream bss;
                        bss << bFile.rdbuf();
                        content = bss.str();
                    }
                    FileBlob* blob = new FileBlob(fName, content);
                    blob->setId(blobId);
                    currentCommit->addFile(blob);
                    currentCommit->setId(currentCommit->getId()); // Preserve stored commit ID
                }
            }
        }
        comIn.close();
    }
}

void Repository::saveIndex() const {
    ofstream idxOut((gitDir + "/index.txt").c_str(), ios::trunc);
    for (int i = 0; i < stagedCount; ++i) {
        idxOut << stagedFiles[i]->getFileName() << "|" << stagedFiles[i]->getId() << "\n";
    }
    idxOut.close();
}

void Repository::appendCommitToDisk(const Commit* newCommit) const {
    ofstream out((gitDir + "/commits.txt").c_str(), ios::app);
    out << "COMMIT|" << newCommit->getId() << "|"
        << newCommit->getAuthor() << "|"
        << newCommit->getTimestamp() << "|"
        << newCommit->getMessage() << "\n";

    for (int i = 0; i < newCommit->getFileCount(); ++i) {
        FileBlob* f = newCommit->getFile(i);
        out << "FILE|" << f->getFileName() << "|" << f->getId() << "\n";
    }
    out.close();
}

// ============================================================================
// Core Commands Implementation
// ============================================================================

void Repository::init() {
    if (isInitialized()) {
        cout << "[MiniGit] Repository is already initialized in .minigit/\n";
        return;
    }
    MKDIR(gitDir.c_str());
    MKDIR((gitDir + "/objects").c_str());

    writeFileContent(gitDir + "/config.txt", "version=1.0\n");
    writeFileContent(gitDir + "/index.txt", "");
    writeFileContent(gitDir + "/commits.txt", "");

    cout << "[MiniGit] Initialized empty MiniGit repository in .minigit/\n";
}

void Repository::uninit() {
    if (!isInitialized()) {
        cout << "[MiniGit] Repository is already uninitialized.\n";
        return;
    }

    // 1. Free all dynamically allocated raw pointers in memory (Commit*, FileBlob*, BareRemote*)
    freeMemory();

    // 2. Remove the .minigit directory from disk
#ifdef _WIN32
    system("rmdir /s /q .minigit 2>nul");
#else
    system("rm -rf .minigit");
#endif

    cout << "[MiniGit] Uninitialized repository and removed .minigit/ folder.\n";
}

void Repository::initBare(const string& bareFolderPath) {
    BareRemote bare(bareFolderPath);
    bare.initBare();
}

void Repository::add(const string& fileName) {
    ensureInitialized();

    string content = readFileContent(fileName);
    FileBlob* newBlob = new FileBlob(fileName, content);

    // Save blob content into .minigit/objects/<hash>.blob
    string blobFilePath = gitDir + "/objects/" + newBlob->getId() + ".blob";
    writeFileContent(blobFilePath, content);

    // Check if file is already staged; if so, replace old pointer
    for (int i = 0; i < stagedCount; ++i) {
        if (stagedFiles[i]->getFileName() == fileName) {
            delete stagedFiles[i];
            stagedFiles[i] = newBlob;
            saveIndex();
            cout << "[MiniGit] Updated staged file: " << fileName
                 << " (Blob ID: " << newBlob->getId() << ")\n";
            return;
        }
    }

    if (stagedCount >= MAX_STAGED) {
        delete newBlob;
        throw MiniGitException("Staging area is full!");
    }

    stagedFiles[stagedCount++] = newBlob;
    saveIndex();
    cout << "[MiniGit] Staged file: " << fileName
         << " (Blob ID: " << newBlob->getId() << ")\n";
}

void Repository::removeFile(const string& fileName) {
    // 1. Remove from staging area if currently staged (freeing the FileBlob* raw pointer)
    bool wasStaged = false;
    for (int i = 0; i < stagedCount; ++i) {
        if (stagedFiles[i]->getFileName() == fileName) {
            delete stagedFiles[i];
            for (int j = i; j + 1 < stagedCount; ++j) {
                stagedFiles[j] = stagedFiles[j + 1];
            }
            stagedFiles[stagedCount - 1] = nullptr;
            stagedCount--;
            saveIndex();
            wasStaged = true;
            break;
        }
    }

    // 2. Delete the file from the working directory on disk
    if (remove(fileName.c_str()) == 0) {
        cout << "[MiniGit] Deleted file from disk: " << fileName << "\n";
    } else if (!wasStaged) {
        throw MiniGitException("rm: cannot remove '" + fileName + "': No such file");
    }

    if (wasStaged) {
        cout << "[MiniGit] Removed '" << fileName << "' from staging area.\n";
    }
}

void Repository::commit(const string& message, const string& author) {
    ensureInitialized();

    if (stagedCount == 0) {
        throw MiniGitException("Nothing to commit! Stage files first using 'minigit add <file>'.");
    }

    // Create new Commit node linked to current head (Linked List)
    Commit* newCommit = new Commit(message, author, head);

    // Carry over previously committed files that weren't overwritten, plus newly staged files
    for (int i = 0; i < stagedCount; ++i) {
        // Deep copy staged FileBlob into the Commit object
        newCommit->addFile(new FileBlob(*stagedFiles[i]));
    }

    if (head != nullptr) {
        for (int i = 0; i < head->getFileCount(); ++i) {
            FileBlob* prevFile = head->getFile(i);
            if (newCommit->findFile(prevFile->getFileName()) == nullptr) {
                newCommit->addFile(new FileBlob(*prevFile));
            }
        }
    }

    newCommit->computeCommitHash();
    head = newCommit;

    // Persist commit to .minigit/commits.txt
    appendCommitToDisk(newCommit);

    // Clear staging area
    for (int i = 0; i < stagedCount; ++i) {
        delete stagedFiles[i];
        stagedFiles[i] = nullptr;
    }
    stagedCount = 0;
    saveIndex();

    cout << "[MiniGit] Created Commit [" << newCommit->getId() << "]: " << message << "\n";
}

void Repository::status() const {
    ensureInitialized();

    cout << "=== MiniGit Status ===\n";
    if (head != nullptr) {
        cout << "Current HEAD Commit : " << head->getId() << " (\"" << head->getMessage() << "\")\n";
    } else {
        cout << "Current HEAD Commit : None (No commits yet)\n";
    }

    if (remote != nullptr) {
        cout << "Configured Remote   : " << remote->getPath() << "\n";
    }

    cout << "\n--- Staged Files (" << stagedCount << ") ---\n";
    if (stagedCount == 0) {
        cout << "  (No files staged)\n";
    } else {
        for (int i = 0; i < stagedCount; ++i) {
            cout << "  [Staged] " << stagedFiles[i]->getFileName()
                 << " (Hash: " << stagedFiles[i]->getId() << ")\n";
        }
    }

    // Check if any committed files were modified in the working directory (using operator==)
    if (head != nullptr) {
        cout << "\n--- Working Directory Changes ---\n";
        bool anyModified = false;
        for (int i = 0; i < head->getFileCount(); ++i) {
            FileBlob* committedBlob = head->getFile(i);
            ifstream f(committedBlob->getFileName().c_str(), ios::binary);
            if (!f.is_open()) {
                cout << "  [Deleted]  " << committedBlob->getFileName() << "\n";
                anyModified = true;
            } else {
                stringstream ss;
                ss << f.rdbuf();
                FileBlob workingBlob(committedBlob->getFileName(), ss.str());

                // Using overloaded operator== on FileBlob!
                if (!(workingBlob == *committedBlob)) {
                    cout << "  [Modified] " << committedBlob->getFileName() << "\n";
                    anyModified = true;
                }
            }
        }
        if (!anyModified) {
            cout << "  Working tree is clean (all tracked files match HEAD).\n";
        }
    }
    cout << "======================\n";
}

void Repository::log() const {
    ensureInitialized();

    if (head == nullptr) {
        cout << "[MiniGit] No commits in history yet.\n";
        return;
    }

    cout << "=== Commit History (Linked List Traversal) ===\n";
    const Commit* curr = head;
    while (curr != nullptr) {
        // Uses overloaded operator<< of Commit
        cout << *curr;
        curr = curr->getParent();
    }
}

void Repository::diff(const string& fileName) const {
    ensureInitialized();

    if (head == nullptr) {
        throw MiniGitException("No commits exist yet to diff against.");
    }

    FileBlob* committedBlob = head->findFile(fileName);
    if (committedBlob == nullptr) {
        throw MiniGitException("File '" + fileName + "' is not tracked in the latest commit.");
    }

    string workingContent = readFileContent(fileName);
    string committedContent = committedBlob->getContent();

    if (workingContent == committedContent) {
        cout << "[Diff] No changes in '" << fileName << "' compared to HEAD [" << head->getId() << "].\n";
        return;
    }

    cout << "=== Diff for " << fileName << " (Working File vs Commit " << head->getId() << ") ===\n";
    stringstream oldSS(committedContent);
    stringstream newSS(workingContent);

    vector<string> oldLines, newLines;
    string line;
    while (getline(oldSS, line)) oldLines.push_back(line);
    while (getline(newSS, line)) newLines.push_back(line);

    size_t maxLines = (oldLines.size() > newLines.size()) ? oldLines.size() : newLines.size();
    for (size_t i = 0; i < maxLines; ++i) {
        string oLine = (i < oldLines.size()) ? oldLines[i] : "<EOF>";
        string nLine = (i < newLines.size()) ? newLines[i] : "<EOF>";

        if (oLine != nLine) {
            if (i < oldLines.size()) {
                cout << "- [Line " << (i + 1) << "] " << oLine << "\n";
            }
            if (i < newLines.size()) {
                cout << "+ [Line " << (i + 1) << "] " << nLine << "\n";
            }
        } else {
            cout << "  [Line " << (i + 1) << "] " << oLine << "\n";
        }
    }
    cout << "=====================================================\n";
}

void Repository::checkout(const string& commitId) {
    ensureInitialized();

    Commit* curr = head;
    while (curr != nullptr) {
        if (curr->getId() == commitId) {
            for (int i = 0; i < curr->getFileCount(); ++i) {
                FileBlob* f = curr->getFile(i);
                writeFileContent(f->getFileName(), f->getContent());
                cout << "  Restored file: " << f->getFileName() << "\n";
            }
            cout << "[Checkout] Working directory restored to Commit [" << commitId << "]\n";
            return;
        }
        curr = curr->getParent();
    }
    throw MiniGitException("Commit ID '" + commitId + "' not found in history.");
}

void Repository::setRemote(const string& bareFolderPath) {
    ensureInitialized();
    delete remote;
    remote = new BareRemote(bareFolderPath);
    writeFileContent(gitDir + "/remote.txt", bareFolderPath + "\n");
    cout << "[MiniGit] Linked local Bare Remote to: " << bareFolderPath << "\n";
}

void Repository::push() {
    ensureInitialized();
    if (remote == nullptr) {
        throw MiniGitException("No remote configured! Run 'minigit remote <bare_folder>' first.");
    }
    remote->push(gitDir);
}

void Repository::pull() {
    ensureInitialized();
    if (remote == nullptr) {
        throw MiniGitException("No remote configured! Run 'minigit remote <bare_folder>' first.");
    }
    remote->pull(gitDir);
    loadState();
}

void Repository::showObjectsPolymorphic() const {
    ensureInitialized();
    cout << "=== Runtime Polymorphism Demo (Base Pointer GitObject*) ===\n";

    // Store both FileBlob* and Commit* inside a single array of abstract base pointers GitObject*
    vector<const GitObject*> objects;

    for (int i = 0; i < stagedCount; ++i) {
        objects.push_back(stagedFiles[i]);
    }

    const Commit* curr = head;
    while (curr != nullptr) {
        objects.push_back(curr);
        for (int i = 0; i < curr->getFileCount(); ++i) {
            objects.push_back(curr->getFile(i));
        }
        curr = curr->getParent();
    }

    if (objects.empty()) {
        cout << "  (No GitObjects created yet. Stage or commit a file first!)\n";
        return;
    }

    for (size_t i = 0; i < objects.size(); ++i) {
        cout << "Object #" << (i + 1) << " [Type: " << objects[i]->getType() << "]:\n";
        // Dynamic dispatch: calls either FileBlob::display() or Commit::display() at runtime!
        objects[i]->display();
    }
}
