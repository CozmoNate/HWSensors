#!/bin/sh

#  PostBuild.sh
#  Versioning
#
#  Created by Kozlek on 13/07/13.
#

find ./Package/ -maxdepth 1 -type f -name "*.pkg" -delete
find ./Binaries/ -maxdepth 1 -type f -name "*.pkg" -delete
find ./Package/ -maxdepth 1 -type f -name "Distribution.xml" -delete

if [ "$1" == "clean" ]
then
    exit 0
fi

project_name=$(/usr/libexec/PlistBuddy -c "Print 'Project Name'" "./version.plist")
project_version=$(/usr/libexec/PlistBuddy -c "Print 'Project Version'" "./version.plist")
last_revision=$(<"./revision.txt")
full_version=${project_version}'.'${last_revision}

# Build package
#cd ./Binaries

#./packagesbuild ${project_name}.pkgproj
#mv ${project_name}.pkg HWMonitor.unsigned.pkg

# Build packages required


pkgbuild --root ./Binaries/HWMonitor.app \
    --identifier "Package.HWMonitor" \
    --version "${full_version}" \
    --install-location "/Applications/HWMonitor.app" \
    ./Package/HWMonitor.pkg

pkgbuild --root ./Binaries/FakeSMC.kext \
    --identifier "Package.FakeSMC" \
    --version "${full_version}" \
    --install-location "/System/Library/Extensions/FakeSMC.kext" \
    ./Package/FakeSMC.pkg

pkgbuild --root ./Binaries/ACPISensors.kext \
    --identifier "Package.ACPISensors" \
    --version "${full_version}" \
    --install-location "/System/Library/Extensions/FakeSMC.kext/Contents/PlugIns/ACPISensors.kext" \
    ./Package/ACPISensors.pkg

pkgbuild --root ./Binaries/CPUSensors.kext \
    --identifier "Package.CPUSensors" \
    --version "${full_version}" \
    --install-location "/System/Library/Extensions/FakeSMC.kext/Contents/PlugIns/CPUSensors.kext" \
    ./Package/CPUSensors.pkg


pkgbuild --root ./Binaries/GPUSensors.kext \
    --identifier "Package.GPUSensors" \
    --version "${full_version}" \
    --install-location "/System/Library/Extensions/FakeSMC.kext/Contents/PlugIns/GPUSensors.kext" \
    ./Package/GPUSensors.pkg

pkgbuild --root ./Binaries/LPCSensors.kext \
    --identifier "Package.LPCSensors" \
    --version "${full_version}" \
    --install-location "/System/Library/Extensions/FakeSMC.kext/Contents/PlugIns/LPCSensors.kext" \
    ./Package/LPCSensors.pkg


# Build Distribution
destribution=$(cat ./Package/distribution.draft)

echo "$destribution" > ./Package/Distribution.xml


echo "  <pkg-ref id=\"Package.HWMonitor\" version=\"${full_version}\" installKBytes=\""$(( $(stat -f %z ./Package/HWMonitor.pkg) / 1000 ))"\">#HWMonitor.pkg</pkg-ref>" >> ./Package/Distribution.xml
echo '  <pkg-ref id="Package.FakeSMC" version="'${full_version}'" installKBytes="'$(( $(stat -f %z ./Package/FakeSMC.pkg) / 1000))'"  onConclusion="RequireRestart">#FakeSMC.pkg</pkg-ref>' >> ./Package/Distribution.xml
echo '  <pkg-ref id="Package.ACPISensors" version="'${full_version}'" installKBytes="'$(( $(stat -f %z ./Package/ACPISensors.pkg) / 1000 ))'"  onConclusion="RequireRestart">#ACPISensors.pkg</pkg-ref>' >> ./Package/Distribution.xml
echo '  <pkg-ref id="Package.CPUSensors" version="'${full_version}'" installKBytes="'$(( $(stat -f %z ./Package/CPUSensors.pkg) / 1000 ))'"  onConclusion="RequireRestart">#CPUSensors.pkg</pkg-ref>' >> ./Package/Distribution.xml
echo '  <pkg-ref id="Package.GPUSensors" version="'${full_version}'" installKBytes="'$(( $(stat -f %z ./Package/GPUSensors.pkg) / 1000 ))'"  onConclusion="RequireRestart">#GPUSensors.pkg</pkg-ref>' >> ./Package/Distribution.xml
echo '  <pkg-ref id="Package.LPCSensors" version="'${full_version}'" installKBytes="'$(( $(stat -f %z ./Package/LPCSensors.pkg) / 1000 ))'"  onConclusion="RequireRestart">#LPCSensors.pkg</pkg-ref>' >> ./Package/Distribution.xml

# Close tag
echo '</installer-gui-script>' >> ./Package/Distribution.xml

# Change title
sed -i '' 's/DISTRIBUTION_TITLE/'${project_name}' 'v${full_version}'/g' ./Package/Distribution.xml

productbuild --distribution "./Package/Distribution.xml"  \
--package-path "./Package/" \
--resources "./Package/Resources" \
"./Binaries/${project_name}.${full_version}.unsigned.pkg"
