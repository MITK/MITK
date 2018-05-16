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

#ifndef _SPHEREINTERPOLATOR
#define _SPHEREINTERPOLATOR

#include <MitkFiberTrackingExports.h>
#include <vnl/vnl_vector_fixed.h>
#include <vector>

/**
 * \brief Lookuptable based trilinear interpolation of spherically arranged scalar values.
 */
class MITKFIBERTRACKING_EXPORT SphereInterpolator
{

public:

    int   size;     // size of LUT
    int   sN;       // (sizeofLUT-1)/2
    int   nverts;   // number of data vertices
    float beta;
    float inva;
    float b;
    bool    m_ValidState;

    std::vector< float > barycoords;
    std::vector< int >   indices;
    vnl_vector_fixed< int, 3 >     idx;
    vnl_vector_fixed< float, 3 >   interpw;

    SphereInterpolator(const std::string& lutPath);

    inline bool IsInValidState() const
    {
        return m_ValidState;
    }

    ~SphereInterpolator();

    inline void getInterpolation(const vnl_vector_fixed<float, 3>& N)
    {
        float nx = N[0];
        float ny = N[1];
        float nz = N[2];

        if (nz > 0.5)
        {
            int x = float2int(nx);
            int y = float2int(ny);
            int i = 3*6*(x+y*size);  // (:,1,x,y)
            idx[0] = indices[i];
            idx[1] = indices[i+1];
            idx[2] = indices[i+2];
            interpw[0] = barycoords[i];
            interpw[1] = barycoords[i+1];
            interpw[2] = barycoords[i+2];
            return;
        }
        if (nz < -0.5)
        {
            int x = float2int(nx);
            int y = float2int(ny);
            int i = 3*(1+6*(x+y*size));  // (:,2,x,y)
            idx[0] = indices[i];
            idx[1] = indices[i+1];
            idx[2] = indices[i+2];
            interpw[0] = barycoords[i];
            interpw[1] = barycoords[i+1];
            interpw[2] = barycoords[i+2];
            return;
        }
        if (nx > 0.5)
        {
            int z = float2int(nz);
            int y = float2int(ny);
            int i = 3*(2+6*(z+y*size));  // (:,2,x,y)
            idx[0] = indices[i];
            idx[1] = indices[i+1];
            idx[2] = indices[i+2];
            interpw[0] = barycoords[i];
            interpw[1] = barycoords[i+1];
            interpw[2] = barycoords[i+2];
            return;
        }
        if (nx < -0.5)
        {
            int z = float2int(nz);
            int y = float2int(ny);
            int i = 3*(3+6*(z+y*size));  // (:,2,x,y)
            idx[0] = indices[i];
            idx[1] = indices[i+1];
            idx[2] = indices[i+2];
            interpw[0] = barycoords[i];
            interpw[1] = barycoords[i+1];
            interpw[2] = barycoords[i+2];
            return;
        }
        if (ny > 0)
        {
            int x = float2int(nx);
            int z = float2int(nz);
            int i = 3*(4+6*(x+z*size));  // (:,1,x,y)
            idx[0] = indices[i];
            idx[1] = indices[i+1];
            idx[2] = indices[i+2];
            interpw[0] = barycoords[i];
            interpw[1] = barycoords[i+1];
            interpw[2] = barycoords[i+2];
            return;
        }
        else
        {
            int x = float2int(nx);
            int z = float2int(nz);
            int i = 3*(5+6*(x+z*size));  // (:,1,x,y)
            idx[0] = indices[i];
            idx[1] = indices[i+1];
            idx[2] = indices[i+2];
            interpw[0] = barycoords[i];
            interpw[1] = barycoords[i+1];
            interpw[2] = barycoords[i+2];
            return;
        }
    }

protected:

    bool LoadLookuptables(const std::string& lutPath);

    bool LoadLookuptables();

    bool LoadLookuptables(std::istream& BaryCoordsStream, std::istream& IndicesStream);

    inline float invrescale(float f) const
    {
        float x = (fabs(f)-b)*inva;
        if (f>0)
            return (x*x-beta);
        else
            return beta - x*x;
    }

    inline int float2int(float x) const
    {
        return int((invrescale(x)+1)*sN-0.5);
    }

};

#endif
