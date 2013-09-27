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

project_name=$(/usr/libexec/PlistBuddy -c "Print 'Project Name'" "./version.plist")
project_version=$(/usr/libexec/PlistBuddy -c "Print 'Project Version'" "./version.plist")
last_revision=$(<"./revision.txt")
full_version=${project_version}'.'${last_revision}
pkg_filename=${project_name}.${full_version}.pkg

productsign --sign "Developer ID Installer" ./Binaries/HWMonitor.unsigned.pkg ./Binaries/HWMonitor.pkg
spctl -a -v --type install ./Binaries/HWMonitor.pkg

if [ $? -eq 0 ]; then
    rm ./Binaries/HWMonitor.unsigned.pkg
else
    exit 1
fi