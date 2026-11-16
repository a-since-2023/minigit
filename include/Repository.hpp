#ifndef REPOSITORY_HPP
#define REPOSITORY_HPP

#include "GitObjects.hpp"
#include "BareRemote.hpp"
#include <string>

using namespace std;

/**
 * @brief Main Repository Controller Class.
 * Manages the staging area (FileBlob* array), Commit history (Commit* linked list),
 * and BareRemote synchronization using raw pointers.
 */
class Repository {
private:
    static const int MAX_STAGED = 50;

    string gitDir;                      // ".minigit"
    Commit* head;                       // Raw pointer to latest Commit (Linked List head)
    FileBlob* stagedFiles[MAX_STAGED];  // Raw pointers to staged FileBlobs
    int stagedCount;
    BareRemote* remote;                 // Raw pointer to BareRemote

    // Internal helpers for persistence (.minigit/ files)
    void ensureInitialized() const;
    static string readFileContent(const string& path);
    static void writeFileContent(const string& path, const string& content);

    void loadState();
    void saveIndex() const;
    void appendCommitToDisk(const Commit* newCommit) const;
    void freeMemory();

public:
    Repository();
    ~Repository(); // Frees Commit* linked list, staged FileBlob* array, and BareRemote*

    bool isInitialized() const;
    string getHeadId() const;
    int getStagedCount() const;

    // Core MiniGit CLI Commands
    void init();
    void uninit();
    void initBare(const string& bareFolderPath);
    void add(const string& fileName);
    void removeFile(const string& fileName);
    void commit(const string& message, const string& author = "BTech_Student");
    void status() const;
    void log() const;
    void diff(const string& fileName) const;
    void checkout(const string& commitId);

    // Remote Synchronization Commands
    void setRemote(const string& bareFolderPath);
    void push();
    void pull();

    // Explicit Runtime Polymorphism Demonstration (Base pointer GitObject* calling virtual display)
    void showObjectsPolymorphic() const;
};

#endif // REPOSITORY_HPP
