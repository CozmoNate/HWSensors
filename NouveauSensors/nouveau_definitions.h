//
//  nouveau_definitions.h
//  HWSensors
//
//  Created by Kozlek on 07.08.12.
//
//

#ifndef HWSensors_nouveau_definitions_h
#define HWSensors_nouveau_definitions_h

#include <IOKit/IOLib.h>

#define NV_DEBUG_ENABLED FALSE
#define NV_TRACE_ENABLED FALSE
#define NV_SPAM_ENABLED FALSE

#define nv_prefix "NouveauSensors"

#define nv_fatal(o,f,a...) do { if (1) { IOLog ("%s: [card%d] [Fatal] " f, nv_prefix, (o)->card_index, ##a); } } while(0)
#define nv_error(o,f,a...) do { if (1) { IOLog ("%s: [card%d] [Error] " f, nv_prefix, (o)->card_index, ##a); } } while(0)
#define nv_warn(o,f,a...) do { if (1) { IOLog ("%s: [card%d] [Warning] " f, nv_prefix, (o)->card_index, ##a); } } while(0)
#define nv_info(o,f,a...) do { if (1) { IOLog ("%s: [card%d] " f, nv_prefix, o->card_index, ##a); } } while(0)
#define nv_debug(o,f,a...) do { if (NV_DEBUG_ENABLED) { IOLog ("%s: [card%d] [Debug] " f, nv_prefix, (o)->card_index, ##a); } } while(0)
#define nv_trace(o,f,a...) do { if (NV_TRACE_ENABLED) { IOLog ("%s: [card%d] [Trace] " f, nv_prefix, (o)->card_index, ##a); } } while(0)
#define nv_spam(o,f,a...) do { if (NV_SPAM_ENABLED) { IOLog ("%s: [card%d] [Spam] " f, nv_prefix, (o)->card_index, ##a); } } while(0)

typedef UInt8 u8;
typedef UInt16 u16;
typedef UInt32 u32;
typedef UInt64 u64;
typedef SInt8 s8;
typedef SInt16 s16;
typedef SInt32 s32;
typedef SInt64 s64;

#endif
