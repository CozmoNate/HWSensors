#!/bin/sh

#  SignInstaller.sh
#  Versioning
#
#  Created by Kozlek on 18/07/13.
#

project_name=$(/usr/libexec/PlistBuddy -c "Print 'Project Name'" "./version.plist")
project_version=$(/usr/libexec/PlistBuddy -c "Print 'Project Version'" "./version.plist")
last_revision=$(<"./revision.txt")
full_version=${project_version}'.'${last_revision}
pkg_filename=${project_name}.${full_version}.pkg
zip_filename=${pkg_filename}.zip

if [ "$1" == "clean" ]
then
    rm ./Binaries/${zip_filename}
    exit 0
fi

zip -r -X ./Binaries/${pkg_filename}.zip ./Binaries/${pkg_filename}

dsa_signature=$(openssl dgst -sha1 -binary < ./Binaries/${pkg_filename}.zip | openssl dgst -dss1 -sign dsa_priv.pem | openssl enc -base64)

echo '<?xml version="1.0" encoding="utf-8"?>' > ./Appcast/appcast.xml
echo '<rss version="2.0" xmlns:sparkle="http://www.andymatuschak.org/xml-namespaces/sparkle"  xmlns:dc="http://purl.org/dc/elements/1.1/">' >> ./Appcast/appcast.xml
echo '<channel>' >> ./Appcast/appcast.xml
echo '<title>'$(project_name)' Changelog</title>' >> ./Appcast/appcast.xml
echo '<link>https://raw.github.com/kozlek/HWSensors/develop/Appcast/appcast.xml</link>' >> ./Appcast/appcast.xml
echo '<description>Most recent changes with links to updates.</description>' >> ./Appcast/appcast.xml
echo '<sparkle:releaseNotesLink>https://github.com/kozlek/HWSensors/releases</sparkle:releaseNotesLink>' >> ./Appcast/appcast.xml
echo '<language>en</language>' >> ./Appcast/appcast.xml
echo '<item>' >> ./Appcast/appcast.xml
echo '<title>Version '${full_version}'</title>' >> ./Appcast/appcast.xml
echo '<pubDate>'$(date +"%a, %d %b %G %T %z")'</pubDate>' >> ./Appcast/appcast.xml
echo '<enclosure url="http://sourceforge.net/projects/hwsensors/files/'./Binaries/${zip_filename}'/download" sparkle:version="'${last_revision}'" sparkle:shortVersionString="'${full_version}'" sparkle:dsaSignature="'${dsa_signature}'" length="'$(stat -f %z ./Binaries/${pkg_filename}.zip)'" type="application/x-compress"/>' >> ./Appcast/appcast.xml
echo '</item>' >> ./Appcast/appcast.xml
echo '</channel>' >> ./Appcast/appcast.xml
echo '</rss>' >> ./Appcast/appcast.xml
