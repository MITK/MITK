/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-07-14 19:11:20 +0200 (Tue, 14 Jul 2009) $
Version:   $Revision: 18127 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef __itkDiffusionMultiShellQballReconstructionImageFilter_h_
#define __itkDiffusionMultiShellQballReconstructionImageFilter_h_

#include "itkImageToImageFilter.h"
#include "vnl/vnl_vector_fixed.h"
#include "vnl/vnl_matrix.h"
#include "vnl/algo/vnl_svd.h"
#include "itkVectorContainer.h"
#include "itkVectorImage.h"
#include <iomanip>



namespace itk{
/** \class DiffusionMultiShellQballReconstructionImageFilter
    Aganj_2010
 */

template< class TReferenceImagePixelType, class TGradientImagePixelType, class TOdfPixelType, int NOrderL, int NrOdfDirections>
class DiffusionMultiShellQballReconstructionImageFilter : public ImageToImageFilter< Image< TReferenceImagePixelType, 3 >, Image< Vector< TOdfPixelType, NrOdfDirections >, 3 > >
{

public:

    typedef DiffusionMultiShellQballReconstructionImageFilter Self;
    typedef SmartPointer<Self>                      Pointer;
    typedef SmartPointer<const Self>                ConstPointer;
    typedef ImageToImageFilter< Image< TReferenceImagePixelType, 3>, Image< Vector< TOdfPixelType, NrOdfDirections >, 3 > > Superclass;
    typedef TReferenceImagePixelType                  ReferencePixelType;
    typedef TGradientImagePixelType                   GradientPixelType;
    typedef Vector< TOdfPixelType, NrOdfDirections >  OdfPixelType;
    typedef typename Superclass::InputImageType       ReferenceImageType;
    typedef Image< OdfPixelType, 3 >                  OdfImageType;
    typedef OdfImageType                              OutputImageType;
    typedef TOdfPixelType                             BZeroPixelType;
    typedef Image< BZeroPixelType, 3 >                BZeroImageType;
    typedef typename Superclass::OutputImageRegionType OutputImageRegionType;
    /** Typedef defining one (of the many) gradient images.  */
    typedef Image< GradientPixelType, 3 >            GradientImageType;
    /** An alternative typedef defining one (of the many) gradient images.
   * It will be assumed that the vectorImage has the same dimension as the
   * Reference image and a vector length parameter of \c n (number of
   * gradient directions)*/
    typedef VectorImage< GradientPixelType, 3 >       GradientImagesType;
    /** Holds the ODF reconstruction matrix */
    typedef vnl_matrix< TOdfPixelType >*              OdfReconstructionMatrixType;
    typedef vnl_matrix< double > *                    CoefficientMatrixType;
    /** Holds each magnetic field gradient used to acquire one DWImage */
    typedef vnl_vector_fixed< double, 3 >             GradientDirectionType;
    /** Container to hold gradient directions of the 'n' DW measurements */
    typedef VectorContainer< unsigned int, GradientDirectionType > GradientDirectionContainerType;

    typedef std::map<double, std::vector<unsigned int> > GradientIndexMap;
    typedef std::map<double, std::vector<unsigned int> >::iterator GradientIndexMapIteraotr;
    typedef std::vector<unsigned int> IndiciesVector;

    // --------------------------------------------------------------------------------------------//

    /** Method for creation through the object factory. */
    itkNewMacro(Self);

    /** Runtime information support. */
    itkTypeMacro(DiffusionMultiShellQballReconstructionImageFilter, ImageToImageFilter);

    /** set method to add gradient directions and its corresponding
   * image. The image here is a VectorImage. The user is expected to pass the
   * gradient directions in a container. The ith element of the container
   * corresponds to the gradient direction of the ith component image the
   * VectorImage.  For the baseline image, a vector of all zeros
   * should be set.*/
    void SetGradientImage( GradientDirectionContainerType *, const GradientImagesType *image);
    void SetGradientIndexMap(GradientIndexMap gradientIdexMap)
    {


        m_GradientIndexMap = gradientIdexMap;

        /*
        std::stringstream s1, s2, s3;
        for(int i = 0; i < m_GradientIndexMap[1000].size() ; i++){
            s1 << m_GradientIndexMap[1000][i] << " ";
            s2 << m_GradientIndexMap[2000][i] << " ";
            s3 << m_GradientIndexMap[3000][i] << " ";
        }

        MITK_INFO << "1 SHELL " << std::endl << s1.str();
        MITK_INFO << "2 SHELL " << std::endl << s2.str();
        MITK_INFO << "3 SHELL " << std::endl << s3.str();
        */


    }

    /** Get reference image */
    virtual ReferenceImageType * GetReferenceImage()
    { return ( static_cast< ReferenceImageType *>(this->ProcessObject::GetInput(0)) ); }

    /** Return the gradient direction. idx is 0 based */
    virtual GradientDirectionType GetGradientDirection( unsigned int idx) const
    {
        if( idx >= m_GradientDirectionContainer->Size() )
        {
            itkExceptionMacro( << "Gradient direction " << idx << "does not exist" );
        }
        return m_GradientDirectionContainer->ElementAt( idx+1 );
    }

    OdfPixelType Normalize(OdfPixelType odf, typename NumericTraits<ReferencePixelType>::AccumulateType b0 );

    void S_S0Normalization( vnl_vector<double> & vec, typename NumericTraits<ReferencePixelType>::AccumulateType b0  = 0 );
    void S_S0Normalization( vnl_matrix<double> & mat, typename NumericTraits<ReferencePixelType>::AccumulateType b0  = 0 );

    void DoubleLogarithm(vnl_vector<double> & vec);

    void Threshold(vnl_vector<double> & vec, double sigma = 0.0001);
    void Threshold(vnl_matrix<double> & mat, double sigma = 0.0001);

    void Projection1( vnl_matrix<double> & mat, double delta = 0.0001);
    void Projection2( vnl_vector<double> & A, vnl_vector<double> & alpha, vnl_vector<double> & beta, double delta = 0.0001);

    /** Threshold on the reference image data. The output ODF will be a null
   * pdf for pixels in the reference image that have a value less than this
   * threshold. */
    itkSetMacro( Threshold, ReferencePixelType );
    itkGetMacro( Threshold, ReferencePixelType );

    itkGetMacro( BZeroImage, typename BZeroImageType::Pointer);
    //itkGetMacro( ODFSumImage, typename BlaImage::Pointer);

    itkSetMacro( BValue, TOdfPixelType);

    itkSetMacro( Lambda, double );
    itkGetMacro( Lambda, double );

    itkGetConstReferenceMacro( BValue, TOdfPixelType);


protected:
    DiffusionMultiShellQballReconstructionImageFilter();
    ~DiffusionMultiShellQballReconstructionImageFilter() {};
    void PrintSelf(std::ostream& os, Indent indent) const;

    void ComputeReconstructionMatrix(std::vector< unsigned int > gradientIndiciesVector);
    bool CheckDuplicateDiffusionGradients();
    void ComputeSphericalHarmonicsBasis(vnl_matrix<double>* QBallReference, vnl_matrix<double>* SHBasisOutput, vnl_matrix<double>* LaplaciaBaltramiOutput, vnl_vector<int>* SHOrderAssociation , vnl_matrix<double> * SHEigenvalues);
    void ComputeFunkRadonTransformationMatrix(vnl_vector<int>* SHOrderAssociationReference, vnl_matrix<double>* FRTMatrixOutput );
    bool CheckHemisphericalArrangementOfGradientDirections();

    void BeforeThreadedGenerateData();
    void ThreadedGenerateData( const OutputImageRegionType &outputRegionForThread, int NumberOfThreads );

    vnl_vector<TOdfPixelType> AnalyticalThreeShellParameterEstimation(const IndiciesVector * shell1, const IndiciesVector * shell2, const IndiciesVector * shell3, vnl_vector<TOdfPixelType> b);
    void StandardOneShellReconstruction(const OutputImageRegionType& outputRegionForThread);
    void AnalyticalThreeShellReconstruction(const OutputImageRegionType& outputRegionForThread);

private:

    enum ReconstructionType
    {
        Analytical3Shells,
        NumericalNShells,
        Standard1Shell
    };



    CoefficientMatrixType m_ReconstructionMatrix;
    CoefficientMatrixType m_CoeffReconstructionMatrix;
    CoefficientMatrixType m_ODFSphericalHarmonicBasisMatrix;
    CoefficientMatrixType m_SignalReonstructionMatrix;
    CoefficientMatrixType m_SHBasisMatrix;

    /** container to hold gradient directions */
    GradientDirectionContainerType::Pointer m_GradientDirectionContainer;

    /** Number of gradient measurements */
    unsigned int m_NumberOfGradientDirections;

    /** Number of baseline images */
    unsigned int m_NumberOfBaselineImages;

    /** Threshold on the reference image data */
    ReferencePixelType m_Threshold;

    /** LeBihan's b-value for normalizing tensors */
    TOdfPixelType m_BValue;

    typename BZeroImageType::Pointer m_BZeroImage;

    GradientIndexMap m_GradientIndexMap;

    double m_Lambda;

    bool m_IsHemisphericalArrangementOfGradientDirections;

    bool m_IsArithmeticProgession;

    int m_NumberCoefficients;

    ReconstructionType m_ReconstructionType;



    template< class VNLType >
    void printMatrix( VNLType * mat );



};

}

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkDiffusionMultiShellQballReconstructionImageFilter.cpp"
#endif

#endif //__itkDiffusionMultiShellQballReconstructionImageFilter_h_

