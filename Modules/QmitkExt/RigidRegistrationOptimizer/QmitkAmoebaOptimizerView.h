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

#ifndef QmitkAmoebaOptimizerViewWidgetHIncluded
#define QmitkAmoebaOptimizerViewWidgetHIncluded

#include "ui_QmitkAmoebaOptimizerControls.h"
#include "QmitkExtExports.h"
#include "QmitkRigidRegistrationOptimizerGUIBase.h"
#include <itkArray.h>
#include <itkObject.h>
#include <itkImage.h>

class QLineEdit;

/*!
* \brief Widget for rigid registration
*
* Displays options for rigid registration.
*/
class QmitkExt_EXPORT QmitkAmoebaOptimizerView : public QmitkRigidRegistrationOptimizerGUIBase
{

public:

  QmitkAmoebaOptimizerView( QWidget* parent = 0, Qt::WindowFlags f = 0 );
  ~QmitkAmoebaOptimizerView();


  virtual itk::Object::Pointer GetOptimizer();

  virtual itk::Array<double> GetOptimizerParameters();

  virtual void SetOptimizerParameters(itk::Array<double> metricValues);

  virtual void SetNumberOfTransformParameters(int transformParameters);

  virtual QString GetName();

  virtual void SetupUI(QWidget* parent);

private:

  void ShowSimplexDelta();

protected:

  Ui::QmitkAmoebaOptimizerControls m_Controls;

  int m_NumberTransformParameters;

  std::list<QLineEdit*> simplexDeltaLineEdits;
  std::list<QLabel*> simplexDeltaLabels;

};

#endif
