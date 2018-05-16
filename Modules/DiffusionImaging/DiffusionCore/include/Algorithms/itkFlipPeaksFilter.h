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
#ifndef __itkFlipPeaksFilter_h_
#define __itkFlipPeaksFilter_h_

#include "itkImageToImageFilter.h"
#include "vnl/vnl_vector_fixed.h"
#include "vnl/vnl_matrix.h"
#include "vnl/algo/vnl_svd.h"
#include "itkVectorContainer.h"
#include "itkVectorImage.h"
#include <mitkFiberBundle.h>
#include <itkOrientationDistributionFunction.h>
#include <mitkPeakImage.h>

namespace itk{

/**
* \brief Flip peaks along specified axes
*/

template< class PixelType >
class FlipPeaksFilter : public ImageToImageFilter< mitk::PeakImage::ItkPeakImageType, mitk::PeakImage::ItkPeakImageType >
{

    public:

    typedef FlipPeaksFilter Self;
    typedef SmartPointer<Self>                      Pointer;
    typedef SmartPointer<const Self>                ConstPointer;
    typedef ImageToImageFilter< mitk::PeakImage::ItkPeakImageType, mitk::PeakImage::ItkPeakImageType > Superclass;

    /** Method for creation through the object factory. */
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    /** Runtime information support. */
    itkTypeMacro(FlipPeaksFilter, ImageToImageFilter)

    typedef typename Superclass::InputImageType InputImageType;
    typedef typename Superclass::OutputImageType OutputImageType;
    typedef typename OutputImageType::RegionType         OutputImageRegionType;

    itkSetMacro( FlipX, bool)                               ///< flip peaks in x direction
    itkSetMacro( FlipY, bool)                               ///< flip peaks in y direction
    itkSetMacro( FlipZ, bool)                               ///< flip peaks in z direction

    protected:
    FlipPeaksFilter();
    ~FlipPeaksFilter(){}

    void BeforeThreadedGenerateData();
    void ThreadedGenerateData( const OutputImageRegionType &outputRegionForThread, ThreadIdType threadID );
    void AfterThreadedGenerateData();

    private:

    bool                                        m_FlipX;
    bool                                        m_FlipY;
    bool                                        m_FlipZ;
};

}

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkFlipPeaksFilter.cpp"
#endif

#endif //__itkFlipPeaksFilter_h_

