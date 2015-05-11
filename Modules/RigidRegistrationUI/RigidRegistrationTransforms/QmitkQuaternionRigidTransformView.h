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

#ifndef QmitkQuaternionRigidTransformViewWidgetHIncluded
#define QmitkQuaternionRigidTransformViewWidgetHIncluded

#include "ui_QmitkQuaternionRigidTransformControls.h"
#include "MitkRigidRegistrationUIExports.h"
#include "QmitkRigidRegistrationTransformsGUIBase.h"
#include <itkImage.h>

/*!
* \brief Widget for rigid registration
*
* Displays options for rigid registration.
*/
class MITKRIGIDREGISTRATIONUI_EXPORT QmitkQuaternionRigidTransformView : public QmitkRigidRegistrationTransformsGUIBase
{

public:

  QmitkQuaternionRigidTransformView( QWidget* parent = nullptr, Qt::WindowFlags f = nullptr );
  ~QmitkQuaternionRigidTransformView();

  virtual mitk::TransformParameters::TransformType GetTransformType() override;

  virtual itk::Object::Pointer GetTransform() override;

  virtual itk::Array<double> GetTransformParameters() override;

  virtual void SetTransformParameters(itk::Array<double> transformValues) override;

  virtual QString GetName() override;

  virtual void SetupUI(QWidget* parent) override;

  virtual itk::Array<double> GetScales() override;

  virtual vtkTransform* Transform(vtkMatrix4x4* vtkmatrix, vtkTransform* vtktransform, itk::Array<double> transformParams) override;

  virtual int GetNumberOfTransformParameters() override;

private:

  template < class TPixelType, unsigned int VImageDimension >
  itk::Object::Pointer GetTransform2(itk::Image<TPixelType, VImageDimension>* itkImage1);

protected:

  Ui::QmitkQuaternionRigidTransformControls m_Controls;

  itk::Object::Pointer m_TransformObject;

  double m_CenterX;
  double m_CenterY;
  double m_CenterZ;

};

#endif
