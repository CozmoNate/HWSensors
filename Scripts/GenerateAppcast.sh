#!/bin/sh

#  SignInstaller.sh
#  Versioning
#
#  Created by Kozlek on 18/07/13.
#

find ./Binaries/ -maxdepth 1 -type f -name "*.tar.gz" -delete
find ./Binaries/ -maxdepth 1 -type f -name "*.zip" -delete
find ./Binaries/ -maxdepth 1 -type f -name "*.tar.gz.dsa" -delete

if [ "$1" == "clean" ]
then
    exit 0
fi

project_name=$(/usr/libexec/PlistBuddy -c "Print 'Project Name'" "./version.plist")
project_version=$(/usr/libexec/PlistBuddy -c "Print 'Project Version'" "./version.plist")
last_revision=$(<"./revision.txt")
full_version=${project_version}'.'${last_revision}
zip_filename=${project_name}.${full_version}.tar.gz

cp ./Binaries/HWSensors.${full_version}.pkg ./Binaries/HWMonitor.pkg
tar -zcvf ./Binaries/${zip_filename} ./Binaries/HWMonitor.pkg
rm ./Binaries/HWMonitor.pkg

dsa_signature=$(openssl dgst -sha1 -binary < ./Binaries/${zip_filename} | openssl dgst -dss1 -sign ./Appcast/dsa_priv.pem | openssl enc -base64)

echo ${dsa_signature} > ./Binaries/${zip_filename}.dsa

# appcast.xml
echo '<?xml version="1.0" encoding="utf-8"?>' > ./Appcast/appcast.xml
echo '<rss version="2.0" xmlns:sparkle="http://www.andymatuschak.org/xml-namespaces/sparkle"  xmlns:dc="http://purl.org/dc/elements/1.1/">' >> ./Appcast/appcast.xml
echo '<channel>' >> ./Appcast/appcast.xml
echo '  <title>'${project_name}' Changelog</title>' >> ./Appcast/appcast.xml
echo '  <link>http://hwsensors.com/appcast/appcast.xml</link>' >> ./Appcast/appcast.xml
echo '  <description>Most recent changes with links to updates.</description>' >> ./Appcast/appcast.xml
echo '  <language>en</language>' >> ./Appcast/appcast.xml
echo '  <item>' >> ./Appcast/appcast.xml
echo '      <sparkle:releaseNotesLink>http://hwsensors.com/appcast/rnotes.html</sparkle:releaseNotesLink>' >> ./Appcast/appcast.xml
echo '      <title>Version '${full_version}'</title>' >> ./Appcast/appcast.xml
echo '      <pubDate>'$(date +"%a, %d %b %G %T %z")'</pubDate>' >> ./Appcast/appcast.xml
echo '      <enclosure url="http://hwsensors.com/downloads/'${zip_filename}'" sparkle:version="'${last_revision}'" sparkle:shortVersionString="'${full_version}'" sparkle:dsaSignature="'${dsa_signature}'" length="'$(stat -f %z ./Binaries/${zip_filename})'" type="application/x-compress"/>' >> ./Appcast/appcast.xml
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
echo '          <table class="dots" width="100%" border="0" cellspacing="0" cellpadding="0" summary="Two column table with heading">' >> ./Appcast/rnotes.html
echo '              <tr>' >> ./Appcast/rnotes.html
echo '                  <td class="blue" colspan="2">' >> ./Appcast/rnotes.html
echo '                      <h3>New in '${project_name}' v'${full_version}'</h3>' >> ./Appcast/rnotes.html
echo '                  </td>' >> ./Appcast/rnotes.html
echo '              </tr>' >> ./Appcast/rnotes.html
echo '              <tr><td><td/><tr/>' >> ./Appcast/rnotes.html
echo '              <tr>' >> ./Appcast/rnotes.html
echo '                  <td valign="top">' >> ./Appcast/rnotes.html
echo '                      <p><b>Bug Fixed/Changes/Features</b></p>' >> ./Appcast/rnotes.html
echo '                      <ul>' >> ./Appcast/rnotes.html
echo                            ${git_log} >> ./Appcast/rnotes.html
echo '                      </ul>' >> ./Appcast/rnotes.html
echo '                  </td>' >> ./Appcast/rnotes.html
echo '              </tr>' >> ./Appcast/rnotes.html
echo '          </table>' >> ./Appcast/rnotes.html
echo '      <br/>' >> ./Appcast/rnotes.html
echo '  </body>' >> ./Appcast/rnotes.html
echo '</html>' >> ./Appcast/rnotes.html
