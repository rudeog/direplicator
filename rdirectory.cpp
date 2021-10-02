#include "stdafx.h"
#include "mainhdr.h"

RDirectory::~RDirectory()
{
    RDirectory *r;
    resetIterateDirectory();
    while(nextDirectory(&r)) {
        delete r;
    }
        
}

RDirectory *
RDirectory::findDirectory(const char *name) 
{
    std::list<RDirectory *>::iterator it;
    
    for(it=mDirectories.begin();it != mDirectories.end(); it++) {
        if(!stricmp(name,(*it)->mName.c_str()))
            return *it;
    }
    return 0;
}

bool
RDirectory::findFile(const char *name, RFileInfo **result) 
{
    std::list<RFileInfo>::iterator it;
    for(it=mFiles.begin(); it != mFiles.end(); it++) {
        if(!stricmp(name, it->mName.c_str())) {
            if(result)
                *result=&(*it);
            return true;
        }
    }
    return false;
}

void
RDirectory::addFile(const RFileInfo &fi)
{
    mFiles.push_back(fi);
}

void
RDirectory::addDirectory(RDirectory *dir)
{
    mDirectories.push_back(dir);
}

const std::string &
RDirectory::getFullName()
{
    return mFullName;
}

const std::string &
RDirectory::getName()
{
    return mName;
}

void 
RDirectory::resetIterateFile()
{
    mFileInfoIterator=mFiles.end();

}

bool 
RDirectory::nextFile(RFileInfo **fileInfo)
{
    if(mFileInfoIterator==mFiles.end()) {
        mFileInfoIterator=mFiles.begin();        
    } else {
        mFileInfoIterator++;
    }

    if(mFileInfoIterator==mFiles.end())
        return false;
     
    *fileInfo=&(*mFileInfoIterator);

    return true;    
}

void 
RDirectory::resetIterateDirectory()
{
    mDirectoryIterator = mDirectories.end();

}

bool 
RDirectory::nextDirectory(RDirectory **directory)
{
    if(mDirectoryIterator==mDirectories.end())
        mDirectoryIterator = mDirectories.begin();
    else
        mDirectoryIterator++;

    if(mDirectoryIterator==mDirectories.end())
        return false;

    *directory=*mDirectoryIterator;
    return true;
}
