/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

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

    typedef std::map<double, std::vector<unsigned int> > BValueMap;
    typedef std::map<double, std::vector<unsigned int> >::iterator BValueMapIteraotr;
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
    void SetGradientImage( GradientDirectionContainerType *, const GradientImagesType *image , float bvalue);//, std::vector<bool> listOfUserSelctedBValues );


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

    void Normalize(OdfPixelType & odf );

    void S_S0Normalization( vnl_vector<double> & vec, typename NumericTraits<ReferencePixelType>::AccumulateType b0  = 0 );
    void S_S0Normalization( vnl_matrix<double> & mat, typename NumericTraits<ReferencePixelType>::AccumulateType b0  = 0 );

    void DoubleLogarithm(vnl_vector<double> & vec);

    void Threshold(vnl_vector<double> & vec, double delta = 0.01);
    void Threshold(vnl_matrix<double> & mat, double delta = 0.01);
    double CalculateThreashold(const double value, const double delta);

    void Projection1( vnl_matrix<double> & mat, double delta = 0.01);
    void Projection2( vnl_vector<double> & A, vnl_vector<double> & alpha, vnl_vector<double> & beta, double delta = 0.01);

    /** Threshold on the reference image data. The output ODF will be a null
   * pdf for pixels in the reference image that have a value less than this
   * threshold. */
    itkSetMacro( Threshold, ReferencePixelType );
    itkGetMacro( Threshold, ReferencePixelType );

    itkGetMacro( BZeroImage, typename BZeroImageType::Pointer);
    //itkGetMacro( ODFSumImage, typename BlaImage::Pointer);

    itkSetMacro( Lambda, double );
    itkGetMacro( Lambda, double );

    itkGetConstReferenceMacro( BValue, TOdfPixelType);

    void SetBValueMap(BValueMap map){this->m_BValueMap = map;}


protected:
    DiffusionMultiShellQballReconstructionImageFilter();
    ~DiffusionMultiShellQballReconstructionImageFilter() { delete m_ODFSphericalHarmonicBasisMatrix; delete m_CoeffReconstructionMatrix; };
    void PrintSelf(std::ostream& os, Indent indent) const;

    void ComputeReconstructionMatrix(IndiciesVector const & refVector);
    void ComputeODFSHBasis();
    bool CheckDuplicateDiffusionGradients();
    bool CheckForDifferingShellDirections();
    void ComputeSphericalHarmonicsBasis(vnl_matrix<double>* QBallReference, vnl_matrix<double>* SHBasisOutput, int Lorder , vnl_matrix<double>* LaplaciaBaltramiOutput =0 , vnl_vector<int>* SHOrderAssociation =0 , vnl_matrix<double> * SHEigenvalues =0);
    //void ComputeFunkRadonTransformationMatrix(vnl_vector<int>* SHOrderAssociationReference, vnl_matrix<double>* FRTMatrixOutput );
    bool CheckHemisphericalArrangementOfGradientDirections();

    void BeforeThreadedGenerateData();
    void ThreadedGenerateData( const OutputImageRegionType &outputRegionForThread, int NumberOfThreads );

    void StandardOneShellReconstruction(const OutputImageRegionType& outputRegionForThread);
    void AnalyticalThreeShellReconstruction(const OutputImageRegionType& outputRegionForThread);
    void NumericalNShellReconstruction(const OutputImageRegionType& outputRegionForThread);
    void GenerateAveragedBZeroImage(const OutputImageRegionType& outputRegionForThread);

private:

    enum ReconstructionType
    {
        Mode_Analytical3Shells,
        Mode_NumericalNShells,
        Mode_Standard1Shell
    };


    // Interpolation
    bool m_Interpolation_Flag;

    //CoefficientMatrixType m_ReconstructionMatrix;
    CoefficientMatrixType m_CoeffReconstructionMatrix;
    CoefficientMatrixType m_ODFSphericalHarmonicBasisMatrix;
    //CoefficientMatrixType m_SignalReonstructionMatrix;
    //CoefficientMatrixType m_SHBasisMatrix;

    /** container to hold gradient directions */
    GradientDirectionContainerType::Pointer m_GradientDirectionContainer;

    /** Number of gradient measurements */
    unsigned int m_NumberOfGradientDirections;

    /** Number of baseline images */
    unsigned int m_NumberOfBaselineImages;

    /** Threshold on the reference image data */
    ReferencePixelType m_Threshold;

    /** LeBihan's b-value for normalizing tensors */
    float m_BValue;

    typename BZeroImageType::Pointer m_BZeroImage;

    BValueMap m_BValueMap;

    double m_Lambda;

    bool m_IsHemisphericalArrangementOfGradientDirections;

    bool m_IsArithmeticProgession;

    //int m_NumberCoefficients;

    ReconstructionType m_ReconstructionType;


    //------------------------- VNL-function ------------------------------------

    template<typename CurrentValue, typename WntValue>
    vnl_vector< WntValue> element_cast (vnl_vector< CurrentValue> const& v1)
    {
      vnl_vector<WntValue> result(v1.size());

      for(int i = 0 ; i < v1.size(); i++)
           result[i] = static_cast< WntValue>(v1[i]);

      return result;
    }

    template<typename type>
    double dot (vnl_vector_fixed< type ,3> const& v1, vnl_vector_fixed< type ,3 > const& v2 )
    {
      double result = (v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2]) / (v1.two_norm() * v2.two_norm());
      return result ;
    }

};

}

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkDiffusionMultiShellQballReconstructionImageFilter.cpp"
#endif

#endif //__itkDiffusionMultiShellQballReconstructionImageFilter_h_

