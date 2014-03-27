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

# compressed update
dmg_filename=${project_name}.${full_version}.Update.dmg
dmg_volumename=${project_name}" v"${full_version}
hdiutil create -megabytes 10 -fs HFS+ -volname "${dmg_volumename}" ./Binaries/TEMP.${dmg_filename}
hdiutil attach ./Binaries/TEMP.${dmg_filename} -readwrite -mount required

cp ./Binaries/${project_name}.${full_version}.pkg "/Volumes/${dmg_volumename}/HWMonitor.pkg"

hdiutil detach "/Volumes/${dmg_volumename}"
hdiutil convert ./Binaries/TEMP.${dmg_filename} -format UDZO -imagekey zlib-level=9 -o ./Binaries/${dmg_filename}
rm ./Binaries/TEMP.${dmg_filename}
openssl="/usr/bin/openssl"
dsa_signature=$(${openssl} dgst -sha1 -binary < ./Binaries/${dmg_filename} | ${openssl} dgst -dss1 -sign ./Appcast/dsa_priv.pem | ${openssl} enc -base64)
echo ${dsa_signature} > ./Binaries/${dmg_filename}.sha1

# appcast.xml
echo '<?xml version="1.0" encoding="utf-8"?>' > ./Appcast/appcast.xml
echo '<rss version="2.0" xmlns:sparkle="http://www.andymatuschak.org/xml-namespaces/sparkle"  xmlns:dc="http://purl.org/dc/elements/1.1/">' >> ./Appcast/appcast.xml
echo '<channel>' >> ./Appcast/appcast.xml
echo '  <title>'${project_name}' Changelog</title>' >> ./Appcast/appcast.xml
echo '  <link>http://hwsensors.com/appcast/appcast.xml</link>' >> ./Appcast/appcast.xml
echo '  <description>Most recent changes with links to updates.</description>' >> ./Appcast/appcast.xml
echo '  <language>en</language>' >> ./Appcast/appcast.xml
echo '  <item>' >> ./Appcast/appcast.xml
echo '      <sparkle:releaseNotesLink>http://hwsensors.com/release-notes</sparkle:releaseNotesLink>' >> ./Appcast/appcast.xml
echo '      <title>Version '${full_version}'</title>' >> ./Appcast/appcast.xml
echo '      <pubDate>'$(date +"%a, %d %b %G %T %z")'</pubDate>' >> ./Appcast/appcast.xml
echo '      <enclosure url="http://hwsensors.com/appcast/hwmonitor/'${dmg_filename}'" sparkle:version="'${last_revision}'" sparkle:shortVersionString="'${full_version}'" sparkle:dsaSignature="'${dsa_signature}'" length="'$(stat -f %z ./Binaries/${dmg_filename})'" type="application/x-compress"/>' >> ./Appcast/appcast.xml
echo '  </item>' >> ./Appcast/appcast.xml
echo '</channel>' >> ./Appcast/appcast.xml
echo '</rss>' >> ./Appcast/appcast.xml

#git_log=$(git log `git describe --tags --abbrev=0`..HEAD --oneline --pretty=format:"• (%ad by %an) %s" --date=short | tr "\n" "|")

git_log=$(git log `git describe --tags --abbrev=0`..HEAD --oneline --pretty=format:"<li>%s</li>\n" --date=short)

#rnotes.html
echo '<html>' > ./Appcast/rnotes.html
echo '  <head>' >> ./Appcast/rnotes.html
echo '      <meta http-equiv="content-type" content="text/html;charset=utf-8">' >> ./Appcast/rnotes.html
echo '      <title>'${project_name}' v'${full_version}'</title>' >> ./Appcast/rnotes.html
echo '      <meta name="robots" content="anchors">' >> ./Appcast/rnotes.html
echo '      <link href="rnotes.css" type="text/css" rel="stylesheet" media="all">' >> ./Appcast/rnotes.html
echo '  </head>' >> ./Appcast/rnotes.html
echo '  <body>' >> ./Appcast/rnotes.html
echo '      <br/>' >> ./Appcast/rnotes.html
echo '      <table class="dots" width="100%" border="0" cellspacing="0" cellpadding="0" summary="Two column table with heading">' >> ./Appcast/rnotes.html
echo '              <tr>' >> ./Appcast/rnotes.html
echo '                  <td class="blue" colspan="2">' >> ./Appcast/rnotes.html
echo '                      <h3>New in '${project_name}' v'${full_version}'</h3>' >> ./Appcast/rnotes.html
echo '                  </td>' >> ./Appcast/rnotes.html
echo '              </tr>' >> ./Appcast/rnotes.html
echo '              <tr><td><td/><tr/>' >> ./Appcast/rnotes.html
echo '              <tr><td><a href="http://hwsensors.com/downloads/'${bin_filename}'">Download compiled binaries</a><td/><tr/>' >> ./Appcast/rnotes.html
echo '              <tr>' >> ./Appcast/rnotes.html
echo '                  <td valign="top">' >> ./Appcast/rnotes.html
echo '                      <p><b>Bug Fixed/Changes/Features</b></p>' >> ./Appcast/rnotes.html
echo '                      <ul>' >> ./Appcast/rnotes.html
echo                            ${git_log} >> ./Appcast/rnotes.html
echo '                      </ul>' >> ./Appcast/rnotes.html
echo '                  </td>' >> ./Appcast/rnotes.html
echo '              </tr>' >> ./Appcast/rnotes.html
echo '      </table>' >> ./Appcast/rnotes.html
echo '      <br/>' >> ./Appcast/rnotes.html
echo '  </body>' >> ./Appcast/rnotes.html
echo '</html>' >> ./Appcast/rnotes.html
