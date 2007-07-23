#!/bin/bash

# File: processchanges.sh
# Author: Ben Hearsum
# Description:
#   This script downloads .info files from a webserver running "sendchange.cgi"
#   It then informs Buildbot that either a patch or Mercurial repository is
#   ready to be tested.
#   It requires the MozillaDownloadPatch and MozillaCustomPatch steps on the
#   buildbot side.
#   It's output is logged in $LOGFILE

# where to retrieve the patches -- make sure this has a trailing slash
PATCHURL="http://localhost/patches/"
# where the patches go
PATCHDIR="."
# this hardlink points to the latest patche
# it is relative to PATCHDIR
LASTFILE="lastpatch"
# where to log any error messages
LOGFILE="downloader.log"

PYTHON_PATH="/usr/bin/python"
BUILDBOT_PATH="/usr/bin/buildbot"
MASTER_HOST="localhost:9989"
PATCH_BRANCH="PATCH_TRY"
HG_BRANCH="HG_TRY"
# if multiple patches are being this controls the delay between each one
# this value should be more than the treeStableTimer on the Scheduler
DELAY=5


# retrieve the files, but only the new ones
# making sure to have wget ignore the funny index files it downloads
wget --no-check-certificate -r -l1 -np -nc -nd -R"index.html*" -P $PATCHDIR $PATCHURL &>/dev/null

# only look at the info files; it's not necessary to look at both
files=$(ls -1 $PATCHDIR | grep "\.info$")

# check for a $LASTFILE hardlink
if [ -e "$PATCHDIR/$LASTFILE" ]
then
  # since $LASTFILE exists, find any changes that are newer than it
  # and forget about them
  for file in $files
  do
    files=$(echo $files | sed -e "s/$file *//")
    if [ "$PATCHDIR/$file" -ef "$PATCHDIR/$LASTFILE" ]
    then
      break
    fi
  done
fi

if [[ $files == "" ]]
then
  echo "`date` - No patches, exiting..." >>$LOGFILE
  exit 1
fi

# any changes that are left need a sendchange generated, so do it!
for file in $files
do
  # extract info from the .info file
  patchfile=""
  codeBranch="" # this will be used once multiple branches are supported
  mozillaRepo=""
  tamarinRepo=""
  user=$(cat "$PATCHDIR/$file" | sed -n '1p' | sed -e 's/[^:]*: *//')
  changeType=$(cat "$PATCHDIR/$file" | sed -n '3p' | sed -e 's/[^:]*: *//')
  if [[ $changeType == "patch" ]]
  then
    patchfile=$(cat "$PATCHDIR/$file" | sed -n '4p' | sed -e 's/[^:]*: *//')
    codeBranch=$(cat "$PATCHDIR/$file" | sed -n '5p' | sed -e 's/[^:]*: *//')
    description=$(cat "$PATCHDIR/$file" | sed -n '6,$p')
    # get rid of the leading newline on the description
    description=$(echo "$description" | sed -e 's/^\n//')

    # TODO: support building different branches
    # send the change
    $PYTHON_PATH $BUILDBOT_PATH sendchange --username $user \
      --master $MASTER_HOST --branch $PATCH_BRANCH --comments "$description" \
      $patchfile &>/dev/null
  elif [[ $changeType == "hg" ]]
  then
    mozillaRepo=$(cat "$PATCHDIR/$file" | sed -n '4p' | sed -e 's/[^:]*: *//')
    tamarinRepo=$(cat "$PATCHDIR/$file" | sed -n '5p' | sed -e 's/[^:]*: *//')
    description=$(cat "$PATCHDIR/$file" | sed -n '6,$p')
    # get rid of the leading newline on the description
    description=$(echo "$description" | sed -e 's/^\n//')

    # send the change to buildbot
    $PYTHON_PATH $BUILDBOT_PATH sendchange --username $user \
      --master $MASTER_HOST --branch $HG_BRANCH --comments "$description" \
      $mozillaRepo $tamarinRepo &>/dev/null
  fi

  # TODO: this should only happen if this isn't the last time through the loop
  sleep $DELAY
done

# now create a hard link to the last file
mostrecent=$(echo $files | sed -e 's/.* //g')
rm "$PATCHDIR/$LASTFILE" 2>/dev/null
ln "$PATCHDIR/$mostrecent" "$PATCHDIR/$LASTFILE" 2>/dev/null

exit 0
