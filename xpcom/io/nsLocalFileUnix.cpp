/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * The contents of this file are subject to the Netscape Public
 * License Version 1.1 (the "License"); you may not use this file
 * except in compliance with the License. You may obtain a copy of
 * the License at http://www.mozilla.org/NPL/
 *
 * Software distributed under the License is distributed on an "AS
 * IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
 * implied. See the License for the specific language governing
 * rights and limitations under the License.
 *
 * The Original Code is Mozilla Communicator client code,
 * released March 31, 1998.
 *
 * The Initial Developer of the Original Code is Netscape Communications
 * Corporation.  Portions created by Netscape are
 * Copyright (C) 1998-1999 Netscape Communications Corporation. All
 * Rights Reserved.
 *
 * Contributor(s):
 *     Mike Shaver <shaver@mozilla.org>
 *     Christopher Blizzard <blizzard@mozilla.org>
 *     Jason Eager <jce2@po.cwru.edu>
 *     Stuart Parmenter <pavlov@netscape.com>
 *     Brendan Eich <brendan@mozilla.org>
 */

/*
 * Implementation of nsIFile for ``Unixy'' systems.
 */

/* we're going to need some autoconf loving, I can just tell */
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <utime.h>
#include <dirent.h>
#include <ctype.h>
#ifdef XP_BEOS
#include <Path.h>
#include <Entry.h>
#endif
#if defined(VMS)
#include <fabdef.h>
#endif

#include "nsCRT.h"
#include "nsCOMPtr.h"
#include "nsMemory.h"
#include "nsIFile.h"
#include "nsILocalFile.h"
#include "nsLocalFileUnix.h"
#include "nsIComponentManager.h"
#include "nsXPIDLString.h"
#include "prproces.h"

// we need these for statfs()

#ifdef HAVE_SYS_STATVFS_H
#if defined(__osf__) && defined(__DECCXX)
    extern "C" int statvfs(const char *, struct statvfs *);
#endif
#include <sys/statvfs.h>
#endif

#ifdef HAVE_SYS_STATFS_H
#include <sys/statfs.h>
#endif

#ifdef HAVE_STATVFS
#define STATFS statvfs
#else
#define STATFS statfs
#endif

// On some platforms file/directory name comparisons need to
// be case-blind.
#if defined(VMS)
#define FILE_STRCMP strcasecmp
#define FILE_STRNCMP strncasecmp
#else
#define FILE_STRCMP strcmp
#define FILE_STRNCMP strncmp
#endif

#define VALIDATE_STAT_CACHE()                   \
    PR_BEGIN_MACRO                              \
        if (!mHaveCachedStat) {                 \
            FillStatCache();                    \
            if (!mHaveCachedStat)               \
               return NSRESULT_FOR_ERRNO();     \
        }                                       \
    PR_END_MACRO

#define CHECK_mPath()                           \
    PR_BEGIN_MACRO                              \
        if (!(const char *)mPath)               \
            return NS_ERROR_NOT_INITIALIZED;    \
    PR_END_MACRO

/* directory enumerator */
class NS_COM
nsDirEnumeratorUnix : public nsISimpleEnumerator
{
  public:
    nsDirEnumeratorUnix();
    virtual ~nsDirEnumeratorUnix();

    // nsISupports interface
    NS_DECL_ISUPPORTS

    // nsISimpleEnumerator interface
    NS_DECL_NSISIMPLEENUMERATOR

    NS_IMETHOD Init(nsIFile *parent, PRBool ignored);

  protected:
    NS_IMETHOD GetNextEntry();

    DIR *mDir;
    struct dirent *mEntry;
    nsXPIDLCString mParentPath;
};

nsDirEnumeratorUnix::nsDirEnumeratorUnix() :
  mDir(nsnull), mEntry(nsnull)
{
    NS_INIT_REFCNT();
}

nsDirEnumeratorUnix::~nsDirEnumeratorUnix()
{
    if (mDir)
        closedir(mDir);
}

NS_IMPL_ISUPPORTS1(nsDirEnumeratorUnix, nsISimpleEnumerator)

NS_IMETHODIMP
nsDirEnumeratorUnix::Init(nsIFile *parent, PRBool resolveSymlinks /*ignored*/)
{
    nsXPIDLCString dirPath;
    if (NS_FAILED(parent->GetPath(getter_Copies(dirPath))) ||
        (const char *)dirPath == nsnull) {
        return NS_ERROR_FILE_INVALID_PATH;
    }

    if (NS_FAILED(parent->GetPath(getter_Copies(mParentPath))))
        return NS_ERROR_FAILURE;

    mDir = opendir(dirPath);
    if (!mDir)
        return NSRESULT_FOR_ERRNO();
    return GetNextEntry();
}

NS_IMETHODIMP
nsDirEnumeratorUnix::HasMoreElements(PRBool *result)
{
    *result = mDir && mEntry;
    return NS_OK;
}

NS_IMETHODIMP
nsDirEnumeratorUnix::GetNext(nsISupports **_retval)
{
    nsresult rv;
    if (!mDir || !mEntry) {
        *_retval = nsnull;
        return NS_OK;
    }

    nsCOMPtr<nsILocalFile> file = new nsLocalFile();
    if (!file)
        return NS_ERROR_OUT_OF_MEMORY;

    if (NS_FAILED(rv = file->InitWithPath(mParentPath)) ||
        NS_FAILED(rv = file->Append(mEntry->d_name))) {
        return rv;
    }
    *_retval = file;
    NS_ADDREF(*_retval);
    return GetNextEntry();
}

NS_IMETHODIMP
nsDirEnumeratorUnix::GetNextEntry()
{
    do {
        errno = 0;
        mEntry = readdir(mDir);

        // end of dir or error
        if (!mEntry)
            return NSRESULT_FOR_ERRNO();

        // keep going past "." and ".."
    } while (mEntry->d_name[0] == '.' &&
             (mEntry->d_name[1] == '\0' ||      // .\0
              (mEntry->d_name[1] == '.' &&
               mEntry->d_name[2] == '\0')));    // ..\0
    return NS_OK;
}

nsLocalFile::nsLocalFile() :
  mHaveCachedStat(PR_FALSE)
{
    NS_INIT_REFCNT();
}

nsLocalFile::~nsLocalFile()
{
}

NS_IMPL_THREADSAFE_ISUPPORTS2(nsLocalFile, nsILocalFile, nsIFile)

nsresult
nsLocalFile::nsLocalFileConstructor(nsISupports *outer, const nsIID &aIID,
                                    void **aInstancePtr)
{
    NS_ENSURE_ARG_POINTER(aInstancePtr);
    NS_ENSURE_NO_AGGREGATION(outer);

    *aInstancePtr = nsnull;

    nsCOMPtr<nsIFile> inst = new nsLocalFile();
    if (!inst)
        return NS_ERROR_OUT_OF_MEMORY;
    return inst->QueryInterface(aIID, aInstancePtr);
}

NS_IMETHODIMP
nsLocalFile::Clone(nsIFile **file)
{
    CHECK_mPath();
    NS_ENSURE_ARG(file);

    nsCOMPtr<nsILocalFile> localFile = new nsLocalFile();
    if (!localFile)
        return NS_ERROR_OUT_OF_MEMORY;

    nsresult rv = localFile->InitWithPath(mPath);
    if (NS_FAILED(rv))
        return rv;

    *file = localFile;
    NS_ADDREF(*file);
    return NS_OK;
}

NS_IMETHODIMP
nsLocalFile::InitWithPath(const char *filePath)
{
    NS_ENSURE_ARG(filePath);

    ssize_t len  = strlen(filePath);
    char   *name = (char *) nsMemory::Clone(filePath, len+1);
    while (name[len-1] == '/' && len > 1)
        name[--len] = '\0';

    mPath = name;
    nsMemory::Free(name);

    InvalidateCache();
    return NS_OK;
}

NS_IMETHODIMP
nsLocalFile::CreateAllAncestors(PRUint32 permissions)
{
    CHECK_mPath();

    // <jband> I promise to play nice
    char *buffer = NS_CONST_CAST(char *, (const char *)mPath),
         *slashp = buffer;

#ifdef DEBUG_NSIFILE
    fprintf(stderr, "nsIFile: before: %s\n", buffer);
#endif

    while ((slashp = strchr(slashp + 1, '/'))) {
        /*
         * Sequences of '/' are equivalent to a single '/'.
         */
        if (slashp[1] == '/')
            continue;

        /*
         * If the path has a trailing slash, don't make the last component here,
         * because we'll get EEXISTS in Create when we try to build the final
         * component again, and it's easier to condition the logic here than
         * there.
         */
        if (slashp[1] == '\0')
            break;

        /* Temporarily NUL-terminate here */
        *slashp = '\0';
#ifdef DEBUG_NSIFILE
        fprintf(stderr, "nsIFile: mkdir(\"%s\")\n", buffer);
#endif
        int result = mkdir(buffer, permissions);

        /* Put the / back before we (maybe) return */
        *slashp = '/';

        /*
         * We could get EEXISTS for an existing file -- not directory --
         * with the name of one of our ancestors, but that's OK: we'll get
         * ENOTDIR when we try to make the next component in the path,
         * either here on back in Create, and error out appropriately.
         */
        if (result == -1 && errno != EEXIST)
            return NSRESULT_FOR_ERRNO();
    }

#ifdef DEBUG_NSIFILE
    fprintf(stderr, "nsIFile: after: %s\n", buffer);
#endif

    return NS_OK;
}


NS_IMETHODIMP
nsLocalFile::OpenNSPRFileDesc(PRInt32 flags, PRInt32 mode, PRFileDesc **_retval)
{
    CHECK_mPath();

    *_retval = PR_Open(mPath, flags, mode);
    if (! *_retval)
        return NS_ERROR_FAILURE;

    return NS_OK;
}

NS_IMETHODIMP
nsLocalFile::OpenANSIFileDesc(const char *mode, FILE **_retval)
{
    CHECK_mPath();

    *_retval = fopen(mPath, mode);
    if (! *_retval)
        return NS_ERROR_FAILURE;

    return NS_OK;
}

static int exclusive_create(const char *path, mode_t mode)
{
    int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC | O_EXCL, mode);
    if (fd >= 0)
        close(fd);
    return fd;
}

static int exclusive_mkdir(const char *path, mode_t mode)
{
    return mkdir(path, mode);
}

NS_IMETHODIMP
nsLocalFile::Create(PRUint32 type, PRUint32 permissions)
{
    CHECK_mPath();
    if (type != NORMAL_FILE_TYPE && type != DIRECTORY_TYPE)
        return NS_ERROR_FILE_UNKNOWN_TYPE;

    int result;
    int (*exclusiveCreateFunc)(const char *, mode_t) =
        (type == NORMAL_FILE_TYPE) ? exclusive_create : exclusive_mkdir;

    result = exclusiveCreateFunc((const char *)mPath, permissions);
    if (result == -1 && errno == ENOENT) {
        /*
         * If we failed because of missing ancestor components, try to create
         * them and then retry the original creation.
         *
         * Ancestor directories get the same permissions as the file we're
         * creating, with the X bit set for each of (user,group,other) with
         * an R bit in the original permissions.  If you want to do anything
         * fancy like setgid or sticky bits, do it by hand.
         */
        int dirperm = permissions;
        if (permissions & S_IRUSR)
            dirperm |= S_IXUSR;
        if (permissions & S_IRGRP)
            dirperm |= S_IXGRP;
        if (permissions & S_IROTH)
            dirperm |= S_IXOTH;

#ifdef DEBUG_NSIFILE
        fprintf(stderr, "nsIFile: perm = %o, dirperm = %o\n", permissions,
                dirperm);
#endif

        if (NS_FAILED(CreateAllAncestors(dirperm)))
            return NS_ERROR_FAILURE;

#ifdef DEBUG_NSIFILE
        fprintf(stderr, "nsIFile: Create(\"%s\") again\n", (const char *)mPath);
#endif
        result = exclusiveCreateFunc((const char *)mPath, permissions);
    }

    return NSRESULT_FOR_RETURN(result);
}

NS_IMETHODIMP
nsLocalFile::Append(const char *fragment)
{
    CHECK_mPath();
    NS_ENSURE_ARG(fragment);

    // only one component of path can be appended
    if (strchr(fragment, '/'))
        return NS_ERROR_FILE_UNRECOGNIZED_PATH;

    return AppendRelativePath(fragment);
}

NS_IMETHODIMP
nsLocalFile::AppendRelativePath(const char *fragment)
{
    CHECK_mPath();
    NS_ENSURE_ARG(fragment);

    // No leading '/' and no ".." component is allowed in the fragment.
    if (*fragment == '/')
        return NS_ERROR_FILE_UNRECOGNIZED_PATH;

    const char *dots = strstr(fragment, "..");
    if (dots &&
        (dots == fragment || dots[-1] == '/') &&
        (dots[2] == '\0' || dots[2] == '/')) {
        // XXXbe what's the difference between UNRECOGNIZED and INVALID?
        return NS_ERROR_FILE_UNRECOGNIZED_PATH;
    }

    char *newPath = (char *)nsMemory::Alloc(strlen(mPath) +
                                            strlen(fragment) +
                                            2);
    if (!newPath)
        return NS_ERROR_OUT_OF_MEMORY;
    strcpy(newPath, mPath);
    strcat(newPath, "/");
    strcat(newPath, fragment);

    // strip trailing slashes that came from fragment
    ssize_t len = strlen(newPath);
    while (newPath[len-1] == '/' && len > 1)
        newPath[--len] = '\0';

    // nsXPIDLCString will copy.
    mPath = newPath;
    InvalidateCache();
    nsMemory::Free(newPath);
    return NS_OK;
}

NS_IMETHODIMP
nsLocalFile::Normalize()
{
    CHECK_mPath();
    char  resolved_path[PATH_MAX];
    char *resolved_path_ptr = nsnull;

#ifdef XP_BEOS
    BEntry be_e((const char *)mPath, true);
    BPath be_p;
    status_t err;
    if ((err = be_e.GetPath(&be_p)) == B_OK) {
        resolved_path_ptr = be_p.Path();
        PL_strncpyz(resolved_path, resolved_path_ptr, PATH_MAX - 1);
    }
#else
    resolved_path_ptr = realpath((const char *)mPath, resolved_path);
#endif
    // if there is an error, the return is null.
    if (!resolved_path_ptr)
        return NSRESULT_FOR_ERRNO();

    // nsXPIDLCString will copy.
    mPath = resolved_path;
    return NS_OK;
}

nsresult
nsLocalFile::GetLeafNameRaw(const char **_retval)
{
    CHECK_mPath();
    const char *leafName = strrchr((const char *)mPath, '/');
    NS_ASSERTION(leafName, "non-canonical mPath?");
    if (!leafName)
        return NS_ERROR_FILE_INVALID_PATH;
    *_retval = leafName+1;
    return NS_OK;
}

NS_IMETHODIMP
nsLocalFile::GetLeafName(char **aLeafName)
{
    NS_ENSURE_ARG_POINTER(aLeafName);
    nsresult rv;
    const char *leafName;
    if (NS_FAILED(rv = GetLeafNameRaw(&leafName)))
        return rv;

    *aLeafName = nsCRT::strdup(leafName);
    if (!*aLeafName)
        return NS_ERROR_OUT_OF_MEMORY;
    return NS_OK;
}

NS_IMETHODIMP
nsLocalFile::SetLeafName(const char *aLeafName)
{
    CHECK_mPath();
    NS_ENSURE_ARG(aLeafName);

    nsresult rv;
    char *leafName;
    if (NS_FAILED(rv = GetLeafNameRaw((const char **)&leafName)))
        return rv;

    char *newPath = (char *)nsMemory::Alloc((leafName - (const char *)mPath) +
                                            strlen(aLeafName) +
                                            1);
    if (!newPath)
        return NS_ERROR_OUT_OF_MEMORY;
    *leafName = '\0';
    strcpy(newPath, mPath);
    strcat(newPath, aLeafName);

    // nsXPIDLCString will copy.
    mPath = newPath;
    InvalidateCache();
    nsMemory::Free(newPath);
    return NS_OK;
}

NS_IMETHODIMP
nsLocalFile::GetPath(char **_retval)
{
    NS_ENSURE_ARG_POINTER(_retval);

    if (!(const char *)mPath) {
        *_retval = nsnull;
        return NS_OK;
    }

    *_retval = nsCRT::strdup((const char *)mPath);
    if (!*_retval)
        return NS_ERROR_OUT_OF_MEMORY;
    return NS_OK;
}

nsresult
nsLocalFile::GetTargetPathName(nsIFile *newParent, const char *newName,
                               char **_retval)
{
    nsresult rv;
    nsCOMPtr<nsIFile> oldParent;

    if (!newParent) {
        if (NS_FAILED(rv = GetParent(getter_AddRefs(oldParent))))
            return rv;
        newParent = oldParent.get();
    } else {
        // check to see if our target directory exists
        PRBool targetExists;
        newParent->Exists(&targetExists);

        if (!targetExists) {
            // XXX create the new directory with some permissions
            rv = newParent->Create(DIRECTORY_TYPE, 0755);
            if (NS_FAILED(rv))
                return rv;
        } else {
            // make sure that the target is actually a directory
            PRBool targetIsDirectory;
            newParent->IsDirectory(&targetIsDirectory);
            if (!targetIsDirectory)
                return NS_ERROR_FILE_DESTINATION_NOT_DIR;
        }
    }

    if (!newName && NS_FAILED(rv = GetLeafNameRaw(&newName)))
        return rv;

    nsXPIDLCString dirName;
    if (NS_FAILED(rv = newParent->GetPath(getter_Copies(dirName))))
        return rv;

    char *newPathName = (char *)nsMemory::Alloc(strlen(dirName) +
                                                strlen(newName) +
                                                2);
    if (!newPathName)
        return NS_ERROR_OUT_OF_MEMORY;

    strcpy(newPathName, dirName);
    strcat(newPathName, "/");
    strcat(newPathName, newName);

    *_retval = newPathName;
    return NS_OK;
}

NS_IMETHODIMP
nsLocalFile::CopyTo(nsIFile *newParent, const char *newName)
{
    nsresult rv;

    // check to make sure that this has been initialized properly
    CHECK_mPath();

    // check to see if we are a directory or if we are a file
    PRBool isDirectory;
    IsDirectory(&isDirectory);

    if (isDirectory) {
        // XXXbe what's the bugzilla.mozilla.org bug number for this?
        rv = NS_ERROR_NOT_IMPLEMENTED;
    } else {
        nsXPIDLCString newPathName;
        rv = GetTargetPathName(newParent, newName, getter_Copies(newPathName));
        if (NS_FAILED(rv))
            return rv;

#ifdef DEBUG_blizzard
        printf("nsLocalFile::CopyTo() %s -> %s\n", (const char *)mPath, (const char *)newPathName);
#endif

        // actually create the file.
        nsLocalFile *newFile = new nsLocalFile();
        if (!newFile) {
            return NS_ERROR_OUT_OF_MEMORY;
        }
        NS_ADDREF(newFile);

        rv = newFile->InitWithPath(newPathName);
        if (NS_FAILED(rv)) {
            NS_RELEASE(newFile);
            return rv;
        }

        // get the old permissions
        PRUint32 myPerms;
        GetPermissions(&myPerms);

        // create the new file with the same permissions
        rv = newFile->Create(NORMAL_FILE_TYPE, myPerms);
        if (NS_FAILED(rv)) {
            NS_RELEASE(newFile);
            return rv;
        }

        // open the new file.
        PRInt32     openFlags = PR_RDWR | PR_CREATE_FILE | PR_TRUNCATE;
        PRInt32     modeFlags = myPerms;
        PRFileDesc *newFD;

        rv = newFile->OpenNSPRFileDesc(openFlags, modeFlags, &newFD);
        if (NS_FAILED(rv)) {
            NS_RELEASE(newFile);
            return rv;
        }

        // open the old file, too
        openFlags = PR_RDONLY;
        PRFileDesc *oldFD;

        rv = OpenNSPRFileDesc(openFlags, modeFlags, &oldFD);
        if (NS_FAILED(rv)) {
            // make sure to clean up properly
            PR_Close(newFD);
            NS_RELEASE(newFile);
            return rv;
        }

#ifdef DEBUG_blizzard
        PRInt32 totalRead = 0;
        PRInt32 totalWritten = 0;
#endif
        char buf[BUFSIZ];
        PRInt32 bytesRead;
        
        while ((bytesRead = PR_Read(oldFD, buf, BUFSIZ)) > 0) {
#ifdef DEBUG_blizzard
            totalRead += bytesRead;
#endif

            // PR_Write promises never to do a short write
            PRInt32 bytesWritten = PR_Write(newFD, buf, bytesRead);
            if (bytesWritten < 0) {
                bytesRead = -1;
                break;
            }
            NS_ASSERTION(bytesWritten == bytesRead, "short PR_Write?");

#ifdef DEBUG_blizzard
            totalWritten += bytesWritten;
#endif
        }

#ifdef DEBUG_blizzard
        printf("read %d bytes, wrote %d bytes\n",
               totalRead, totalWritten);
#endif

        // close the files
        PR_Close(newFD);
        PR_Close(oldFD);

        // free our resources
        NS_RELEASE(newFile);

        // check for read (or write) error after cleaning up
        if (bytesRead < 0)
            return NSRESULT_FOR_ERRNO();
    }

    return rv;
}

NS_IMETHODIMP
nsLocalFile::CopyToFollowingLinks(nsIFile *newParent, const char *newName)
{
    return CopyTo(newParent, newName);
}

NS_IMETHODIMP
nsLocalFile::MoveTo(nsIFile *newParent, const char *newName)
{
    nsresult rv;

    // check to make sure that this has been initialized properly
    CHECK_mPath();

    // check to make sure that we have a new parent
    nsXPIDLCString newPathName;
    rv = GetTargetPathName(newParent, newName, getter_Copies(newPathName));
    if (NS_FAILED(rv))
        return rv;

    // try for atomic rename, falling back to copy/delete
    if (rename((const char *)mPath, (const char *)newPathName) < 0) {
        if (errno == EXDEV) {
            rv = CopyTo(newParent, newName);
            if (NS_SUCCEEDED(rv))
                rv = Delete(PR_TRUE);
        } else {
            rv = NSRESULT_FOR_ERRNO();
        }
    }
    return rv;
}

NS_IMETHODIMP
nsLocalFile::Delete(PRBool recursive)
{
    CHECK_mPath();

    VALIDATE_STAT_CACHE();
    PRBool isDir = S_ISDIR(mCachedStat.st_mode);

    /* XXX ?
     * if (!isDir && recursive)
     *     return NS_ERROR_INVALID_ARG;
     */
    InvalidateCache();

    if (isDir) {
        if (recursive) {
            nsCOMPtr<nsDirEnumeratorUnix> dir = new nsDirEnumeratorUnix();
            if (!dir)
                return NS_ERROR_OUT_OF_MEMORY;

            nsresult rv = dir->Init(this, PR_FALSE);
            if (NS_FAILED(rv))
                return rv;

            PRBool more;
            while (dir->HasMoreElements(&more), more) {
                nsCOMPtr<nsISupports> item;
                rv = dir->GetNext(getter_AddRefs(item));
                if (NS_FAILED(rv))
                    return NS_ERROR_FAILURE;

                nsCOMPtr<nsIFile> file = do_QueryInterface(item, &rv);
                if (NS_FAILED(rv))
                    return NS_ERROR_FAILURE;
                if (NS_FAILED(rv = file->Delete(recursive)))
                    return rv;
            }
        }

        if (rmdir(mPath) == -1)
            return NSRESULT_FOR_ERRNO();
    } else {
        if (unlink(mPath) == -1)
            return NSRESULT_FOR_ERRNO();
    }

    return NS_OK;
}

NS_IMETHODIMP
nsLocalFile::GetLastModificationDate(PRInt64 *aLastModTime)
{
    CHECK_mPath();
    NS_ENSURE_ARG(aLastModTime);

    PRFileInfo64 info;
    if (PR_GetFileInfo64(mPath, &info) != PR_SUCCESS)
        return NSRESULT_FOR_ERRNO();

    // PRTime is a 64 bit value
    // microseconds -> milliseconds
    PRInt64 usecPerMsec;
    LL_I2L(usecPerMsec, PR_USEC_PER_MSEC);
    LL_DIV(*aLastModTime, info.modifyTime, usecPerMsec);
    return NS_OK;
}

NS_IMETHODIMP
nsLocalFile::SetLastModificationDate(PRInt64 aLastModTime)
{
    CHECK_mPath();

    int result;
    if (aLastModTime) {
        VALIDATE_STAT_CACHE();
        struct utimbuf ut;
        ut.actime = mCachedStat.st_atime;

        // convert milliseconds to seconds since the unix epoch
        double dTime;
        LL_L2D(dTime, aLastModTime);
        ut.modtime = (time_t) (dTime / PR_MSEC_PER_SEC);
        result = utime(mPath, &ut);
    } else {
        result = utime(mPath, nsnull);
    }
    InvalidateCache();
    return NSRESULT_FOR_RETURN(result);
}

NS_IMETHODIMP
nsLocalFile::GetLastModificationDateOfLink(PRInt64 *aLastModTimeOfLink)
{
    CHECK_mPath();
    NS_ENSURE_ARG(aLastModTimeOfLink);

    struct stat sbuf;
    if (lstat(mPath, &sbuf) == -1)
        return NSRESULT_FOR_ERRNO();
    LL_I2L(*aLastModTimeOfLink, (PRInt32)sbuf.st_mtime);

    // lstat returns st_mtime in seconds
    PRInt64 msecPerSec;
    LL_I2L(msecPerSec, PR_MSEC_PER_SEC);
    LL_MUL(*aLastModTimeOfLink, *aLastModTimeOfLink, msecPerSec);

    return NS_OK;
}

/*
 * utime(2) may or may not dereference symlinks, joy.
 */
NS_IMETHODIMP
nsLocalFile::SetLastModificationDateOfLink(PRInt64 aLastModTimeOfLink)
{
    return SetLastModificationDate(aLastModTimeOfLink);
}

/*
 * only send back permissions bits: maybe we want to send back the whole
 * mode_t to permit checks against other file types?
 */
#define NORMALIZE_PERMS(mode)  ((mode)& (S_IRWXU | S_IRWXG | S_IRWXO))

NS_IMETHODIMP
nsLocalFile::GetPermissions(PRUint32 *aPermissions)
{
    NS_ENSURE_ARG(aPermissions);
    VALIDATE_STAT_CACHE();
    *aPermissions = NORMALIZE_PERMS(mCachedStat.st_mode);
    return NS_OK;
}

NS_IMETHODIMP
nsLocalFile::GetPermissionsOfLink(PRUint32 *aPermissionsOfLink)
{
    CHECK_mPath();
    NS_ENSURE_ARG(aPermissionsOfLink);

    struct stat sbuf;
    if (lstat(mPath, &sbuf) == -1)
        return NSRESULT_FOR_ERRNO();
    *aPermissionsOfLink = NORMALIZE_PERMS(sbuf.st_mode);
    return NS_OK;
}

NS_IMETHODIMP
nsLocalFile::SetPermissions(PRUint32 aPermissions)
{
    CHECK_mPath();

    InvalidateCache();
    return NSRESULT_FOR_RETURN(chmod(mPath, aPermissions));
}

NS_IMETHODIMP
nsLocalFile::SetPermissionsOfLink(PRUint32 aPermissions)
{
    return SetPermissions(aPermissions);
}

NS_IMETHODIMP
nsLocalFile::GetFileSize(PRInt64 *aFileSize)
{
    NS_ENSURE_ARG_POINTER(aFileSize);
    VALIDATE_STAT_CACHE();

#if defined(VMS)
    /* Only two record formats can report correct file content size */
    if ((mCachedStat.st_fab_rfm != FAB$C_STMLF) &&
        (mCachedStat.st_fab_rfm != FAB$C_STMCR)) {
	return NS_ERROR_FAILURE;
    }
#endif

    /* XXX autoconf for and use stat64 if available */
    if (S_ISDIR(mCachedStat.st_mode)) {
        *aFileSize = LL_ZERO;
    } else {
        LL_UI2L(*aFileSize, (PRUint32)mCachedStat.st_size);
    }
    return NS_OK;
}

NS_IMETHODIMP
nsLocalFile::SetFileSize(PRInt64 aFileSize)
{
    CHECK_mPath();

    PRInt32 size;
    LL_L2I(size, aFileSize);
    /* XXX truncate64? */
    InvalidateCache();
    if (truncate((const char *)mPath, (off_t)size) == -1)
        return NSRESULT_FOR_ERRNO();
    return NS_OK;
}

NS_IMETHODIMP
nsLocalFile::GetFileSizeOfLink(PRInt64 *aFileSize)
{
    CHECK_mPath();
    NS_ENSURE_ARG(aFileSize);

    struct stat sbuf;
    if (lstat(mPath, &sbuf) == -1)
        return NSRESULT_FOR_ERRNO();
    /* XXX autoconf for and use lstat64 if available */
    LL_UI2L(*aFileSize, (PRUint32)sbuf.st_size);
    return NS_OK;
}

NS_IMETHODIMP
nsLocalFile::GetDiskSpaceAvailable(PRInt64 *aDiskSpaceAvailable)
{
    NS_ENSURE_ARG_POINTER(aDiskSpaceAvailable);

    // These systems have the operations necessary to check disk space.

#if defined(HAVE_SYS_STATFS_H) || defined(HAVE_SYS_STATVFS_H)

    // check to make sure that mPath is properly initialized
    CHECK_mPath();

    struct STATFS fs_buf;

    // Members of the STATFS struct that you should know about:
    // f_bsize = block size on disk.
    // f_bavail = number of free blocks available to a non-superuser.
    // f_bfree = number of total free blocks in file system.

    if (STATFS(mPath, &fs_buf) < 0) {
        // The call to STATFS failed.
#ifdef DEBUG
        printf("ERROR: GetDiskSpaceAvailable: STATFS call FAILED. \n");
#endif
        return NS_ERROR_FAILURE;
    }
#ifdef DEBUG_DISK_SPACE
    printf("DiskSpaceAvailable: %d bytes\n",
       fs_buf.f_bsize * (fs_buf.f_bavail - 1));
#endif

    // The number of Bytes free = The number of free blocks available to
    // a non-superuser, minus one as a fudge factor, multiplied by the size
    // of the beforementioned blocks.

    LL_I2L(*aDiskSpaceAvailable, fs_buf.f_bsize * (fs_buf.f_bavail - 1));
    return NS_OK;

#else
    /*
    ** This platform doesn't have statfs or statvfs.
    ** I'm sure that there's a way to check for free disk space
    ** on platforms that don't have statfs
    ** (I'm SURE they have df, for example).
    **
    ** Until we figure out how to do that, lets be honest
    ** and say that this command isn't implemented
    ** properly for these platforms yet.
    */
#ifdef DEBUG
    printf("ERROR: GetDiskSpaceAvailable: Not implemented for plaforms without statfs.\n");
#endif
    return NS_ERROR_NOT_IMPLEMENTED;

#endif /* HAVE_SYS_STATFS_H or HAVE_SYS_STATVFS_H */

}

NS_IMETHODIMP
nsLocalFile::GetParent(nsIFile **aParent)
{
    CHECK_mPath();
    NS_ENSURE_ARG_POINTER(aParent);
    *aParent = nsnull;

    // <brendan, after jband> I promise to play nice
    char *buffer = NS_CONST_CAST(char *, (const char *)mPath),
         *slashp = buffer;

    // find the last significant slash in buffer
    slashp = strrchr(buffer, '/');
    NS_ASSERTION(slashp, "non-canonical mPath?");
    if (!slashp)
        return NS_ERROR_FILE_INVALID_PATH;

    // for the case where we are at '/'
    if (slashp == buffer)
        slashp++;

    // temporarily terminate buffer at the last significant slash
    *slashp = '\0';
    nsCOMPtr<nsILocalFile> localFile;
    nsresult rv = NS_NewLocalFile(buffer, PR_TRUE, getter_AddRefs(localFile));
    *slashp = '/';

    if (NS_SUCCEEDED(rv) && localFile)
        rv = localFile->QueryInterface(NS_GET_IID(nsIFile), (void**)aParent);
    return rv;
}

/*
 * The results of Exists, isWritable and isReadable are not cached.
 */

NS_IMETHODIMP
nsLocalFile::Exists(PRBool *_retval)
{
    CHECK_mPath();
    NS_ENSURE_ARG_POINTER(_retval);

    *_retval = (access(mPath, F_OK) == 0);
    return NS_OK;
}

NS_IMETHODIMP
nsLocalFile::IsWritable(PRBool *_retval)
{
    CHECK_mPath();
    NS_ENSURE_ARG_POINTER(_retval);

    *_retval = (access(mPath, W_OK) == 0);
    if (*_retval || errno == EACCES)
        return NS_OK;
    return NSRESULT_FOR_ERRNO();
}

NS_IMETHODIMP
nsLocalFile::IsReadable(PRBool *_retval)
{
    CHECK_mPath();
    NS_ENSURE_ARG_POINTER(_retval);

    *_retval = (access(mPath, R_OK) == 0);
    if (*_retval || errno == EACCES)
        return NS_OK;
    return NSRESULT_FOR_ERRNO();
}

NS_IMETHODIMP
nsLocalFile::IsExecutable(PRBool *_retval)
{
    CHECK_mPath();
    NS_ENSURE_ARG_POINTER(_retval);

    *_retval = (access(mPath, X_OK) == 0);
    if (*_retval || errno == EACCES)
        return NS_OK;
    return NSRESULT_FOR_ERRNO();
}

NS_IMETHODIMP
nsLocalFile::IsDirectory(PRBool *_retval)
{
    NS_ENSURE_ARG_POINTER(_retval);
    VALIDATE_STAT_CACHE();
    *_retval = S_ISDIR(mCachedStat.st_mode);
    return NS_OK;
}

NS_IMETHODIMP
nsLocalFile::IsFile(PRBool *_retval)
{
    NS_ENSURE_ARG_POINTER(_retval);
    VALIDATE_STAT_CACHE();
    *_retval = S_ISREG(mCachedStat.st_mode);
    return NS_OK;
}

NS_IMETHODIMP
nsLocalFile::IsHidden(PRBool *_retval)
{
    NS_ENSURE_ARG_POINTER(_retval);
    nsresult rv;
    const char *leafName;
    if (NS_FAILED(rv = GetLeafNameRaw(&leafName)))
        return rv;
    *_retval = (leafName[0] == '.');
    return NS_OK;
}

NS_IMETHODIMP
nsLocalFile::IsSymlink(PRBool *_retval)
{
    NS_ENSURE_ARG_POINTER(_retval);
    VALIDATE_STAT_CACHE();
    *_retval = S_ISLNK(mCachedStat.st_mode);
    return NS_OK;
}

NS_IMETHODIMP
nsLocalFile::IsSpecial(PRBool *_retval)
{
    NS_ENSURE_ARG_POINTER(_retval);
    VALIDATE_STAT_CACHE();
    *_retval = !S_ISLNK(mCachedStat.st_mode) &&
               !S_ISREG(mCachedStat.st_mode) &&
               !S_ISDIR(mCachedStat.st_mode);
    return NS_OK;
}

NS_IMETHODIMP
nsLocalFile::Equals(nsIFile *inFile, PRBool *_retval)
{
    NS_ENSURE_ARG(inFile);
    NS_ENSURE_ARG_POINTER(_retval);
    *_retval = PR_FALSE;

    nsresult rv;
    nsXPIDLCString myPath, inPath;

    if (NS_FAILED(rv = GetPath(getter_Copies(myPath))))
        return rv;
    if (NS_FAILED(rv = inFile->GetPath(getter_Copies(inPath))))
        return rv;

    *_retval = !FILE_STRCMP(inPath, myPath);
    return NS_OK;
}

NS_IMETHODIMP
nsLocalFile::Contains(nsIFile *inFile, PRBool recur, PRBool *_retval)
{
    CHECK_mPath();
    NS_ENSURE_ARG(inFile);
    NS_ENSURE_ARG_POINTER(_retval);

    nsXPIDLCString inPath;
    nsresult rv;

    if (NS_FAILED(rv = inFile->GetPath(getter_Copies(inPath))))
        return rv;

    *_retval = PR_FALSE;

    ssize_t len = strlen(mPath);
    if (FILE_STRNCMP(mPath, inPath, len) == 0) {
        // Now make sure that the |inFile|'s path has a separator at len,
        // which implies that it has more components after len.
        if (inPath[len] == '/')
            *_retval = PR_TRUE;
    }

    return NS_OK;
}

NS_IMETHODIMP
nsLocalFile::GetTarget(char **_retval)
{
    CHECK_mPath();
    NS_ENSURE_ARG_POINTER(_retval);

    VALIDATE_STAT_CACHE();
    if (!S_ISLNK(mCachedStat.st_mode))
        return NS_ERROR_FILE_INVALID_PATH;

    PRInt64 targetSize64;
    if (NS_FAILED(GetFileSizeOfLink(&targetSize64)))
        return NS_ERROR_FAILURE;

    PRInt32 size;
    LL_L2I(size, targetSize64);
    char *target = (char *)nsMemory::Alloc(size + 1);
    if (!target)
        return NS_ERROR_OUT_OF_MEMORY;

    if (readlink(mPath, target, (size_t)size) < 0) {
        nsMemory::Free(target);
        return NSRESULT_FOR_ERRNO();
    }
    target[size] = '\0';
    *_retval = target;
    return NS_OK;
}

NS_IMETHODIMP
nsLocalFile::Spawn(const char **args, PRUint32 count)
{
    CHECK_mPath();

    // make sure that when we allocate we have 1 greater than the
    // count since we need to null terminate the list for the argv to
    // pass into PR_CreateProcessDetached
    char **my_argv = (char **)malloc(sizeof(char *) * (count + 2) );
    if (!my_argv)
        return NS_ERROR_OUT_OF_MEMORY;

    // copy the args
    PRUint32 i;
    for (i=0; i < count; i++)
        my_argv[i+1] = (char *)args[i];

    // we need to set argv[0] to the program name.
    my_argv[0] = (char *)(const char *)mPath;

    // null terminate the array
    my_argv[count+1] = nsnull;

    PRStatus rv = PR_CreateProcessDetached(mPath, my_argv, nsnull, nsnull);

    // free up our argv
    free(my_argv);

    if (rv != PR_SUCCESS)
        return NS_ERROR_FILE_EXECUTION_FAILED;
    return NS_OK;
}

/* attribute PRBool followLinks; */
NS_IMETHODIMP
nsLocalFile::GetFollowLinks(PRBool *aFollowLinks)
{
    *aFollowLinks = PR_TRUE;
    return NS_OK;
}
NS_IMETHODIMP
nsLocalFile::SetFollowLinks(PRBool aFollowLinks)
{
    return NS_OK;
}

NS_IMETHODIMP
nsLocalFile::GetDirectoryEntries(nsISimpleEnumerator **entries)
{
    nsCOMPtr<nsDirEnumeratorUnix> dir = new nsDirEnumeratorUnix();
    if (!dir)
        return NS_ERROR_OUT_OF_MEMORY;

    nsresult rv = dir->Init(this, PR_FALSE);
    if (NS_FAILED(rv))
        return rv;

    /* QI needed?  If not, need to ADDREF. */
    return dir->QueryInterface(NS_GET_IID(nsISimpleEnumerator),
                               (void **)entries);
}

NS_IMETHODIMP nsLocalFile::GetURL(char * *aURL)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP nsLocalFile::SetURL(const char * aURL)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP
nsLocalFile::Load(PRLibrary **_retval)
{
    CHECK_mPath();
    NS_ENSURE_ARG_POINTER(_retval);

    *_retval = PR_LoadLibrary(mPath);
    if (!*_retval)
        return NS_ERROR_FAILURE;
    return NS_OK;
}

NS_IMETHODIMP
nsLocalFile::GetPersistentDescriptor(char **aPersistentDescriptor)
{
    NS_ENSURE_ARG_POINTER(aPersistentDescriptor);
    return GetPath(aPersistentDescriptor);
}

NS_IMETHODIMP
nsLocalFile::SetPersistentDescriptor(const char *aPersistentDescriptor)
{
    NS_ENSURE_ARG(aPersistentDescriptor);
    return InitWithPath(aPersistentDescriptor);
}

nsresult
NS_NewLocalFile(const char *path, PRBool followSymlinks, nsILocalFile **result)
{
    nsLocalFile *file = new nsLocalFile();
    if (!file)
        return NS_ERROR_OUT_OF_MEMORY;
    NS_ADDREF(file);

    if (path) {
        nsresult rv = file->InitWithPath(path);
        if (NS_FAILED(rv)) {
            NS_RELEASE(file);
            return rv;
        }
    }
    *result = file;
    return NS_OK;
}
