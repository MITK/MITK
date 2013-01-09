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

    typedef ResampleDwiImageFilter Self;
    typedef SmartPointer<Self>                      Pointer;
    typedef SmartPointer<const Self>                ConstPointer;
    typedef ImageToImageFilter< itk::VectorImage<TScalarType,3>, itk::VectorImage<TScalarType,3> >
    Superclass;

    typedef itk::Vector< double, 3 > SamplingFactorType;
    typedef itk::VectorImage<TScalarType,3> DwiImageType;
    typedef itk::Image<TScalarType,3> DwiChannelType;

    /** Method for creation through the object factory. */
    itkNewMacro(Self)

    /** Runtime information support. */
    itkTypeMacro(ResampleDwiImageFilter, ImageToImageFilter)

    itkGetMacro(SamplingFactor, SamplingFactorType)
    itkSetMacro(SamplingFactor, SamplingFactorType)

    protected:
        ResampleDwiImageFilter();
    ~ResampleDwiImageFilter(){}

    void GenerateData();

    SamplingFactorType m_SamplingFactor;
};


} // end of namespace


#ifndef ITK_MANUAL_INSTANTIATION
#include "itkResampleDwiImageFilter.txx"
#endif


#endif
