#include "../include/GitObjects.hpp"
#include <sstream>
#include <iomanip>
#include <ctime>
using namespace std;

// ============================================================================
// SimpleHash Implementation (DJB2 Algorithm -> 8-character Hex String)
// ============================================================================
string SimpleHash::generate(const string& input) {
    unsigned long hash = 5381;
    for (size_t i = 0; i < input.length(); ++i) {
        hash = ((hash << 5) + hash) + static_cast<unsigned char>(input[i]); // hash * 33 + c
    }
    stringstream ss;
    ss << hex << setw(8) << setfill('0') << (hash & 0xFFFFFFFFUL);
    return ss.str();
}

// ============================================================================
// GitObject Base Class Implementation
// ============================================================================
GitObject::GitObject() : id("00000000") {}

GitObject::GitObject(const string& objectId) : id(objectId) {}

GitObject::~GitObject() {}

string GitObject::getId() const {
    return id;
}

void GitObject::setId(const string& newId) {
    id = newId;
}

// ============================================================================
// FileBlob Implementation
// ============================================================================
FileBlob::FileBlob(const string& name, const string& fileContent)
    : GitObject(), fileName(name), content(fileContent) {
    // Hash is computed from filename + content
    id = SimpleHash::generate(fileName + ":" + content);
}

FileBlob::FileBlob(const FileBlob& other)
    : GitObject(other.id), fileName(other.fileName), content(other.content) {}

FileBlob::~FileBlob() {}

string FileBlob::getFileName() const {
    return fileName;
}

string FileBlob::getContent() const {
    return content;
}

void FileBlob::display() const {
    cout << "[Blob " << id << "] File: " << fileName
         << " (" << content.length() << " bytes)\n";
}

string FileBlob::getType() const {
    return "Blob";
}

bool FileBlob::operator==(const FileBlob& other) const {
    return (this->id == other.id) && (this->content == other.content);
}

// ============================================================================
// Commit Implementation (Dynamic Raw Pointers & Deep Copy)
// ============================================================================
static string getCurrentTimeStr() {
    time_t now = time(nullptr);
    char buf[64];
    tm* lt = localtime(&now);
    if (lt) {
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", lt);
        return string(buf);
    }
    return "2026-10-29";
}

Commit::Commit(const string& msg, const string& auth, Commit* parentCommit)
    : GitObject(), message(msg), author(auth), timestamp(getCurrentTimeStr()),
      files(nullptr), fileCount(0), capacity(10), parent(parentCommit) {
    // Allocate dynamic array of raw FileBlob* pointers on the heap
    files = new FileBlob*[capacity];
    for (int i = 0; i < capacity; ++i) {
        files[i] = nullptr;
    }
    computeCommitHash();
}

Commit::Commit(const string& existingId, const string& msg,
               const string& auth, const string& timeStr, Commit* parentCommit)
    : GitObject(existingId), message(msg), author(auth), timestamp(timeStr),
      files(nullptr), fileCount(0), capacity(10), parent(parentCommit) {
    files = new FileBlob*[capacity];
    for (int i = 0; i < capacity; ++i) {
        files[i] = nullptr;
    }
}

// Deep Copy Constructor (Demonstrates Rule of Three with raw pointers)
Commit::Commit(const Commit& other)
    : GitObject(other.id), message(other.message), author(other.author),
      timestamp(other.timestamp), files(nullptr), fileCount(other.fileCount),
      capacity(other.capacity), parent(other.parent) {
    files = new FileBlob*[capacity];
    for (int i = 0; i < capacity; ++i) {
        if (i < fileCount && other.files[i] != nullptr) {
            files[i] = new FileBlob(*other.files[i]); // Deep copy each FileBlob
        } else {
            files[i] = nullptr;
        }
    }
}

// Destructor: Explicitly frees all dynamically allocated FileBlob* pointers
Commit::~Commit() {
    if (files != nullptr) {
        for (int i = 0; i < fileCount; ++i) {
            delete files[i];
        }
        delete[] files;
        files = nullptr;
    }
}

void Commit::addFile(FileBlob* blob) {
    if (blob == nullptr) return;

    // Resize array if full
    if (fileCount >= capacity) {
        int newCap = capacity * 2;
        FileBlob** newArr = new FileBlob*[newCap];
        for (int i = 0; i < fileCount; ++i) {
            newArr[i] = files[i];
        }
        for (int i = fileCount; i < newCap; ++i) {
            newArr[i] = nullptr;
        }
        delete[] files;
        files = newArr;
        capacity = newCap;
    }

    files[fileCount++] = blob;
    computeCommitHash();
}

void Commit::computeCommitHash() {
    string raw = message + "|" + author + "|" + timestamp;
    if (parent != nullptr) {
        raw += "|" + parent->getId();
    }
    for (int i = 0; i < fileCount; ++i) {
        if (files[i] != nullptr) {
            raw += "|" + files[i]->getId();
        }
    }
    id = SimpleHash::generate(raw);
}

string Commit::getMessage() const { return message; }
string Commit::getAuthor() const { return author; }
string Commit::getTimestamp() const { return timestamp; }
int Commit::getFileCount() const { return fileCount; }

FileBlob* Commit::getFile(int index) const {
    if (index < 0 || index >= fileCount) return nullptr;
    return files[index];
}

FileBlob* Commit::findFile(const string& name) const {
    for (int i = 0; i < fileCount; ++i) {
        if (files[i] != nullptr && files[i]->getFileName() == name) {
            return files[i];
        }
    }
    return nullptr;
}

Commit* Commit::getParent() const { return parent; }
void Commit::setParent(Commit* parentCommit) { parent = parentCommit; }

void Commit::display() const {
    cout << *this;
}

string Commit::getType() const {
    return "Commit";
}

// Operator Overloading for printing a Commit
ostream& operator<<(ostream& os, const Commit& commit) {
    os << "------------------------------------------------\n";
    os << "Commit ID : " << commit.id << "\n";
    if (commit.parent != nullptr) {
        os << "Parent ID : " << commit.parent->getId() << "\n";
    } else {
        os << "Parent ID : None (Initial Commit)\n";
    }
    os << "Author    : " << commit.author << "\n";
    os << "Date      : " << commit.timestamp << "\n";
    os << "Message   : " << commit.message << "\n";
    os << "Files (" << commit.fileCount << ") : ";
    for (int i = 0; i < commit.fileCount; ++i) {
        os << commit.files[i]->getFileName() << " [" << commit.files[i]->getId() << "] ";
    }
    os << "\n------------------------------------------------\n";
    return os;
}
