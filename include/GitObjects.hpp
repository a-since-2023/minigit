#ifndef GIT_OBJECTS_HPP
#define GIT_OBJECTS_HPP

#include <iostream>
#include <string>
#include <stdexcept>

using namespace std;

// ============================================================================
// 1. EXCEPTION HANDLING: Custom Exception Class inheriting from runtime_error
// ============================================================================
class MiniGitException : public runtime_error {
public:
    explicit MiniGitException(const string& msg)
        : runtime_error("[MiniGit Error] " + msg) {}
};

// ============================================================================
// 2. SIMPLE HASH UTILITY: Easy 10-line DJB2 Hash Function (returns 8-char hex)
// ============================================================================
class SimpleHash {
public:
    static string generate(const string& input);
};

// ============================================================================
// 3. ABSTRACTION: Abstract Base Class for all Git Objects
// ============================================================================
class GitObject {
protected:
    string id; // 8-character hash ID

public:
    GitObject();
    explicit GitObject(const string& objectId);
    virtual ~GitObject(); // Virtual destructor for safe polymorphic deletion

    string getId() const;
    void setId(const string& newId);

    // Pure Virtual Functions (Enforces Polymorphism in derived classes)
    virtual void display() const = 0;
    virtual string getType() const = 0;
};

// ============================================================================
// 4. INHERITANCE & OPERATOR OVERLOADING: FileBlob Class (Stores a file snapshot)
// ============================================================================
class FileBlob : public GitObject {
private:
    string fileName;
    string content;

public:
    FileBlob(const string& name, const string& fileContent);
    FileBlob(const FileBlob& other); // Copy Constructor
    ~FileBlob() override;

    string getFileName() const;
    string getContent() const;

    // Polymorphic overrides
    void display() const override;
    string getType() const override;

    // Operator Overloading: Check if two FileBlobs have the exact same content hash
    bool operator==(const FileBlob& other) const;
};

// ============================================================================
// 5. RAW POINTERS, COMPOSITION & LINKED LIST: Commit Class
// ============================================================================
class Commit : public GitObject {
private:
    string message;
    string author;
    string timestamp;

    FileBlob** files;   // Dynamic array of raw FileBlob* pointers
    int fileCount;
    int capacity;

    Commit* parent;     // Raw pointer to previous Commit (Linked List node)

public:
    Commit(const string& msg, const string& auth, Commit* parentCommit = nullptr);
    Commit(const string& existingId, const string& msg,
           const string& auth, const string& timeStr, Commit* parentCommit = nullptr);
    Commit(const Commit& other); // Deep Copy Constructor (Rule of Three)
    ~Commit() override;          // Destructor freeing dynamic FileBlob* array

    void addFile(FileBlob* blob);
    void computeCommitHash();

    string getMessage() const;
    string getAuthor() const;
    string getTimestamp() const;
    int getFileCount() const;
    FileBlob* getFile(int index) const;
    FileBlob* findFile(const string& name) const;

    Commit* getParent() const;
    void setParent(Commit* parentCommit);

    // Polymorphic overrides
    void display() const override;
    string getType() const override;

    // Operator Overloading: Print commit nicely using cout << commitObj
    friend ostream& operator<<(ostream& os, const Commit& commit);
};

#endif // GIT_OBJECTS_HPP
