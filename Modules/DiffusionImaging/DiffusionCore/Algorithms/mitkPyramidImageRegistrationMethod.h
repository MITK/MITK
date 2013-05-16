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

#include <DiffusionCoreExports.h>

#include <itkObject.h>

#include "mitkImage.h"
#include "mitkBaseProcess.h"


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
 * The error metric is internally chosen based on the selected task type ( @sa SetCrossModalityOn )
 * It uses
 *   - MattesMutualInformation for CrossModality=on ( default ) and
 *   - NormalizedCorrelation for CrossModality=off.
 */
class DiffusionCore_EXPORT PyramidImageRegistrationMethod :
    public itk::Object
{
public:

  /** Typedefs */
  mitkClassMacro(PyramidImageRegistrationMethod, itk::Object)

  /** Smart pointer support */
  itkNewMacro(Self)

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

  void Update();


protected:
  PyramidImageRegistrationMethod();

  ~PyramidImageRegistrationMethod();

  /** Fixed image, used as reference for registration */
  mitk::Image::Pointer m_FixedImage;

  /** Moving image, will be transformed */
  mitk::Image::Pointer m_MovingImage;

  bool m_CrossModalityRegistration;

  bool m_UseAffineTransform;

};

} // end namespace

#endif // MITKPYRAMIDIMAGEREGISTRATION_H
