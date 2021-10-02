#include "stdafx.h"
#include "mainhdr.h"
#include "ucshelper.h"
#define PROGRAM_NAME "Direplicator"
#define PROGRAM_VERSION "1.2"

bool run_mirror();
bool build_config(const char *configfile, const char *src, const char *dest);
bool read_config(const char *configfile);

// GLOBALS

// These are options. Set the defaults here

// level of logging desired
RLogLevel gCurrentLogLevel = LOG_ERROR;
// whether to log only
bool gLogOnly = false;
// whether to confirm deletions
bool gNoConfirm = true;
// whether to prevent any deletion
bool gNoDelete = false;
// name of log file
std::string gLogFileName;
// if it sees this file in a dir, it skips the dir
std::string gSkipDirFileName;
// continue running after error
bool gContinueOnErrors;

// avoid programming errors causing mass destruction
std::string gTargetSanityCheck;

// list of exclusions
RExclusionList *gExclusions;
RDirectory *gRootDir;
std::list<RConfigEntry> gConfigEntries;

FILE *gLogFH = 0;

int gNumFilesCopied = 0;
int gNumDirsCreated = 0;
int gNumDirsRemoved = 0;
int gNumFilesRemoved = 0;


int helpExit();
// MAIN

int _tmain(int argc, _TCHAR* argv[])
{
   if (argc < 3)
      return helpExit();

   UCToUTF8 cfg(argv[2]);

   if (!_wcsicmp(argv[1], L"-run")) {

      if (!read_config(cfg.utf8String()))
         goto cleanup;
      if (!run_mirror())
         goto cleanup;

   }
   else if (!_wcsicmp(argv[1], L"-buildconfig")) {
      if (argc == 5) {
         UCToUTF8 cfg3(argv[3]);
         UCToUTF8 cfg4(argv[4]);

         if (!build_config(cfg.utf8String(), cfg3.utf8String(), cfg4.utf8String()))
            goto cleanup;
      }
      else
         return helpExit();
   }
   else
      return helpExit();

   fprintf(stderr, PROGRAM_NAME " " PROGRAM_VERSION " terminated with success.\n");
   return 0;
cleanup:
   fprintf(stderr, PROGRAM_NAME " " PROGRAM_VERSION " terminated with failure.\n");
   return 1;
}


int
helpExit() 
{
   printf(PROGRAM_NAME " " PROGRAM_VERSION" Copyright (c)2006-2021 Andrew Shakinovsky\n"
      "Home: https://github.com/rudeog/direplicator\n"
      "Arguments:\n"
      "  -run config_file_name\n"
      "or, to create a backup config file:\n"
      "  -buildconfig config_file_name srcdir destdir\n"
      "where srcdir is the source directory to backup and destdir\n"
      "is the target for the backup.\n\n"
      "DISCLAIMER: This program is provided 'AS IS' without warranty of any kind,\n"
      "either expressed implied, including, but not limited to, the implied \n"
      "warranties of merchantability and fitness for a particular purpose.\n"
      "The entire risk as to the quality and performance of the program is with\n"
      "you. Should the program prove defective, you assume the entire cost of\n"
      "all necessary servicing, repair, and correction.\n\n"
      "THIS PROGRAM HAS THE CAPABILITY OF DELETING FILES FROM YOUR HARD DRIVE!\n"
      "Please read the instructions and study the configuration file before\n"
      "placing in service.\n");

   return 0;
}

void
trim_char(char *val, char ch)
{
   char *pos;
   pos = val;
   if (!(*pos))
      return;

   while (*pos)
      pos++;
   pos--;
   if (*pos == ch)
      *pos = 0;

}

bool
read_config(const char *configfile)
{
   FILE *fh;
   char linebuf[1024];
   char *pos, *val;
   int lineno = 0;
   RConfigEntry *curEntry = 0;

   fh = fopen(configfile, "r");
   if (!fh) {
      fprintf(stderr, "Open config file '%s' failed\n", configfile);
      return false;
   }
   linebuf[1023] = 0;
   while (fgets(linebuf, 1024, fh)) {
      lineno++;
      if (linebuf[0] == '#' || linebuf[0] == '\r' || linebuf[0] == '\n')
         continue;

      trim_char(linebuf, '\n');
      trim_char(linebuf, '\r');

      pos = strchr(linebuf, ' ');
      if (!pos) {
         val = 0;
      }
      else {
         *pos = 0;
         pos++;
         val = pos;
      }

      if (_stricmp(linebuf, "OPTIONS") != 0 && val == 0) {
         fprintf(stderr, "Error in config file on line %d. Incomplete entry.\n", lineno);
         return false;
      }

      if (!_stricmp(linebuf, "SOURCE")) {
         // create a new RConfigEntry
         gConfigEntries.push_back(RConfigEntry());
         curEntry = &gConfigEntries.back();
         trim_char(val, '\\');
         curEntry->mSourceDir = val;

      }
      else if (!_stricmp(linebuf, "DEST")) {
         if (curEntry) {
            trim_char(val, '\\');
            curEntry->mDestDir = val;
         }
         else {
            fprintf(stderr, "Config error. DEST not seen where expected at line %d\n", lineno);
            goto cleanup;
         }

      }
      else if (!_stricmp(linebuf, "EXCLUDEDIR")) {
         if (!curEntry) {
            fprintf(stderr, "Config error. EXCLUDEDIR not seen where expected at line %d\n", lineno);
            goto cleanup;
         }
         curEntry->mDirExclusions.push_back(val);

      }
      else if (!_stricmp(linebuf, "EXCLUDEFILE")) {
         if (!curEntry) {
            fprintf(stderr, "Config error. EXCLUDEFILE not seen where expected at line %d\n", lineno);
            goto cleanup;
         }
         curEntry->mFileExclusions.push_back(val);

      }
      else if (!_stricmp(linebuf, "OPTIONS")) {
         curEntry = 0;

      }
      else if (!_stricmp(linebuf, "LOGLEVEL")) {
         if (curEntry) {
            fprintf(stderr, "Config error. LOGLEVEL not seen where expected at line %d\n", lineno);
            goto cleanup;
         }
         if (!_stricmp(val, "VERBOSE"))
            gCurrentLogLevel = LOG_VERBOSE;
         else if (!_stricmp(val, "BRIEF"))
            gCurrentLogLevel = LOG_BRIEF;
         else if (!_stricmp(val, "ERROR"))
            gCurrentLogLevel = LOG_ERROR;
         else {
            fprintf(stderr, "Invalid LOGLEVEL value at line %d", lineno);
            goto cleanup;
         }
      }
      else if (!_stricmp(linebuf, "CONFIRMDELETE")) {
         if (curEntry) {
            fprintf(stderr, "Config error. CONFIRMDELETE not seen where expected at line %d\n", lineno);
            goto cleanup;
         }
         if (!_stricmp(val, "ON"))
            gNoConfirm = false;
         else if (!_stricmp(val, "OFF"))
            gNoConfirm = true;
         else {
            fprintf(stderr, "Invalid CONFIRMDELETE value at line %d", lineno);
            goto cleanup;
         }
      }
      else if (!_stricmp(linebuf, "NODELETE")) {
         if (curEntry) {
            fprintf(stderr, "Config error. NODELETE not seen where expected at line %d\n", lineno);
            goto cleanup;
         }
         if (!_stricmp(val, "ON"))
            gNoDelete = true;
         else if (!_stricmp(val, "OFF"))
            gNoDelete = false;
         else {
            fprintf(stderr, "Invalid CONFIRMDELETE value at line %d", lineno);
            goto cleanup;
         }
      }
      else if (!_stricmp(linebuf, "SKIPFILENAME")) {
         if (curEntry) {
            fprintf(stderr, "Config error. SKIPFILENAME not seen where expected at line %d\n", lineno);
            goto cleanup;
         }
         gSkipDirFileName = val;

      }
      else if (!_stricmp(linebuf, "LOGFILE")) {
         if (curEntry) {
            fprintf(stderr, "Config error. LOGFILE not seen where expected at line %d\n", lineno);
            goto cleanup;
         }
         gLogFileName = val;
      }
      else if (!_stricmp(linebuf, "LOGONLY")) {
         if (curEntry) {
            fprintf(stderr, "Config error. LOGONLY not seen where expected at line %d\n", lineno);
            goto cleanup;
         }
         if (!_stricmp(val, "ON"))
            gLogOnly = true;
         else if (!_stricmp(val, "OFF"))
            gLogOnly = false;
         else {
            fprintf(stderr, "Invalid LOGONLY value at line %d", lineno);
            goto cleanup;
         }
      }
      else if (!_stricmp(linebuf, "IGNOREERRORS")) {
         if (curEntry) {
            fprintf(stderr, "Config error. IGNOREERRORS not seen where expected at line %d\n", lineno);
            goto cleanup;
         }
         if (!_stricmp(val, "ON"))
            gContinueOnErrors = true;
         else if (!_stricmp(val, "OFF"))
            gContinueOnErrors = false;
         else {
            fprintf(stderr, "Invalid LOGONLY value at line %d", lineno);
            goto cleanup;
         }
      }
   }
   if (ferror(fh)) {
      fprintf(stderr, "Error occurred reading file '%s'\n", configfile);
      goto cleanup;
   }

   fclose(fh);
   return true;

cleanup:
   fclose(fh);
   return false;
}

bool
run_mirror()
{
   RConfigEntry *cur;
   std::string errmsg;
   int fdret;


   std::list<RConfigEntry>::iterator it;
   std::list<std::string>::iterator strit;

   if (gLogFileName.length()) {
      gLogFH = fopen(gLogFileName.c_str(), "a");
      if (!gLogFH) {
         logMessage(LOG_ERROR, "Logfile ", gLogFileName, " couldn't be opened");
         goto cleanup;
      }
      SYSTEMTIME st;
      GetLocalTime(&st);
      fprintf(gLogFH, "Log opened %.04d-%.02d-%.02d %.02d:%.02d:%.02d\n", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
   }

   for (it = gConfigEntries.begin(); it != gConfigEntries.end(); it++) {
      cur = &(*it);
      // do some sanity checking
      if (!_strnicmp(cur->mSourceDir.c_str(), cur->mDestDir.c_str(), cur->mDestDir.length())) {
         logMessage(LOG_ERROR, "Source ", cur->mSourceDir, ": Destination directory is invalid, or is a parent of source directory");
         goto cleanup;
      }

      if (cur->mDestDir.length() < 3) {
         logMessage(LOG_ERROR, "Destination ", cur->mDestDir, ": For safety reasons, you must specify a directory name at least one level deep");
         goto cleanup;
      }
   }

   logMessage(LOG_VERBOSE, "Settings in effect:");

   if (gLogOnly)
      logMessage(LOG_VERBOSE, "No file operations will take place (log only)");
   else
      logMessage(LOG_VERBOSE, "File operations will take place");
   if (gNoConfirm)
      logMessage(LOG_VERBOSE, "No file delete confirmations will take place");
   else
      logMessage(LOG_VERBOSE, "Confirmation will take place before files are deleted");
   if (gNoDelete)
      logMessage(LOG_VERBOSE, "No file delete will take place");
   else
      logMessage(LOG_VERBOSE, "Files may be deleted");
   if (gContinueOnErrors)
      logMessage(LOG_VERBOSE, "Program will continue running when errors are encountered");
   else
      logMessage(LOG_VERBOSE, "Program will stop running when an error is encountered");

   if (gSkipDirFileName.length())
      logMessage(LOG_VERBOSE, "Directories containing the file ", gSkipDirFileName, " will be skipped");


   for (it = gConfigEntries.begin(); it != gConfigEntries.end(); it++) {
      cur = &(*it);
      logMessage(LOG_VERBOSE, "Settings for ", cur->mSourceDir, ":");
      logMessage(LOG_VERBOSE, "Destination: ", cur->mDestDir);

      gRootDir = new RDirectory(":::::", cur->mSourceDir);
      gTargetSanityCheck = cur->mDestDir;

      gExclusions = new RExclusionList();
      for (strit = cur->mDirExclusions.begin(); strit != cur->mDirExclusions.end(); strit++)
         gExclusions->addDirExclusion(strit->c_str());

      for (strit = cur->mFileExclusions.begin(); strit != cur->mFileExclusions.end(); strit++)
         gExclusions->addFileExclusion(strit->c_str());

      logMessage(LOG_VERBOSE, "Processing ", cur->mSourceDir);

      fdret = fillDirectory(gRootDir, true, true, &errmsg);
      if (fdret == FILLDIRECTORY_FAILURE) {
         logMessage(LOG_ERROR, errmsg);
         goto cleanup;
      }

      if (!processDirectory(gRootDir, cur->mDestDir, &errmsg)) {
         logMessage(LOG_ERROR, errmsg);
         goto cleanup;
      }

      delete gExclusions;
      delete gRootDir;
   }
   FILE *tfh;
   if (gLogFH)
      tfh = gLogFH;
   else
      tfh = stdout;
   fprintf(tfh, "%d files copied, %d files deleted, %d directories created, %d directories deleted\n",
      gNumFilesCopied, gNumFilesRemoved, gNumDirsCreated, gNumDirsRemoved);

   if (gLogFH)
      fclose(gLogFH);
   return true;

cleanup:
   if (gLogFH)
      fclose(gLogFH);
   return false;
}

int
fillDirectory(RDirectory *dirobj, bool recurse, bool checkExclusions, std::string *error)
{
   WIN32_FIND_DATA find_info;
   HANDLE hFind;
   std::string newdirname, dirspec;
   bool addtolist;

   if (dirobj->getFullName().length() == 0) {
      *error = "Invalid directory. Cannot be blank.";
      return FILLDIRECTORY_FAILURE;
   }

   if (gSkipDirFileName.length() && checkExclusions) {
      dirspec = dirobj->getFullName();
      dirspec.append("\\");
      dirspec.append(gSkipDirFileName);
      Utf8ToUC ds;
      ds.set(dirspec.c_str());
      hFind = FindFirstFile(ds.ucString(), &find_info);
      if (hFind != INVALID_HANDLE_VALUE) {
         logMessage(LOG_VERBOSE, "SKIPDIR ", dirobj->getFullName());
         FindClose(hFind);
         return FILLDIRECTORY_SKIP;
      }
   }


   dirspec = dirobj->getFullName();
   dirspec.append("\\*");

   Utf8ToUC ds;
   ds.set(dirspec.c_str());
   hFind = FindFirstFile(ds.ucString(), &find_info);
   if (hFind == INVALID_HANDLE_VALUE) {
      error->assign(dirobj->getFullName());
      error->append(": ");
      error->append(getSystemErrorMessage("Find directory"));
      return FILLDIRECTORY_FAILURE;
   }
   do {
      UCToUTF8 fn(find_info.cFileName);
      

      //logMessage(LOG_VERBOSE,"Found ", find_info.cFileName);
      if ((find_info.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) != 0) {
         // this is a junction that should never be backed up
         
         logMessage(LOG_VERBOSE, "SKIP REPARSE POINT ", fn.utf8String());
         goto skip_item;
      }
      if ((find_info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) {
         
         // is directory
         if (!strcmp(fn.utf8String(), ".."))
            goto skip_item;
         if (!strcmp(fn.utf8String(), "."))
            goto skip_item;
         if (checkExclusions && gExclusions->isDirectoryExcluded(fn.utf8String())) {
            logMessage(LOG_VERBOSE, "SKIPDIR ", dirobj->getFullName(), "\\", fn.utf8String());
            goto skip_item;
         }

         newdirname = concatPath(dirobj->getFullName(), fn.utf8String());
         RDirectory *newdirobj =
            new RDirectory(fn.utf8String(),
               concatPath(dirobj->getFullName(), fn.utf8String()));

         addtolist = true;
         if (recurse) {
            int fdret;
            fdret = fillDirectory(newdirobj, true, checkExclusions, error);
            if (fdret == FILLDIRECTORY_SKIP) {
               addtolist = false;
            }
            else if (fdret == FILLDIRECTORY_FAILURE) {
               delete newdirobj;
               return FILLDIRECTORY_FAILURE;
            }
         }

         if (addtolist)
            dirobj->addDirectory(newdirobj);

      }
      else {
         // is file
         RFileInfo fi;
         if (checkExclusions && gExclusions->isFileExcluded(fn.utf8String())) {
            logMessage(LOG_VERBOSE, "SKIPFILE ", dirobj->getFullName(), "\\", fn.utf8String());
            goto skip_item;
         }

         fi.mName = fn.utf8String(); //take copy
         fi.mFileSizeHi = find_info.nFileSizeHigh;
         fi.mFileSizeLo = find_info.nFileSizeLow;
         fi.mFileTime = find_info.ftLastWriteTime;
         fi.mAttributes = find_info.dwFileAttributes;
         dirobj->addFile(fi);
      }
   skip_item:
      ;

   } while (FindNextFile(hFind, &find_info));

   FindClose(hFind);

   return FILLDIRECTORY_SUCCESS;

}

// This function does the bulk of the work and is called recursively
bool
processDirectory(RDirectory *dirobj, const std::string targetdir, std::string *error)
{
   RFileInfo *curFileInfo;
   RDirectory *curDir;
   bool docopy;
   RFileInfo *fi;
   std::string tmp;
   bool dirwascreated;


   RDirectory targ(":::::", targetdir);
   // get info about the target dir
   if (fillDirectory(&targ, false, false, error) != FILLDIRECTORY_SUCCESS)
      return false;

   // first see if there are any directories in the target that don't exist in this
   // and delete them if so
   targ.resetIterateDirectory();
   while (targ.nextDirectory(&curDir)) {
      if (dirobj->findDirectory(curDir->getName().c_str()) == 0) {
         if (!deleteDirTree(curDir->getFullName(), error)) {
            return false;
         }
      }
   }

   // see if there are any files in target that dont exist here, if so, delete them from targ
   targ.resetIterateFile();
   while (targ.nextFile(&curFileInfo)) {
      if (!dirobj->findFile(curFileInfo->mName.c_str())) {
         tmp = concatPath(targ.getFullName(), curFileInfo->mName);
         if (!rDeleteFile(tmp, curFileInfo->mAttributes, error))
            return false;
      }
   }

   // For each file here, find it in target and check date/size, if diff, copy
   dirobj->resetIterateFile();
   while (dirobj->nextFile(&curFileInfo)) {
      ULARGE_INTEGER dttarg, dtsrc;
      docopy = false;
      if (targ.findFile(curFileInfo->mName.c_str(), &fi)) {
         dttarg.HighPart = fi->mFileTime.dwHighDateTime;
         dttarg.LowPart = fi->mFileTime.dwLowDateTime;
         dtsrc.HighPart = curFileInfo->mFileTime.dwHighDateTime;
         dtsrc.LowPart = curFileInfo->mFileTime.dwLowDateTime;

         if (fi->mFileSizeHi != curFileInfo->mFileSizeHi ||
            fi->mFileSizeLo != curFileInfo->mFileSizeLo ||
            (dttarg.QuadPart < dtsrc.QuadPart - (10000000000)) /* if it's one second or more older, we
                                                                  back it up. Because of samba chopping
                                                                  off some of the time's precision
                                                              */
            /*CompareFileTime(&fi->mFileTime,&curFileInfo->mFileTime) != 0*/)
            docopy = true;
      }
      else {
         docopy = true;
      }
      if (docopy) {
         if (!rCopyFile(dirobj->getFullName(), targ.getFullName(), curFileInfo->mName, error))
            return false;
      }
   }

   // For each dir here, find it in target, if not found, create it, then recurse
   dirobj->resetIterateDirectory();
   while (dirobj->nextDirectory(&curDir)) {
      dirwascreated = false;
      // see if the directory exists in the target struct
      tmp = concatPath(targ.getFullName(), curDir->getName());
      if (!targ.findDirectory(curDir->getName().c_str())) {
         // if not, create the dir
         if (!rCreateDirectory(tmp, error))
            return false;
         dirwascreated = true;
      }

      if (dirwascreated && gLogOnly) {
         logMessage(LOG_BRIEF, "Log only mode, directory will not be processed");
         continue; // since we can't enter that directory
      }


      // process the subdirectory
      if (!processDirectory(curDir, tmp.c_str(), error))
         return false;
   }

   return true;

}

bool deleteDirTree(const std::string &name, std::string *error)
{
   std::string fn;
   RFileInfo *curFile;
   RDirectory *curDir;
   RDirectory targ(":::::", name);
   if (fillDirectory(&targ, false, false, error) != FILLDIRECTORY_SUCCESS)
      return false;

   // delete files in the dir
   targ.resetIterateFile();
   while (targ.nextFile(&curFile)) {
      fn = concatPath(targ.getFullName(), curFile->mName);
      if (!rDeleteFile(fn, curFile->mAttributes, error))
         return false;
   }

   // delete sub dirs
   targ.resetIterateDirectory();
   while (targ.nextDirectory(&curDir)) {
      if (!deleteDirTree(curDir->getFullName(), error))
         return false;
   }

   // delete this directory
   if (!rRemoveDirectory(name, error))
      return false;

   return true;
}

bool
rCreateDirectory(const std::string &path, std::string *error)
{
   logMessage(LOG_BRIEF, "MD ", path);
   if (gLogOnly)
      return true;
   Utf8ToUC p;
   p.set(path.c_str());
   if (!CreateDirectory(p.ucString(), NULL)) {
      std::string tmp = "Create directory ";
      tmp.append(path);
      *error = getSystemErrorMessage(tmp);
      return false;
   }
   gNumDirsCreated++;

   return true;
}

bool
rCopyFile(const std::string &srcpath, const std::string &destpath, const std::string &name,
   std::string *error)
{
   std::string src;
   std::string dest;

   logMessage(LOG_VERBOSE, "COPY ", name, " from ", srcpath, " to ", destpath);
   if (gLogOnly)
      return true;
   src = concatPath(srcpath, name);
   dest = concatPath(destpath, name);

   Utf8ToUC srcL;
   srcL.set(src.c_str());
   Utf8ToUC destL;
   destL.set(dest.c_str());

   if (!CopyFile(srcL.ucString(), destL.ucString(), false)) {
      *error = getSystemErrorMessage("Copy file");
      if (gContinueOnErrors) {
         logMessage(LOG_ERROR, "COPY ", srcpath, "\\", name, ": ", *error);
         return true;
      }
      return false;
   }
   gNumFilesCopied++;
   return true;
}

bool
rDeleteFile(const std::string &name, DWORD attributes, std::string *error)
{
   logMessage(LOG_VERBOSE, "DEL ", name);
   Utf8ToUC nameL;
   nameL.set(name.c_str());
   if ((attributes & FILE_ATTRIBUTE_READONLY) != 0) {
      // remove this attribute
      attributes &= ~FILE_ATTRIBUTE_READONLY;
      if (!gLogOnly && !gNoDelete) {
         if (!SetFileAttributes(nameL.ucString(), attributes)) {
            *error = getSystemErrorMessage("Remove read-only attribute");
            return false;
         }
      }
   }
   if (!gLogOnly && !gNoDelete) {
      // avoid programming bugs from accidentally deleting outside the destination
      // area
      if (name.compare(0, gTargetSanityCheck.length(), gTargetSanityCheck) != 0) {
         *error = "Delete from target sanity check failed!";
         return false;
      }

      if (!confirm("Delete file ", name)) {
         *error = "Aborted";
         return false;
      }

      if (!DeleteFile(nameL.ucString())) {
         *error = getSystemErrorMessage("Delete file");
         if (gContinueOnErrors) {
            logMessage(LOG_ERROR, "DEL ", name, ": ", *error);
            return true;
         }
         return false;
      }
      gNumFilesRemoved++;
   }

   return true;
}
bool
rRemoveDirectory(const std::string &name, std::string *error)
{
   DWORD attributes;
   logMessage(LOG_BRIEF, "RD ", name);
   Utf8ToUC nameL;
   nameL.set(name.c_str());

   attributes = GetFileAttributes(nameL.ucString());
   if ((attributes & FILE_ATTRIBUTE_READONLY) != 0) {
      // remove this attribute
      attributes &= ~FILE_ATTRIBUTE_READONLY;
      logMessage(LOG_VERBOSE, "CLEAR DIR R/O ATTR ", name);
      if (!gLogOnly && !gNoDelete) {
         if (!SetFileAttributes(nameL.ucString(), attributes)) {
            *error = getSystemErrorMessage("Remove read-only attribute");
            return false;
         }
      }
   }

   if (!gLogOnly && !gNoDelete) {
      if (!confirm("Remove directory ", name)) {
         *error = "Aborted";
         return false;
      }

      if (!RemoveDirectory(nameL.ucString())) {
         *error = getSystemErrorMessage("Remove directory");
         if (gContinueOnErrors) {
            logMessage(LOG_ERROR, "RD ", name, ": ", *error);
            return true;
         }
         return false;
      }
      gNumDirsRemoved++;
   }

   return true;
}


std::string
concatPath(const std::string &path, const std::string &name)
{
   std::string full;
   full = path;
   full.append("\\");
   full.append(name);
   return full;
}

std::string
getSystemErrorMessage(const std::string &fn)
{
   TCHAR szBuf[256];
   DWORD dw = GetLastError();

   FormatMessage(
      FORMAT_MESSAGE_FROM_SYSTEM,
      NULL,
      dw,
      MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
      szBuf,
      256, NULL);


   std::string ret = fn;
   ret.append(" failed with error: ");
   UCToUTF8 buf(szBuf);
   ret.append(buf.utf8String());
   return ret;
}

void
logMessage(RLogLevel logLevel, const std::string &msg,
   const std::string &msg2,
   const std::string &msg3,
   const std::string &msg4,
   const std::string &msg5,
   const std::string &msg6)
{
   FILE *fh;
   if (gLogFH)
      fh = gLogFH;
   else
      fh = stdout;
   if (logLevel <= gCurrentLogLevel) {
      fprintf(fh, "%s%s%s%s%s%s\n", msg.c_str(),
         msg2.c_str(), msg3.c_str(), msg4.c_str(), msg5.c_str(), msg6.c_str());
   }
}

bool confirm(const std::string &a, const std::string b) {
   std::string tmp;

   if (gNoConfirm)
      return true;
   tmp = a;
   tmp.append(" ");
   tmp.append(b);
   Utf8ToUC tmpL;
   tmpL.set(tmp.c_str());

   if (MessageBox(0, tmpL.ucString(), L"Confirm", MB_YESNO) == IDYES)
      return true;


   return false;
}

bool
build_config(const char *configfile, const char *src, const char *dest)
{
   FILE *fh;
   fh = fopen(configfile, "w");
   if (!fh) {
      fprintf(stderr, "Error creating file %s\n", configfile);
      return false;
   }

   fputs("#" PROGRAM_NAME " " PROGRAM_VERSION " config file\n", fh);
   fputs("# Create a new SOURCE/DEST section for each directory to backup\n", fh);
   fprintf(fh, "SOURCE %s\n", src);
   fprintf(fh, "DEST %s\n", dest);
   fputs("# Other options that can be entered for each SOURCE/DEST:\n", fh);
   fputs("# EXCLUDEDIR *.nobackup\n", fh);
   fputs("# EXCLUDEDIR dont_backup\n", fh);
   fputs("# EXCLUDEFILE *.bak\n", fh);
   fputs("# EXCLUDEFILE pagefile.sys\n", fh);
   fputs("#   Any directory/file name can be specified. Multiple EXCLUDEDIR/EXCLUDEFILE lines can be used\n", fh);
   fputs("#   File/directory names can be the whole name or a wildcard. Only a single * can be used in a\n", fh);
   fputs("#   wildcard. Eg. *abc, or abc*, but not *xyz*\n", fh);
   fputs("# Options can either be entered at the top of the file, or in an OPTIONS section\n", fh);
   fputs("OPTIONS\n", fh);
   fputs("# Logging level. Default is ERROR\n", fh);
   fputs("# VERBOSE | BRIEF | ERROR\n", fh);
   fputs("LOGLEVEL ERROR\n", fh);
   fputs("Delete confirmation. Default is OFF\n", fh);
   fputs("# ON | OFF\n", fh);
   fputs("CONFIRMDELETE OFF\n", fh);
   fputs("Prevent deletions. Default is OFF\n", fh);
   fputs("# ON | OFF\n", fh);
   fputs("NODELETE OFF\n", fh);
   fputs("# log file name. Default is printed to standard output. Uncomment to enable.\n", fh);
   fputs("# LOGFILE backup.log\n", fh);
   fputs("# Log only, don't do any file operations. Use this for testing. Default is OFF\n", fh);
   fputs("# ON | OFF\n", fh);
   fputs("LOGONLY OFF\n", fh);
   fputs("# This causes " PROGRAM_NAME " to skip directories that contain this file\n", fh);
   fputs("SKIPFILENAME " PROGRAM_NAME ".skipdir\n", fh);
   fputs("# If a file copy, file delete or directory delete operation fails, the failure will be ignored\n"
      "# if this option is set to ON. Default is OFF\n"
      "# ON | OFF\n", fh);
   fputs("IGNOREERRORS OFF\n", fh);

   fclose(fh);
   return true;
}

