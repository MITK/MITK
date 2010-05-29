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
#ifndef __itkAnalyticalDiffusionQballReconstructionImageFilter_h_
#define __itkAnalyticalDiffusionQballReconstructionImageFilter_h_

#include "itkImageToImageFilter.h"
//#include "vnl/vnl_matrix.h"
#include "vnl/vnl_vector_fixed.h"
#include "vnl/vnl_matrix.h"
#include "vnl/algo/vnl_svd.h"
#include "itkVectorContainer.h"
#include "itkVectorImage.h"
#include "QuadProg.h"

namespace itk{
/** \class AnalyticalDiffusionQballReconstructionImageFilter
 * \brief This class takes as input one or more reference image (acquired in the 
 * absence of diffusion sensitizing gradients) and 'n' diffusion
 * weighted images and their gradient directions and computes an image of 
 * orientation distribution function coefficients in a spherical harmonic basis.
 *
 * \par Inputs and Usage
 * \par
 * When you have the 'n' gradient and one or more reference images in a single 
 * multi-component image (VectorImage), you can specify the images as
 * \code
 *       filter->SetGradientImage( directionsContainer, vectorImage );
 * \endcode
 * Note that this method is used to specify both the reference and gradient images.
 * This is convenient when the DWI images are read in using the 
 * <a href="http://wiki.na-mic.org/Wiki/index.php/NAMIC_Wiki:DTI:Nrrd_format">NRRD</a> 
 * format. Like the Nrrd format, the reference images are those components of the 
 * vectorImage whose gradient direction is (0,0,0). If more than one reference image
 * is present, they are averaged prior to the reconstruction.
 *
 * \par Outputs
 * The output image is an image of vectors that must be understood as ODFs:
 * \code
 *       Image< Vector< TPixelType, OdfNrDirections >, 3 >
 * \endcode
 *
 * \par Parameters
 * \li Threshold -  Threshold on the reference image data. The output ODF will 
 * be a null pdf for pixels in the reference image that have a value less 
 * than this.
 * \li BValue - See the documentation of SetBValue().
 * \li At least 6 gradient images must be specified for the filter to be able 
 * to run. If the input gradient directions g_i are majorly sampled on one half
 * of the sqhere, then each input image I_i will be duplicated and assign -g_i
 * in order to guarantee stability of the algorithm.
 * \li OdfDirections - directions of resulting orientation distribution function
 * \li EquatorNrSamplingPoints - number of sampling points on equator when
 * performing Funk Radeon Transform (FRT)
 * \li BasisFunctionCenters - the centers of the basis functions are used for
 * the sRBF (spherical radial basis functions interpolation). If not set, they
 * will be defaulted to equal m_EquatorNrSamplingPoints
 * 
 * \par Template parameters
 * The class is templated over 
 * \li the pixel type of the reference and gradient images 
 * (expected to be scalar data types) 
 * \li the internal representation of the ODF pixels (double, float etc).
 * \li the number of OdfDirections
 * \li the number of basis function centers for the sRBF
 *  
 * \par References:
 * \li<a href="http://www3.interscience.wiley.com/cgi-bin/fulltext/109800293/PDFSTART">[1]</a> 
 * <em>Tuch DS,
 * "Q-ball imaging", Magn Reson Med. 2004 Dec;52(6):1358-72.</em>
 * 
 */

template< class TReferenceImagePixelType, 
          class TGradientImagePixelType,
          class TOdfPixelType,
          int NOrderL,
          int NrOdfDirections>
class AnalyticalDiffusionQballReconstructionImageFilter :
  public ImageToImageFilter< Image< TReferenceImagePixelType, 3 >, 
                             Image< Vector< TOdfPixelType, /*(NOrderL*NOrderL + NOrderL + 2.0)/2.0 + NOrderL*/NrOdfDirections >, 3 > >
{

public:

  enum Normalization {
    QBAR_STANDARD,
    QBAR_B_ZERO_B_VALUE,
    QBAR_B_ZERO,
    QBAR_NONE,
    QBAR_ADC_ONLY,
    QBAR_RAW_SIGNAL,
    QBAR_SOLID_ANGLE,
    QBAR_NONNEG_SOLID_ANGLE
  };

  typedef AnalyticalDiffusionQballReconstructionImageFilter Self;
  typedef SmartPointer<Self>                      Pointer;
  typedef SmartPointer<const Self>                ConstPointer;
  typedef ImageToImageFilter< Image< TReferenceImagePixelType, 3>, 
          Image< Vector< TOdfPixelType, /*NOrderL*NOrderL + NOrderL + 2.0)/2.0 + NOrderL*/NrOdfDirections >, 3 > >
                          Superclass;
  
   /** Method for creation through the object factory. */
  itkNewMacro(Self);  

  /** Runtime information support. */
  itkTypeMacro(AnalyticalDiffusionQballReconstructionImageFilter, 
                                                   ImageToImageFilter);
 
  typedef TReferenceImagePixelType                 ReferencePixelType;

  typedef TGradientImagePixelType                  GradientPixelType;

  typedef Vector< TOdfPixelType, /*NOrderL*NOrderL + NOrderL + 2.0)/2.0 + NOrderL*/NrOdfDirections >
                                                   OdfPixelType;

  /** Reference image data,  This image is aquired in the absence 
   * of a diffusion sensitizing field gradient */
  typedef typename Superclass::InputImageType      ReferenceImageType;
  
  typedef Image< OdfPixelType, 3 >                 OdfImageType;

  typedef OdfImageType                             OutputImageType;

  typedef TOdfPixelType                            BZeroPixelType;

  typedef Image< BZeroPixelType, 3 >      BZeroImageType;

  typedef typename Superclass::OutputImageRegionType
                                                   OutputImageRegionType;

  /** Typedef defining one (of the many) gradient images.  */
  typedef Image< GradientPixelType, 3 >            GradientImageType;

  /** An alternative typedef defining one (of the many) gradient images. 
   * It will be assumed that the vectorImage has the same dimension as the 
   * Reference image and a vector length parameter of \c n (number of
   * gradient directions)*/
  typedef VectorImage< GradientPixelType, 3 >      GradientImagesType;

  /** Holds the ODF reconstruction matrix */
  typedef vnl_matrix< TOdfPixelType >*
                                                   OdfReconstructionMatrixType;
  
  typedef vnl_matrix< double >                     CoefficientMatrixType;

  /** Holds each magnetic field gradient used to acquire one DWImage */
  typedef vnl_vector_fixed< double, 3 >            GradientDirectionType;

  /** Container to hold gradient directions of the 'n' DW measurements */
  typedef VectorContainer< unsigned int, 
          GradientDirectionType >                  GradientDirectionContainerType;
  
  /** set method to add gradient directions and its corresponding
   * image. The image here is a VectorImage. The user is expected to pass the 
   * gradient directions in a container. The ith element of the container 
   * corresponds to the gradient direction of the ith component image the 
   * VectorImage.  For the baseline image, a vector of all zeros
   * should be set.*/
  void SetGradientImage( GradientDirectionContainerType *, 
                                             const GradientImagesType *image);
  
  /** Get reference image */
  virtual ReferenceImageType * GetReferenceImage() 
  { return ( static_cast< ReferenceImageType *>(this->ProcessObject::GetInput(0)) ); }

  /** Return the gradient direction. idx is 0 based */
  virtual GradientDirectionType GetGradientDirection( unsigned int idx) const
    {
    if( idx >= m_NumberOfGradientDirections )
      {
      itkExceptionMacro( << "Gradient direction " << idx << "does not exist" );
      }
    return m_GradientDirectionContainer->ElementAt( idx+1 );
    }

  static void tofile2(vnl_matrix<double> *A, std::string fname);
  static double factorial(int number);
  static void Cart2Sph(double x, double y, double z, double* cart);
  static double legendre0(int l);
  static double spherical_harmonic(int m,int l,double theta,double phi, bool complexPart);
  static double Yj(int m, int k, double theta, double phi);

  OdfPixelType Normalize(OdfPixelType odf, typename NumericTraits<ReferencePixelType>::AccumulateType b0 );

  vnl_vector<TOdfPixelType> PreNormalize( vnl_vector<TOdfPixelType> vec, typename NumericTraits<ReferencePixelType>::AccumulateType b0  );

  /** Threshold on the reference image data. The output ODF will be a null
   * pdf for pixels in the reference image that have a value less than this
   * threshold. */
  itkSetMacro( Threshold, ReferencePixelType );
  itkGetMacro( Threshold, ReferencePixelType );

  itkSetMacro( NormalizationMethod, Normalization);
  itkGetMacro( NormalizationMethod, Normalization );

  itkGetMacro( BZeroImage, typename BZeroImageType::Pointer);

  itkSetMacro( BValue, TOdfPixelType);
#ifdef GetBValue
#undef GetBValue
#endif
  itkGetConstReferenceMacro( BValue, TOdfPixelType);

  itkSetMacro( Lambda, double );
  itkGetMacro( Lambda, double );

#ifdef ITK_USE_CONCEPT_CHECKING
  /** Begin concept checking */
  itkConceptMacro(ReferenceEqualityComparableCheck,
    (Concept::EqualityComparable<ReferencePixelType>));
  itkConceptMacro(TensorEqualityComparableCheck,
    (Concept::EqualityComparable<OdfPixelType>));
  itkConceptMacro(GradientConvertibleToDoubleCheck,
    (Concept::Convertible<GradientPixelType, double>));
  itkConceptMacro(DoubleConvertibleToTensorCheck,
    (Concept::Convertible<double, OdfPixelType>));
  itkConceptMacro(GradientReferenceAdditiveOperatorsCheck,
    (Concept::AdditiveOperators<GradientPixelType, GradientPixelType,
                                ReferencePixelType>));
  itkConceptMacro(ReferenceOStreamWritableCheck,
    (Concept::OStreamWritable<ReferencePixelType>));
  itkConceptMacro(TensorOStreamWritableCheck,
    (Concept::OStreamWritable<OdfPixelType>));
  /** End concept checking */
#endif

protected:
  AnalyticalDiffusionQballReconstructionImageFilter();
  ~AnalyticalDiffusionQballReconstructionImageFilter() {};
  void PrintSelf(std::ostream& os, Indent indent) const;

  void ComputeReconstructionMatrix();
  
  void BeforeThreadedGenerateData();
  void ThreadedGenerateData( const 
      OutputImageRegionType &outputRegionForThread, int);
  
private:
  
  OdfReconstructionMatrixType                       m_ReconstructionMatrix;
  
  OdfReconstructionMatrixType                       m_CoeffReconstructionMatrix;

  OdfReconstructionMatrixType                       m_SphericalHarmonicBasisMatrix;

  /** container to hold gradient directions */
  GradientDirectionContainerType::Pointer           m_GradientDirectionContainer;

  /** Number of gradient measurements */
  unsigned int                                      m_NumberOfGradientDirections;

  /** Number of baseline images */
  unsigned int                                      m_NumberOfBaselineImages;

  /** Threshold on the reference image data */
  ReferencePixelType                                m_Threshold;

  /** LeBihan's b-value for normalizing tensors */
  TOdfPixelType                                     m_BValue;

  typename BZeroImageType::Pointer                  m_BZeroImage;

  double                                            m_Lambda;

  bool                                              m_DirectionsDuplicated;

  Normalization                                     m_NormalizationMethod;

  int                                               m_NumberCoefficients;

  QuadProgPP::Matrix<double>                        m_G, m_CE, m_CI;
  QuadProgPP::Vector<double>                        m_g0, m_ce0, m_ci0, m_x;
  vnl_matrix<double>*                               m_B_t;
  vnl_vector<double>*                               m_LP;
  
};

}

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkAnalyticalDiffusionQballReconstructionImageFilter.cpp"
#endif

#endif //__itkAnalyticalDiffusionQballReconstructionImageFilter_h_

