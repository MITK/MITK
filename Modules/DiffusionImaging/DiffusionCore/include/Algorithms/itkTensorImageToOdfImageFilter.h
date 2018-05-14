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
Module:    $URL: svn://scm.gforge.inria.fr/svn/ttk/trunk/Algorithms/itkTensorImageToOdfImageFilter.h $
Language:  C++
Date:      $Date: 2010-06-07 13:39:13 +0200 (Mo, 07 Jun 2010) $
Version:   $Revision: 68 $

Copyright (c) INRIA 2010. All rights reserved.
See LICENSE.txt for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef _itk_TensorImageToOdfImageFilter_h_
#define _itk_TensorImageToOdfImageFilter_h_

#include "itkImageToImageFilter.h"
#include <mitkOdfImage.h>
#include <itkDiffusionTensor3D.h>


namespace itk
{

  template <class TInputScalarType, class TOutputScalarType>
  class TensorImageToOdfImageFilter
    : public ImageToImageFilter<itk::Image<itk::DiffusionTensor3D<TInputScalarType>,3>, itk::Image<itk::Vector<TOutputScalarType,ODF_SAMPLING_SIZE>,3> >
  {

  public:

    typedef TInputScalarType                          InputScalarType;
    typedef itk::DiffusionTensor3D<InputScalarType>   InputPixelType;
    typedef itk::Image<InputPixelType,3>              InputImageType;
    typedef typename InputImageType::RegionType       InputImageRegionType;

    typedef TOutputScalarType                            OutputScalarType;
    typedef itk::Vector<OutputScalarType,ODF_SAMPLING_SIZE>  OutputPixelType;
    typedef itk::Image<OutputPixelType,3>                OutputImageType;
    typedef typename OutputImageType::RegionType         OutputImageRegionType;

    typedef TensorImageToOdfImageFilter Self;
    typedef ImageToImageFilter<InputImageType, OutputImageType> Superclass;

    typedef SmartPointer<Self> Pointer;
    typedef SmartPointer<const Self> ConstPointer;

    itkTypeMacro (TensorImageToOdfImageFilter, ImageToImageFilter);

    itkStaticConstMacro (ImageDimension, unsigned int,
      OutputImageType::ImageDimension);

    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)


  protected:
    TensorImageToOdfImageFilter(){};
    ~TensorImageToOdfImageFilter() override{};

    void PrintSelf (std::ostream& os, Indent indent) const override
    {
      Superclass::PrintSelf (os, indent);
    }

    void BeforeThreadedGenerateData( void ) override;

    void ThreadedGenerateData( const
      OutputImageRegionType &outputRegionForThread, ThreadIdType) override;

    //void GenerateData();

  private:

    TensorImageToOdfImageFilter (const Self&);
    void operator=(const Self&);

    typename InputImageType::Pointer m_ItkTensorImage;
    typename OutputImageType::Pointer m_ItkOdfImage;
  };

} // end of namespace


#ifndef ITK_MANUAL_INSTANTIATION
#include "itkTensorImageToOdfImageFilter.txx"
#endif


#endif
