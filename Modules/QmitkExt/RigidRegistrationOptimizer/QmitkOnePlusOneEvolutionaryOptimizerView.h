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

#ifndef QmitkOnePlusOneEvolutionaryOptimizerViewWidgetHIncluded
#define QmitkOnePlusOneEvolutionaryOptimizerViewWidgetHIncluded

#include "ui_QmitkOnePlusOneEvolutionaryOptimizerControls.h"
#include "QmitkExtExports.h"
#include "QmitkRigidRegistrationOptimizerGUIBase.h"
#include <itkArray.h>
#include <itkObject.h>
#include <itkImage.h>

/*!
* \brief Widget for rigid registration
*
* Displays options for rigid registration.
*/
class QmitkExt_EXPORT QmitkOnePlusOneEvolutionaryOptimizerView : public QmitkRigidRegistrationOptimizerGUIBase
{

public:

  QmitkOnePlusOneEvolutionaryOptimizerView( QWidget* parent = 0, Qt::WindowFlags f = 0 );
  ~QmitkOnePlusOneEvolutionaryOptimizerView();


  virtual itk::Object::Pointer GetOptimizer();

  virtual itk::Array<double> GetOptimizerParameters();

  virtual void SetOptimizerParameters(itk::Array<double> metricValues);

  virtual void SetNumberOfTransformParameters(int transformParameters);

  virtual QString GetName();

  virtual void SetupUI(QWidget* parent);

protected:

  Ui::QmitkOnePlusOneEvolutionaryOptimizerControls m_Controls;

  int m_NumberTransformParameters;

};

#endif
