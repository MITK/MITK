/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date$
Version:   $Revision: 9502 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#include "mitkTransformParameters.h"


namespace mitk {

  TransformParameters* TransformParameters::m_Instance = NULL;

  /**
   * Get the instance of this TransformParameters
   */
  TransformParameters* TransformParameters::GetInstance()
  {
    if (m_Instance == NULL)
    {
      m_Instance = new TransformParameters();
    }
    return m_Instance;
  }

  TransformParameters::TransformParameters()
  {
    m_Transform = TRANSLATIONTRANSFORM;
    m_Initializer = true;
    m_Moments = true;
    m_UseOptimizerScales = true;
    m_Angle = 0.0;
    m_Scale = 1.0;
    m_InitialParameters.SetSize(16);
    m_InitialParameters.Fill(0.0);
    m_Scales.SetSize(16);
    m_Scales.Fill(1.0);
    m_TransformCenterX = 0.0;
    m_TransformCenterY = 0.0;
    m_TransformCenterZ = 0.0;
  }

  TransformParameters::~TransformParameters()
  {
  }

  void TransformParameters::SetTransform(int transform)
  {
	  m_Transform = transform;
  }

  int TransformParameters::GetTransform()
  {
	  return m_Transform;
  }
  
  void TransformParameters::SetScales(itk::Array<double> scales)
  {
    m_Scales = scales;
  }

  itk::Array<double> TransformParameters::GetScales()
  {
    return m_Scales;
  }

  void TransformParameters::SetInitialParameters(itk::Array<double> initialParameters)
  {
    m_InitialParameters = initialParameters;
  }

  itk::Array<double> TransformParameters::GetInitialParameters()
  {
    return m_InitialParameters;
  }

  void TransformParameters::TransformInitializerOn(bool initializer)
  {
    m_Initializer = initializer;
  }

  bool TransformParameters::GetTransformInitializerOn()
  {
    return m_Initializer;
  }

  void TransformParameters::MomentsOn(bool moments)
  {
    m_Moments = moments;
  }

  bool TransformParameters::GetMomentsOn()
  {
    return m_Moments;
  }

  void TransformParameters::UseOptimizerScales(bool useScales)
  {
    m_UseOptimizerScales = useScales;
  }

  bool TransformParameters::GetUseOptimizerScales()
  {
    return m_UseOptimizerScales;
  }

  void TransformParameters::SetAngle(float angle)
  {
    m_Angle = angle;
  }

  float TransformParameters::GetAngle()
  {
    return m_Angle;
  }

  void TransformParameters::SetScale(float scale)
  {
    m_Scale = scale;
  }

  float TransformParameters::GetScale()
  {
    return m_Scale;
  }

  void TransformParameters::SetTransformCenterX(double centerX)
  {
    m_TransformCenterX = centerX;
  }

  double TransformParameters::GetTransformCenterX()
  {
    return m_TransformCenterX;
  }

  void TransformParameters::SetTransformCenterY(double centerY)
  {
    m_TransformCenterY = centerY;
  }

  double TransformParameters::GetTransformCenterY()
  {
    return m_TransformCenterY;
  }

  void TransformParameters::SetTransformCenterZ(double centerZ)
  {
    m_TransformCenterZ = centerZ;
  }

  double TransformParameters::GetTransformCenterZ()
  {
    return m_TransformCenterZ;
  }
} // namespace mitk
