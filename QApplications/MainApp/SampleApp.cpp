/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medickal and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/ for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "SampleApp.h"

#include <QmitkStdMultiWidget.h>
#include <QmitkSelectableGLWidget.h>
#include <QmitkCommonFunctionality.h>

#include <mitkStatusBar.h>

#include <QmitkFunctionality.h>
#include <QmitkFunctionalityFactory.h>
#include <QmitkFctMediator.h>
#include <QmitkControlsRightFctLayoutTemplate.h>
#include <QmitkControlsLeftFctLayoutTemplate.h>

#include <qlayout.h>

void RegisterFunctionalities();

SampleApp::SampleApp( QWidget* parent, const char* name, WFlags fl, const char* testingParameter ):
QmitkMainTemplate( parent, name, fl ), m_ControlsLeft ( false ), m_TestingParameter(testingParameter)
{
  RegisterFunctionalities();
  this->setCaption("MITK Application");
  std::cout << "Instantiating new SampleApp..." << std::endl;

  QmitkMainTemplate::Initialize();

  parseCommandLine();

  // TODO: Move this slot to MainApp! Or find a good reason to keep it here
  connect (this, SIGNAL(ShowWidgetPlanesToggled(bool)), this, SLOT(SetWidgetPlanesEnabled(bool)));
  resize(1024,768);

}
/*
*  Destroys the object and frees any allocated resources
*/
SampleApp::~SampleApp()
{
  // no need to delete child widgets, Qt does it all for us
}

void SampleApp::InitializeFunctionality()
{
  m_MultiWidget->mitkWidget4->GetRenderer()->SetMapperID(2);
  //create and add functionalities. Functionalities are also invisible objects, which can be asked to 
  //create their different parts (main widget, control widget, toolbutton for selection).
  mitk::DataTreePreOrderIterator iterator(m_Tree);

  QmitkFunctionalityFactory& qff = QmitkFunctionalityFactory::GetInstance();
  for (std::list<QmitkFunctionalityFactory::CreateFunctionalityPtr>::const_iterator it = qff.GetCreateFunctionalityPtrList().begin() ; it != qff.GetCreateFunctionalityPtrList().end(); it++) {
    QmitkFunctionality* functionalityInstance = (*it)(qfm,m_MultiWidget,&iterator);
    if (!m_TestingParameter) {
      qfm->AddFunctionality(functionalityInstance); 
    } else if (strcmp(m_TestingParameter,functionalityInstance->name()) == 0) {
      std::cout << "adding selected " << functionalityInstance->name() << std::endl;
      qfm->AddFunctionality(functionalityInstance); 
    } else {
      std::cout << "rejecting functionality " << functionalityInstance->name() << std::endl;
    }
  }

  mitk::StatusBar::GetInstance()->DisplayText("Functionalities added",3000);
}

void SampleApp::InitializeQfm()
{
  if (m_ControlsLeft) {
    //create an QmitkFctMediator. This is an invisible object that controls, manages and mediates functionalities
    qfm=new QmitkFctMediator(this);

    //create an QmitkButtonFctLayoutTemplate. This is an simple example for an layout of the different widgets, of which
    //a functionality and the management consists: the main widget, the control widget and a menu for selecting the
    //active functionality.
    QmitkControlsLeftFctLayoutTemplate* layoutTemplate=new QmitkControlsLeftFctLayoutTemplate(this, "LayoutTemplate");
    setCentralWidget(layoutTemplate);

    //let the QmitkFctMediator know about the layout. This includes the toolbar and the layoutTemplate.
    qfm->Initialize(this);
  }
  else
  {
    QmitkMainTemplate::InitializeQfm();
  }
}

void SampleApp::SetWidgetPlanesEnabled(bool enable)
{
  CommonFunctionality::SetWidgetPlanesEnabled(m_Tree, enable);
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}
