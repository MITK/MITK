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

#include <MitkFiberTrackingExports.h>
#include <vnl/vnl_vector_fixed.h>


namespace mitk
{

/**
* \brief A particle is the basic element of the Gibbs fiber tractography method.   */

class MITKFIBERTRACKING_EXPORT Particle
{
public:

    Particle()
    {
        label = 0;
        pID = -1;
        mID = -1;
    }

    ~Particle()
    {
    }

    int gridindex;          // index in the grid where it is living
    int ID;                 // particle ID
    int pID;                // successor ID
    int mID;                // predecessor ID
    unsigned char label;    // label used in the fiber building process

    vnl_vector_fixed<float, 3>& GetPos()
    {
      return pos;
    }

    vnl_vector_fixed<float, 3>& GetDir()
    {
      return dir;
    }

private:
    // this pragma ignores the following warning:
    // warning C4251: 'mitk::Particle::pos' : class   'ATL::CStringT'   needs to have dll-interface to be used   by clients of class 'Particle'
    vnl_vector_fixed<float, 3> pos; // particle position (world coordinates. corner based voxels. not accounted for image rotation.
    vnl_vector_fixed<float, 3> dir; // normalized direction vector
};

class MITKFIBERTRACKING_EXPORT EndPoint
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
