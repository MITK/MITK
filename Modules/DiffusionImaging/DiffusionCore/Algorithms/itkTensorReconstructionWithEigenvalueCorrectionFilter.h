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
#ifndef _itk_TensorReconstructionWithEigenvalueCorrectionFilter_h_
#define _itk_TensorReconstructionWithEigenvalueCorrectionFilter_h_

#include "itkImageToImageFilter.h"
#include <itkDiffusionTensor3D.h>
#include <itkVectorImage.h>
#include <vnl/algo/vnl_symmetric_eigensystem.h>

#include <math.h>



namespace itk
{

  template <class TDiffusionPixelType, class TTensorPixelType>
  class TensorReconstructionWithEigenvalueCorrectionFilter
    : public ImageToImageFilter< itk::Image< TDiffusionPixelType, 3 >, itk::Image<itk::DiffusionTensor3D<TTensorPixelType>,3> >
  {

  public:


    typedef itk::VectorImage<short, 3>  ImageType;



    typedef TensorReconstructionWithEigenvalueCorrectionFilter                  Self;
    typedef SmartPointer<Self>                          Pointer;
    typedef SmartPointer<const Self>                    ConstPointer;



    typedef ImageToImageFilter< Image< TDiffusionPixelType, 3>,
      Image< DiffusionTensor3D< TTensorPixelType >, 3 > >
      Superclass;

    /** Method for creation through the object factory. */
    itkNewMacro(Self)

    /** Runtime information support. */
    itkTypeMacro(TensorReconstructionWithEigenvalueCorrectionFilter, ImageToImageFilter)


    typedef TDiffusionPixelType                       GradientPixelType;
    typedef DiffusionTensor3D< TTensorPixelType >     TensorPixelType;

    typedef Image< TensorPixelType, 3 >              TensorImageType;

    typedef typename Superclass::OutputImageRegionType
      OutputImageRegionType;


    /** An alternative typedef defining one (of the many) gradient images.
    * It will be assumed that the vectorImage has the same dimension as the
    * Reference image and a vector length parameter of \c n (number of
    * gradient directions) */
    typedef VectorImage< GradientPixelType, 3 >      GradientImagesType;
    typedef typename GradientImagesType::PixelType         GradientVectorType;



    /** Holds each magnetic field gradient used to acquire one DWImage */
    typedef vnl_vector_fixed< double, 3 >            GradientDirectionType;

    /** Container to hold gradient directions of the 'n' DW measurements */
    typedef VectorContainer< unsigned int,
      GradientDirectionType >                  GradientDirectionContainerType;



    /** Another set method to add a gradient directions and its corresponding
    * image. The image here is a VectorImage. The user is expected to pass the
    * gradient directions in a container. The ith element of the container
    * corresponds to the gradient direction of the ith component image the
    * VectorImage.  For the baseline image, a vector of all zeros
    * should be set. */
    void SetGradientImage( GradientDirectionContainerType *,
      const GradientImagesType *image);




    itkSetMacro( BValue, TTensorPixelType)
    itkSetMacro( B0Threshold, float)

    itkGetMacro(PseudoInverse, vnl_matrix<double>)
    itkGetMacro(H, vnl_matrix<double>)
    itkGetMacro(BVec, vnl_vector<double>)
    itkGetMacro(B0Mask, vnl_vector<short>)


    ImageType::Pointer GetCorrectedDiffusionVolumes()
    {
      return m_CorrectedDiffusionVolumes;
    }

    itk::Image<short, 3>::Pointer GetMask()
    {
      return m_MaskImage;
    }


  protected:

    TensorReconstructionWithEigenvalueCorrectionFilter();
    ~TensorReconstructionWithEigenvalueCorrectionFilter() {}


    void GenerateData();


    typedef enum
    {
      GradientIsInASingleImage = 1,
      GradientIsInManyImages,
      Else
    } GradientImageTypeEnumeration;



  private:

    double CheckNeighbours(int x, int y, int z,int f, itk::Size<3> size, itk::Image<short, 3> ::Pointer mask,itk::VectorImage<short, 3>::Pointer corrected_diffusion_temp);

    void CalculateAttenuation(vnl_vector<double> org_data, vnl_vector<double> &atten,int nof,int numberb0);

    void CorrectDiffusionImage(int nof,int numberb0,itk::Size<3> size,itk::VectorImage<short, 3>::Pointer corrected_diffusion,itk::Image<short, 3>::Pointer mask,vnl_vector< double> pixel_max,vnl_vector< double> pixel_min);

    void GenerateTensorImage(int nof,int numberb0,itk::Size<3> size,itk::VectorImage<short, 3>::Pointer corrected_diffusion,itk::Image<short, 3>::Pointer mask,double what_mask,itk::Image< itk::DiffusionTensor3D<float>, 3 >::Pointer tensorImg );

    void DeepCopyTensorImage(itk::Image< itk::DiffusionTensor3D<float>, 3 >::Pointer tensorImg, itk::Image< itk::DiffusionTensor3D<float>, 3 >::Pointer temp_tensorImg);

    void DeepCopyDiffusionImage(itk::VectorImage<short, 3>::Pointer corrected_diffusion, itk::VectorImage<short, 3>::Pointer corrected_diffusion_temp,int nof);


    void TurnMask( itk::Size<3> size, itk::Image<short, 3>::Pointer mask, double previous_mask, double set_mask);

    double CheckNegatives ( itk::Size<3> size, itk::Image<short, 3>::Pointer mask, itk::Image< itk::DiffusionTensor3D<float>, 3 >::Pointer tensorImg );


    /** Gradient image was specified in a single image or in multiple images */
    GradientImageTypeEnumeration                      m_GradientImageTypeEnumeration;

    /** Number of gradient measurements */
    unsigned int                                      m_NumberOfGradientDirections;

    /** container to hold gradient directions */
    GradientDirectionContainerType::Pointer           m_GradientDirectionContainer;


    /** b-value */
    TTensorPixelType                                  m_BValue;

    /** Number of baseline images */
    unsigned int                                      m_NumberOfBaselineImages;

    ImageType::Pointer                                m_CorrectedDiffusionVolumes;

    float                                             m_B0Threshold;


    itk::Image<short, 3>::Pointer m_MaskImage;
    vnl_matrix<double> m_PseudoInverse;
    vnl_matrix<double> m_H;
    vnl_vector<double> m_BVec;

    /** m_B0Mask indicates whether a volume identified by an index is B0-weighted or not */
    vnl_vector<short> m_B0Mask;

    typename GradientImagesType::Pointer m_GradientImagePointer;

  };






} // end of namespace


#ifndef ITK_MANUAL_INSTANTIATION
#include "itkTensorReconstructionWithEigenvalueCorrectionFilter.txx"
#endif


#endif
