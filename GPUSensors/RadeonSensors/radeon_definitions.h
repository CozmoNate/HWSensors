//
//  radeon_definitions.h
//  HWSensors
//
//  Created by Natan Zalkin on 08.01.13.
//
//

#ifndef HWSensors_radeon_definitions_h
#define HWSensors_radeon_definitions_h

#define RADEON_DEBUG_ENABLED 0
#define RADEON_TRACE_ENABLED 0
#define RADEON_SPAM_ENABLED 0

#define radeon_prefix "RadeonSensors"

#define radeon_fatal(o,f,a...) do { if (1) { IOLog ("%s (%d): [Fatal] " f, radeon_prefix, (o)->card_index, ##a); } } while(0)
#define radeon_error(o,f,a...) do { if (1) { IOLog ("%s (%d): [Error] " f, radeon_prefix, (o)->card_index, ##a); } } while(0)
#define radeon_warn(o,f,a...) do { if (1) { IOLog ("%s (%d): [Warning] " f, radeon_prefix, (o)->card_index, ##a); } } while(0)
#define radeon_info(o,f,a...) do { if (1) { IOLog ("%s (%d): " f, radeon_prefix, (o)->card_index, ##a); } } while(0)
#define radeon_debug(o,f,a...) do { if (RADEON_DEBUG_ENABLED) { IOLog ("%s (%d): [Debug] " f, radeon_prefix, (o)->card_index, ##a); } } while(0)
#define radeon_trace(o,f,a...) do { if (RADEON_TRACE_ENABLED) { IOLog ("%s (%d): [Trace] " f, radeon_prefix, (o)->card_index, ##a); } } while(0)
#define radeon_spam(o,f,a...) do { if (RADEON_SPAM_ENABLED) { IOLog ("%s (%d): [Spam] " f, radeon_prefix, (o)->card_index, ##a); } } while(0)

#endif
