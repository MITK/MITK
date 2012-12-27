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
#include <exception>

using namespace std;


/**
* \brief Lookuptable based trilinear interpolation of spherically arranged scalar values.   */

class MitkDiffusionImaging_EXPORT SphereInterpolator
{

public:

    int   size;     // size of LUT
    int   sN;       // (sizeofLUT-1)/2
    int   nverts;   // number of data vertices
    float beta;
    float inva;
    float b;
    bool    m_ValidState;

    vector< float > barycoords;
    vector< int >   indices;
    vnl_vector_fixed< int, 3 >     idx;
    vnl_vector_fixed< float, 3 >   interpw;

    SphereInterpolator(string lutPath)
    {
        m_ValidState = true;
        if (lutPath.length()==0)
        {
            if (!LoadLookuptables())
            {
                m_ValidState = false;
                return;
            }
        }
        else
        {
            if (!LoadLookuptables(lutPath))
            {
                m_ValidState = false;
                return;
            }
        }

        size = 301;
        sN = (size-1)/2;
        nverts = QBALL_ODFSIZE;
        beta = 0.5;

        inva = (sqrt(1+beta)-sqrt(beta));
        b = 1/(1-sqrt(1/beta + 1));
    }

    inline bool IsInValidState()
    {
        return m_ValidState;
    }

    ~SphereInterpolator()
    {

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

    bool LoadLookuptables(string lutPath)
    {
        MITK_INFO << "SphereInterpolator: loading lookuptables from custom path: " << lutPath;

        string path = lutPath; path.append("FiberTrackingLUTBaryCoords.bin");
        std::ifstream BaryCoordsStream;
        BaryCoordsStream.open(path.c_str(), ios::in | ios::binary);
        MITK_INFO << "SphereInterpolator: 1 " << path;
        if (BaryCoordsStream.is_open())
        {
            try
            {
                float tmp;
                BaryCoordsStream.seekg (0, ios::beg);
                MITK_INFO << "SphereInterpolator: 2";
                while (!BaryCoordsStream.eof())
                {
                    BaryCoordsStream.read((char *)&tmp, sizeof(tmp));
                    barycoords.push_back(tmp);
                }
                MITK_INFO << "SphereInterpolator: 3";
                BaryCoordsStream.close();
            }
            catch (const std::exception& e)
            {
                MITK_INFO << e.what();
            }
        }
        else
        {
            MITK_INFO << "SphereInterpolator: could not load FiberTrackingLUTBaryCoords.bin from " << path;
            return false;
        }
        MITK_INFO << "SphereInterpolator: first lut loaded successfully";

        ifstream IndicesStream;
        path = lutPath; path.append("FiberTrackingLUTIndices.bin");
        IndicesStream.open(path.c_str(), ios::in | ios::binary);
        MITK_INFO << "SphereInterpolator: 1 " << path;
        if (IndicesStream.is_open())
        {
            try
            {
                int tmp;
                IndicesStream.seekg (0, ios::beg);
                MITK_INFO << "SphereInterpolator: 2";
                while (!IndicesStream.eof())
                {
                    IndicesStream.read((char *)&tmp, sizeof(tmp));
                    indices.push_back(tmp);
                }
                MITK_INFO << "SphereInterpolator: 3";
                IndicesStream.close();
            }
            catch (const std::exception& e)
            {
                MITK_INFO << e.what();
            }
        }
        else
        {
            MITK_INFO << "SphereInterpolator: could not load FiberTrackingLUTIndices.bin from " << path;
            return false;
        }
        MITK_INFO << "SphereInterpolator: second lut loaded successfully";

        return true;
    }

    bool LoadLookuptables()
    {
        MITK_INFO << "SphereInterpolator: loading lookuptables";
        QString applicationDir = QCoreApplication::applicationDirPath();
        applicationDir.append("/");
        mitk::StandardFileLocations::GetInstance()->AddDirectoryForSearch( applicationDir.toStdString().c_str(), false );
        applicationDir.append("../");
        mitk::StandardFileLocations::GetInstance()->AddDirectoryForSearch( applicationDir.toStdString().c_str(), false );
        applicationDir.append("../../");
        mitk::StandardFileLocations::GetInstance()->AddDirectoryForSearch( applicationDir.toStdString().c_str(), false );
        applicationDir = QCoreApplication::applicationDirPath();
        applicationDir.append("/bin/");
        mitk::StandardFileLocations::GetInstance()->AddDirectoryForSearch( applicationDir.toStdString().c_str(), false );

        string lutPath = mitk::StandardFileLocations::GetInstance()->FindFile("FiberTrackingLUTBaryCoords.bin");
        std::ifstream BaryCoordsStream;
        BaryCoordsStream.open(lutPath.c_str(), ios::in | ios::binary);
        if (BaryCoordsStream.is_open())
        {
            try
            {
                float tmp;
                BaryCoordsStream.seekg (0, ios::beg);
                while (!BaryCoordsStream.eof())
                {
                    BaryCoordsStream.read((char *)&tmp, sizeof(tmp));
                    barycoords.push_back(tmp);
                }
                BaryCoordsStream.close();
            }
            catch (const std::exception& e)
            {
                MITK_INFO << e.what();
            }
        }
        else
        {
            MITK_INFO << "SphereInterpolator: could not load FiberTrackingLUTBaryCoords.bin from " << lutPath;
            return false;
        }

        ifstream IndicesStream;
        lutPath = mitk::StandardFileLocations::GetInstance()->FindFile("FiberTrackingLUTIndices.bin");
        IndicesStream.open(lutPath.c_str(), ios::in | ios::binary);
        if (IndicesStream.is_open())
        {
            try
            {
                int tmp;
                IndicesStream.seekg (0, ios::beg);
                while (!IndicesStream.eof())
                {
                    IndicesStream.read((char *)&tmp, sizeof(tmp));
                    indices.push_back(tmp);
                }
                IndicesStream.close();
            }
            catch (const std::exception& e)
            {
                MITK_INFO << e.what();
            }
        }
        else
        {
            MITK_INFO << "SphereInterpolator: could not load FiberTrackingLUTIndices.bin from " << lutPath;
            return false;
        }

        return true;
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
