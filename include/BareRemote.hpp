#ifndef BARE_REMOTE_HPP
#define BARE_REMOTE_HPP

#include <string>

using namespace std;

/**
 * @brief Represents a Local "Bare" Remote Repository (no working files, only objects & commits).
 * Allows pushing and pulling commits between local folders (like a local GitHub server).
 */
class BareRemote {
private:
    string remotePath;

    // Helper functions for cross-platform directory creation and file copying
    static void makeDirectory(const string& dirPath);
    static bool copySingleFile(const string& srcPath, const string& destPath);

public:
    explicit BareRemote(const string& path);
    ~BareRemote();

    string getPath() const;

    // Initializes a bare repository folder (commits.txt + objects/ folder)
    void initBare() const;

    // Pushes local commits and blob snapshots to the bare repository
    void push(const string& localGitDir) const;

    // Pulls commits and blob snapshots from the bare repository into the local repo
    void pull(const string& localGitDir) const;
};

#endif // BARE_REMOTE_HPP
