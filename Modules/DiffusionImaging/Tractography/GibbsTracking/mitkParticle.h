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

#ifndef _PARTICLE
#define _PARTICLE

#include <MitkDiffusionImagingExports.h>
#include <vnl/vnl_vector_fixed.h>

namespace mitk
{

class MitkDiffusionImaging_EXPORT Particle
{
public:

    Particle()
    {
        label = 0;
        pID = -1;
        mID = -1;
        inserted = false;
    }

    ~Particle()
    {
    }

    vnl_vector_fixed<float, 3> R;
    vnl_vector_fixed<float, 3> N;
    float cap;
    float len;

    int gridindex; // index in the grid where it is living
    int ID;
    int pID;
    int mID;

    int label;
    int numerator;
    bool inserted;
};

class MitkDiffusionImaging_EXPORT EndPoint
{
public:
    EndPoint()
    {}

    EndPoint(Particle *p,int ep)
    {
        this->p = p;
        this->ep = ep;
    }
    Particle *p;
    int ep;

    inline bool operator==(EndPoint P)
    {
        return (P.p == p) && (P.ep == ep);
    }
};

}

#endif
