#!/bin/sh

#  PostBuild.sh
#  Versioning
#
#  Created by Kozlek on 13/07/13.
#

# Exit on clean action
if [ "$1" == "clean" ]
then
    find ./Binaries/ -maxdepth 1 -type f -name "*.pkg" -delete
    exit 0
fi

project_name=$(/usr/libexec/PlistBuddy -c "Print 'Project Name'" "./version.plist")
project_version=$(/usr/libexec/PlistBuddy -c "Print 'Project Version'" "./version.plist")
last_revision=$(<"./revision.txt")
full_version=${project_version}'.'${last_revision}

# Build package
cd ./Binaries

./packagesbuild ${project_name}.pkgproj
mv ${project_name}.pkg HWMonitor.unsigned.pkg