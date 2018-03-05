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

#ifndef MITKDIFFUSIONIMAGECORRECTIONFILTER_H
#define MITKDIFFUSIONIMAGECORRECTIONFILTER_H

#include "mitkImageSource.h"
#include <MitkDiffusionCoreExports.h>

namespace mitk
{
/**
 * @class DiffusionImageCorrectionFilter
 */
class MITKDIFFUSIONCORE_EXPORT DiffusionImageCorrectionFilter
        : public ImageSource
{
public:
  /** class macros */
  mitkClassMacro( DiffusionImageCorrectionFilter,
                  ImageSource )

  itkSimpleNewMacro(Self)

  typedef short DiffusionPixelType;
  typedef vnl_vector_fixed< double, 3 >      GradientDirectionType;
  typedef vnl_matrix_fixed< double, 3, 3 >   TransformMatrixType;
  typedef itk::VectorContainer< unsigned int, GradientDirectionType >
                                             GradientDirectionContainerType;
  typedef GradientDirectionContainerType::Pointer GradientDirectionContainerPointerType;

  typedef std::vector< TransformMatrixType >  TransformsVectorType;

  typedef Superclass::OutputType DiffusionImageType;
  typedef DiffusionImageType::Pointer    DiffusionImageTypePointer;
  typedef itk::VectorImage<DiffusionPixelType,3>     ImageType;

  /**
   * @brief Set the mitk image ( a 3d+t image ) which is to be reinterpreted as dw image
   * @param mitkImage
   */
  void SetImage( DiffusionImageTypePointer input )
  {
    m_SourceImage = input;
  }

  /**
   * @brief Correct each gradient direction according to the given transform
   *
   * The size of the input is expected to correspond to the count of gradient images in the image.
   */
  void CorrectDirections( const TransformsVectorType& );

  /**
   * @brief Correct all gradient directions according to the given transform
   *
   * This will apply the same rotation to all directions.
   */
  void CorrectDirections( const TransformMatrixType& );

  void GenerateOutputInformation() override {}


protected:
  DiffusionImageCorrectionFilter();
  ~DiffusionImageCorrectionFilter() override {}

  /**
   * @brief Get the rotation component following the Finite Strain
   *
   * For a given transformation \f$A\f$ its rotation component is defined as \f$ (AA^{T})^{-1/2}\f$.
   *
   * The computation first computes \f$ B = AA^T \f$ and then estimates the square root. Square root of
   * diagonal matrices is defined as
   *  \f$ S = Q * \sqrt{C} * Q^{-1} \f$ with \f$ C \f$ having the eigenvalues on the diagonal.
   *
   */
  TransformMatrixType GetRotationComponent( const TransformMatrixType& );

  DiffusionImageTypePointer m_SourceImage;
};

}

#endif // MITKDIFFUSIONIMAGECORRECTIONFILTER_H
