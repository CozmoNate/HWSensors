#!/bin/sh

#  BumpMajorVersion.sh
#  Versioning
#
#  Created by Kozlek on 13/07/13.
#

# Do nothing on clean
if [ "$1" == "clean" ]
then
    exit 0
fi

# 2013 (c) sekati
# https://gist.github.com/sekati/3172554
# xcode-version-bump.sh
# @desc Auto-increment the version number (only) when a project is archived for export.
# @usage
# 1. Select: your Target in Xcode
# 2. Select: Build Phases Tab
# 3. Select: Add Build Phase -> Add Run Script
# 4. Paste code below in to new "Run Script" section
# 5. Check the checkbox "Run script only when installing"
# 6. Drag the "Run Script" below "Link Binaries With Libraries"
# 7. Insure your starting version number is in SemVer format (e.g. 1.0.0)

# This splits a two-decimal version string, such as "0.45.123", allowing us to increment the third position.
VERSIONNUM=$(/usr/libexec/PlistBuddy -c "Print 'Project Version'" "${PROJECT_DIR}/version.plist")
#NEWSUBVERSION=`echo $VERSIONNUM | awk -F "." '{print $3}'`
NEWSUBVERSION=`echo $VERSIONNUM | awk -F "." '{print $1}'`
NEWSUBVERSION=$(($NEWSUBVERSION + 1))
#NEWVERSIONSTRING=`echo $VERSIONNUM | awk -F "." '{print $1 "." $2 ".'$NEWSUBVERSION'" }'`
NEWVERSIONSTRING=`echo $VERSIONNUM | awk -F "." '{print '$NEWSUBVERSION' ".1"}'`
/usr/libexec/PlistBuddy -c "Set :'Project Version' $NEWVERSIONSTRING" "${PROJECT_DIR}/version.plist"

PROJECTNAME=$(/usr/libexec/PlistBuddy -c "Print 'Project Name'" "${PROJECT_DIR}/version.plist")
cd ${PROJECT_DIR}
git tag -a -f ${NEWVERSIONSTRING} -m "$PROJECTNAME Major v$NEWVERSIONSTRING"
git push --tags