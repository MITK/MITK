/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
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

#ifndef QmitkScaleLogarithmicTransformViewWidgetHIncluded
#define QmitkScaleLogarithmicTransformViewWidgetHIncluded

#include "ui_QmitkScaleLogarithmicTransformControls.h"
#include "MitkRigidRegistrationUIExports.h"
#include "QmitkRigidRegistrationTransformsGUIBase.h"
#include <itkImage.h>

/*!
* \brief Widget for rigid registration
*
* Displays options for rigid registration.
*/
class MITK_RIGIDREGISTRATION_UI_EXPORT QmitkScaleLogarithmicTransformView : public QmitkRigidRegistrationTransformsGUIBase
{

public:

  QmitkScaleLogarithmicTransformView( QWidget* parent = 0, Qt::WindowFlags f = 0 );
  ~QmitkScaleLogarithmicTransformView();


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

  Ui::QmitkScaleLogarithmicTransformControls m_Controls;

  itk::Object::Pointer m_TransformObject;

};

#endif
