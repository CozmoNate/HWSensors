#!/bin/sh

#  SignKexts.sh
#  Versioning & Destribution
#
#  Created by Kozlek on 26/10/13.
#

# Do nothing on clean
if [ "$1" == "clean" ]
then
    exit 0
fi

authority="Developer ID Application"

#codesign -s "${authority}" ./Binaries/FakeSMC.kext/Contents/Plugins/FakeSMCKeyStore.kext
codesign -v -f -s "${authority}" --deep ./Binaries/FakeSMC.kext
spctl -a -v --type execute ./Binaries/FakeSMC.kext

codesign -v -f -s "${authority}" ./Binaries/FakeSMCKeyStore.kext
spctl -a -v --type execute ./Binaries/FakeSMCKeyStore.kext

codesign -v -f -s "${authority}" ./Binaries/ACPISensors.kext
spctl -a -v --type execute ./Binaries/ACPISensors.kext

codesign -v -f -s "${authority}" ./Binaries/CPUSensors.kext
spctl -a -v --type execute ./Binaries/CPUSensors.kext

codesign -v -f -s "${authority}" ./Binaries/GPUSensors.kext
spctl -a -v --type execute ./Binaries/GPUSensors.kext

codesign -v -f -s "${authority}" ./Binaries/LPCSensors.kext
spctl -a -v --type execute ./Binaries/LPCSensors.kext
