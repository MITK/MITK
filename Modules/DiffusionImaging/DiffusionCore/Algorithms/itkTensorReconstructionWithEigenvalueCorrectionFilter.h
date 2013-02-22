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
/*=========================================================================

Program:   Tensor ToolKit - TTK
Module:    $URL: svn://scm.gforge.inria.fr/svn/ttk/trunk/Algorithms/itkTensorImageToDiffusionImageFilter.h $
Language:  C++
Date:      $Date: 2010-06-07 13:39:13 +0200 (Mo, 07 Jun 2010) $
Version:   $Revision: 68 $

Copyright (c) INRIA 2010. All rights reserved.
See LICENSE.txt for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef _itk_TensorReconstructionWithEigenvalueCorrectionFilter_h_
#define _itk_TensorReconstructionWithEigenvalueCorrectionFilter_h_

#include "itkImageToImageFilter.h"
#include <itkDiffusionTensor3D.h>
#include <itkVectorImage.h>
#include <vnl/algo/vnl_symmetric_eigensystem.h>

#include <math.h>
#include <mitkDiffusionImage.h>


typedef itk::VectorImage<short, 3>  ImageType;




namespace itk
{

  template <class TDiffusionPixelType, class TTensorPixelType>
  class TensorReconstructionWithEigenvalueCorrectionFilter
    : public ImageToImageFilter< itk::Image< TDiffusionPixelType, 3 >, itk::Image<itk::DiffusionTensor3D<TTensorPixelType>,3> >
  {

  public:

    typedef TensorReconstructionWithEigenvalueCorrectionFilter                  Self;
    typedef SmartPointer<Self>                          Pointer;
    typedef SmartPointer<const Self>                    ConstPointer;
    typedef ImageToImageFilter< Image< TDiffusionPixelType, 3>,
      Image< DiffusionTensor3D< TTensorPixelType >, 3 > >
      Superclass;

    /** Method for creation through the object factory. */
    itkNewMacro(Self);

    /** Runtime information support. */
    itkTypeMacro(TensorReconstructionWithEigenvalueCorrectionFilter, ImageToImageFilter);

    typedef TDiffusionPixelType                       ReferencePixelType;
    typedef TDiffusionPixelType                       GradientPixelType;
    typedef DiffusionTensor3D< TTensorPixelType >     TensorPixelType;


    /** Reference image data,  This image is aquired in the absence
    * of a diffusion sensitizing field gradient */
    typedef typename Superclass::InputImageType      ReferenceImageType;
    typedef Image< TensorPixelType, 3 >              TensorImageType;
    typedef TensorImageType                          OutputImageType;
    typedef typename Superclass::OutputImageRegionType
      OutputImageRegionType;

    /** Typedef defining one (of the many) gradient images.  */
    typedef Image< GradientPixelType, 3 >            GradientImageType;

    /** An alternative typedef defining one (of the many) gradient images.
    * It will be assumed that the vectorImage has the same dimension as the
    * Reference image and a vector length parameter of \c n (number of
    * gradient directions) */
    typedef VectorImage< GradientPixelType, 3 >      GradientImagesType;
    typedef typename GradientImagesType::PixelType         GradientVectorType;


    /*
    /** Holds the tensor basis coefficients G_k
    typedef vnl_matrix_fixed< double, 6, 6 >         TensorBasisMatrixType;

    typedef vnl_matrix< double >                     CoefficientMatrixType;
    */

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

    /** Set method to set the reference image. */
    void SetReferenceImage( ReferenceImageType *referenceImage )
    {
      if( m_GradientImageTypeEnumeration == GradientIsInASingleImage)
      {
        itkExceptionMacro( << "Cannot call both methods:"
          << "AddGradientImage and SetGradientImage. Please call only one of them.");
      }

      this->ProcessObject::SetNthInput( 0, referenceImage );

      m_GradientImageTypeEnumeration = GradientIsInManyImages;
    }

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

    itkSetMacro( BValue, TTensorPixelType);
    itkSetMacro( B0Threshold, float);

    itkGetMacro(PseudoInverse, vnl_matrix<double>);
    itkGetMacro(H, vnl_matrix<double>);
    itkGetMacro(BVec, vnl_vector<double>);
    itkGetMacro(B0Mask, vnl_vector<short>);
    itkGetMacro(Voxdim, vnl_vector<double>);

    mitk::DiffusionImage<short>::Pointer GetOutputDiffusionImage()
    {
      return m_OutputDiffusionImage;
    }

    ImageType::Pointer GetVectorImage()
    {
      return m_VectorImage;
    }

    itk::Image<short, 3>::Pointer GetMask()
    {
      return m_MaskImage;
    }


    //itkGetMacro(OutputDiffusionImage, mitk::DiffusionImage<double>)

    //itkGetMacro( GradientDirectionContainer, GradientDirectionContainerType::Pointer);

  protected:

    TensorReconstructionWithEigenvalueCorrectionFilter();
    ~TensorReconstructionWithEigenvalueCorrectionFilter() {};


    void GenerateData();


    typedef enum
    {
      GradientIsInASingleImage = 1,
      GradientIsInManyImages,
      Else
    } GradientImageTypeEnumeration;



  private:

    short CheckNeighbours(int x, int y, int z,int f, itk::Size<3> size);

    void CalculateAttenuation(vnl_vector<double> org_data, vnl_vector<double> &atten,int nof,int numberb0);


    void CalculateTensor(vnl_matrix<double> pseudoInverse,vnl_vector<double> atten, vnl_vector<double> &tensor,int nof,int numberb0);

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

    mitk::DiffusionImage<short>::Pointer              m_OutputDiffusionImage;

    ImageType::Pointer                                m_VectorImage;

    float                                             m_B0Threshold;



    itk::Image<short, 3>::Pointer m_MaskImage;
    vnl_matrix<double> m_PseudoInverse;
    vnl_matrix<double> m_H;
    vnl_vector<double> m_BVec;
    vnl_vector<short> m_B0Mask;
    vnl_vector<double> m_Voxdim;

    typename GradientImagesType::Pointer m_GradientImagePointer;

  };






} // end of namespace


#ifndef ITK_MANUAL_INSTANTIATION
#include "itkTensorReconstructionWithEigenvalueCorrectionFilter.txx"
#endif


#endif
