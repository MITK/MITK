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
#include <itkArray.h>
#include "mitkCommon.h"

namespace mitk {

  class MITK_CORE_EXPORT TransformParameters : public itk::Object
  {
  public:
    mitkClassMacro(TransformParameters, ::itk::Object);
    itkNewMacro(Self);

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

    itkSetMacro( Transform, int );
    itkGetMacro( Transform, int );

    void SetScales(itk::Array<double> scales);
    itk::Array<double> GetScales();

    void SetInitialParameters(itk::Array<double> initialParameters);
    itk::Array<double> GetInitialParameters();

    itkSetMacro( TransformInitializerOn, bool );
    itkGetMacro( TransformInitializerOn, bool );

    itkSetMacro( MomentsOn, bool );
    itkGetMacro( MomentsOn, bool );

    itkSetMacro( UseOptimizerScales, bool );
    itkGetMacro( UseOptimizerScales, bool );

    itkSetMacro( Angle, float );
    itkGetMacro( Angle, float );

    itkSetMacro( Scale, float );
    itkGetMacro( Scale, float );

    itkSetMacro( TransformCenterX, double );
    itkGetMacro( TransformCenterX, double );

    itkSetMacro( TransformCenterY, double );
    itkGetMacro( TransformCenterY, double );

    itkSetMacro( TransformCenterZ, double );
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
