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
/*=========================================================================

Program:   Tensor ToolKit - TTK
Module:    $URL: svn://scm.gforge.inria.fr/svn/ttk/trunk/Algorithms/itkResampleDwiImageFilter.h $
Language:  C++
Date:      $Date: 2010-06-07 13:39:13 +0200 (Mo, 07 Jun 2010) $
Version:   $Revision: 68 $

Copyright (c) INRIA 2010. All rights reserved.
See LICENSE.txt for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef _itk_ResampleDwiImageFilter_h_
#define _itk_ResampleDwiImageFilter_h_

#include <itkImageToImageFilter.h>
#include <itkVectorImage.h>
#include <itkPointShell.h>

namespace itk
{

/**
* \brief Resample DWI channel by channel.   */

template <class TScalarType>
class ResampleDwiImageFilter
        : public ImageToImageFilter<itk::VectorImage<TScalarType,3>, itk::VectorImage<TScalarType,3> >
{

public:

    enum Interpolation {
        Interpolate_NearestNeighbour,
        Interpolate_Linear,
        Interpolate_BSpline,
        Interpolate_WindowedSinc
    };

    typedef ResampleDwiImageFilter Self;
    typedef SmartPointer<Self>                      Pointer;
    typedef SmartPointer<const Self>                ConstPointer;
    typedef ImageToImageFilter< itk::VectorImage<TScalarType,3>, itk::VectorImage<TScalarType,3> >
    Superclass;

    typedef itk::Vector< double, 3 > DoubleVectorType;
    typedef itk::VectorImage<TScalarType,3> DwiImageType;
    typedef itk::Image<TScalarType,3> DwiChannelType;

    /** Method for creation through the object factory. */
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    /** Runtime information support. */
    itkTypeMacro(ResampleDwiImageFilter, ImageToImageFilter)

    itkSetMacro( Interpolation, Interpolation )

    void SetSamplingFactor(DoubleVectorType sampling)
    {
        m_NewSpacing = this->GetInput()->GetSpacing();
        m_NewSpacing[0] /= sampling[0];
        m_NewSpacing[1] /= sampling[1];
        m_NewSpacing[2] /= sampling[2];
        m_NewImageRegion = this->GetInput()->GetLargestPossibleRegion();
        m_NewImageRegion.SetSize(0, m_NewImageRegion.GetSize(0)*sampling[0]);
        m_NewImageRegion.SetSize(1, m_NewImageRegion.GetSize(1)*sampling[1]);
        m_NewImageRegion.SetSize(2, m_NewImageRegion.GetSize(2)*sampling[2]);
    }
    void SetNewSpacing(DoubleVectorType spacing)
    {
        DoubleVectorType oldSpacing = this->GetInput()->GetSpacing();
        DoubleVectorType sampling;
        sampling[0] = oldSpacing[0]/spacing[0];
        sampling[1] = oldSpacing[1]/spacing[1];
        sampling[2] = oldSpacing[2]/spacing[2];

        m_NewSpacing = spacing;
        m_NewImageRegion = this->GetInput()->GetLargestPossibleRegion();
        m_NewImageRegion.SetSize(0, m_NewImageRegion.GetSize(0)*sampling[0]);
        m_NewImageRegion.SetSize(1, m_NewImageRegion.GetSize(1)*sampling[1]);
        m_NewImageRegion.SetSize(2, m_NewImageRegion.GetSize(2)*sampling[2]);
    }
    void SetNewImageSize(ImageRegion<3> region)
    {
        ImageRegion<3> oldRegion = this->GetInput()->GetLargestPossibleRegion();
        DoubleVectorType sampling;
        sampling[0] = (double)region.GetSize(0)/oldRegion.GetSize(0);
        sampling[1] = (double)region.GetSize(1)/oldRegion.GetSize(1);
        sampling[2] = (double)region.GetSize(2)/oldRegion.GetSize(2);

        m_NewImageRegion = region;
        m_NewSpacing = this->GetInput()->GetSpacing();
        m_NewSpacing[0] /= sampling[0];
        m_NewSpacing[1] /= sampling[1];
        m_NewSpacing[2] /= sampling[2];
    }

    protected:
        ResampleDwiImageFilter();
    ~ResampleDwiImageFilter(){}

    void GenerateData();

    DoubleVectorType m_NewSpacing;
    ImageRegion<3>   m_NewImageRegion;
    Interpolation    m_Interpolation;
};


} // end of namespace


#ifndef ITK_MANUAL_INSTANTIATION
#include "itkResampleDwiImageFilter.txx"
#endif


#endif
