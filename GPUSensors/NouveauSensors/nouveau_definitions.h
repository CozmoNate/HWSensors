//
//  nouveau_definitions.h
//  HWSensors
//
//  Created by Kozlek on 07.08.12.
//
//

#ifndef HWSensors_nouveau_definitions_h
#define HWSensors_nouveau_definitions_h

#include "linux_definitions.h"

#define NV_DEBUG_ENABLED 0
#define NV_TRACE_ENABLED 0
#define NV_SPAM_ENABLED 0

#define nv_prefix "GeForceSensors"

#define nv_fatal(o,f,a...) do { if (1) { IOLog ("%s (pci%d): [Fatal] " f, nv_prefix, (o)->pcidev->getBusNumber(), ##a); } } while(0)
#define nv_error(o,f,a...) do { if (1) { IOLog ("%s (pci%d): [Error] " f, nv_prefix, (o)->pcidev->getBusNumber(), ##a); } } while(0)
#define nv_warn(o,f,a...) do { if (1) { IOLog ("%s (pci%d): [Warning] " f, nv_prefix, (o)->pcidev->getBusNumber(), ##a); } } while(0)
#define nv_info(o,f,a...) do { if (1) { IOLog ("%s (pci%d): " f, nv_prefix, (o)->pcidev->getBusNumber(), ##a); } } while(0)
#define nv_debug(o,f,a...) do { if (NV_DEBUG_ENABLED) { IOLog ("%s (pci%d): [Debug] " f, nv_prefix, (o)->pcidev->getBusNumber(), ##a); } } while(0)
#define nv_trace(o,f,a...) do { if (NV_TRACE_ENABLED) { IOLog ("%s (pci%d): [Trace] " f, nv_prefix, (o)->pcidev->getBusNumber(), ##a); } } while(0)
#define nv_spam(o,f,a...) do { if (NV_SPAM_ENABLED) { IOLog ("%s (pci%d): [Spam] " f, nv_prefix, (o)->pcidev->getBusNumber(), ##a); } } while(0)

#endif
