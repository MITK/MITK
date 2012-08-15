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
Module:    $URL: svn://scm.gforge.inria.fr/svn/ttk/trunk/Algorithms/itkMergeDiffusionImagesFilter.h $
Language:  C++
Date:      $Date: 2010-06-07 13:39:13 +0200 (Mo, 07 Jun 2010) $
Version:   $Revision: 68 $

Copyright (c) INRIA 2010. All rights reserved.
See LICENSE.txt for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef _itk_MergeDiffusionImagesFilter_h_
#define _itk_MergeDiffusionImagesFilter_h_

#include "itkImageToImageFilter.h"
#include <itkDiffusionTensor3D.h>
#include <itkVectorImage.h>
#include <itkVectorContainer.h>

namespace itk
{

template <class ScalarType>
class MergeDiffusionImagesFilter
        : public ImageSource<itk::VectorImage<ScalarType,3> >
{

public:

    typedef itk::VectorImage<ScalarType,3>                  DwiImageType;
    typedef MergeDiffusionImagesFilter Self;
    typedef ImageSource<DwiImageType> Superclass;

    typedef SmartPointer<Self> Pointer;
    typedef SmartPointer<const Self> ConstPointer;

    typedef typename DwiImageType::PixelType                DwiPixelType;
    typedef typename DwiImageType::RegionType               DwiRegionType;
    typedef typename std::vector< typename DwiImageType::Pointer >   DwiImageContainerType;

    typedef vnl_vector_fixed< double, 3 >                       GradientType;
    typedef itk::VectorContainer< unsigned int, GradientType >  GradientListType;
    typedef typename std::vector< GradientListType::Pointer >   GradientListContainerType;

    itkTypeMacro(MergeDiffusionImagesFilter, ImageToImageFilter)
    itkNewMacro(Self)

    void SetImageVolumes(DwiImageContainerType cont);
    void SetGradientLists(GradientListContainerType cont);
    void SetBValues(std::vector< double > bvals);

    GradientListType::Pointer GetOutputGradients();
    double GetBValue();

protected:

    MergeDiffusionImagesFilter();
    ~MergeDiffusionImagesFilter();

    void GenerateData();

    DwiImageContainerType       m_ImageVolumes;
    GradientListContainerType   m_GradientLists;
    std::vector< double >       m_BValues;
    int                         m_NumGradients;
    GradientListType::Pointer   m_OutputGradients;
    double                      m_BValue;
};


} // end of namespace


#ifndef ITK_MANUAL_INSTANTIATION
#include "itkMergeDiffusionImagesFilter.txx"
#endif


#endif
