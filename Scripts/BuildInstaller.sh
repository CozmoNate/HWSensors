#!/bin/sh

#  PostBuild.sh
#  Versioning
#
#  Created by Kozlek on 13/07/13.
#

# Clean packages every time
cd ${PROJECT_DIR}
find ./ -name "*.pkg" -exec sh -c 'rm $0' '{}' \;

# Exit on clean action
if [ "$1" == "clean" ]
then
    exit 0
fi

project_name=$(/usr/libexec/PlistBuddy -c "Print 'Project Name'" "${PROJECT_DIR}/version.plist")
project_version=$(/usr/libexec/PlistBuddy -c "Print 'Project Version'" "${PROJECT_DIR}/version.plist")
last_revision=$(<"${PROJECT_DIR}/revision.txt")
full_version=${project_version}'.'${last_revision}

# Build package
cd ${PROJECT_DIR}/Binaries

./packagesbuild ${project_name}.pkgproj
#mv ${project_name}.pkg ${project_name}.${full_version}.pkg