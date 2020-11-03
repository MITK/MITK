/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef __MITK_IMAGEGENERATIONHELPER_H
#define __MITK_IMAGEGENERATIONHELPER_H

#include "itkImage.h"
#include "itkImageRegionIterator.h"

#include "mitkImage.h"
#include "mitkImagePixelReadAccessor.h"



#include "MitkPharmacokineticsExports.h"

namespace mitk
{

/** @todo #3 this is a helper class for generating a 4D dynamic image of dimensions (x,y,z, t) from an itk::Array<double> (t)
 * The Array will be set at every (x,y,z), so the result is a homogeneous image in 3D, with the 4th dimension as the Array.
 * Function GenerateDynamicImageMITK was copied from TestingHelper/TestArtifactGenerator. Maybe there is a better way to do this.
 */
class MITKPHARMACOKINETICS_EXPORT ImageGenerationHelper : public itk::Object
{

public:
    mitkClassMacroItkParent(ImageGenerationHelper, ::itk::Object);

    itkNewMacro(Self);

    typedef itk::Image<int> TestImageType;
    typedef itk::Array<double> TimeGridType;
    typedef itk::Array<double> CurveType;

    itkSetMacro(DimX, unsigned int);
    itkSetMacro(DimY, unsigned int);
    itkSetMacro(DimZ, unsigned int);
    itkGetConstReferenceMacro(DimX, unsigned int);
    itkGetConstReferenceMacro(DimY, unsigned int);
    itkGetConstReferenceMacro(DimZ, unsigned int);

    itkSetMacro(Grid,TimeGridType);
    itkGetConstReferenceMacro(Grid,TimeGridType);

    itkSetMacro(Curve, CurveType);
    itkGetConstReferenceMacro(Curve,CurveType);

    Image::Pointer  GenerateDynamicImageMITK();

private:
    ImageGenerationHelper(): m_DimX(0), m_DimY(0), m_DimZ(0) {};
    ~ImageGenerationHelper() override{};

    mitk::Image::Pointer GenerateTestFrame(unsigned int timePointIndex);
    unsigned int m_DimX, m_DimY, m_DimZ;
    TimeGridType m_Grid;
    CurveType m_Curve;


};
}

#endif //__MITK_IMAGEGENERATIONHELPER_H
