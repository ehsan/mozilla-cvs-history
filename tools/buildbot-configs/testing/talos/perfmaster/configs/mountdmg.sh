#!/bin/bash

filename=$1
executablepath=$2

# answer license prompt
result=`expect hdiutil-expect.ex $filename`
# now get the volume data
#result=`hdiutil attach $filename`
disk=`echo $result | sed 's@.*\(/dev/[^ ]*\).*/dev.*/dev.*@\1@'`
# remove the carriage return inserted by expect
volume=`echo $result | sed "s|[^a-zA-Z0-9/]||g" | sed 's@.*\(/Volumes/.*\)@\1@'`
echo "disk=$disk"
echo "volume=$volume"
if [[ -z "$disk" || -z "$volume" ]]; then
    echo "mounting disk image: $result"
fi

for app in $volume/*.app; do
    cp -R $app $executablepath
done

hdiutil detach $disk
