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
#ifndef __itkOdfMaximaExtractionFilter_h_
#define __itkOdfMaximaExtractionFilter_h_

#include <mitkFiberBundleX.h>

#include <itkImageToImageFilter.h>
#include <itkVectorContainer.h>
#include <itkOrientationDistributionFunction.h>
#include <itkAnalyticalDiffusionQballReconstructionImageFilter.h>

namespace itk{
/** \class OdfMaximaExtractionFilter
Class that estimates the maxima of the 4th order SH representation of an ODF using analytic calculations (according to Aganj et al, MICCAI, 2010)
 */

template< class TOdfPixelType >
class OdfMaximaExtractionFilter : public ProcessObject
{

public:

  enum NormalizationMethods {
    NO_NORM,
    SINGLE_VEC_NORM,
    MAX_VEC_NORM
  };

  typedef OdfMaximaExtractionFilter Self;
  typedef SmartPointer<Self>                      Pointer;
  typedef SmartPointer<const Self>                ConstPointer;
  typedef ProcessObject Superclass;

   /** Method for creation through the object factory. */
  itkNewMacro(Self)

  /** Runtime information support. */
  itkTypeMacro(OdfMaximaExtractionFilter, ImageToImageFilter)

  typedef itk::AnalyticalDiffusionQballReconstructionImageFilter<short,short,float,4,QBALL_ODFSIZE> QballReconstructionFilterType;
  typedef QballReconstructionFilterType::CoefficientImageType                                       CoefficientImageType;
  typedef CoefficientImageType::PixelType                                                           CoefficientPixelType;
  typedef itk::VectorImage< short, 3 >                                                              DiffusionImageType;
  typedef vnl_vector_fixed< double, 3 >                                                             Vector3D;
  typedef VectorContainer< unsigned int, Vector3D >                                                 DirectionContainerType;
  typedef VectorContainer< unsigned int, DirectionContainerType::Pointer >                          ContainerType;
  typedef itk::Image<unsigned char, 3>                                                              ItkUcharImgType;
  typedef itk::Image< itk::Vector< float, 3 >, 3>                                                   ItkDirectionImage;
  typedef itk::VectorContainer< unsigned int, ItkDirectionImage::Pointer >                          ItkDirectionImageContainer;

  // output
  itkGetMacro( OutputFiberBundle, mitk::FiberBundleX::Pointer)
  itkGetMacro( NumDirectionsImage, ItkUcharImgType::Pointer)
  itkGetMacro( DirectionImageContainer, ItkDirectionImageContainer::Pointer)

  // input
  itkSetMacro( MaskImage, ItkUcharImgType::Pointer)
  itkSetMacro( NormalizationMethod, NormalizationMethods)
  itkSetMacro( UseAdaptiveStepWidth, bool)
  itkSetMacro( DiffusionGradients, DirectionContainerType::Pointer)
  itkSetMacro( DiffusionImage, DiffusionImageType::Pointer)
  itkSetMacro( ShCoeffImage, CoefficientImageType::Pointer)

  itkSetMacro( MaxNumPeaks, unsigned int)
  itkGetMacro( MaxNumPeaks, unsigned int)

  itkSetMacro( PeakThreshold, double)
  itkGetMacro( PeakThreshold, double)

  itkSetMacro( Bvalue, float)
  itkGetMacro( Bvalue, float)

  void GenerateData();

protected:
  OdfMaximaExtractionFilter();
  ~OdfMaximaExtractionFilter(){}

  // CSA Qball reconstruction (SH order 4)
  bool ReconstructQballImage();

  // calculate roots of cubic equation ax³ + bx² + cx + d = 0 using cardanos method
  std::vector<double> SolveCubic(const double& a, const double& b, const double& c, const double& d);

  // derivatives of SH representation of the ODF
  double ODF_dtheta2(const double& sn, const double& cs, const double& A, const double& B, const double& C, const double& D, const double& E, const double& F, const double& G, const double& H);
  double ODF_dphi2(const double& sn, const double& cs, const double& A, const double& B, const double& C, const double& D, const double& E, const double& F, const double& G, const double& H);
  double ODF_dtheta(const double& sn, const double& cs, const double& A, const double& B, const double& C, const double& D, const double& E, const double& F, const double& G, const double& H);

  // calculate all directions fullfilling the maximum consitions
  void FindCandidatePeaks(const CoefficientPixelType& SHcoeff);

  // cluster the peaks detected by FindCandidatePeaks and retain maximum m_MaxNumPeaks
  std::vector< Vector3D > ClusterPeaks(const CoefficientPixelType& shCoeff);

  // TODO
  void Cart2Sph(const std::vector< Vector3D >& dir,  vnl_matrix<double>& sphCoords);
  vnl_matrix<double> CalcShBasis(vnl_matrix<double>& sphCoords, const int& shOrder);

  // diffusion weighted image (mandatory input)
  DirectionContainerType::Pointer           m_DiffusionGradients;
  DiffusionImageType::Pointer               m_DiffusionImage;
  float                                     m_Bvalue;

  // binary mask image (optional input)
  ItkUcharImgType::Pointer                  m_MaskImage;

  // input parameters
  NormalizationMethods                      m_NormalizationMethod;
  bool                                      m_UseAdaptiveStepWidth;
  bool                                      m_ScaleByGfa;
  double                                    m_PeakThreshold;
  unsigned int                              m_MaxNumPeaks;

  // intermediate results
  CoefficientImageType::Pointer                 m_ShCoeffImage;
  std::vector< vnl_vector_fixed< double, 2 > >  m_CandidatePeaks;

  // output data
  mitk::FiberBundleX::Pointer               m_OutputFiberBundle;
  ItkUcharImgType::Pointer                  m_NumDirectionsImage;
  ItkDirectionImageContainer::Pointer       m_DirectionImageContainer;

private:

};

}

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkOdfMaximaExtractionFilter.cpp"
#endif

#endif //__itkOdfMaximaExtractionFilter_h_

