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

/**
* \brief Merges diffusion weighted images, e.g. to generate one multishell volume from several single shell volumes.   */

template <class TScalarType>
class MergeDiffusionImagesFilter
        : public ImageSource<itk::VectorImage<TScalarType,3> >
{

public:


    typedef MergeDiffusionImagesFilter Self;
    typedef ImageSource< itk::VectorImage<TScalarType,3> > Superclass;
    typedef SmartPointer<Self> Pointer;
    typedef SmartPointer<const Self> ConstPointer;

    /** Method for creation through the object factory. */
   itkNewMacro(Self)

   /** Runtime information support. */
   itkTypeMacro(MergeDiffusionImagesFilter, ImageSource)

    typedef itk::VectorImage<TScalarType,3>                  DwiImageType;
    typedef typename DwiImageType::PixelType                DwiPixelType;
    typedef typename DwiImageType::RegionType               DwiRegionType;
    typedef typename std::vector< typename DwiImageType::Pointer >   DwiImageContainerType;

    typedef vnl_vector_fixed< double, 3 >                       GradientType;
    typedef itk::VectorContainer< unsigned int, GradientType >  GradientListType;
    typedef typename std::vector< GradientListType::Pointer >   GradientListContainerType;

   // input
    void SetImageVolumes(DwiImageContainerType cont);       ///< input DWI image volume container
    void SetGradientLists(GradientListContainerType cont);  ///< gradients of all input images
    void SetBValues(std::vector< double > bvals);           ///< b-values of all input images

    // output
    GradientListType::Pointer GetOutputGradients();         ///< gradient list of the merged output image
    double GetB_Value();                                    ///< main b-value of the merged output image

protected:

    MergeDiffusionImagesFilter();
    ~MergeDiffusionImagesFilter();

    void GenerateData();

    DwiImageContainerType       m_ImageVolumes;     ///< contains input images
    GradientListContainerType   m_GradientLists;    ///< contains gradients of all input images
    std::vector< double >       m_BValues;          ///< contains b-values of all input images
    int                         m_NumGradients;     ///< number of gradients in the output image
    GradientListType::Pointer   m_OutputGradients;  ///< container for output gradients
    double                      m_BValue;           ///< main output b-value
};


} // end of namespace


#ifndef ITK_MANUAL_INSTANTIATION
#include "itkMergeDiffusionImagesFilter.txx"
#endif


#endif
