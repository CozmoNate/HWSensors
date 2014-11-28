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

authority="Developer ID Application:"

/usr/libexec/PlistBuddy -c "Add :SUFeedURL string http://hwsensors.com/appcast/appcast.xml" "./Binaries/HWMonitor.app/Contents/info.plist"

#codesign --verbose --force --sign "Developer ID Application" ./Binaries/HWMonitor.app/Contents/Frameworks/Sparkle.framework/Versions/A/Resources/Sparkle\ Updater.app
codesign --verbose --force --sign "${authority}" ./Binaries/HWMonitor.app/Contents/Frameworks/Sparkle.framework/Versions/A
codesign --verbose --force --sign "${authority}" ./Binaries/HWMonitor.app/Contents/Frameworks/Growl.framework/Versions/A
codesign --verbose --force --sign "${authority}" ./Binaries/HWMonitor.app/Contents/Library/LaunchServices/org.hwsensors.HWMonitorHelper

codesign --verbose --force --sign "${authority}" ./Binaries/HWMonitor.app

codesign --display --verbose=4 ./Binaries/HWMonitor.app