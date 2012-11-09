/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef _SIMPSAMP
#define _SIMPSAMP

#include <MitkDiffusionImagingExports.h>
#include <mitkParticle.h>
#include <stdlib.h>

using namespace std;

namespace mitk
{

/**
* \brief Samples new tract from surrounding fiber segments.   */

class MitkDiffusionImaging_EXPORT SimpSamp
{

    float *P;
    int cnt;

public:
    EndPoint* objs;


    SimpSamp()
    {
        P = (float*) malloc(sizeof(float)*1000);
        objs = (EndPoint*) malloc(sizeof(EndPoint)*1000);
    }
    ~SimpSamp()
    {
        free(P);
        free(objs);
    }

    inline void clear()
    {
        cnt = 1;
        P[0] = 0;
    }

    inline void add(float p, EndPoint obj)
    {
        P[cnt] = P[cnt-1] + p;
        objs[cnt-1] = obj;
        cnt++;
    }

    inline int draw(float prob)
    {
        float r = prob*P[cnt-1];
        int j;
        int rl = 1;
        int rh = cnt-1;
        while(rh != rl)
        {
            j = rl + (rh-rl)/2;
            if (r < P[j])
            {
                rh = j;
                continue;
            }
            if (r > P[j])
            {
                rl = j+1;
                continue;
            }
            break;
        }
        return rh-1;
    }

    inline EndPoint drawObj(float prob)
    {
        return objs[draw(prob)];
    }

    inline bool isempty()
    {
        if (cnt == 1)
            return true;
        else
            return false;
    }


    float probFor(int idx)
    {
        return (P[idx+1]-P[idx])/P[cnt-1];
    }

    float probFor(EndPoint& t)
    {
        for (int i = 1; i< cnt;i++)
        {
            if (t == objs[i-1])
                return probFor(i-1);
        }
        return 0;
    }
};

}

#endif


