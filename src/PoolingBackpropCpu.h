// Copyright Hugh Perkins 2014 hughperkins at gmail
//
// This Source Code Form is subject to the terms of the Mozilla Public License, 
// v. 2.0. If a copy of the MPL was not distributed with this file, You can 
// obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "PoolingBackprop.h"

#define VIRTUAL virtual
#define STATIC static

class PoolingBackpropCpu : public PoolingBackprop {
public:

    // [[[cog
    // import cog_addheaders
    // cog_addheaders.add()
    // ]]]
    // generated, using cog:
    PoolingBackpropCpu( OpenCLHelper *cl, bool padZeros, int numPlanes, int inputImageSize, int poolingSize );
    VIRTUAL void backpropErrors( int batchSize,  float *errors, int *selectors, float *errorsForUpstream );
    VIRTUAL void backpropErrors( int batchSize, CLWrapper *errorsWrapper, CLWrapper *selectorsWrapper,
    CLWrapper *errorsForUpstreamWrapper );

    // [[[end]]]
};

