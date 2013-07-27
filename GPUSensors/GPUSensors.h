//
//  GPUSensors.h
//  HWSensors
//
//  Created by Kozlek on 27/07/13.
//
//

#ifndef __HWSensors__GPUSensors__
#define __HWSensors__GPUSensors__

#include "FakeSMCPlugin.h"

class GPUSensors : public FakeSMCPlugin {
	OSDeclareAbstractStructors(GPUSensors)
	
private:
    
    
protected:

    
public:
	virtual bool start(IOService *provider);
    
};

#endif /* defined(__HWSensors__GPUSensors__) */
