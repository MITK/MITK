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
#ifndef __itkEvaluateDirectionImagesFilter_h_
#define __itkEvaluateDirectionImagesFilter_h_

#include <itkProcessObject.h>
#include <itkVectorContainer.h>
#include <itkImageSource.h>

namespace itk{
/** \brief Evaluates the voxel-wise angular error between two sets of directions.
 */

template< class PixelType >
class EvaluateDirectionImagesFilter : public ImageSource< Image< PixelType, 3 > >
{

public:

    typedef EvaluateDirectionImagesFilter Self;
    typedef SmartPointer<Self>                          Pointer;
    typedef SmartPointer<const Self>                    ConstPointer;
    typedef ImageSource< Image< PixelType, 3 > >        Superclass;
    typedef typename Superclass::OutputImageRegionType  OutputImageRegionType;
    typedef typename Superclass::OutputImageType        OutputImageType;

    /** Method for creation through the object factory. */
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    /** Runtime information support. */
    itkTypeMacro(EvaluateDirectionImagesFilter, ImageToImageFilter)

    typedef Vector< float, 3 >                                  DirectionType;
    typedef Image< DirectionType, 3 >                           DirectionImageType;
    typedef VectorContainer< unsigned int, DirectionImageType::Pointer > DirectionImageContainerType;
    typedef Image< float, 3 >                                   FloatImageType;
    typedef Image< bool, 3 >                                    BoolImageType;
    typedef Image< unsigned char, 3 >                           UCharImageType;

    itkSetMacro( ImageSet , DirectionImageContainerType::Pointer)           ///< test image containers
    itkSetMacro( ReferenceImageSet , DirectionImageContainerType::Pointer)  ///< reference image containers
    itkSetMacro( MaskImage , UCharImageType::Pointer)                       ///< Calculation is only performed inside of the mask image.
    itkSetMacro( IgnoreMissingDirections , bool)                            ///< If in one voxel, the number of directions differs between the test container and the reference, the excess directions are ignored. Otherwise, the error to the next closest direction is calculated.
    itkSetMacro( IgnoreEmptyVoxels , bool)                                  ///< Don't increase error if either reference or test voxel is empty.

    /** Output statistics of the measured angular errors. */
    itkGetMacro( MeanAngularError, float)
    itkGetMacro( MinAngularError, float)
    itkGetMacro( MaxAngularError, float)
    itkGetMacro( VarAngularError, float)
    itkGetMacro( MedianAngularError, float)

    /** Output statistics of the measured peak length errors. */
    itkGetMacro( MeanLengthError, float)
    itkGetMacro( MinLengthError, float)
    itkGetMacro( MaxLengthError, float)
    itkGetMacro( VarLengthError, float)
    itkGetMacro( MedianLengthError, float)

protected:
    EvaluateDirectionImagesFilter();
    ~EvaluateDirectionImagesFilter() override {}

    void GenerateData() override;

    UCharImageType::Pointer                  m_MaskImage;
    DirectionImageContainerType::Pointer     m_ImageSet;
    DirectionImageContainerType::Pointer     m_ReferenceImageSet;
    bool                                     m_IgnoreMissingDirections;
    bool                                     m_IgnoreEmptyVoxels;
    double                                   m_MeanAngularError;
    double                                   m_MedianAngularError;
    double                                   m_MaxAngularError;
    double                                   m_MinAngularError;
    double                                   m_VarAngularError;
    std::vector< double >                    m_AngularErrorVector;
    double                                   m_MeanLengthError;
    double                                   m_MedianLengthError;
    double                                   m_MaxLengthError;
    double                                   m_MinLengthError;
    double                                   m_VarLengthError;
    std::vector< double >                    m_LengthErrorVector;

    double                                  m_Eps;
};

}

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkEvaluateDirectionImagesFilter.cpp"
#endif

#endif //__itkEvaluateDirectionImagesFilter_h_

