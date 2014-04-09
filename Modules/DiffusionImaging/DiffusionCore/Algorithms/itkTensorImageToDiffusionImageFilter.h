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
Module:    $URL: svn://scm.gforge.inria.fr/svn/ttk/trunk/Algorithms/itkTensorImageToDiffusionImageFilter.h $
Language:  C++
Date:      $Date: 2010-06-07 13:39:13 +0200 (Mo, 07 Jun 2010) $
Version:   $Revision: 68 $

Copyright (c) INRIA 2010. All rights reserved.
See LICENSE.txt for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef _itk_TensorImageToDiffusionImageFilter_h_
#define _itk_TensorImageToDiffusionImageFilter_h_

#include "itkImageToImageFilter.h"

#include <itkDiffusionTensor3D.h>
#include <itkVectorContainer.h>

namespace itk
{

  template <class TInputScalarType, class TOutputScalarType>
  class TensorImageToDiffusionImageFilter
    : public ImageToImageFilter<itk::Image<itk::DiffusionTensor3D<TInputScalarType>,3>, itk::VectorImage<TOutputScalarType,3> >
  {

  public:

    typedef TInputScalarType                          InputScalarType;
    typedef itk::DiffusionTensor3D<InputScalarType>   InputPixelType;
    typedef itk::Image<InputPixelType,3>              InputImageType;
    typedef typename InputImageType::RegionType       InputImageRegionType;

    typedef TOutputScalarType                         OutputScalarType;
    typedef itk::VectorImage<OutputScalarType,3>      OutputImageType;
    typedef typename OutputImageType::PixelType       OutputPixelType;
    typedef typename OutputImageType::RegionType      OutputImageRegionType;

    typedef OutputScalarType                          BaselineScalarType;
    typedef BaselineScalarType                        BaselinePixelType;
    typedef typename itk::Image<BaselinePixelType,3>  BaselineImageType;
    typedef typename BaselineImageType::RegionType    BaselineImageRegionType;

    typedef itk::Image< short, 3>                     MaskImageType;
    typedef MaskImageType::RegionType                  MaskImageRegionType;

    typedef TensorImageToDiffusionImageFilter Self;
    typedef ImageToImageFilter<InputImageType, OutputImageType> Superclass;

    typedef SmartPointer<Self> Pointer;
    typedef SmartPointer<const Self> ConstPointer;

    itkTypeMacro (TensorImageToDiffusionImageFilter, ImageToImageFilter);

    itkStaticConstMacro (ImageDimension, unsigned int,
      OutputImageType::ImageDimension);

    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    typedef vnl_vector_fixed<double,3>    GradientType;
    typedef VectorContainer<unsigned int, GradientType> GradientListType;
    typedef GradientListType::Pointer     GradientListPointerType;

    /** Manually Set/Get a list of gradients */
    void SetGradientList(const GradientListPointerType list)
    {
      m_GradientList = list;
      this->Modified();
    }
    GradientListPointerType GetGradientList(void) const
    {return m_GradientList;}


    void SetBValue( const double& bval)
    { m_BValue = bval; }

    void SetMaskImage( MaskImageType::Pointer maskimage )
    {
      m_MaskImage = maskimage;
    }


    /**
     * @brief Set an external baseline image for signal generation (optional)
     *
     * An option to enforce a specific baseline image. If none provided (default) the filter uses
     * the itk::TensorToL2NormImageFilter to generate the modelled baseline image.
     */
    void SetExternalBaselineImage( typename BaselineImageType::Pointer bimage)
    {
      m_BaselineImage = bimage;
    }

    itkSetMacro(Min, OutputScalarType);
    itkSetMacro(Max, OutputScalarType);


  protected:
    TensorImageToDiffusionImageFilter()
    {
      m_BValue = 1.0;
      m_BaselineImage = 0;
      m_Min = 0.0;
      m_Max = 10000.0;
    }

    virtual ~TensorImageToDiffusionImageFilter(){}

    void PrintSelf (std::ostream& os, Indent indent) const
    {
      Superclass::PrintSelf (os, indent);
    }

    virtual void BeforeThreadedGenerateData( void );

    virtual void ThreadedGenerateData( const
      OutputImageRegionType &outputRegionForThread, ThreadIdType);

    //void GenerateData();

    GradientListPointerType              m_GradientList;
    double                               m_BValue;
    typename BaselineImageType::Pointer  m_BaselineImage;

    OutputScalarType                     m_Min;
    OutputScalarType                     m_Max;

    MaskImageType::Pointer               m_MaskImage;

  private:

    TensorImageToDiffusionImageFilter (const Self&);
    void operator=(const Self&);

  };


} // end of namespace


#ifndef ITK_MANUAL_INSTANTIATION
#include "itkTensorImageToDiffusionImageFilter.txx"
#endif


#endif
