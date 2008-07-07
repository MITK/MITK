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

#include <itkObject.h>
#include "itkArray.h"
#include "mitkCommon.h"

namespace mitk {

  class MITK_CORE_EXPORT TransformParameters : public itk::Object
  {
  public:
    itkTypeMacro(TransformParameters, itk::Object);

    static TransformParameters* GetInstance(); //singleton

    static const int TRANSLATIONTRANSFORM = 0;
    static const int SCALETRANSFORM = 1;
    static const int SCALELOGARITHMICTRANSFORM = 2;
    static const int AFFINETRANSFORM = 3;
    static const int FIXEDCENTEROFROTATIONAFFINETRANSFORM = 4;
    static const int RIGID3DTRANSFORM = 5;
    static const int EULER3DTRANSFORM = 6;
    static const int CENTEREDEULER3DTRANSFORM = 7;
    static const int QUATERNIONRIGIDTRANSFORM = 8;
    static const int VERSORTRANSFORM = 9;
    static const int VERSORRIGID3DTRANSFORM = 10;
    static const int SCALESKEWVERSOR3DTRANSFORM = 11;
    static const int SIMILARITY3DTRANSFORM = 12;
    static const int RIGID2DTRANSFORM = 13;
    static const int CENTEREDRIGID2DTRANSFORM = 14;
    static const int EULER2DTRANSFORM = 15;
    static const int SIMILARITY2DTRANSFORM = 16;
    static const int CENTEREDSIMILARITY2DTRANSFORM = 17;

    void SetTransform(int transform);
    int GetTransform();

    void SetScales(itk::Array<double> scales);
    itk::Array<double> GetScales();

    void SetInitialParameters(itk::Array<double> initialParameters);
    itk::Array<double> GetInitialParameters();

    void TransformInitializerOn(bool initializer);
    bool GetTransformInitializerOn();

    void MomentsOn(bool moments);
    bool GetMomentsOn();

    void UseOptimizerScales(bool useScales);
    bool GetUseOptimizerScales();

    void SetAngle(float angle);
    float GetAngle();

    void SetScale(float scale);
    float GetScale();

    void SetTransformCenterX(double centerX);
    double GetTransformCenterX();

    void SetTransformCenterY(double centerY);
    double GetTransformCenterY();

    void SetTransformCenterZ(double centerZ);
    double GetTransformCenterZ();

  protected:
    TransformParameters(); // hidden, access through GetInstance()
    ~TransformParameters();
    static TransformParameters* m_Instance;
    int m_Transform;
    float m_Angle;
    float m_Scale;
    bool m_Initializer;
    bool m_Moments;
    bool m_UseOptimizerScales;
    itk::Array<double> m_Scales;
    itk::Array<double> m_InitialParameters;
    double m_TransformCenterX;
    double m_TransformCenterY;
    double m_TransformCenterZ;
  };

} // namespace mitk

#endif // MITKTRANSFORMPARAMETERS_H
