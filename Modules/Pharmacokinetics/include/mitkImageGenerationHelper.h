/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkImageGenerationHelper_h
#define mitkImageGenerationHelper_h

#include <itkImage.h>
#include <itkImageRegionIterator.h>

#include <mitkImage.h>
#include <mitkImagePixelReadAccessor.h>



#include <MitkPharmacokineticsExports.h>

namespace mitk
{

/**
 * \brief Helper class for generating a spatially homogeneous 4D dynamic image from a 1D curve.
 *
 * Creates a 4D image of dimensions (DimX, DimY, DimZ, t) where every spatial voxel (x,y,z)
 * contains the same time-varying curve. The time dimension is defined by the Grid and Curve
 * arrays. This is primarily useful for testing pharmacokinetic models with synthetic data.
 *
 * \todo Consider consolidating with TestingHelper/TestArtifactGenerator functionality.
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

    /**
     * \brief Generates and returns the 4D dynamic MITK image.
     *
     * Each 3D frame is filled with the curve value at the corresponding time point.
     * \pre DimX, DimY, DimZ, Grid, and Curve must be set.
     * \return Smart pointer to the generated 4D image.
     */
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

#endif
