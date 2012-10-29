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
#ifndef __itkFiniteDiffOdfMaximaExtractionFilter_h_
#define __itkFiniteDiffOdfMaximaExtractionFilter_h_

#include "itkImageToImageFilter.h"
#include "vnl/vnl_vector_fixed.h"
#include "vnl/vnl_matrix.h"
#include "vnl/algo/vnl_svd.h"
#include "itkVectorContainer.h"
#include "itkVectorImage.h"
#include <mitkFiberBundleX.h>
#include <itkOrientationDistributionFunction.h>

namespace itk{
/** \class FiniteDiffOdfMaximaExtractionFilter
 */

template< class PixelType, int ShOrder, int NrOdfDirections >
class FiniteDiffOdfMaximaExtractionFilter :
        public ImageToImageFilter< Image< Vector< PixelType, (ShOrder*ShOrder + ShOrder + 2)/2 + ShOrder >, 3 >,
Image< Vector< PixelType, 3 >, 3 > >
{

    public:

    enum NormalizationMethods {
        NO_NORM,
        SINGLE_VEC_NORM,
        MAX_VEC_NORM
    };

    typedef FiniteDiffOdfMaximaExtractionFilter Self;
    typedef SmartPointer<Self>                      Pointer;
    typedef SmartPointer<const Self>                ConstPointer;
    typedef ImageToImageFilter< Image< Vector< PixelType, (ShOrder*ShOrder + ShOrder + 2)/2 + ShOrder >, 3 >,
            Image< Vector< PixelType, 3 >, 3 > > Superclass;

    /** Method for creation through the object factory. */
    itkNewMacro(Self)

            /** Runtime information support. */
            itkTypeMacro(FiniteDiffOdfMaximaExtractionFilter, ImageToImageFilter)

            typedef typename Superclass::InputImageType           CoefficientImageType;
    typedef typename CoefficientImageType::RegionType     CoefficientImageRegionType;
    typedef typename CoefficientImageType::PixelType      CoefficientPixelType;

    typedef typename Superclass::OutputImageType          OutputImageType;
    typedef typename Superclass::OutputImageRegionType    OutputImageRegionType;

    typedef OrientationDistributionFunction<PixelType, NrOdfDirections>   OdfType;
    typedef itk::Image<unsigned char, 3>                                  ItkUcharImgType;

    typedef vnl_vector_fixed< double, 3 >                                 DirectionType;
    typedef Image< Vector< float, 3 >, 3>                                 ItkDirectionImage;
    typedef VectorContainer< unsigned int, ItkDirectionImage::Pointer >   ItkDirectionImageContainer;

    // input
    itkSetMacro( MaxNumPeaks, unsigned int)
    itkSetMacro( PeakThreshold, double)
    itkSetMacro( AbsolutePeakThreshold, double)
    itkSetMacro( ClusteringThreshold, double)
    itkSetMacro( AngularThreshold, double)
    itkSetMacro( MaskImage, ItkUcharImgType::Pointer)
    itkSetMacro( NormalizationMethod, NormalizationMethods)

    // output
    itkGetMacro( OutputFiberBundle, mitk::FiberBundleX::Pointer)
    itkGetMacro( DirectionImageContainer, ItkDirectionImageContainer::Pointer)
    itkGetMacro( NumDirectionsImage, ItkUcharImgType::Pointer)

    protected:
    FiniteDiffOdfMaximaExtractionFilter();
    ~FiniteDiffOdfMaximaExtractionFilter(){}

//    void GenerateData();

    void BeforeThreadedGenerateData();
    void ThreadedGenerateData( const OutputImageRegionType &outputRegionForThread, int threadID );
    void AfterThreadedGenerateData();

    void FindCandidatePeaks(OdfType& odf, double odfMax, std::vector< DirectionType >& inDirs);
    std::vector< DirectionType > MeanShiftClustering(std::vector< DirectionType >& inDirs);

    void Cart2Sph(const std::vector< DirectionType >& dir, vnl_matrix<double>& sphCoords);
    vnl_matrix<double> CalcShBasis(vnl_matrix<double>& sphCoords);

    private:

    NormalizationMethods                        m_NormalizationMethod;
    unsigned int                                m_NumOdfDirections;
    unsigned int                                m_MaxNumPeaks;
    double                                      m_PeakThreshold;
    double                                      m_AbsolutePeakThreshold;
    vnl_matrix< double >                        m_ShBasis;
    double                                      m_ClusteringThreshold;
    double                                      m_AngularThreshold;
    const int                                   m_NumCoeffs;

    mitk::FiberBundleX::Pointer               m_OutputFiberBundle;
    ItkDirectionImageContainer::Pointer       m_DirectionImageContainer;
    ItkUcharImgType::Pointer                  m_NumDirectionsImage;
    ItkUcharImgType::Pointer                  m_MaskImage;
};

}

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkFiniteDiffOdfMaximaExtractionFilter.cpp"
#endif

#endif //__itkFiniteDiffOdfMaximaExtractionFilter_h_

