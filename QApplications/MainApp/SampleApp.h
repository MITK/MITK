/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/ for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "QmitkMainTemplate.h"

/*!
\brief This application demonstrates how to code a new MITK application 

*/
class SampleApp : public QmitkMainTemplate
{
  Q_OBJECT

  bool m_ControlsLeft;
  const char* m_TestingParameter;

public:
  /*!
  \brief Default constructor
  \param testingParameter something that was passed after '-testing' on the command line.
  */
  SampleApp( QWidget* parent = 0, const char* name = 0, WFlags fl = WType_TopLevel, const char* testingParameter=NULL );

  /*!
  \brief Default destructor
  */
  virtual ~SampleApp();

  /*!
  \brief Overrides initializeFunctionality() from QmitKMainTemplate

  In this method the user adds the application specific functionalities
  to the application template
  */
  virtual void InitializeFunctionality();

  virtual void InitializeQfm();

  virtual void SetDefaultWidgetSize();
};

