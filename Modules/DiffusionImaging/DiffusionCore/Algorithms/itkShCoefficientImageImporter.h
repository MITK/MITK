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
  Converts FSL reconstructions of diffusionweighted images (4D images containing the sh coefficients) to qball images or 3D sh-coefficient images.
*/

template< class PixelType, int ShOrder >
class ShCoefficientImageImporter : public ProcessObject
{

public:

    enum NormalizationMethods {
        NO_NORM,
        SINGLE_VEC_NORM,
        SPACING_COMPENSATION
    };

    enum Toolkit {  ///< SH coefficient convention (depends on toolkit)
        FSL,
        MRTRIX
    };

    typedef ShCoefficientImageImporter Self;
    typedef SmartPointer<Self>                      Pointer;
    typedef SmartPointer<const Self>                ConstPointer;
    typedef ProcessObject                           Superclass;
    typedef itk::Image< float, 4 >                  InputImageType;
    typedef Image< Vector< PixelType, (ShOrder*ShOrder + ShOrder + 2)/2 + ShOrder >, 3 > CoefficientImageType;
    typedef Image< Vector< PixelType, QBALL_ODFSIZE >, 3 > QballImageType;

    /** Method for creation through the object factory. */
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    /** Runtime information support. */
    itkTypeMacro(ShCoefficientImageImporter, ProcessObject)

    // input
    itkSetMacro( InputImage, InputImageType::Pointer) ///< sh coefficient image in FSL file format

    // output
    itkGetMacro( CoefficientImage, typename CoefficientImageType::Pointer)    ///< mitk style image containing the SH coefficients
    itkGetMacro( QballImage, typename QballImageType::Pointer)                ///< mitk Q-Ball image generated from the coefficients

    itkSetMacro( Toolkit, Toolkit)  ///< define SH coefficient convention (depends on toolkit)
    itkGetMacro( Toolkit, Toolkit)  ///< SH coefficient convention (depends on toolkit)

    void GenerateData() override;

protected:
    ShCoefficientImageImporter();
    ~ShCoefficientImageImporter(){}

    void CalcShBasis();
    vnl_matrix_fixed<double, 2, QBALL_ODFSIZE> GetSphericalOdfDirections();

    InputImageType::Pointer                   m_InputImage;
    typename CoefficientImageType::Pointer    m_CoefficientImage; ///< mitk style image containing the SH coefficients
    typename QballImageType::Pointer          m_QballImage;       ///< mitk Q-Ball image generated from the coefficients
    vnl_matrix<double>                        m_ShBasis;
    Toolkit                                   m_Toolkit;

private:

};

}

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkShCoefficientImageImporter.cpp"
#endif

#endif //__itkShCoefficientImageImporter_h_

