#ifndef MAINHDR_H_
#define MAINHDR_H_

// different log levels
enum RLogLevel {
    LOG_ERROR,
    LOG_BRIEF,
    LOG_VERBOSE
};

// Information about a file
struct RFileInfo {
    std::string mName;
    DWORD mAttributes;
    FILETIME mFileTime;
    DWORD mFileSizeHi;
    DWORD mFileSizeLo;
};

// info about a directory
class RDirectory {
    std::list<RDirectory *>::iterator mDirectoryIterator;
    std::list<RFileInfo>::iterator mFileInfoIterator;
    // directory name
    std::string mName;
    std::string mFullName;
    // files in this directory
    std::list<RFileInfo> mFiles;
    // other directories in this directory
    std::list<RDirectory *> mDirectories;

    
public:
    RDirectory(const std::string name, const std::string fullname) : 
        mName(name), mFullName(fullname)
        {}
    ~RDirectory();
    void addDirectory(RDirectory *dir);
    void addFile(const RFileInfo &fi);

    bool findFile(const char *name, RFileInfo **result=0);
    RDirectory *findDirectory(const char *name);
    const std::string &getFullName();
    const std::string &getName();

    void resetIterateFile();
    bool nextFile(RFileInfo **fileInfo);
    void resetIterateDirectory();
    bool nextDirectory(RDirectory **directory);

    

};

// Valid values
//   filename   -EXACT
//   filename*  -STARTSWITH
//   *filename  -ENDSWITH

class RExclusionList {
    enum GlobType {
        EXACT,
        STARTSWITH,
        ENDSWITH,
        CONTAINS
    };
    struct Exclusion {
        GlobType mType;
        std::string mValue;
    };
    typedef std::list<Exclusion>::iterator ExclIt;
    std::list<Exclusion> mFileExclusions;
    std::list<Exclusion> mDirExclusions;
    bool findInList(std::list<Exclusion> &list, const char *name);
    void addToList(std::list<Exclusion> &list, const std::string &name, const std::string &desc);
public:
    bool isFileExcluded(const char *name);
    bool isDirectoryExcluded(const char *name);
    void addFileExclusion(const char *name);
    void addDirExclusion(const char *name);    
};


struct RConfigEntry {
    std::string mSourceDir;
    std::string mDestDir;
    std::list<std::string> mDirExclusions;
    std::list<std::string> mFileExclusions;
};

// function prototypes
#define FILLDIRECTORY_SUCCESS   1
#define FILLDIRECTORY_FAILURE   0
#define FILLDIRECTORY_SKIP      2
int fillDirectory(RDirectory *dirobj, bool recurse, bool checkExclusions, std::string *error);
bool processDirectory(RDirectory *dirobj, const std::string targetdir, std::string *error);

std::string getSystemErrorMessage(const std::string &fn);
void logMessage(RLogLevel logLevel, const std::string &msg, 
                const std::string &msg2=std::string(),
                const std::string &msg3=std::string(),
                const std::string &msg4=std::string(),
                const std::string &msg5=std::string(),
                const std::string &msg6=std::string());
           
bool confirm(const std::string &a, const std::string b);

bool deleteDirTree(const std::string &name, std::string *error);
bool rCreateDirectory(const std::string &path, std::string *error);
bool rCopyFile(const std::string &srcpath, const std::string &destpath, const std::string &name, std::string *error);
std::string concatPath(const std::string &path, const std::string &name);
bool rDeleteFile(const std::string &name, DWORD attributes, std::string *error);
bool rRemoveDirectory(const std::string &name, std::string *error);


#endif
