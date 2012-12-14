#!/bin/sh

# https://github.com/eshurakov/XcodeAutoBundleVersion
# (c) Evgeniy Shurakov

if [ -z "${PROJECT_DIR}" ]; then
	PROJECT_DIR=`pwd`
fi

if [ -z "${PREFIX}" ]; then
	PREFIX=""
fi

SVN_DIR="${PROJECT_DIR}/.svn"
GIT_DIR="${PROJECT_DIR}/.git"

if [ -d "${GIT_DIR}" ]; then
	if [ -z "${GIT_BRANCH}" ]; then
		GIT_BRANCH="master"
	fi
	
	BUILD_NUMBER=`xcrun git rev-list ${GIT_BRANCH} | wc -l | tr -d ' '`
	BUILD_HASH=`xcrun git rev-parse --short --verify ${GIT_BRANCH} | tr -d ' '`
elif [ -d "${SVN_DIR}" ]; then
	BUILD_NUMBER=`xcrun svnversion -nc "${PROJECT_DIR}" | sed -e 's/^[^:]*://;s/[A-Za-z]//' | tr -d ' '`
	BUILD_HASH="${BUILD_NUMBER}"
else 
	BUILD_NUMBER="1"
	BUILD_HASH="1"
fi

if [ -z "$1" ]; then
	if [ "${BUILD_NUMBER}" == "${BUILD_HASH}" ]; then
		echo "${BUILD_NUMBER}"
	else
		echo "${BUILD_NUMBER}/${BUILD_HASH}"
	fi
else
	echo "#define ${PREFIX}BUILD_NUMBER ${BUILD_NUMBER}" > $1
	echo "#define ${PREFIX}BUILD_HASH ${BUILD_HASH}" >> $1

	find "${PROJECT_DIR}" -iname "*.plist" -maxdepth 1 -exec touch {} \;	
fi