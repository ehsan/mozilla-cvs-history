#!/bin/bash

# File: downloadpatches.sh
# Author: Ben Hearsum
# Description:
#   This file downloads patches from a webserver and generates sendchange
#   commands for each one. It keeps track of the most recent patch by
#   maintaining a hardlink to it. It is intended to be used by the
#   MozillaDownloadPatch and MozillaCustomPatch steps on the buildbot side.
#   It's output is logged in $LOGFILE

# where to retrieve the patches -- make sure this has a trailing slash
PATCHURL="http://localhost//patches/"
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
BRANCH="HEAD_TRY"
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
  user=$(cat "$PATCHDIR/$file" | sed -n '1p' | sed -e 's/.*: *//')
  patchfile=$(cat "$PATCHDIR/$file" | sed -n '3p' | sed -e 's/.*: *//')
  description=$(cat "$PATCHDIR/$file" | sed -n '4p' | sed -e 's/.*: *//')
  cat "$PATCHDIR/$file" | sed -n '5,$p' > /tmp/patcher-$$
  while read line
  do
    description="$description\n$line"
  done < /tmp/patcher-$$

  # send the change to buildbot
  $PYTHON_PATH $BUILDBOT_PATH sendchange --username $user \
    --master $MASTER_HOST --branch $BRANCH --comments "$description" \
    $patchfile &>/dev/null

  # TODO: this should only happen if this isn't the last time through the loop
  sleep $DELAY
done

rm /tmp/patcher-$$

# now create a hard link to the last file
mostrecent=$(echo $files | sed -e 's/.* //g')
rm "$PATCHDIR/$LASTFILE" 2>/dev/null
ln "$PATCHDIR/$mostrecent" "$PATCHDIR/$LASTFILE" 2>/dev/null

exit 0
