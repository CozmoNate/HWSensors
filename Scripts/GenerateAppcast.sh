#!/bin/sh

#  SignInstaller.sh
#  Versioning
#
#  Created by Kozlek on 18/07/13.
#

if [ "$1" == "clean" ]
then
find ./Binaries/ -maxdepth 1 -type f -name "*.zip" -delete
exit 0
fi

project_name=$(/usr/libexec/PlistBuddy -c "Print 'Project Name'" "./version.plist")
project_version=$(/usr/libexec/PlistBuddy -c "Print 'Project Version'" "./version.plist")
last_revision=$(<"./revision.txt")
full_version=${project_version}'.'${last_revision}
pkg_filename=HWMonitor.pkg
zip_filename=${project_name}.${full_version}.zip

zip -r -X ./Binaries/${zip_filename} ./Binaries/${pkg_filename}

dsa_signature=$(openssl dgst -sha1 -binary < ./Binaries/${zip_filename} | openssl dgst -dss1 -sign ./dsa_priv.pem | openssl enc -base64)

# appcast.xml
echo '<?xml version="1.0" encoding="utf-8"?>' > ./appcast.xml
echo '<rss version="2.0" xmlns:sparkle="http://www.andymatuschak.org/xml-namespaces/sparkle"  xmlns:dc="http://purl.org/dc/elements/1.1/">' >> ./appcast.xml
echo '<channel>' >> ./appcast.xml
echo '  <title>'${project_name}' Changelog</title>' >> ./appcast.xml
echo '  <link>https://raw.github.com/kozlek/HWSensors/master/appcast.xml</link>' >> ./appcast.xml
echo '  <description>Most recent changes with links to updates.</description>' >> ./appcast.xml
echo '  <language>en</language>' >> ./appcast.xml
echo '  <item>' >> ./appcast.xml
echo '      <sparkle:releaseNotesLink>https://raw.github.com/kozlek/HWSensors/master/rnotes.html</sparkle:releaseNotesLink>' >> ./appcast.xml
echo '      <title>Version '${full_version}'</title>' >> ./appcast.xml
echo '      <pubDate>'$(date +"%a, %d %b %G %T %z")'</pubDate>' >> ./appcast.xml
echo '      <enclosure url="http://sourceforge.net/projects/hwsensors/files/'${zip_filename}'" sparkle:version="'${last_revision}'" sparkle:shortVersionString="'${full_version}'" sparkle:dsaSignature="'${dsa_signature}'" length="'$(stat -f %z ./Binaries/${zip_filename})'" type="application/x-compress"/>' >> ./appcast.xml
echo '  </item>' >> ./appcast.xml
echo '</channel>' >> ./appcast.xml
echo '</rss>' >> ./appcast.xml

#git_log=$(git log `git describe --tags --abbrev=0`..HEAD --oneline --pretty=format:"• (%ad by %an) %s" --date=short | tr "\n" "|")

git_log=$(git log `git describe --tags --abbrev=0`..HEAD --oneline --pretty=format:"• %s" --date=short | tr "\n" "|")

#rnotes.html
echo '<html>' > ./rnotes.html
echo '  <head>' >> ./rnotes.html
echo '      <meta http-equiv="content-type" content="text/html;charset=utf-8">' >> ./rnotes.html
echo '          <title>'${project_name}' v'${full_version}'</title>' >> ./rnotes.html
echo '      <meta name="robots" content="anchors">' >> ./rnotes.html
echo '  <style type="text/css">' >> ./rnotes.html
echo '  .blue {background-color: #e6edff;margin-top: -3px;margin-bottom: -3px;padding-top: -3px;padding-bottom: -3px}' >> ./rnotes.html
echo '  .dots {border: dotted 1px #ccc}' >> ./rnotes.html
echo '  hr {text-decoration: none;border: solid 1px #bfbfbf}' >> ./rnotes.html
echo '  td {padding: 6px}' >> ./rnotes.html
echo '  p {font-size: 9pt;font-family: "Lucida Grande", Arial, sans-serif;line-height: 12pt;text-decoration: none}' >> ./rnotes.html
echo '  h3 {font-size: 9pt;font-family: "Lucida Grande", Arial, sans-serif;font-weight: bold;margin-top: -4px;margin-bottom: -4px}' >> ./rnotes.html
echo '  </style>' >> ./rnotes.html
echo '  </head>' >> ./rnotes.html
echo '  <body>' >> ./rnotes.html
echo '      <br />' >> ./rnotes.html
echo '          <table class="dots" width="100%" border="0" cellspacing="0" cellpadding="0" summary="Two column table with heading">' >> ./rnotes.html
echo '              <tr>' >> ./rnotes.html
echo '                  <td class="blue" colspan="2">' >> ./rnotes.html
echo '                      <h3>'${project_name}' v'${full_version}' Changelog</h3>' >> ./rnotes.html
echo '                  </td>' >> ./rnotes.html
echo '              </tr>' >> ./rnotes.html
echo '              <tr>' >> ./rnotes.html
echo '                  <td valign="top">' >> ./rnotes.html
echo '                      <p>' >> ./rnotes.html
echo ${git_log//|/\<br />} >> ./rnotes.html
#echo ${git_log} >> ./rnotes.html
echo '                      </p>' >> ./rnotes.html
echo '                  </td>' >> ./rnotes.html
echo '              </tr>' >> ./rnotes.html
echo '          </table>' >> ./rnotes.html
echo '      <br>' >> ./rnotes.html
echo '  </body>' >> ./rnotes.html
echo '</html>' >> ./rnotes.html
