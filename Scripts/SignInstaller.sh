#!/bin/sh

#  SignInstaller.sh
#  Versioning
#
#  Created by Kozlek on 18/07/13.
#

# Do nothing on clean
if [ "$1" == "clean" ]
then
    exit 0
fi

project_name=$(/usr/libexec/PlistBuddy -c "Print 'Project Name'" "${PROJECT_DIR}/version.plist")
project_version=$(/usr/libexec/PlistBuddy -c "Print 'Project Version'" "${PROJECT_DIR}/version.plist")
last_revision=$(<"${PROJECT_DIR}/revision.txt")
full_version=${project_version}'.'${last_revision}

productsign --sign "Developer ID Installer" ${PROJECT_DIR}/Binaries/${project_name}.pkg ${PROJECT_DIR}/Binaries/${project_name}.${full_version}.pkg
spctl -a -v --type install ${PROJECT_DIR}/Binaries/${project_name}.${full_version}.pkg

if [ $? -eq 0 ]; then
    rm ${PROJECT_DIR}/Binaries/${project_name}.pkg
else
    exit 1
fi