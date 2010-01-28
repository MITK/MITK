/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile: mitkImageToImageFilter.h,v $
Language:  C++
Date:      $Date$
Version:   $Revision: 11215 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef MITKTRANSFORMPARAMETERS_H
#define MITKTRANSFORMPARAMETERS_H

#include <itkObjectFactory.h>
#include "MitkExtExports.h"
#include <itkArray.h>
#include "mitkCommon.h"

namespace mitk {


  /*!
  \brief This class is used to hold all transform parameters needed for a rigid registration process.  

  To use the rigid registration framework you have to create an instance of this class and fill it with the parameters
  belonging to the selected transformation. To let the rigid registration work properly, this instance has to be given 
  to mitkImageRegistrationMethod before calling the update() method in mitkImageRegistrationMethod.

  Also instances of the classes mitkMetricParameters and mitkOptimizerParameters have to be set in mitkImageRegistrationMethod 
  before calling the update() method.


  \ingroup RigidRegistration

  \author Daniel Stein
  */

  class MitkExt_EXPORT TransformParameters : public itk::Object
  {
  public:
    mitkClassMacro(TransformParameters, ::itk::Object);
    itkNewMacro(Self);

    /**
      \brief Unique integer value for every transform.
    */
    enum TransformType {
      TRANSLATIONTRANSFORM = 0,
      SCALETRANSFORM = 1,
      SCALELOGARITHMICTRANSFORM = 2,
      AFFINETRANSFORM = 3,
      FIXEDCENTEROFROTATIONAFFINETRANSFORM = 4,
      RIGID3DTRANSFORM = 5,
      EULER3DTRANSFORM = 6,
      CENTEREDEULER3DTRANSFORM = 7,
      QUATERNIONRIGIDTRANSFORM = 8,
      VERSORTRANSFORM = 9,
      VERSORRIGID3DTRANSFORM = 10,
      SCALESKEWVERSOR3DTRANSFORM = 11,
      SIMILARITY3DTRANSFORM = 12,
      RIGID2DTRANSFORM = 13,
      CENTEREDRIGID2DTRANSFORM = 14,
      EULER2DTRANSFORM = 15,
      SIMILARITY2DTRANSFORM = 16,
      CENTEREDSIMILARITY2DTRANSFORM = 17
    };

    /**
      \brief Sets the transform used for registration by its unique integer value.
    */
    itkSetMacro( Transform, int );
    /**
      \brief Returns the transform used for registration by its unique integer value.
    */
    itkGetMacro( Transform, int );

    /**
      \brief Sets an array that holds the magnitudes of changes for the transforms degrees of freedom.
      
      The optimizer changes the transforms values corresponding to this array values.  
    */
    void SetScales(itk::Array<double> scales);
    /**
      \brief Returns an array that holds the magnitudes of changes for the transforms degrees of freedom.

      The optimizer changes the transforms values corresponding to this array values.  
    */
    itk::Array<double> GetScales();

    /**
      \brief Sets an array that holds the initial transform parameters. 
    */
    void SetInitialParameters(itk::Array<double> initialParameters);
    /**
      \brief Returns an array that holds the initial transform parameters. 
    */
    itk::Array<double> GetInitialParameters();

    /**
    \brief Sets whether a transform initializer will be used.

    True = initializer is used, initializer is not used.
    */
    itkSetMacro( TransformInitializerOn, bool );
    /**
    \brief Returns whether a transform initializer will be used.

    True = initializer is used, false = initializer is not used.
    */
    itkGetMacro( TransformInitializerOn, bool );

    /**
    \brief Sets whether the transform initializer will be used to align the images moments or their centers.

    True = image moments will be aligned, false = image centers will be aligned.
    */
    itkSetMacro( MomentsOn, bool );
    /**
    \brief Returns whether the transform initializer will be used to align the images moments or their centers.

    True = image moments will be aligned, false = image centers will be aligned.
    */
    itkGetMacro( MomentsOn, bool );

    /**
    \brief Sets whether the optimizer scales will be used to let the optimizer know about different magnitudes for
    the transforms degrees of freedom.

    True = optimizer scales will be used, false = optimizer scales will not be used.
    */
    itkSetMacro( UseOptimizerScales, bool );
    /**
    \brief Returns whether the optimizer scales will be used to let the optimizer know about different magnitudes for
    the transforms degrees of freedom.

    True = optimizer scales will be used, false = optimizer scales will not be used.
    */
    itkGetMacro( UseOptimizerScales, bool );

    /**
    \brief Sets the initial angle for transforms.
    */
    itkSetMacro( Angle, float );
    /**
    \brief Returns the initial angle for transforms.
    */
    itkGetMacro( Angle, float );

    /**
    \brief Sets the transforms initial scale.
    */
    itkSetMacro( Scale, float );
    /**
    \brief Returns the transforms initial scale.
    */
    itkGetMacro( Scale, float );

    /**
    \brief This setter is used by the mitkTransformFactory to set the transforms initial center in X direction.
    */
    itkSetMacro( TransformCenterX, double );
    /**
    \brief Returns the transforms initial center in X direction.
    */
    itkGetMacro( TransformCenterX, double );

    /**
    \brief This setter is used by the mitkTransformFactory to set the transforms initial center in Y direction.
    */
    itkSetMacro( TransformCenterY, double );
    /**
    \brief Returns the transforms initial center in Y direction.
    */
    itkGetMacro( TransformCenterY, double );

    /**
    \brief This setter is used by the mitkTransformFactory to set the transforms initial center in Z direction.
    */
    itkSetMacro( TransformCenterZ, double );
    /**
    \brief Returns the transforms initial center in Z direction.
    */
    itkGetMacro( TransformCenterZ, double );

  protected:

    TransformParameters();
    ~TransformParameters() {};

    int m_Transform;
    float m_Angle;
    float m_Scale;
    bool m_TransformInitializerOn;
    bool m_MomentsOn;
    bool m_UseOptimizerScales;
    double m_TransformCenterX;
    double m_TransformCenterY;
    double m_TransformCenterZ;
    itk::Array<double> m_Scales;
    itk::Array<double> m_InitialParameters;
  };

} // namespace mitk

#endif // MITKTRANSFORMPARAMETERS_H
