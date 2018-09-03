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

/*===================================================================

This file is based heavily on a corresponding ITK filter.

===================================================================*/
#ifndef __itkEvaluateTractogramDirectionsFilter_h_
#define __itkEvaluateTractogramDirectionsFilter_h_

#include <itkProcessObject.h>
#include <itkVectorContainer.h>
#include <mitkFiberBundle.h>
#include <itkImageSource.h>

namespace itk{
/** \brief Calculates the voxel-wise angular error of the input tractogram to a set of voxel-wise directions.
 */

template< class PixelType >
class EvaluateTractogramDirectionsFilter : public ImageSource< Image< PixelType, 3 > >
{

public:

    typedef EvaluateTractogramDirectionsFilter Self;
    typedef SmartPointer<Self>                          Pointer;
    typedef SmartPointer<const Self>                    ConstPointer;
    typedef ImageSource< Image< PixelType, 3 > >        Superclass;
    typedef typename Superclass::OutputImageRegionType  OutputImageRegionType;
    typedef typename Superclass::OutputImageType        OutputImageType;

    /** Method for creation through the object factory. */
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    /** Runtime information support. */
    itkTypeMacro(EvaluateTractogramDirectionsFilter, ImageToImageFilter)

    typedef mitk::FiberBundle                                  FiberBundleType;
    typedef Vector< float, 3 >                                  DirectionType;
    typedef Image< DirectionType, 3 >                           DirectionImageType;
    typedef VectorContainer< int, DirectionImageType::Pointer > DirectionImageContainerType;
    typedef Image< float, 3 >                                   FloatImageType;
    typedef Image< bool, 3 >                                    BoolImageType;
    typedef Image< unsigned char, 3 >                           UCharImageType;
    typedef Image< double, 3 >                                  DoubleImageType;

    itkSetMacro( Tractogram, FiberBundleType::Pointer)                      ///< Input tractogram
    itkSetMacro( ReferenceImageSet , DirectionImageContainerType::Pointer)  ///< Input images containing one reference direction per voxel.
    itkSetMacro( MaskImage , UCharImageType::Pointer)                       ///< Calculation is only performed inside of the mask image.
    itkSetMacro( IgnoreMissingDirections , bool)                            ///< If in one voxel, the number of directions differs between the input tractogram and the reference, the excess directions are ignored. Otherwise, the error to the next closest direction is calculated.
    itkSetMacro( UseInterpolation , bool)                                   ///< Use trilinear interpolation.

    /** Output statistics. */
    itkGetMacro( MeanAngularError, float)
    itkGetMacro( MinAngularError, float)
    itkGetMacro( MaxAngularError, float)
    itkGetMacro( VarAngularError, float)
    itkGetMacro( MedianAngularError, float)

protected:
    EvaluateTractogramDirectionsFilter();
    ~EvaluateTractogramDirectionsFilter() {}

    void GenerateData();

    itk::Vector<PixelType, 3> GetItkVector(double point[3]);
    vnl_vector_fixed<PixelType, 3> GetVnlVector(double point[3]);
    vnl_vector_fixed<PixelType, 3> GetVnlVector(Vector< PixelType, 3 >& vector);

    UCharImageType::Pointer                 m_MaskImage;
    DirectionImageContainerType::Pointer    m_ReferenceImageSet;
    bool                                    m_IgnoreMissingDirections;
    double                                  m_MeanAngularError;
    double                                  m_MedianAngularError;
    double                                  m_MaxAngularError;
    double                                  m_MinAngularError;
    double                                  m_VarAngularError;
    std::vector< double >                   m_AngularErrorVector;
    double                                  m_Eps;
    FiberBundleType::Pointer                m_Tractogram;
    bool                                    m_UseInterpolation;
};

}

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkEvaluateTractogramDirectionsFilter.cpp"
#endif

#endif //__itkEvaluateTractogramDirectionsFilter_h_

