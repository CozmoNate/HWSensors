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

codesign -s "Developer ID Application" -f ./Binaries/HWMonitor.app
spctl -a -v --type execute ./Binaries/HWMonitor.app