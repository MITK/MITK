/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __mitkBilateralFilter_h
#define __mitkBilateralFilter_h

//MITK
#include <mitkImage.h>
#include "mitkImageToImageFilter.h"

#include <itkImage.h>

namespace mitk
{

  class MITK_CORE_EXPORT BilateralFilter : public ImageToImageFilter
  {
  public: 

    mitkClassMacro( BilateralFilter , ImageToImageFilter );
    itkNewMacro( Self );

    itkSetMacro(DomainSigma,float);
    itkSetMacro(RangeSigma,float);
    itkSetMacro(AutoKernel,bool);
    itkSetMacro(KernelRadius,unsigned int);

    itkGetMacro(DomainSigma,float);
    itkGetMacro(RangeSigma,float);
    itkGetMacro(AutoKernel,bool);
    itkGetMacro(KernelRadius,unsigned int);

  protected:
    /*!
    \brief standard constructor
    */
    BilateralFilter();
    /*!
    \brief standard destructor
    */
    ~BilateralFilter();
    /*!
    \brief Method generating the output information of this filter (e.g. image dimension, image type, etc.).
    The interface ImageToImageFilter requires this implementation. Everything is taken from the input image.
    */
    virtual void GenerateOutputInformation();
    /*!
    \brief Method generating the output of this filter. Called in the updated process of the pipeline.
    This method generates the smoothed output image.
    */
    virtual void GenerateData();

    /*!
    \brief Internal templated method calling the ITK bilteral filter. Here the actual filtering is performed.
    */
    template<typename TPixel, unsigned int VImageDimension>
    void ItkImageProcessing( itk::Image<TPixel,VImageDimension>* itkImage );

    float m_DomainSigma; ///Sigma of the gaussian kernel. See ITK docu
    float m_RangeSigma; ///Sigma of the range mask kernel. See ITK docu
    bool m_AutoKernel; //true: kernel size is calculated from DomainSigma. See ITK Doc; false: set by m_KernelRadius
    unsigned int m_KernelRadius; //use in combination with m_AutoKernel = true
  };
} //END mitk namespace
#endif
