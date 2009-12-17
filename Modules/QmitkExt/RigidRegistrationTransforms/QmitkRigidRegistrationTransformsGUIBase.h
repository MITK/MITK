/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile: mitkPropertyManager.cpp,v $
Language:  C++
Date:      $Date$
Version:   $Revision: 1.12 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef QmitkRigidRegistrationTransformsGUIBaseH
#define QmitkRigidRegistrationTransformsGUIBaseH

#include <QWidget>
#include <itkArray.h>
#include <itkObject.h>
#include "mitkImage.h"
#include <QString>
#include <vtkTransform.h>
#include <vtkMatrix4x4.h>

/*!
* \brief Widget for rigid registration
*
* Displays options for rigid registration.
*/
class QMITKEXT_EXPORT QmitkRigidRegistrationTransformsGUIBase : public QWidget
{

public:

  QmitkRigidRegistrationTransformsGUIBase(QWidget* parent = 0, Qt::WindowFlags f = 0);
  ~QmitkRigidRegistrationTransformsGUIBase();

  virtual itk::Object::Pointer GetTransform() = 0;

  virtual itk::Array<double> GetTransformParameters() = 0;

  virtual void SetTransformParameters(itk::Array<double> transformValues) = 0;

  virtual int GetNumberOfTransformParameters() = 0;

  virtual itk::Array<double> GetScales() = 0;

  virtual QString GetName() = 0;

  virtual void SetupUI(QWidget* parent) = 0;

  virtual vtkTransform* Transform(vtkMatrix4x4* vtkmatrix, vtkTransform* vtktransform, itk::Array<double> transformParams) = 0;

  void SetFixedImage(mitk::Image::Pointer fixedImage);

  void SetMovingImage(mitk::Image::Pointer movingImage);


protected:

  mitk::Image::Pointer m_FixedImage;
  mitk::Image::Pointer m_MovingImage;

};

#endif //QmitkRigidRegistrationTransformsGUIBaseH
