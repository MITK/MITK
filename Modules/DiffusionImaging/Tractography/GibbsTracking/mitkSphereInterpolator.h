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

#include <MitkDiffusionImagingExports.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <QFile>
#include <itkOrientationDistributionFunction.h>
#include <QCoreApplication>
#include <mitkStandardFileLocations.h>

using namespace std;

class MitkDiffusionImaging_EXPORT SphereInterpolator
{

public:

    int   size;     // size of LUT
    int   sN;       // (sizeofLUT-1)/2
    int   nverts;   // number of data vertices
    float beta;
    float inva;
    float b;

    float*    barycoords;
    int*      indices;
    int*      idx;
    float*    interpw;

    SphereInterpolator()
    {
        if (!LoadLookuptables())
            return;

        size = 301;
        sN = (size-1)/2;
        nverts = QBALL_ODFSIZE;
        beta = 0.5;

        inva = (sqrt(1+beta)-sqrt(beta));
        b = 1/(1-sqrt(1/beta + 1));
    }

    ~SphereInterpolator()
    {
        delete[] barycoords;
        delete[] indices;
    }

    bool LoadLookuptables()
    {
        std::cout << "SphereInterpolator: loading lookuptables" << std::endl;
        QString applicationDir = QCoreApplication::applicationDirPath();
        applicationDir.append("/");
        mitk::StandardFileLocations::GetInstance()->AddDirectoryForSearch( applicationDir.toStdString().c_str(), false );
        applicationDir.append("../");
        mitk::StandardFileLocations::GetInstance()->AddDirectoryForSearch( applicationDir.toStdString().c_str(), false );
        applicationDir.append("../../");
        mitk::StandardFileLocations::GetInstance()->AddDirectoryForSearch( applicationDir.toStdString().c_str(), false );

        string lutPath = mitk::StandardFileLocations::GetInstance()->FindFile("FiberTrackingLUTBaryCoords.bin");
        std::ifstream BaryCoordsStream;
        BaryCoordsStream.open(lutPath.c_str(), ios::in | ios::binary);
        if (BaryCoordsStream.is_open())
        {
            float tmp;
            barycoords = new float [1630818];
            BaryCoordsStream.seekg (0, ios::beg);
            for (int i=0; i<1630818; i++)
            {
                BaryCoordsStream.read((char *)&tmp, sizeof(tmp));
                barycoords[i] = tmp;
            }
            BaryCoordsStream.close();
        }
        else
            return false;

        ifstream IndicesStream;
        lutPath = mitk::StandardFileLocations::GetInstance()->FindFile("FiberTrackingLUTIndices.bin");
        IndicesStream.open(lutPath.c_str(), ios::in | ios::binary);
        if (IndicesStream.is_open())
        {
            int tmp;
            indices = new int [1630818];
            IndicesStream.seekg (0, ios::beg);
            for (int i=0; i<1630818; i++)
            {
                IndicesStream.read((char *)&tmp, 4);
                indices[i] = tmp;
            }
            IndicesStream.close();
        }
        else
            return false;

        return true;
    }

    inline void getInterpolation(vnl_vector_fixed<float, 3> N)
    {
        float nx = N[0];
        float ny = N[1];
        float nz = N[2];

        if (nz > 0.5)
        {
            int x = float2int(nx);
            int y = float2int(ny);
            int i = 3*6*(x+y*size);  // (:,1,x,y)
            idx = indices+i;
            interpw = barycoords +i;
            return;
        }
        if (nz < -0.5)
        {
            int x = float2int(nx);
            int y = float2int(ny);
            int i = 3*(1+6*(x+y*size));  // (:,2,x,y)
            idx = indices+i;
            interpw = barycoords +i;
            return;
        }
        if (nx > 0.5)
        {
            int z = float2int(nz);
            int y = float2int(ny);
            int i = 3*(2+6*(z+y*size));  // (:,2,x,y)
            idx = indices+i;
            interpw = barycoords +i;
            return;
        }
        if (nx < -0.5)
        {
            int z = float2int(nz);
            int y = float2int(ny);
            int i = 3*(3+6*(z+y*size));  // (:,2,x,y)
            idx = indices+i;
            interpw = barycoords +i;
            return;
        }
        if (ny > 0)
        {
            int x = float2int(nx);
            int z = float2int(nz);
            int i = 3*(4+6*(x+z*size));  // (:,1,x,y)
            idx = indices+i;
            interpw = barycoords +i;
            return;
        }
        else
        {
            int x = float2int(nx);
            int z = float2int(nz);
            int i = 3*(5+6*(x+z*size));  // (:,1,x,y)
            idx = indices+i;
            interpw = barycoords +i;
            return;
        }

    }


    inline float invrescale(float f)
    {
        float x = (fabs(f)-b)*inva;
        if (f>0)
            return (x*x-beta);
        else
            return beta - x*x;
    }

    inline int float2int(float x)
    {
        return int((invrescale(x)+1)*sN-0.5);

    }


};

#endif
