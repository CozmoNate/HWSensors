#!/bin/sh

#  SignApp.sh
#  Versioning
#
#  Created by Kozlek on 18/07/13.
#

# Do nothing on clean
if [ "$1" == "clean" ]
then
    exit 0
fi

/usr/libexec/PlistBuddy -c "Add :SUFeedURL string http://hwsensors.com/appcast/appcast.xml" "./Binaries/HWMonitor.app/Contents/info.plist"
/usr/libexec/PlistBuddy -c "Add :SUPublicDSAKeyFile string dsa_pub.pem" "./Binaries/HWMonitor.app/Contents/info.plist"

# This for testing updates
#/usr/libexec/PlistBuddy -c "Set :CFBundleVersion 839" "./Binaries/HWMonitor.app/Contents/info.plist"
#/usr/libexec/PlistBuddy -c "Set :CFBundleShortVersionString 5.3.839" "./Binaries/HWMonitor.app/Contents/info.plist"

codesign -s "Developer ID Application" -f ./Binaries/HWMonitor.app
spctl -a -v --type execute ./Binaries/HWMonitor.app