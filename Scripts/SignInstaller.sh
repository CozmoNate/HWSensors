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

project_name="HWSensors"
project_version=$(git describe --tags)
last_revision=$(<"./revision.txt")
full_version=${project_version}'.'${last_revision}
unsigned_pkg=${project_name}.${full_version}.unsigned.pkg
signed_pkg=${project_name}.${full_version}.pkg

productsign --sign "Developer ID Installer: " ./Binaries/${unsigned_pkg} ./Binaries/${signed_pkg}
spctl -a -v --type install ./Binaries/${signed_pkg}

if [ $? -eq 0 ]; then
    rm ./Binaries/${unsigned_pkg}
else
    exit 1
fi