/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkDiffusionTensor3DReconstructionImageFilter.h,v $
  Language:  C++
  Date:      $Date: 2006-03-27 17:01:06 $
  Version:   $Revision: 1.12 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __itkShToOdfImageFilter_h_
#define __itkShToOdfImageFilter_h_

#include <itkOrientationDistributionFunction.h>
#include <itkImageToImageFilter.h>

namespace itk{
/** \class ShToOdfImageFilter

*/

template< class PixelType, int ShOrder >
class ShToOdfImageFilter : public
    ImageToImageFilter< itk::Image< itk::Vector< PixelType, (ShOrder*ShOrder + ShOrder + 2)/2 + ShOrder >, 3 >, itk::Image< itk::Vector<PixelType,ODF_SAMPLING_SIZE>,3> >
{

public:

    enum Toolkit {  ///< SH coefficient convention (depends on toolkit)
        FSL,
        MRTRIX
    };

    typedef itk::Vector< PixelType, (ShOrder*ShOrder + ShOrder + 2)/2 + ShOrder >   InputPixelType;
    typedef itk::Image<InputPixelType,3>              InputImageType;
    typedef typename InputImageType::RegionType       InputImageRegionType;

    typedef itk::Vector<PixelType,ODF_SAMPLING_SIZE>  OutputPixelType;
    typedef itk::Image<OutputPixelType,3>             OutputImageType;
    typedef typename OutputImageType::RegionType      OutputImageRegionType;

    typedef ShToOdfImageFilter Self;
    typedef itk::ImageToImageFilter<InputImageType, OutputImageType> Superclass;

    typedef SmartPointer<Self>                      Pointer;
    typedef SmartPointer<const Self>                ConstPointer;

    /** Method for creation through the object factory. */
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    /** Runtime information support. */
    itkTypeMacro(ShToOdfImageFilter, ImageToImageFilter)

    itkSetMacro( Toolkit, Toolkit)  ///< define SH coefficient convention (depends on toolkit)
    itkGetMacro( Toolkit, Toolkit)  ///< SH coefficient convention (depends on toolkit)

    void BeforeThreadedGenerateData();
    void ThreadedGenerateData( const OutputImageRegionType &outputRegionForThread, ThreadIdType);

protected:
    ShToOdfImageFilter();
    ~ShToOdfImageFilter(){}

    void CalcShBasis();

    vnl_matrix<float>                         m_ShBasis;
    Toolkit                                   m_Toolkit;

private:

};

}

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkShToOdfImageFilter.cpp"
#endif

#endif //__itkShToOdfImageFilter_h_

