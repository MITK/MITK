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
#ifndef __itkShCoefficientImageImporter_h_
#define __itkShCoefficientImageImporter_h_

#include <itkOrientationDistributionFunction.h>

namespace itk{
/** \class ShCoefficientImageImporter
  Converts converts 4D SH coefficient images (MRtrix) to 3D vector images containing the SH coefficients
*/

template< class PixelType, int ShOrder >
class ShCoefficientImageImporter : public ProcessObject
{

public:

    typedef ShCoefficientImageImporter Self;
    typedef SmartPointer<Self>                      Pointer;
    typedef SmartPointer<const Self>                ConstPointer;
    typedef ProcessObject                           Superclass;
    typedef itk::Image< float, 4 >                  InputImageType;
    typedef Image< Vector< PixelType, (ShOrder*ShOrder + ShOrder + 2)/2 + ShOrder >, 3 > CoefficientImageType;
    typedef Image< Vector< PixelType, ODF_SAMPLING_SIZE >, 3 > OdfImageType;

    /** Method for creation through the object factory. */
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    /** Runtime information support. */
    itkTypeMacro(ShCoefficientImageImporter, ProcessObject)

    // input
    itkSetMacro( InputImage, InputImageType::Pointer) ///< sh coefficient image in FSL file format

    // output
    itkGetMacro( CoefficientImage, typename CoefficientImageType::Pointer)    ///< mitk style image containing the SH coefficients

    void GenerateData() override;

protected:
    ShCoefficientImageImporter();
    ~ShCoefficientImageImporter() override{}

    InputImageType::Pointer                   m_InputImage;
    typename CoefficientImageType::Pointer    m_CoefficientImage; ///< mitk style image containing the SH coefficients

private:

};

}

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkShCoefficientImageImporter.cpp"
#endif

#endif //__itkShCoefficientImageImporter_h_

