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

#include <itkImageToImageFilter.h>

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
    typedef typename Superclass::OutputImageRegionType OutputImageRegionType;

    typedef TReferenceImagePixelType                  ReferencePixelType;

    /** GradientImageType
    * (e.g. type short)*/
    typedef TGradientImagePixelType                   GradientPixelType;

    /** GradientImageType
    * 3D VectorImage containing GradientPixelTypes */

    typedef VectorImage< GradientPixelType, 3 >       GradientImagesType;

    /** ODF PixelType */
    typedef Vector< TOdfPixelType, NrOdfDirections >  OdfPixelType;
    /** ODF ImageType */
    typedef Image< OdfPixelType, 3 >                  OdfImageType;
    /** BzeroImageType */
    typedef Image< TOdfPixelType, 3 >                 BZeroImageType;


    /** Container to hold gradient directions of the 'n' DW measurements */
    typedef VectorContainer< unsigned int, vnl_vector_fixed< double, 3 > > GradientDirectionContainerType;

    typedef Image< Vector< TOdfPixelType, (NOrderL*NOrderL + NOrderL + 2)/2 + NOrderL >, 3 > CoefficientImageType;

    typedef std::map<double, std::vector<unsigned int> > BValueMap;
    typedef std::map<double, std::vector<unsigned int> >::iterator BValueMapIteraotr;
    typedef std::vector<unsigned int> IndiciesVector;

    // --------------------------------------------------------------------------------------------//

    /** Method for creation through the object factory. */
    itkNewMacro(Self);
    /** Runtime information support. */
    itkTypeMacro(DiffusionMultiShellQballReconstructionImageFilter, ImageToImageFilter);

    /** Get reference image */
    virtual typename Superclass::InputImageType * GetInputImage()
    { return ( static_cast< typename Superclass::InputImageType *>(this->ProcessObject::GetInput(0)) ); }

    /** Replaces the Input method.
      * Var vols = mitk::DiffusionImage<type>
      * -----------------------------------------------------
      * GradientDirectionContainerType-Input gradientDirectionContainer (e.g. vols->GetDirections)
      * GradientImagesType-Input gradientImage (e.g. vols->GetVectorImage)
      * float-Input bvalue (e.g. vols->GetB_Value) */
    void SetGradientImage( GradientDirectionContainerType * gradientDirectionContainer, const GradientImagesType *gradientImage , float bvalue);//, std::vector<bool> listOfUserSelctedBValues );

   /** Set a BValue Map (key = bvalue, value = indicies splittet for each shell)
     * If the input image containes more than three q-shells
     * (e.g. b-Values of 0, 1000, 2000, 3000, 4000, ...).
     * For the Analytical-Reconstruction it is needed to set a
     * BValue Map containing three shells in an arithmetic series
     *  (e.g. 0, 1000, 2000, 3000).
     */
    inline void SetBValueMap(BValueMap map){this->m_BValueMap = map;}

    /** Threshold on the reference image data. The output ODF will be a null
   * pdf for pixels in the reference image that have a value less than this
   * threshold. */
    itkSetMacro( Threshold, ReferencePixelType );
    itkGetMacro( Threshold, ReferencePixelType );

    itkGetMacro( CoefficientImage, typename CoefficientImageType::Pointer );
    /** Return non-diffusion weighted images */
    itkGetMacro( BZeroImage, typename BZeroImageType::Pointer);

    /** Factor for Laplacian-Baltrami smoothing of the SH-coefficients*/
    itkSetMacro( Lambda, double );
    itkGetMacro( Lambda, double );

protected:
    DiffusionMultiShellQballReconstructionImageFilter();
    ~DiffusionMultiShellQballReconstructionImageFilter() { };
    void PrintSelf(std::ostream& os, Indent indent) const;
    void BeforeThreadedGenerateData();
    void ThreadedGenerateData( const OutputImageRegionType &outputRegionForThread, int NumberOfThreads );


private:

    enum ReconstructionType
    {
        Mode_Analytical3Shells,
        Mode_NumericalNShells,
        Mode_Standard1Shell
    };

    // Interpolation
    bool m_Interpolation_Flag;
    vnl_matrix< double > * m_Interpolation_SHT1_inv;
    vnl_matrix< double > * m_Interpolation_SHT2_inv;
    vnl_matrix< double > * m_Interpolation_SHT3_inv;
    vnl_matrix< double > * m_TARGET_SH_shell1;
    vnl_matrix< double > * m_TARGET_SH_shell2;
    vnl_matrix< double > * m_TARGET_SH_shell3;
    int m_MaxDirections;

    vnl_matrix< double > * m_CoeffReconstructionMatrix;
    vnl_matrix< double > * m_ODFSphericalHarmonicBasisMatrix;

    /** container to hold gradient directions */
    GradientDirectionContainerType::Pointer m_GradientDirectionContainer;

    /** Number of gradient measurements */
    unsigned int m_NumberOfGradientDirections;

    /** Number of baseline images */
    unsigned int m_NumberOfBaselineImages;

    /** Threshold on the reference image data */
    ReferencePixelType m_Threshold;

    typename BZeroImageType::Pointer m_BZeroImage;

    typename CoefficientImageType::Pointer m_CoefficientImage;

    float m_BValue;

    BValueMap m_BValueMap;

    double m_Lambda;

    bool m_IsHemisphericalArrangementOfGradientDirections;

    bool m_IsArithmeticProgession;

    //int m_NumberCoefficients;

    ReconstructionType m_ReconstructionType;

    void ComputeReconstructionMatrix(IndiciesVector const & refVector);
    void ComputeODFSHBasis();
    bool CheckDuplicateDiffusionGradients();
    bool CheckForDifferingShellDirections();
    IndiciesVector GetAllDirections();
    void ComputeSphericalHarmonicsBasis(vnl_matrix<double>* QBallReference, vnl_matrix<double>* SHBasisOutput, int Lorder , vnl_matrix<double>* LaplaciaBaltramiOutput =0 , vnl_vector<int>* SHOrderAssociation =0 , vnl_matrix<double> * SHEigenvalues =0);
    void Normalize(OdfPixelType & odf );
    void S_S0Normalization( vnl_vector<double> & vec, double b0  = 0 );
    void DoubleLogarithm(vnl_vector<double> & vec);
    double CalculateThreashold(const double value, const double delta);
    void Projection1(vnl_vector<double> & vec, double delta = 0.01);
    void Projection2( vnl_vector<double> & E1, vnl_vector<double> & E2, vnl_vector<double> & E3, double delta = 0.01);
    void Projection3( vnl_vector<double> & A, vnl_vector<double> & alpha, vnl_vector<double> & beta, double delta = 0.01);
    void StandardOneShellReconstruction(const OutputImageRegionType& outputRegionForThread);
    void AnalyticalThreeShellReconstruction(const OutputImageRegionType& outputRegionForThread);
    void NumericalNShellReconstruction(const OutputImageRegionType& outputRegionForThread);
    void GenerateAveragedBZeroImage(const OutputImageRegionType& outputRegionForThread);
    void ComputeSphericalFromCartesian(vnl_matrix<double> * Q, const IndiciesVector & refShell);


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

