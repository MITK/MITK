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

#include "itkImageToImageFilter.h"
#include "vnl/vnl_vector_fixed.h"
#include "vnl/vnl_matrix.h"
#include "vnl/algo/vnl_svd.h"
#include "itkVectorContainer.h"
#include "itkVectorImage.h"
#include <mitkFiberBundle.h>
#include <mitkPeakImage.h>
#include <mitkShImage.h>
#include <itkOrientationDistributionFunction.h>
#include <vnl/algo/vnl_lbfgsb.h>
#include <mitkDiffusionFunctionCollection.h>

class VnlCostFunction : public vnl_cost_function
{
public:

  bool mrtrix;
  int ShOrder;
  vnl_vector<float> coeffs;
  double max;
  void SetProblem(vnl_vector<float>& coeffs, int ShOrder, bool mrtrix, double max)
  {
    this->coeffs = coeffs;
    this->ShOrder = ShOrder;
    this->mrtrix = mrtrix;
    this->max = max;
  }

  VnlCostFunction(const int NumVars=2) : vnl_cost_function(NumVars)
  {
  }

  // cost function
  double f(vnl_vector<double> const &x)
  {
    return -mitk::sh::GetValue(coeffs, ShOrder, x[0], x[1], mrtrix);
  }

  // gradient of cost function
  void gradf(vnl_vector<double> const &x, vnl_vector<double> &dx)
  {
    fdgradf(x, dx);
  }
};


namespace itk{

/**
* \brief Extract ODF peaks by searching all local maxima on a roughly sampled sphere and a successive gradient descent optimization
*/

template< class PixelType, int ShOrder, int NrOdfDirections >
class OdfMaximaExtractionFilter : public ImageToImageFilter< Image< Vector< PixelType, (ShOrder*ShOrder + ShOrder + 2)/2 + ShOrder >, 3 >,
Image< unsigned char, 3 > >
{

    public:

    enum Toolkit {  ///< SH coefficient convention (depends on toolkit)
        FSL,
        MRTRIX
    };

    enum NormalizationMethods {
        NO_NORM,            ///< no length normalization of the output peaks
        SINGLE_VEC_NORM,    ///< normalize the single peaks to length 1
        MAX_VEC_NORM        ///< normalize all peaks according to their length in comparison to the largest peak in the respective voxel (0-1)
    };

    typedef OdfMaximaExtractionFilter Self;
    typedef SmartPointer<Self>                      Pointer;
    typedef SmartPointer<const Self>                ConstPointer;
    typedef ImageToImageFilter< Image< Vector< PixelType, (ShOrder*ShOrder + ShOrder + 2)/2 + ShOrder >, 3 >,
            Image< unsigned char, 3 > > Superclass;

    /** Method for creation through the object factory. */
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    /** Runtime information support. */
    itkTypeMacro(OdfMaximaExtractionFilter, ImageToImageFilter)

    typedef typename Superclass::InputImageType           CoefficientImageType;
    typedef typename CoefficientImageType::RegionType     CoefficientImageRegionType;
    typedef typename CoefficientImageType::PixelType      CoefficientPixelType;
    typedef typename Superclass::OutputImageType          OutputImageType;
    typedef typename Superclass::OutputImageRegionType    OutputImageRegionType;
    typedef typename Superclass::InputImageRegionType     InputImageRegionType;
    typedef mitk::PeakImage::ItkPeakImageType             PeakImageType;

    typedef OrientationDistributionFunction<PixelType, NrOdfDirections>   OdfType;
    typedef itk::Image<unsigned char, 3>                                  ItkUcharImgType;

    typedef vnl_vector_fixed< double, 3 >                                  DirectionType;

    // input
    itkSetMacro( MaxNumPeaks, unsigned int)                 ///< maximum number of peaks per voxel. if more peaks are detected, only the largest are kept.
    itkSetMacro( RelativePeakThreshold, double)             ///< threshold on the peak length relative to the largest peak inside the current voxel
    itkSetMacro( AbsolutePeakThreshold, double)             ///< hard threshold on the peak length of all local maxima
    itkSetMacro( AngularThreshold, double)                  ///< directions closer together than the specified threshold are discarded
    itkSetMacro( MaskImage, ItkUcharImgType::Pointer)       ///< only voxels inside the binary mask are processed
    itkSetMacro( NormalizationMethod, NormalizationMethods) ///< normalization method of ODF peaks
    itkSetMacro( FlipX, bool)                               ///< flip peaks in x direction
    itkSetMacro( FlipY, bool)                               ///< flip peaks in y direction
    itkSetMacro( FlipZ, bool)                               ///< flip peaks in z direction
    itkSetMacro( ApplyDirectionMatrix, bool)
    itkSetMacro( ScaleByGfa, bool)
    itkSetMacro( Iterations, int)

    // output
    itkGetMacro( NumDirectionsImage, ItkUcharImgType::Pointer )
    itkGetMacro( PeakImage, PeakImageType::Pointer )

    itkSetMacro( Toolkit, Toolkit)  ///< define SH coefficient convention (depends on toolkit)
    itkGetMacro( Toolkit, Toolkit)  ///< SH coefficient convention (depends on toolkit)

    protected:
    OdfMaximaExtractionFilter();
    ~OdfMaximaExtractionFilter(){}

    void BeforeThreadedGenerateData();
    void ThreadedGenerateData( const OutputImageRegionType &outputRegionForThread, ThreadIdType threadID );
    void AfterThreadedGenerateData();

    /** Extract all local maxima from the densely sampled ODF surface. Thresholding possible. **/
    double FindCandidatePeaks(OdfType& odf, double odfMax, std::vector< DirectionType >& inDirs);

    /** Cluster input directions within a certain angular threshold **/
    std::vector< DirectionType > MeanShiftClustering(std::vector< DirectionType >& inDirs);

    private:

    NormalizationMethods                        m_NormalizationMethod;  ///< normalization method of ODF peaks
    unsigned int                                m_MaxNumPeaks;          ///< maximum number of peaks per voxel. if more peaks are detected, only the largest are kept.
    double                                      m_RelativePeakThreshold;        ///< threshold on the peak length relative to the largest peak inside the current voxel
    double                                      m_AbsolutePeakThreshold;///< hard threshold on the peak length of all local maxima
    vnl_matrix< float >                         m_ShBasis;              ///< container for evaluated SH base functions
    double                                      m_AngularThreshold;
    const int                                   m_NumCoeffs;            ///< number of spherical harmonics coefficients

    PeakImageType::Pointer                      m_PeakImage;
    ItkUcharImgType::Pointer                    m_NumDirectionsImage;     ///< number of peaks per voxel
    ItkUcharImgType::Pointer                    m_MaskImage;              ///< only voxels inside the binary mask are processed

    Toolkit                                     m_Toolkit;
    bool                                        m_FlipX;
    bool                                        m_FlipY;
    bool                                        m_FlipZ;
    bool                                        m_ApplyDirectionMatrix;
    bool                                        m_ScaleByGfa;
    int                                         m_Iterations;
};

}

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkOdfMaximaExtractionFilter.cpp"
#endif

#endif //__itkOdfMaximaExtractionFilter_h_

