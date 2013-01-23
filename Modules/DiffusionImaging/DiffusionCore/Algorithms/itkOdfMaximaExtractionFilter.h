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
    NO_NORM,            ///< don't normalize peaks
    SINGLE_VEC_NORM,    ///< normalize peaks to length 1
    MAX_VEC_NORM        ///< largest peak is normalized to length 1, other peaks relative to it
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
  itkGetMacro( OutputFiberBundle, mitk::FiberBundleX::Pointer)                  ///< vector field (peak sizes rescaled for visualization purposes)
  itkGetMacro( NumDirectionsImage, ItkUcharImgType::Pointer)                    ///< number of peaks per voxel
  itkGetMacro( DirectionImageContainer, ItkDirectionImageContainer::Pointer)    ///< container for output peaks

  // input
  itkSetMacro( MaskImage, ItkUcharImgType::Pointer)                 ///< only voxels inside the binary mask are processed
  itkSetMacro( NormalizationMethod, NormalizationMethods)           ///< normalization method of ODF peaks
  itkSetMacro( DiffusionGradients, DirectionContainerType::Pointer) ///< input for qball reconstruction
  itkSetMacro( DiffusionImage, DiffusionImageType::Pointer)         ///< input for qball reconstruction
  itkSetMacro( Bvalue, float)                                       ///< input for qball reconstruction
  itkSetMacro( ShCoeffImage, CoefficientImageType::Pointer)         ///< conatins spherical harmonic coefficients
  itkSetMacro( MaxNumPeaks, unsigned int)                           ///< if more peaks are found, only the largest are kept
  itkSetMacro( PeakThreshold, double)                               ///< threshold on peak length relative to the largest peak in the current voxel

  void GenerateData();

protected:
  OdfMaximaExtractionFilter();
  ~OdfMaximaExtractionFilter(){}

  /** CSA Qball reconstruction (SH order 4) **/
  bool ReconstructQballImage();

  /** calculate roots of cubic equation ax³ + bx² + cx + d = 0 using cardanos method **/
  std::vector<double> SolveCubic(const double& a, const double& b, const double& c, const double& d);

  /** derivatives of SH representation of the ODF **/
  double ODF_dtheta2(const double& sn, const double& cs, const double& A, const double& B, const double& C, const double& D, const double& E, const double& F, const double& G, const double& H);

  /** derivatives of SH representation of the ODF **/
  double ODF_dphi2(const double& sn, const double& cs, const double& A, const double& B, const double& C, const double& D, const double& E, const double& F, const double& G, const double& H);

  /** derivatives of SH representation of the ODF **/
  double ODF_dtheta(const double& sn, const double& cs, const double& A, const double& B, const double& C, const double& D, const double& E, const double& F, const double& G, const double& H);

  /** calculate all directions fullfilling the maximum consitions **/
  void FindCandidatePeaks(const CoefficientPixelType& SHcoeff);

  /** cluster the peaks detected by FindCandidatePeaks and retain maximum m_MaxNumPeaks **/
  std::vector< Vector3D > ClusterPeaks(const CoefficientPixelType& shCoeff);

  void Cart2Sph(const std::vector< Vector3D >& dir,  vnl_matrix<double>& sphCoords);
  vnl_matrix<double> CalcShBasis(vnl_matrix<double>& sphCoords, const int& shOrder);

  // diffusion weighted image (mandatory input)
  DirectionContainerType::Pointer           m_DiffusionGradients;   ///< input for qball reconstruction
  DiffusionImageType::Pointer               m_DiffusionImage;       ///< input for qball reconstruction
  float                                     m_Bvalue;               ///< input for qball reconstruction

  // binary mask image (optional input)
  ItkUcharImgType::Pointer                  m_MaskImage;            ///< only voxels inside the binary mask are processed

  // input parameters
  NormalizationMethods                      m_NormalizationMethod;  ///< normalization for peaks
  double                                    m_PeakThreshold;        ///< threshold on peak length relative to the largest peak in the current voxel
  unsigned int                              m_MaxNumPeaks;          ///< if more peaks are found, only the largest are kept

  // intermediate results
  CoefficientImageType::Pointer                 m_ShCoeffImage;     ///< conatins spherical harmonic coefficients
  std::vector< vnl_vector_fixed< double, 2 > >  m_CandidatePeaks;   ///< container for candidate peaks (all extrema, also minima)

  // output data
  mitk::FiberBundleX::Pointer               m_OutputFiberBundle;        ///< vector field (peak sizes rescaled for visualization purposes)
  ItkUcharImgType::Pointer                  m_NumDirectionsImage;       ///< number of peaks per voxel
  ItkDirectionImageContainer::Pointer       m_DirectionImageContainer;  ///< output peaks

private:

};

}

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkOdfMaximaExtractionFilter.cpp"
#endif

#endif //__itkOdfMaximaExtractionFilter_h_

