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

codesign -s "Developer ID Application" -f ./Binaries/HWMonitor.app/Contents/Frameworks/Sparkle.framework/Versions/A/Resources/Sparkle\ Updater.app
spctl -a -v --type execute ./Binaries/HWMonitor.app/Contents/Frameworks/Sparkle.framework/Versions/A/Resources/Sparkle\ Updater.app

codesign -s "Developer ID Application" -f ./Binaries/HWMonitor.app/Contents/Frameworks/Sparkle.framework
spctl -a -v --type execute ./Binaries/HWMonitor.app/Contents/Frameworks/Sparkle.framework

codesign -s "Developer ID Application" -f ./Binaries/HWMonitor.app/Contents/Frameworks/Growl.framework
spctl -a -v --type execute ./Binaries/HWMonitor.app/Contents/Frameworks/Growl.framework

codesign -s "Developer ID Application" -f ./Binaries/HWMonitor.app
spctl -a -v --type execute ./Binaries/HWMonitor.app