#!/bin/sh

#  PreBuild.sh
#  Versioning
#
#  Created by Kozlek on 13/07/13.
#

# Do nothing on clean
if [ "$1" == "clean" ]
then
    exit 0
fi

version_file="${PROJECT_DIR}/Shared/version.h"
project_name=$(/usr/libexec/PlistBuddy -c "Print 'Project Name'" "${PROJECT_DIR}/version.plist")
uppercased_name=$(echo $project_name | tr [[:lower:]] [[:upper:]])
project_version=$(/usr/libexec/PlistBuddy -c "Print 'Project Version'" "${PROJECT_DIR}/version.plist")

cd ${PROJECT_DIR}

sc_revision=$(svnversion)

# Fallback to git commits count
if [ "$sc_revision" == "exported" ]
then
    sc_revision=$(git rev-list --count HEAD)
fi

echo "" > ${version_file}
echo "#define ${uppercased_name}_REVISION ${sc_revision}" >> ${version_file}
echo "" >> ${version_file}
echo "#define ${uppercased_name}_VERSION ${project_version}.${sc_revision}" >> ${version_file}
echo "" >> ${version_file}
echo "#define ${uppercased_name}_VERSION_STRING \"${project_version}.${sc_revision}\"" >> ${version_file}
echo "" >> ${version_file}