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

#ifndef MITKPYRAMIDIMAGEREGISTRATION_H
#define MITKPYRAMIDIMAGEREGISTRATION_H

#include <MitkDiffusionCoreExports.h>

#include <itkObject.h>
#include "itkImageMaskSpatialObject.h"
#include "itkNotImageFilter.h"
#include <itkCenteredVersorTransformInitializer.h>
#include <vnl/vnl_matrix_fixed.h>

#include "mitkImage.h"
#include "mitkBaseProcess.h"

#include "mitkPyramidRegistrationMethodHelper.h"
#include <itkWindowedSincInterpolateImageFunction.h>

#include "mitkImageToItk.h"
#include "mitkImageCast.h"
#include "mitkImageCaster.h"
#include "mitkITKImageImport.h"
#include "mitkIOUtil.h"


namespace mitk
{

/**
 * @brief The PyramidImageRegistration class implements a multi-scale registration method
 *
 * The PyramidImageRegistration class is suitable for aligning (f.e.) brain MR images. The method offers two
 * transform types
 *   - Rigid: optimizing translation and rotation only and
 *   - Affine ( default ): with scaling in addition ( 12 DOF )
 *
 * The error metric is internally chosen based on the selected task type : \sa SetCrossModalityOn
 *
 * It uses
 *   - MattesMutualInformation for CrossModality=on ( default ) and
 *   - NormalizedCorrelation for CrossModality=off.
 */
class MitkDiffusionCore_EXPORT PyramidImageRegistrationMethod :
    public itk::Object
{
public:

  /** Typedefs */
  mitkClassMacro(PyramidImageRegistrationMethod, itk::Object)

  /** Smart pointer support */
  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)

  typedef itk::OptimizerParameters<double> ParametersType;

  /** Typedef for the transformation matrix, corresponds to the InternalMatrixType from ITK transforms */
  typedef vnl_matrix_fixed< double, 3, 3> TransformMatrixType;

  typedef itk::AffineTransform< double >  AffineTransformType;
  typedef itk::Euler3DTransform< double >  RigidTransformType;

  /** Registration is between modalities - will take MattesMutualInformation as error metric */
  void SetCrossModalityOn()
  {
    m_CrossModalityRegistration = true;
  }

  /** Registration is between modalities - will take NormalizedCorrelation as error metric */
  void SetCrossModalityOff()
  {
    m_CrossModalityRegistration = false;
  }

  /** Turn the cross-modality on/off */
  void SetCrossModality(bool flag)
  {
    if( flag )
      this->SetCrossModalityOn();
    else
      this->SetCrossModalityOff();
  }

  /** Controll the verbosity of the registration output
   *
   * for true, each iteration step of the optimizer is watched and passed to the std::cout
   */
  void SetVerbose(bool flag)
  {
    if( flag )
      this->SetVerboseOn();
    else
      this->SetVerboseOff();
  }

  /** Turn verbosity on, \sa SetVerbose */
  void SetVerboseOn()
  {
    m_Verbose = true;
  }

  /** Turn verbosity off, \sa SetVerbose */
  void SetVerboseOff()
  {
    m_Verbose = false;
  }

  /** A rigid ( 6dof : translation + rotation ) transform is optimized */
  void SetTransformToRigid()
  {
    m_UseAffineTransform = false;
  }

  /** An affine ( 12dof : Rigid + scale + skew ) transform is optimized */
  void SetTransformToAffine()
  {
    m_UseAffineTransform = true;
  }

  /** Input image, the reference one */
  void SetFixedImage( mitk::Image::Pointer );

  /** Input image, the one to be transformed */
  void SetMovingImage( mitk::Image::Pointer );

  /** Fixed image mask, excludes the masked voxels from the registration metric*/
  void SetFixedImageMask( mitk::Image::Pointer mask);

  /** \brief Use the itk::CenteredVersorTransformInitializer to perform initialization step for the registration
   *
   *  The initializer takes the geometries and their features to perform an initialization for the successive registration
   */
  void SetInitializeByGeometry(bool flag)
  {
    m_InitializeByGeometry = flag;
  }

  void Update();

  /**
   * @brief Get the number of parameters optimized ( 12 or 6 )
   *
   * @return number of paramters
   */
  unsigned int GetNumberOfParameters()
  {
    unsigned int retValue = 12;
    if(!m_UseAffineTransform)
      retValue = 6;

    return retValue;
  }

  /**
   * @brief Copies the estimated parameters to the given array
   * @param paramArray, target array for copy, make sure to allocate enough space
   */
  void GetParameters( double* paramArray)
  {
    if( m_EstimatedParameters == NULL )
    {
      mitkThrow() << "No parameters were estimated yet, call Update() first.";
    }

    unsigned int dim = 12;
    if( !m_UseAffineTransform )
      dim = 6;

    for( unsigned int i=0; i<dim; i++)
    {
      *(paramArray+i) = m_EstimatedParameters[i];
    }
  }

  /**
   * @brief Retrieve the parameters of the last registration computed
   *
   * @return Parameters array ( as an itk::Array )
   */
  ParametersType GetLastRegistrationParameters()
  {
    if( m_EstimatedParameters == NULL )
    {
      mitkThrow() << "No parameters were estimated yet, call Update() first.";
    }

    unsigned int dim = 12;
    if( !m_UseAffineTransform )
      dim = 6;

    ParametersType params(dim);

    params.SetData( m_EstimatedParameters );
    return params;
  }

  /**
   * @brief Set the starting position of the registration
   *
   * @param params make sure the parameters have the same dimension as the selected transform type
   * otherwise the update call will throw an exception
   */
  void SetInitialParameters( ParametersType& params )
  {
    m_InitialParameters = params;
  }

  /**
   * @brief Control the interpolator used for resampling.
   *
   * The Windowed Sinc Interpolator is more time consuming, but the interpolation preserves more structure
   * in the data
   *
   * The class uses the
   *  - Linear interpolator on default ( flag = false )
   *  - WindowedSinc interpolator if called with true
   */
  void SetUseAdvancedInterpolation( bool flag)
  {
    m_UseWindowedSincInterpolator = flag;
  }

  /**
   * @brief Control the interpolator used for resampling.
   *
   * The class uses the
   *  - Linear interpolator on default ( flag = false )
   *  - Nearest neighbor interpolation if this is true
   *
   *  used to resample e.g. segmentations.
   */
  void SetUseNearestNeighborInterpolation( bool flag)
  {
    m_UseNearestNeighborInterpolator = flag;
  }

  /**
   * @brief Set if fixed image mask is used to exclude a region.
   * @param flag , true if mask is to be used, false if mask is to be ignored (default)
   */
  void SetUseFixedImageMask( bool flag)
  {
    m_UseMask = flag;
  }

  /**
   * @brief Returns the moving image transformed according to the estimated transformation and resampled
   * to the geometry of the fixed/resampling reference image
   *
   */
  mitk::Image::Pointer GetResampledMovingImage();

  /**
   * @brief Returns a provided moving image transformed according to the given transformation and resampled
   * to the geometry of the fixed/resampling reference image
   */
  mitk::Image::Pointer GetResampledMovingImage(mitk::Image::Pointer movingImage, double *transform );

  /**
   * @brief Get the rotation part of the transformation as a vnl_fixed_matrix<double, 3,3>
   *
   * It returns identity if the internal parameters are not-yet allocated ( i.e. the filter did not run yet )
   *
   * @return \sa TransformMatrixType
   */
  TransformMatrixType GetLastRotationMatrix();


protected:
  PyramidImageRegistrationMethod();

  ~PyramidImageRegistrationMethod();

  /** Fixed image, used as reference for registration */
  mitk::Image::Pointer m_FixedImage;

  /** Moving image, will be transformed */
  mitk::Image::Pointer m_MovingImage;

  mitk::Image::Pointer m_FixedImageMask;

  bool m_CrossModalityRegistration;

  bool m_UseAffineTransform;

  bool m_UseWindowedSincInterpolator;

  bool m_UseNearestNeighborInterpolator;

  bool m_UseMask;

  double* m_EstimatedParameters;

  ParametersType m_InitialParameters;

  /** Control the verbosity of the regsitistration output */
  bool m_Verbose;

  bool m_InitializeByGeometry;

  /**
   * @brief The method takes two itk::Images and performs a multi-scale registration on them
   *
   * Can be accessed by the AccessTwoImagesFixedDimensionByItk macro.
   *
   * @note Currently the access is currently reduced only for short and float ( the only types occuring with mitk::DiffusionImage ) and
   *       hence the method is accessed by means of the \sa AccessTwoImagesFixedDimensionTypeSubsetByItk macro.
   */
template <typename TPixel1, unsigned int VImageDimension1, typename TPixel2, unsigned int VImageDimension2>
  void RegisterTwoImagesV4(itk::Image<TPixel1, VImageDimension1>* itkImage1, itk::Image<TPixel2, VImageDimension2>* itkImage2);

  /**
   * @brief ResampleMitkImage applies the functionality of an the itk::ResampleImageFilter to the given
   *        mitk::Image.
   *
   * The API of the function is conform to the interface of \sa AccessByItk_1 macros.
   */
  template< typename TPixel, unsigned int VDimension>
  void ResampleMitkImage( itk::Image<TPixel, VDimension>* itkImage,
                          mitk::Image::Pointer& outputImage );

};

} // end namespace

#endif // MITKPYRAMIDIMAGEREGISTRATION_H
