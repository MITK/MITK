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
#include <mitkFiberBundle.h>
#include <itkOrientationDistributionFunction.h>

namespace itk{

/**
* \brief Extract ODF peaks by searching all local maxima on a densely sampled ODF und clustering these maxima to get the underlying fiber direction.
* NrOdfDirections: number of sampling points on the ODF surface (about 20000 is a good value)
*/

template< class PixelType, int ShOrder, int NrOdfDirections >
class FiniteDiffOdfMaximaExtractionFilter : public ImageToImageFilter< Image< Vector< PixelType, (ShOrder*ShOrder + ShOrder + 2)/2 + ShOrder >, 3 >,
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

    typedef FiniteDiffOdfMaximaExtractionFilter Self;
    typedef SmartPointer<Self>                      Pointer;
    typedef SmartPointer<const Self>                ConstPointer;
    typedef ImageToImageFilter< Image< Vector< PixelType, (ShOrder*ShOrder + ShOrder + 2)/2 + ShOrder >, 3 >,
            Image< unsigned char, 3 > > Superclass;

    /** Method for creation through the object factory. */
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    /** Runtime information support. */
    itkTypeMacro(FiniteDiffOdfMaximaExtractionFilter, ImageToImageFilter)

    typedef typename Superclass::InputImageType           CoefficientImageType;
    typedef typename CoefficientImageType::RegionType     CoefficientImageRegionType;
    typedef typename CoefficientImageType::PixelType      CoefficientPixelType;
    typedef typename Superclass::OutputImageType          OutputImageType;
    typedef typename Superclass::OutputImageRegionType    OutputImageRegionType;
    typedef typename Superclass::InputImageRegionType     InputImageRegionType;
    typedef Image< float, 4 >                             PeakImageType;

    typedef OrientationDistributionFunction<PixelType, NrOdfDirections>   OdfType;
    typedef itk::Image<unsigned char, 3>                                  ItkUcharImgType;

    typedef vnl_vector_fixed< double, 3 >                                  DirectionType;

    // input
    itkSetMacro( MaxNumPeaks, unsigned int)                 ///< maximum number of peaks per voxel. if more peaks are detected, only the largest are kept.
    itkSetMacro( PeakThreshold, double)                     ///< threshold on the peak length relative to the largest peak inside the current voxel
    itkSetMacro( AbsolutePeakThreshold, double)             ///< hard threshold on the peak length of all local maxima
    itkSetMacro( ClusteringThreshold, double)               ///< directions closer together than the specified angular threshold will be clustered (in rad)
    itkSetMacro( AngularThreshold, double)                  ///< directions closer together than the specified threshold that remain after clustering are discarded (largest is kept) (in rad)
    itkSetMacro( MaskImage, ItkUcharImgType::Pointer)       ///< only voxels inside the binary mask are processed
    itkSetMacro( NormalizationMethod, NormalizationMethods) ///< normalization method of ODF peaks
    itkSetMacro( FlipX, bool)                               ///< flip peaks in x direction
    itkSetMacro( FlipY, bool)                               ///< flip peaks in y direction
    itkSetMacro( FlipZ, bool)                               ///< flip peaks in z direction
    itkSetMacro( ApplyDirectionMatrix, bool)

    // output
    itkGetMacro( NumDirectionsImage, ItkUcharImgType::Pointer )
    itkGetMacro( PeakImage, PeakImageType::Pointer )

    itkSetMacro( Toolkit, Toolkit)  ///< define SH coefficient convention (depends on toolkit)
    itkGetMacro( Toolkit, Toolkit)  ///< SH coefficient convention (depends on toolkit)

    protected:
    FiniteDiffOdfMaximaExtractionFilter();
    ~FiniteDiffOdfMaximaExtractionFilter(){}

    void BeforeThreadedGenerateData();
    void ThreadedGenerateData( const OutputImageRegionType &outputRegionForThread, ThreadIdType threadID );
    void AfterThreadedGenerateData();

    /** Extract all local maxima from the densely sampled ODF surface. Thresholding possible. **/
    void FindCandidatePeaks(OdfType& odf, double odfMax, std::vector< DirectionType >& inDirs);

    /** Cluster input directions within a certain angular threshold **/
    std::vector< DirectionType > MeanShiftClustering(std::vector< DirectionType >& inDirs);

    /** Convert direction vector to matrix **/
    void CreateDirMatrix(const std::vector< DirectionType >& dir, vnl_matrix<double>& sphCoords);

    private:

    NormalizationMethods                        m_NormalizationMethod;  ///< normalization method of ODF peaks
    unsigned int                                m_MaxNumPeaks;          ///< maximum number of peaks per voxel. if more peaks are detected, only the largest are kept.
    double                                      m_PeakThreshold;        ///< threshold on the peak length relative to the largest peak inside the current voxel
    double                                      m_AbsolutePeakThreshold;///< hard threshold on the peak length of all local maxima
    vnl_matrix< float >                         m_ShBasis;              ///< container for evaluated SH base functions
    double                                      m_ClusteringThreshold;  ///< directions closer together than the specified angular threshold will be clustered (in rad)
    double                                      m_AngularThreshold;     ///< directions closer together than the specified threshold that remain after clustering are discarded (largest is kept) (in rad)
    const int                                   m_NumCoeffs;            ///< number of spherical harmonics coefficients

    PeakImageType::Pointer                      m_PeakImage;
    ItkUcharImgType::Pointer                    m_NumDirectionsImage;     ///< number of peaks per voxel
    ItkUcharImgType::Pointer                    m_MaskImage;              ///< only voxels inside the binary mask are processed

    Toolkit                                     m_Toolkit;
    bool                                        m_FlipX;
    bool                                        m_FlipY;
    bool                                        m_FlipZ;
    bool                                        m_ApplyDirectionMatrix;
};

}

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkFiniteDiffOdfMaximaExtractionFilter.cpp"
#endif

#endif //__itkFiniteDiffOdfMaximaExtractionFilter_h_

