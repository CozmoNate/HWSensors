#!/bin/sh

#  PreBuild.sh
#  Versioning
#
#  Created by Kozlek on 13/07/13.
#

version_file="./Shared/version.h"
revision_file="./revision.txt"

if [ "$1" == "clean" ]
then
    rm -f ${version_file}
    rm -f ${revision_file}
    exit 0
fi

project_name="HWSensors"
uppercased_name=$(echo $project_name | tr [[:lower:]] [[:upper:]])
project_version=$(git describe --tags)

if [ -f $revision_file ]
then
    last_revision=$(<$revision_file)
else
    last_revision="-"
fi

echo Last project revision: ${last_revision}

cd .

sc_revision=$(svnversion)

# Fallback to git commits count
if [ "$sc_revision" == "exported" ] || [ "$sc_revision" == "Unversioned directory" ]
then
    sc_revision=$(git rev-list --count HEAD)
fi

if [ "$last_revision" != "$sc_revision" ]
then
    echo New project revision: ${sc_revision}

    echo "${sc_revision}" > ${revision_file}

    echo "" > ${version_file}
    echo "#define ${uppercased_name}_REVISION ${sc_revision}" >> ${version_file}
    echo "" >> ${version_file}
    echo "#define ${uppercased_name}_VERSION ${project_version}.${sc_revision}" >> ${version_file}
    echo "#define ${uppercased_name}_VERSION_STRING \"${project_version}.${sc_revision}\"" >> ${version_file}
fi
