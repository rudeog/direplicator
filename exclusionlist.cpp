#include "stdafx.h"
#include "mainhdr.h"

void RExclusionList::addToList(std::list<Exclusion> &list, const std::string &name, const std::string &desc) 
{
    const char *str;
    std::string forlog;
    size_t len;
    Exclusion ex;
    str=name.c_str();
    len=name.length();
    if(!*str)
        return;
    if(str[0]=='*') {
        ex.mType=ENDSWITH;
        ex.mValue=&str[1];
        forlog="ends with";
    } else if(str[len-1]=='*') {
        ex.mType=STARTSWITH;
        ex.mValue.assign(str,len-1);
        forlog="starts with";
    } else {
        ex.mType=EXACT;
        ex.mValue=str;
        forlog="exact match to";
    }
    logMessage(LOG_VERBOSE,"SKIP ", desc," RULE: ", forlog," ",ex.mValue);
    list.push_back(ex);
}

bool
RExclusionList::findInList(std::list<Exclusion> &list, const char *name) 
{
    const char *lstr;
    size_t len, namelen;
    Exclusion *e;
    namelen=strlen(name);
    for(ExclIt it=list.begin();it!=list.end();it++) {
        e=&(*it);
        lstr=e->mValue.c_str();
        len=e->mValue.length();
        switch(e->mType) {
        case EXACT:
            if(!stricmp(lstr,name))
                return true;
            break;
        case STARTSWITH:
            if(!strnicmp(lstr,name,len))
                return true;
            break;
        case ENDSWITH:
            if(!stricmp(lstr,&name[namelen-len]))
                return true;
            break;            
        }
    }

    return false;
}

bool 
RExclusionList::isFileExcluded(const char *name) 
{
    return findInList(mFileExclusions,name);
}

bool 
RExclusionList::isDirectoryExcluded(const char *name) 
{
    return findInList(mDirExclusions,name);
}

void
RExclusionList::addFileExclusion(const char *name) 
{
    addToList(mFileExclusions,name,"FILE");
}

void 
RExclusionList::addDirExclusion(const char *name) 
{
    addToList(mDirExclusions,name,"DIR");
}
