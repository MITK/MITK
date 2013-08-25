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
#ifndef _TRACKVIS
#define _TRACKVIS

#include <mitkCommon.h>
#include <mitkFiberBundleX.h>
#include <vtkSmartPointer.h>
#include <vtkPolyData.h>
#include <vtkCellArray.h>
#include <vtkPoints.h>
#include <vtkPolyLine.h>
#include <itkSize.h>

using namespace std;

// Structure to hold metadata of a TrackVis file
// ---------------------------------------------
struct TrackVis_header
{
    char                id_string[6];
    short int           dim[3];
    float               voxel_size[3];
    float               origin[3];
    short int           n_scalars;
    char                scalar_name[10][20];
    short int           n_properties;
    char                property_name[10][20];
    char                reserved[508];
    char                voxel_order[4];
    char                pad2[4];
    float               image_orientation_patient[6];
    char                pad1[2];
    unsigned char       invert_x;
    unsigned char       invert_y;
    unsigned char       invert_z;
    unsigned char       swap_xy;
    unsigned char       swap_yz;
    unsigned char       swap_zx;
    int                 n_count;
    int                 version;
    int                 hdr_size;
};

// Class to handle TrackVis files.
// -------------------------------
class FiberTracking_EXPORT TrackVis
{
private:
    string                filename;
    FILE*                 fp;
    int                 maxSteps;  // [TODO] should be related to the variable defined for fiber-tracking

public:
    TrackVis_header        hdr;

    short     create( string filename, itk::Size<3> size, itk::Point<float,3> origin );
    short     open( string filename );
    short     read( mitk::FiberBundleX* fib );
    short    append( mitk::FiberBundleX* fib );
    void    writeHdr();
    void    updateTotal( int totFibers );
    void    close();

    TrackVis();
    ~TrackVis();
};

#endif
