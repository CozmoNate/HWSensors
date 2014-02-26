/*
 *  SuperIOFamily.cpp
 *  HWSensors
 *
 *  Created by kozlek on 08/10/10.
 *
 */

//  The MIT License (MIT)
//
//  Copyright (c) 2012 Natan Zalkin <natan.zalkin@me.com>. All rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy of this software
//  and associated documentation files (the "Software"), to deal in the Software without restriction,
//  including without limitation the rights to use, copy, modify, merge, publish, distribute,
//  sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in all copies or
//  substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
//  NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
//  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
//  DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.


#include "FakeSMCDefinitions.h"
#include "LPCSensors.h"
#include "SuperIODevice.h"
//#include "OEMInfo.h"
#include "timer.h"

#include <IOKit/IOTimerEventSource.h>

#define ABS(x) ((x) >= 0 ? (x) : -(x))
#define SGN(x) ((x) > 0 ? 1.0 : -1.0)

// PID fan control algorithm, reference article: http://www.codeproject.com/Articles/36459/PID-process-control-a-Cruise-Control-example

#define kLPCSensorsWorkloopSamplingTimeout  1000
#define kLPCSensorsInitialTicks             60
#define kLPCSensorsKp   0.01000
#define kLPCSensorsKi   0.00015
#define kLPCSensorsKd   0.00045

#define CLIP_CONTROL(x) (x) < 0 ? 0 : (x) > 100 ? 100 : (x)

//#define kHWSensorsDebug 1

#define super FakeSMCPlugin
OSDefineMetaClassAndAbstractStructors(LPCSensors, FakeSMCPlugin)

bool LPCSensors::checkConfigurationNode(OSObject *node, const char *name)
{
    if (node) {
        if (OSString *configName = OSDynamicCast(OSString, node)) {
            if (!configName->isEqualTo(name))
                return false;
        }
        else if (OSDictionary *configDict = OSDynamicCast(OSDictionary, node)) {
            if ((configName = OSDynamicCast(OSString, configDict->getObject("name")))) {
                if (!configName->isEqualTo(name))
                    return false;
            }
            else return false;
        }
        else return false;
    }

    return true;
}

bool LPCSensors::addSensorFromConfigurationNode(OSObject *node, const char *key, const char *type, UInt8 size, UInt32 group, UInt32 index)
{
    float reference = 0, gain = 0, offset = 0;

    if (OSDictionary *dictionary = OSDynamicCast(OSDictionary, node))
        FakeSMCSensor::parseModifiers(dictionary, &reference, &gain, &offset);

    if (!this->addSensor(key, type, size, group, index, reference, gain, offset)) {
        const char *group_name;

        switch (group) {
            case kFakeSMCTemperatureSensor:
                group_name = "temperature";
                break;
            case kFakeSMCTachometerSensor:
                group_name = "tachometer";
                break;
            case kFakeSMCVoltageSensor:
                group_name = "voltage";
                break;
            case kFakeSMCFrequencySensor:
                group_name = "frequency";
                break;
            case kFakeSMCMultiplierSensor:
                group_name = "multiplier";
                break;

            default:
                group_name = "";
                break;
        }

        HWSensorsWarningLog("failed to add %s sensor for key %s", group_name, key);

        return false;
    }

    return true;
}

bool LPCSensors::addTemperatureSensors(OSDictionary *configuration)
{
    HWSensorsDebugLog("adding temperature sensors...");

    for (int i = 0; i < temperatureSensorsLimit(); i++)
    {
        char key[8];

        snprintf(key, 8, "TEMPIN%X", i);

        if (OSObject* node = configuration->getObject(key)) {
            if (!addSensor(node, kFakeSMCCategoryTemperature, kFakeSMCTemperatureSensor, i)) {
                if (gpuIndex < 0)
                    gpuIndex = takeVacantGPUIndex();

                if (gpuIndex >= 0 && checkConfigurationNode(configuration, "GPU Die")) {
                    snprintf(key, 5, KEY_FORMAT_GPU_DIODE_TEMPERATURE, gpuIndex);
                    if (!addSensorFromConfigurationNode(node, key, TYPE_SP78, TYPE_SPXX_SIZE, kFakeSMCTemperatureSensor, i)) {
                        releaseGPUIndex(gpuIndex);
                        gpuIndex = -1;
                    }
                }
            }
        }
    }

    return true;
}

bool LPCSensors::addVoltageSensors(OSDictionary *configuration)
{
    HWSensorsDebugLog("adding voltage sensors...");

    for (int i = 0; i < voltageSensorsLimit(); i++)
    {
        char key[5];

        snprintf(key, 5, "VIN%X", i);

        if (OSObject* node = configuration->getObject(key)) {
            if (!addSensor(node, kFakeSMCCategoryVoltage, kFakeSMCVoltageSensor, i)) {
                if (gpuIndex < 0)
                    gpuIndex = takeVacantGPUIndex();

                if (gpuIndex >= 0 && checkConfigurationNode(configuration, "GPU Core")) {
                    snprintf(key, 5, KEY_FORMAT_GPU_VOLTAGE, gpuIndex);
                    if (!addSensorFromConfigurationNode(node, key, TYPE_FP2E, TYPE_FPXX_SIZE, kFakeSMCVoltageSensor, i)) {
                        releaseGPUIndex(gpuIndex);
                        gpuIndex = -1;
                    }
                }
            }
        }
    }

    return true;
}

bool LPCSensors::addTachometerSensors(OSDictionary *configuration)
{
    HWSensorsDebugLog("adding tachometer sensors...");

    char key[7];
    UInt16 value = 0;

    // FAN manual control key
    addSensor(KEY_FAN_MANUAL, TYPE_UI16, TYPE_UI16_SIZE, kLPCSensorsFanManualSwitch, 0);

    FanLocationType location = LEFT_LOWER_FRONT;

    for (int i = 0; i < tachometerSensorsLimit(); i++) {
        SInt8 fanIndex;

        snprintf(key, 7, "FANIN%X", i);

        if (OSString* name = OSDynamicCast(OSString, configuration->getObject(key))){
            if (addTachometer(i, name->getLength() > 0 ? name->getCStringNoCopy() : 0, FAN_RPM, 0, location++, &fanIndex)){

                if (supportsTachometerControl() && fanIndex > -1) {

                    tachometerControls[i].number = fanIndex;
                    tachometerControls[i].target = 0;

                    // Minimum RPM and fan control sensor
                    snprintf(key, 5, KEY_FORMAT_FAN_MIN, fanIndex);
                    addSensor(key, TYPE_FPE2, TYPE_FPXX_SIZE, kLPCSensorsFanMinController, i);

                    // Maximum RPM
                    snprintf(key, 5, KEY_FORMAT_FAN_MAX, fanIndex);
                    fakeSMCPluginEncodeFloatValue(kLPCSensorsMaxRPM, TYPE_FPE2, TYPE_FPXX_SIZE, &value);
                    setKeyValue(key, TYPE_FPE2, TYPE_FPXX_SIZE, &value);

                    // Target RPM and fan control sensor
                    snprintf(key, 5, KEY_FORMAT_FAN_TARGET, fanIndex);
                    addSensor(key, TYPE_FPE2, TYPE_FPXX_SIZE, kLPCSensorsFanTargetController, i);
                }
            }
            else HWSensorsWarningLog("failed to add tachometer sensor %d", i);
        }
    }

    return true;
}

UInt8 LPCSensors::temperatureSensorsLimit()
{
    return 3;
}

UInt8 LPCSensors::voltageSensorsLimit()
{
    return 9;
}

UInt8 LPCSensors::tachometerSensorsLimit()
{
    return 5;
}

float LPCSensors::readTemperature(UInt32 index)
{
	return 0;
}

float LPCSensors::readVoltage(UInt32 index)
{
	return 0;
}

float LPCSensors::readTachometer(UInt32 index)
{
	return 0;
}

bool LPCSensors::supportsTachometerControl()
{
    return false;
}

UInt8 LPCSensors::readTachometerControl(UInt32 index)
{
    return 0;
}

void LPCSensors::writeTachometerControl(UInt32 index, UInt8 percent)
{
    //
}

void LPCSensors::disableTachometerControl(UInt32 index)
{
    //
}

bool LPCSensors::initialize()
{
    return true;
}

#pragma mark
#pragma mark Events

bool LPCSensors::willReadSensorValue(FakeSMCSensor *sensor, float *outValue)
{
    if (sensor) {
        switch (sensor->getGroup()) {
            case kFakeSMCTemperatureSensor:
                *outValue = sensor->getOffset() + readTemperature(sensor->getIndex());
                break;

            case kFakeSMCVoltageSensor: {
                float v = readVoltage(sensor->getIndex());
                *outValue = sensor->getOffset() + v + (v - sensor->getReference()) * sensor->getGain();
                break;
            }

            case kFakeSMCTachometerSensor:
                *outValue = readTachometer(sensor->getIndex());
                break;

            case kLPCSensorsFanManualSwitch:
            case kLPCSensorsFanMinController:
            case kLPCSensorsFanTargetController:
            default:
                // Just return stored key value
                return false;
        }

        return true;
    }

	return false;
}

bool LPCSensors::didWriteSensorValue(FakeSMCSensor *sensor, float value)
{
    if (sensor) {
        switch (sensor->getGroup()) {
            case kLPCSensorsFanManualSwitch: {
                bool active = false;

                for (int i = 0; i < tachometerSensorsLimit(); i++) {
                    if (0 == (((UInt16)value >> tachometerControls[sensor->getIndex()].number) & 0x1)) {
                        tachometerControlCancel(i);
                    }

                    if (tachometerControls[i].active) {
                        active = true;
                    }
                }
                
                break;
            }

            case kLPCSensorsFanMinController:
                tachometerControlInit(sensor->getIndex(), value);
                break;

            case kLPCSensorsFanTargetController: {
                UInt16 buffer;
                int manual;

                getKeyValue(KEY_FAN_MANUAL, &buffer);

                if (fakeSMCPluginDecodeIntValue(TYPE_UI16, TYPE_UI16_SIZE, &buffer, &manual)) {
                    if ((manual >> tachometerControls[sensor->getIndex()].number) & 0x1) {
                        tachometerControlInit(sensor->getIndex(), value);
                    }
                }
                break;
            }

            default:
                return false;
        }

        return true;
    }

    return false;
}

void LPCSensors::willPowerOff()
{
    //
}

void LPCSensors::hasPoweredOn()
{
    //
}

#pragma mark
#pragma mark Tachometer Controller

void LPCSensors::tachometerControlInit(UInt8 number, float target)
{
    float value = readTachometer(number);

    tachometerControls[number].error = target - value;

    HWSensorsDebugLog("fan control [%d] init with target = %d", number, (int)target);

    tachometerControls[number].target = target;
    tachometerControls[number].control = readTachometerControl(number);
    tachometerControls[number].prevError = 0;
    tachometerControls[number].integral = 0;
    tachometerControls[number].ticks = kLPCSensorsInitialTicks;
    tachometerControls[number].active = true;

    if (!timerScheduled) {
        timerEventSource->setTimeoutMS(kLPCSensorsWorkloopSamplingTimeout);
        timerScheduled = true;
        HWSensorsDebugLog("timer scheduled");
    }
}

bool LPCSensors::tachometerControlSample(UInt8 number)
{
    if (tachometerControls[number].active) {

        float value = readTachometer(number);

        tachometerControls[number].error = tachometerControls[number].target - value;
        tachometerControls[number].integral += tachometerControls[number].error *
        (kLPCSensorsWorkloopSamplingTimeout / 1000.0);
        float difference = tachometerControls[number].error - tachometerControls[number].prevError;
        double  derivative = difference / (kLPCSensorsWorkloopSamplingTimeout / 1000.0);

        tachometerControls[number].prevError = tachometerControls[number].error;

        float drive =
        (tachometerControls[number].error * kLPCSensorsKp +
         tachometerControls[number].integral * kLPCSensorsKi +
         derivative * kLPCSensorsKd);

        tachometerControls[number].control +=  drive;

        tachometerControls[number].control = CLIP_CONTROL(tachometerControls[number].control);

        HWSensorsDebugLog("fan control [%d] probe error = %d control = %d drive = %d", number, (int)tachometerControls[number].error, (int)tachometerControls[number].control, (int)drive);

        writeTachometerControl(number, tachometerControls[number].control);

        if (--tachometerControls[number].ticks <= 0) {
            HWSensorsDebugLog("fan control [%d] finished", number);
            tachometerControls[number].active = false;
            return false;
        }

        return true;
    }

    return false;
}

void LPCSensors::tachometerControlCancel(UInt8 number)
{
    disableTachometerControl(number);

    tachometerControls[number].active = false;

    HWSensorsDebugLog("fan control [%d] canceled", number);
}

IOReturn LPCSensors::woorkloopTimerEvent(void)
{
    bool active = false;

    for (int index = 0; index < tachometerSensorsLimit(); index ++) {
        if (tachometerControls[index].active && tachometerControlSample(index)) {
            active = true;
        }
    }

    if (active) {
        timerEventSource->setTimeoutMS(kLPCSensorsWorkloopSamplingTimeout);
        timerScheduled = true;
        HWSensorsDebugLog("timer scheduled");
    }
    else {
        timerScheduled = false;
    }
    
    return kIOReturnSuccess;
}

#pragma mark
#pragma mark Overridden Methods

bool LPCSensors::init(OSDictionary *properties)
{
	if (!super::init(properties))
		return false;

    address = 0;
    port = 0;
   	model = 0;

    modelName = "unknown";
    vendorName = "unknown";

    gpuIndex = -1;

	return true;
}

bool LPCSensors::start(IOService *provider)
{
	if (!super::start(provider))
        return false;

    OSNumber *number = OSDynamicCast(OSNumber, provider->getProperty(kSuperIOHWMAddress));

    if (!number || !(address = number->unsigned16BitValue())) {
        HWSensorsFatalLog("wrong address provided");
        return false;
    }

    number = OSDynamicCast(OSNumber, provider->getProperty(kSuperIOControlPort));

    if (!number || !(port = number->unsigned8BitValue())) {
        HWSensorsFatalLog("wrong port provided");
        return false;
    }

    number = OSDynamicCast(OSNumber, provider->getProperty(kSuperIOModelValue));

    if (!number || !(model = number->unsigned16BitValue())) {
        HWSensorsFatalLog("wrong model provided");
        return false;
    }

    OSString *string = OSDynamicCast(OSString, provider->getProperty(kSuperIOModelName));

    if (!string || !(modelName = string->getCStringNoCopy())) {
        HWSensorsFatalLog("wrong model name provided");
        return false;
    }

    string = OSDynamicCast(OSString, provider->getProperty(kSuperIOVendorName));

    if (!string || !(vendorName = string->getCStringNoCopy())) {
        HWSensorsFatalLog("wrong vendor name provided");
        return false;
    }

    if (!initialize())
        return false;

    OSString *modelString = OSString::withCString(modelName);

	if (OSDictionary *configuration = getConfigurationNode(modelString))
    {
        addTemperatureSensors(configuration);
        addVoltageSensors(configuration);
        addTachometerSensors(configuration);
    }
    else HWSensorsWarningLog("no platform profile provided");

    OSSafeReleaseNULL(modelString);

    // woorkloop
    if (!(workloop = getWorkLoop())) {
        HWSensorsFatalLog("Failed to obtain workloop");
        return false;
    }

    if (!(timerEventSource = IOTimerEventSource::timerEventSource(this, OSMemberFunctionCast(IOTimerEventSource::Action, this, &LPCSensors::woorkloopTimerEvent)))) {
        HWSensorsFatalLog("failed to initialize timer event source");
        return false;
    }

    if (kIOReturnSuccess != workloop->addEventSource(timerEventSource))
    {
        HWSensorsFatalLog("failed to add timer event source into workloop");
        return false;
    }

    // two power states - off and on
	static const IOPMPowerState powerStates[2] = {
        { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 1, IOPMDeviceUsable, IOPMPowerOn, IOPMPowerOn, 0, 0, 0, 0, 0, 0, 0, 0 }
    };

    // register interest in power state changes
	PMinit();
	provider->joinPMtree(this);
	registerPowerDriver(this, (IOPMPowerState *)powerStates, 2);

    registerService();

    HWSensorsInfoLog("started");

	return true;
}

IOReturn LPCSensors::setPowerState(unsigned long powerState, IOService *device)
{
    switch (powerState) {
        case 0: // Power Off
//            if (timerScheduled) {
//                timerEventSource->cancelTimeout();
//            }
            willPowerOff();
            break;

        case 1: // Power On
            hasPoweredOn();
//            if (timerScheduled) {
//                timerEventSource->setTimeoutMS(250);
//            }
            break;

        default:
            break;
    }
    
	return(IOPMAckImplied);
}

void LPCSensors::stop(IOService *provider)
{
    timerEventSource->cancelTimeout();
    workloop->removeEventSource(timerEventSource);
    
    if (gpuIndex >= 0)
        releaseGPUIndex(gpuIndex);
    
    super::stop(provider);
}