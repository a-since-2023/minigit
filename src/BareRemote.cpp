#include "../include/BareRemote.hpp"
#include "../include/GitObjects.hpp"
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;

#ifdef _WIN32
#include <direct.h>
#define MKDIR(path) _mkdir(path)
#else
#include <sys/stat.h>
#define MKDIR(path) mkdir(path, 0777)
#endif

BareRemote::BareRemote(const string& path) : remotePath(path) {}

BareRemote::~BareRemote() {}

string BareRemote::getPath() const {
    return remotePath;
}

void BareRemote::makeDirectory(const string& dirPath) {
    MKDIR(dirPath.c_str());
}

bool BareRemote::copySingleFile(const string& srcPath, const string& destPath) {
    ifstream src(srcPath.c_str(), ios::binary);
    if (!src.is_open()) return false;

    ofstream dest(destPath.c_str(), ios::binary | ios::trunc);
    if (!dest.is_open()) return false;

    dest << src.rdbuf();
    return true;
}

void BareRemote::initBare() const {
    makeDirectory(remotePath);
    makeDirectory(remotePath + "/objects");

    ofstream commitsFile((remotePath + "/commits.txt").c_str(), ios::app);
    commitsFile.close();

    cout << "[BareRemote] Initialized empty bare repository at: " << remotePath << "\n";
}

void BareRemote::push(const string& localGitDir) const {
    // Ensure bare remote exists
    makeDirectory(remotePath);
    makeDirectory(remotePath + "/objects");

    string localCommitsPath = localGitDir + "/commits.txt";
    ifstream inCommits(localCommitsPath.c_str());
    if (!inCommits.is_open()) {
        throw MiniGitException("Nothing to push. No commits found in local repository.");
    }

    // Copy commits.txt to bare repo
    copySingleFile(localCommitsPath, remotePath + "/commits.txt");

    // Parse commits.txt to find all referenced blob IDs and copy each .blob file
    int blobCount = 0;
    string line;
    while (getline(inCommits, line)) {
        if (line.rfind("FILE|", 0) == 0) {
            // Format: FILE|<filename>|<blobId>
            size_t lastPipe = line.rfind('|');
            if (lastPipe != string::npos) {
                string blobId = line.substr(lastPipe + 1);
                string srcBlob = localGitDir + "/objects/" + blobId + ".blob";
                string dstBlob = remotePath + "/objects/" + blobId + ".blob";
                if (copySingleFile(srcBlob, dstBlob)) {
                    blobCount++;
                }
            }
        }
    }
    inCommits.close();

    cout << "[Push Success] Pushed commits and " << blobCount
         << " file snapshot(s) to Bare Remote: " << remotePath << "\n";
}

void BareRemote::pull(const string& localGitDir) const {
    string remoteCommitsPath = remotePath + "/commits.txt";
    ifstream inCommits(remoteCommitsPath.c_str());
    if (!inCommits.is_open()) {
        throw MiniGitException("Bare remote not found or has no commits at: " + remotePath);
    }

    makeDirectory(localGitDir);
    makeDirectory(localGitDir + "/objects");

    // Copy remote commits.txt to local .minigit/commits.txt
    copySingleFile(remoteCommitsPath, localGitDir + "/commits.txt");

    // Copy all referenced blob files and restore latest files into working folder
    int pulledBlobs = 0;
    string line;
    while (getline(inCommits, line)) {
        if (line.rfind("FILE|", 0) == 0) {
            // Format: FILE|<filename>|<blobId>
            size_t firstPipe = line.find('|');
            size_t secondPipe = line.rfind('|');
            if (firstPipe != string::npos && secondPipe != firstPipe) {
                string fileName = line.substr(firstPipe + 1, secondPipe - firstPipe - 1);
                string blobId = line.substr(secondPipe + 1);

                string srcBlob = remotePath + "/objects/" + blobId + ".blob";
                string dstBlob = localGitDir + "/objects/" + blobId + ".blob";

                if (copySingleFile(srcBlob, dstBlob)) {
                    // Also update the file in the current working directory
                    copySingleFile(dstBlob, fileName);
                    pulledBlobs++;
                }
            }
        }
    }
    inCommits.close();

    cout << "[Pull Success] Pulled latest commits and restored " << pulledBlobs
         << " file(s) from Bare Remote: " << remotePath << "\n";
}
