#!/bin/sh

#  SignInstaller.sh
#  Versioning
#
#  Created by Kozlek on 18/07/13.
#

find ./Binaries/ -maxdepth 1 -type f -name "*.dmg" -delete
find ./Binaries/ -maxdepth 1 -type f -name "*.tar.gz" -delete
find ./Binaries/ -maxdepth 1 -type f -name "*.dsa" -delete
find ./Binaries/ -maxdepth 1 -type f -name "*.sha1" -delete

if [ "$1" == "clean" ]
then
    exit 0
fi

project_name="HWSensors"
project_version=$(git describe --tags)
last_revision=$(<"./revision.txt")
full_version=${project_version}'.'${last_revision}

# binaries
bin_filename=${project_name}.${full_version}.Binaries.dmg
bin_volumename=${project_name}" Binaries v"${full_version}
hdiutil create -megabytes 10 -fs HFS+ -volname "${bin_volumename}" ./Binaries/TEMP.${bin_filename}
hdiutil attach ./Binaries/TEMP.${bin_filename} -readwrite -mount required

find ./Binaries/ -maxdepth 1 -type d -name "*.app" -exec cp -R {} /Volumes/"${bin_volumename}" \;
find ./Binaries/ -maxdepth 1 -type d -name "*.kext" -exec cp -R {} /Volumes/"${bin_volumename}" \;

hdiutil detach "/Volumes/${bin_volumename}"
hdiutil convert ./Binaries/TEMP.${bin_filename} -format UDZO -imagekey zlib-level=9 -o ./Binaries/${bin_filename}
rm ./Binaries/TEMP.${bin_filename}


