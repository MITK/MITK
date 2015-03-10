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

#ifndef QmitkTranslationTransformViewWidgetHIncluded
#define QmitkTranslationTransformViewWidgetHIncluded

#include "ui_QmitkTranslationTransformControls.h"
#include "MitkRigidRegistrationUIExports.h"
#include "QmitkRigidRegistrationTransformsGUIBase.h"
#include <itkImage.h>

/*!
* \brief Widget for rigid registration
*
* Displays options for rigid registration.
*/
class MITKRIGIDREGISTRATIONUI_EXPORT QmitkTranslationTransformView : public QmitkRigidRegistrationTransformsGUIBase
{

public:

  QmitkTranslationTransformView( QWidget* parent = 0, Qt::WindowFlags f = 0 );
  ~QmitkTranslationTransformView();

  virtual mitk::TransformParameters::TransformType GetTransformType();

  virtual itk::Object::Pointer GetTransform();

  virtual itk::Array<double> GetTransformParameters();

  virtual void SetTransformParameters(itk::Array<double> transformValues);

  virtual QString GetName();

  virtual void SetupUI(QWidget* parent);

  virtual itk::Array<double> GetScales();

  virtual vtkTransform* Transform(vtkMatrix4x4* vtkmatrix, vtkTransform* vtktransform, itk::Array<double> transformParams);

  virtual int GetNumberOfTransformParameters();



private:

  template < class TPixelType, unsigned int VImageDimension >
  itk::Object::Pointer GetTransform2(itk::Image<TPixelType, VImageDimension>* itkImage1);

protected:

  Ui::QmitkTranslationTransformControls m_Controls;

  itk::Object::Pointer m_TransformObject;

};

#endif
