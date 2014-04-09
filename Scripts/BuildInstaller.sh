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

project_name="HWSensors"
project_version=$(git describe --tags)
last_revision=$(<"./revision.txt")
full_version=${project_version}'.'${last_revision}

# Build package
#cd ./Binaries

#./packagesbuild ${project_name}.pkgproj
#mv ${project_name}.pkg HWMonitor.unsigned.pkg

# Build packages required

# Hackintosh

pkgbuild --component Binaries/HWMonitor.app \
    --info ./Package/HWMonitor.PackageInfo \
    --ownership preserve \
    --identifier "org.hwsensors.HWMonitor" \
    --version "${full_version}" \
    --install-location "/Applications" \
    ./Package/HWMonitor.pkg

pkgbuild --component Binaries/FakeSMC.kext \
    --identifier "org.hwsensors.FakeSMC" \
    --scripts ./Package/Scripts \
    --version "${full_version}" \
    --install-location "/System/Library/Extensions" \
    ./Package/FakeSMC.pkg

pkgbuild --component Binaries/ACPISensors.kext \
    --identifier "org.hwsensors.ACPISensors" \
    --version "${full_version}" \
    --install-location "/System/Library/Extensions/FakeSMC.kext/Contents/PlugIns" \
    ./Package/ACPISensors.pkg

pkgbuild --component Binaries/CPUSensors.kext \
    --identifier "org.hwsensors.CPUSensors" \
    --version "${full_version}" \
    --install-location "/System/Library/Extensions/FakeSMC.kext/Contents/PlugIns" \
    ./Package/CPUSensors.pkg


pkgbuild --component Binaries/GPUSensors.kext \
    --identifier "org.hwsensors.GPUSensors" \
    --version "${full_version}" \
    --install-location "/System/Library/Extensions/FakeSMC.kext/Contents/PlugIns" \
    ./Package/GPUSensors.pkg

pkgbuild --component Binaries/LPCSensors.kext \
    --identifier "org.hwsensors.LPCSensors" \
    --version "${full_version}" \
    --install-location "/System/Library/Extensions/FakeSMC.kext/Contents/PlugIns" \
    ./Package/LPCSensors.pkg


# Build Distribution
destribution=$(cat ./Package/distribution.draft)

echo "$destribution" > ./Package/Distribution.xml

# Hackintosh

echo "  <pkg-ref id=\"org.hwsensors.HWMonitor\" version=\"${full_version}\" installKBytes=\""$(( $(stat -f %z ./Package/HWMonitor.pkg) / 1000 ))"\">#HWMonitor.pkg</pkg-ref>" >> ./Package/Distribution.xml
echo '  <pkg-ref id="org.hwsensors.FakeSMC" version="'${full_version}'" installKBytes="'$(( $(stat -f %z ./Package/FakeSMC.pkg) / 1000))'">#FakeSMC.pkg</pkg-ref>' >> ./Package/Distribution.xml
echo '  <pkg-ref id="org.hwsensors.ACPISensors" version="'${full_version}'" installKBytes="'$(( $(stat -f %z ./Package/ACPISensors.pkg) / 1000 ))'">#ACPISensors.pkg</pkg-ref>' >> ./Package/Distribution.xml
echo '  <pkg-ref id="org.hwsensors.CPUSensors" version="'${full_version}'" installKBytes="'$(( $(stat -f %z ./Package/CPUSensors.pkg) / 1000 ))'">#CPUSensors.pkg</pkg-ref>' >> ./Package/Distribution.xml
echo '  <pkg-ref id="org.hwsensors.GPUSensors" version="'${full_version}'" installKBytes="'$(( $(stat -f %z ./Package/GPUSensors.pkg) / 1000 ))'">#GPUSensors.pkg</pkg-ref>' >> ./Package/Distribution.xml
echo '  <pkg-ref id="org.hwsensors.LPCSensors" version="'${full_version}'" installKBytes="'$(( $(stat -f %z ./Package/LPCSensors.pkg) / 1000 ))'">#LPCSensors.pkg</pkg-ref>' >> ./Package/Distribution.xml

# Close tag
echo '</installer-gui-script>' >> ./Package/Distribution.xml

# Change title
sed -i '' 's/DISTRIBUTION_TITLE/'${project_name}' 'v${full_version}'/g' ./Package/Distribution.xml

installer_file='./Binaries/'${project_name}'.'${full_version}'.unsigned.pkg'

productbuild --distribution "./Package/Distribution.xml"  \
--package-path "./Package/" \
--resources "./Package/Resources" \
"${installer_file}"

