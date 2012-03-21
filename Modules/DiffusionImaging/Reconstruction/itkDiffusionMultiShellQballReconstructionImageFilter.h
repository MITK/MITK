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
    typedef vnl_matrix< double >                      CoefficientMatrixType;
    /** Holds each magnetic field gradient used to acquire one DWImage */
    typedef vnl_vector_fixed< double, 3 >             GradientDirectionType;
    /** Container to hold gradient directions of the 'n' DW measurements */
    typedef VectorContainer< unsigned int, GradientDirectionType > GradientDirectionContainerType;
    typedef Image<float, 3> BlaImage;

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

    OdfPixelType Normalize(OdfPixelType odf, typename NumericTraits<ReferencePixelType>::AccumulateType b0 );

    vnl_vector<TOdfPixelType> PreNormalize( vnl_vector<TOdfPixelType> vec, typename NumericTraits<ReferencePixelType>::AccumulateType b0  );

    /** Threshold on the reference image data. The output ODF will be a null
   * pdf for pixels in the reference image that have a value less than this
   * threshold. */
    itkSetMacro( Threshold, ReferencePixelType );
    itkGetMacro( Threshold, ReferencePixelType );

    itkGetMacro( BZeroImage, typename BZeroImageType::Pointer);
    itkGetMacro( ODFSumImage, typename BlaImage::Pointer);

    itkSetMacro( BValue, TOdfPixelType);

    itkSetMacro( Lambda, double );
    itkGetMacro( Lambda, double );

    itkGetConstReferenceMacro( BValue, TOdfPixelType);


protected:
    DiffusionMultiShellQballReconstructionImageFilter();
    ~DiffusionMultiShellQballReconstructionImageFilter() {};
    void PrintSelf(std::ostream& os, Indent indent) const;

    void ComputeReconstructionMatrix();
    bool CheckDuplicateDiffusionGradients();
    void ComputeSphericalHarmonicsBasis(vnl_matrix<double>* QBallReference, vnl_matrix<double>* SHBasisOutput, vnl_matrix<double>* LaplaciaBaltramiOutput, vnl_vector<int>* SHOrderAssociation );
    void ComputeFunkRadonTransformationMatrix(vnl_vector<int>* SHOrderAssociationReference, vnl_matrix<double>* FRTMatrixOutput );

    void BeforeThreadedGenerateData();
    void ThreadedGenerateData( const OutputImageRegionType &outputRegionForThread, int NumberOfThreads );

private:

    OdfReconstructionMatrixType m_ReconstructionMatrix;
    OdfReconstructionMatrixType m_CoeffReconstructionMatrix;
    OdfReconstructionMatrixType m_SphericalHarmonicBasisMatrix;

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

    double m_Lambda;

    bool m_DirectionsDuplicated;

    int m_NumberCoefficients;

    BlaImage::Pointer m_ODFSumImage;
};

}

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkDiffusionMultiShellQballReconstructionImageFilter.cpp"
#endif

#endif //__itkDiffusionMultiShellQballReconstructionImageFilter_h_

