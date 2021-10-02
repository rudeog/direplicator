DIREPLICATOR - A directory replication tool.
Copyright (c) 2006-2021 Andrew Shakinovsky
Please visit https://github.com/rudeog/direplicator for latest version.


Purpose:
This program will replicate one directory into another directory. This is 
useful for providing backup for your data when you have two hard drives in
your system. It is similar to using the XCOPY /S command, but with these
important distinctions:

1. It will erase files from the destination that no longer exist in the source.

2. It will not copy files to the destination if they already exist there and 
   have the same date/time and size.

3. It has some other configuration options that can be specified in the 
   configuration file which allow you to exclude files and directories based
   upon wildcards, and/or a sentinel file placed in a directory.
   
Getting started:
The easiest way to get started is to run:
  direplicator -buildconfig <config file name> <src dir> <dest dir>
Where src dir and dest dir are the names of your source directory (the directory
you would like to backup), and destination directory (where you want your
backups to end up) respectively. Config file name is the file that will be created.
It is a text file. After running this command open the newly created config file,
and edit as needed. It will contain comments to get you started. 
You can specify multiple source and target entries in this file. In addition, you 
can exclude certain directories or files from being backed up. 
One option which you may want to employ initially in the config file is:

CONFIRMDELETE ON

This option will prompt you before deleting any files. It can be used as a safety
measure to validate your backup. It can be removed once the backup is running
smoothly. 

NODELETE ON
This will prevent deletions in the target. Normally direplicator keeps source and
target in sync by deleting files in the target that are not in the source. This
allows the target to keep all files even if they are deleted from source.

Another useful option is:

LOGONLY ON

This will log actions only without actually doing anything on the disk.
After making certain that the config file is to your liking, you must ensure that
both the source and destination directory exist. Also ENSURE THAT THE DESTINATION
DIRECTORY CONTAINS NO FILES! If any files exist in this directory, and do not exist
in the source directory, they will be ERASED. As a safety measure, the program will
not function if the destination directory is at the root level of a drive. It must
be one level deep. Eg. "d:\backup" would work, "d:\" would not. 
When you are ready to launch the backup, enter:
direplicator -run <config file name>
The backup should proceed. If you encounter any problems, check the log file. Setting
the LOGLEVEL to VERBOSE is useful for diagnosing problems.

Config file:
The config file contains lines beginning with keywords and followed by values. 
The config file is divided into sections. Each section is started by either SOURCE or
OPTIONS keyword, and continues until the next occurrence of one of these keywords, or
the end of the file. Keywords are case-insensitive. Lines beginning with # and empty lines 
are ignored. 
About wildcards: Wildcards are limited to the following:
*text - Anything ending in the word text
text* - Anything starting with the word text

SOURCE section

This specifies the source directory to backup. The word SOURCE is followed by the name of
the directory. Inside the SOURCE section, the following keywords are valid.

  DEST <directory name>
Specifies the target for the backup. The target directory must exist.

  EXCLUDEDIR <dirspec>
Specifies a single directory to exclude. This can be a directory name (not full path, just name),
or it can be a wildcard, in which case any directory name matching the wildcard will be excluded. 

  EXCLUDEFILE <filespec>
Specifies a single filename, or wildcard to exclude. Example: *.bak would prevent the backup of
"backup" files which some programs create. 

OPTIONS section

This specifies the start of the options section. The options section is global to the file, and
will apply to any SOURCEs that are backed up.

  LOGLEVEL <value>
Values can be VERBOSE, BRIEF, ERROR. Verbose will output all the details of the backup. Brief
will only output notable events, and Error will output only failures. 

  CONFIRMDELETE <value> 
Values can be ON or OFF. If On, the program will pop up a message box asking you to confirm each
file and directory deletion. 

  LOGFILE <filename>
Specifies the name of a file to use for logging info. The file will be appended to. If not specified,
output will be sent to standard out (the screen). The log file includes date and time info of each
backup.

LOGONLY <value>
Values can be ON or OFF. If On, the program will record actions to the log but will not perform any
file operations. Useful for testing your backup.


  SKIPFILENAME <filename>
This causes Direplicator to skip directories that contain the file specified. This may be a more
flexible way of selectively backing up directories. If you create a file with the name specified
in a directory, that entire directory and it's subdirectories will not be backed up at all. 

  IGNOREERRORS <value>
Values can be ON or OFF. If On, and file delete or directory delete operations fail, 
the failure will be ignored, and the backup will continue. This is useful for situations where files
are exclusively locked and cannot be backed up at that time. The backup will continue anyway. If off,
the backup will abort then and there. 


Please send comments and suggestions to the address at the top of this file.
-End


