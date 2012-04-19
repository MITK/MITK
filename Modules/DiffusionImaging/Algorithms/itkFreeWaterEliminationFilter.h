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
#ifndef _itk_FreeWaterEliminationFilter_h_
#define _itk_FreeWaterEliminationFilter_h_

#include "itkImageToImageFilter.h"
#include <itkDiffusionTensor3D.h>
#include <itkVectorImage.h>
#include <vnl/algo/vnl_symmetric_eigensystem.h>

#include <math.h>



typedef itk::VectorImage<short, 3>  ImageType;




namespace itk
{

  template <class TDiffusionPixelType, class TTensorPixelType>
  class FreeWaterEliminationFilter
    : public ImageToImageFilter< itk::VectorImage< TDiffusionPixelType, 3 >, itk::Image<itk::DiffusionTensor3D<TTensorPixelType>,3> >
  {

  public:

    typedef FreeWaterEliminationFilter                  Self;
    typedef SmartPointer<Self>                          Pointer;
    typedef SmartPointer<const Self>                    ConstPointer;
    typedef ImageToImageFilter< Image< TDiffusionPixelType, 3>,
      Image< DiffusionTensor3D< TTensorPixelType >, 3 > >
      Superclass;

    /** Method for creation through the object factory. */
    itkNewMacro(Self);

    /** Runtime information support. */
    itkTypeMacro(FreeWaterEliminationFilter, ImageToImageFilter);

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



    /** Holds each magnetic field gradient used to acquire one DWImage */
    typedef vnl_vector_fixed< double, 3 >            GradientDirectionType;

    /** Container to hold gradient directions of the 'n' DW measurements */
    typedef VectorContainer< unsigned int,
      GradientDirectionType >                  GradientDirectionContainerType;




    itkSetMacro( BValue, TTensorPixelType);
    itkSetMacro( B0Threshold, float);
    itkSetMacro(PseudoInverse, vnl_matrix<double>);
    itkSetMacro(H, vnl_matrix<double>);
    itkSetMacro(BVec, vnl_vector<double>);


    void SetCorrectedVols(ImageType::Pointer correctedVols)
    {
      m_CorrectedVols = correctedVols;
    }

    void SetMask( itk::Image<double, 3>::Pointer mask)
    {
      m_MaskImage = mask;
    }




  protected:

    FreeWaterEliminationFilter();
    ~FreeWaterEliminationFilter() {};


    void GenerateData();



  private:



    /** Gradient image was specified in a single image or in multiple images */
    //GradientImageTypeEnumeration                      m_GradientImageTypeEnumeration;

    /** Number of gradient measurements */
    unsigned int                                      m_NumberOfGradientDirections;

    /** container to hold gradient directions */
    GradientDirectionContainerType::Pointer           m_GradientDirectionContainer;


    /** b-value */
    TTensorPixelType                                  m_BValue;

    /** Number of baseline images */
    unsigned int                                      m_NumberOfBaselineImages;


    ImageType::Pointer                                m_CorrectedVols;

    float                                             m_B0Threshold;

    itk::Image<double, 3>::Pointer m_MaskImage;
    vnl_matrix<double> m_PseudoInverse;
    vnl_matrix<double> m_H;
    vnl_vector<double> m_BVec;


  };






} // end of namespace


#ifndef ITK_MANUAL_INSTANTIATION
#include "itkFreeWaterEliminationFilter.txx"
#endif


#endif
