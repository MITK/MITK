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

    typedef TensorImageToDiffusionImageFilter Self;
    typedef ImageToImageFilter<InputImageType, OutputImageType> Superclass;

    typedef SmartPointer<Self> Pointer;
    typedef SmartPointer<const Self> ConstPointer;

    itkTypeMacro (TensorImageToDiffusionImageFilter, ImageToImageFilter);

    itkStaticConstMacro (ImageDimension, unsigned int,
      OutputImageType::ImageDimension);

    itkNewMacro (Self);

    typedef Vector<double,3>          GradientType;
    typedef std::vector<GradientType> GradientListType;

    /** Manually Set/Get a list of gradients */
    void SetGradientList(const GradientListType list)
    {
      m_GradientList = list;
      this->Modified();
    }
    GradientListType GetGradientList(void) const
    {return m_GradientList;}


    void SetBValue( const double& bval)
    { m_BValue = bval; }

  protected:
    TensorImageToDiffusionImageFilter()
    {
      m_BValue = 1.0;
      m_BaselineImage = 0;
    };
    ~TensorImageToDiffusionImageFilter(){};

    void PrintSelf (std::ostream& os, Indent indent) const
    {
      Superclass::PrintSelf (os, indent);      
    }

    void BeforeThreadedGenerateData( void );

    void ThreadedGenerateData( const 
      OutputImageRegionType &outputRegionForThread, int);
  
    //void GenerateData();

  private:

    TensorImageToDiffusionImageFilter (const Self&);
    void operator=(const Self&);

    GradientListType                     m_GradientList;
    double                               m_BValue;
    typename BaselineImageType::Pointer  m_BaselineImage;

  };    


} // end of namespace


#ifndef ITK_MANUAL_INSTANTIATION
#include "itkTensorImageToDiffusionImageFilter.txx"
#endif


#endif
