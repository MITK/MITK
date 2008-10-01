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

#include "SampleApp.h"
#include "SampleAppConfig.h"

#include "mitkStatusBar.h"

#include "QmitkStdMultiWidget.h"
#include "QmitkSelectableGLWidget.h"
#include "QmitkCommonFunctionality.h"

#include "QmitkFunctionality.h"
#include "QmitkFunctionalityFactory.h"
#include "QmitkFctMediator.h"
#include "QmitkControlsRightFctLayoutTemplate.h"
#include "QmitkControlsLeftFctLayoutTemplate.h"

#include "QmitkSliderDialogBar.h"
#include "QmitkStandardViewsDialogBar.h"
#include "QmitkStereoOptionsDialogBar.h"

#include <qapplication.h>

#include <qlayout.h>

#include <qsplitter.h>

void RegisterFunctionalities();

SampleApp::SampleApp( QWidget* parent, const char* name, WFlags fl, const char* testingParameter ):
QmitkMainTemplate( parent, name, fl ), m_ControlsLeft ( false ), m_TestingParameter(testingParameter)
{
  RegisterFunctionalities();
  this->setCaption("MITK Application");
  std::cout << "Instantiating new SampleApp..." << std::endl;

  QmitkMainTemplate::CheckStatemachineDebugMode();

  QmitkMainTemplate::Initialize();

  parseCommandLine();
}
/*
*  Destroys the object and frees any allocated resources
*/
SampleApp::~SampleApp()
{
  // Make sure that all pending events are processed to avoid dangling
  // RenderingRequestEvents that would otherwise lead to an application
  // crash
  while ( qApp->hasPendingEvents() )
  {
    qApp->processEvents();
  }
  // no need to delete child widgets, Qt does it all for us
}

void SampleApp::InitializeFunctionality()
{
  //create and add functionalities. Functionalities are also invisible objects, which can be asked to 
  //create their different parts (main widget, control widget, toolbutton for selection).
  mitk::DataTreePreOrderIterator iterator(m_Tree);

  QmitkFunctionalityFactory& qff = QmitkFunctionalityFactory::GetInstance();
  if (!m_TestingParameter) 
  {
    // add separator
    qfm->AddSeparator();

    // add dialog bars
    QmitkSliderDialogBar *sliderDialogBar = new QmitkSliderDialogBar( m_MultiWidget, "slicerbar", m_MultiWidget );
    qfm->AddDialogBar( sliderDialogBar );
    QmitkStandardViewsDialogBar *standardViewsDialogBar = new QmitkStandardViewsDialogBar( m_MultiWidget, "viewsbar", m_MultiWidget );
    qfm->AddDialogBar( standardViewsDialogBar );
    QmitkStereoOptionsDialogBar *stereoOptionsDialogBar = new QmitkStereoOptionsDialogBar( m_MultiWidget, "stereobar", m_MultiWidget );
    qfm->AddDialogBar( stereoOptionsDialogBar );

    // initialize toolbars
    qfm->InitToolbars();

    // add MITK_STARTUP_FUNCTIONALITY if requested
    QmitkFunctionalityFactory::CreateFunctionalityPtr createFunction = qff.GetCreateFunctionalityPtrByName(MITK_STARTUP_FUNCTIONALITY);
    if (createFunction) 
    {
      QmitkFunctionality* functionalityInstance = createFunction(qfm,m_MultiWidget,&iterator);
      qfm->AddFunctionality(functionalityInstance);
    }

    // add all known functionalities
    for (QmitkFunctionalityFactory::CreateFunctionalityPtrMap::const_iterator it = qff.GetCreateFunctionalityPtrMap().begin() ; it != qff.GetCreateFunctionalityPtrMap().end(); it++)
    {
      if ( qfm->GetFunctionalityByName( (*it).first.c_str() ) == NULL )
      {
        QmitkFunctionality* functionalityInstance = ((*it).second)(qfm,m_MultiWidget,&iterator);
        qfm->AddFunctionality(functionalityInstance);
      }
    }
  }
  else
  {
    //
    QmitkFunctionalityFactory::CreateFunctionalityPtr createFunction = qff.GetCreateFunctionalityPtrByName(m_TestingParameter);
    if (createFunction) 
    {
      QmitkFunctionality* functionalityInstance = createFunction(qfm,m_MultiWidget,&iterator);
      qfm->AddFunctionality(functionalityInstance);
    }
  }

  mitk::StatusBar::GetInstance()->DisplayText("Functionalities added",3000);
}

void SampleApp::SetDefaultWidgetSize()
{
  //B/ Setup MainApp Widget size (default: maximized) ////
  mitk::Point3dProperty* sizeProp = dynamic_cast<mitk::Point3dProperty*>(this->m_Options->GetProperty("Startup window size"));
  if(sizeProp)
  {
    mitk::Point3D p = sizeProp->GetValue();
    if(p[0] == 0.0 && p[1] == 0.0)
      this->showMaximized();
    else
    {
      this->resize((int)sizeProp->GetValue()[0], (int) sizeProp->GetValue()[1]);
      this->show();
    }
  }
  else
    this->showMaximized();

  this->RaiseDialogBars();

  //B/ Setup  MultiWidget size (default: 2/3 of total MainApp width) ////
  mitk::Point3dProperty* splitterSizeProp = dynamic_cast<mitk::Point3dProperty*>(this->m_Options->GetProperty("Main Splitter ratio"));

  QmitkControlsRightFctLayoutTemplate* fctwidget = (QmitkControlsRightFctLayoutTemplate*) this->centralWidget();
  if(fctwidget)
  {
    QValueList<int> i;

    if(splitterSizeProp)
    {
      mitk::Point3D p = splitterSizeProp->GetValue();
      if(p[0] == 0.0 && p[1] == 0.0)
      {
        i.push_back(this->width()/3*2);
        i.push_back(this->width()/3*1);
      }
      else
      {
        i.push_back((int)p[0]);
        i.push_back((int)p[1]);
      }
    }
    else
    {
      i.push_back(this->width()/3*2);
      i.push_back(this->width()/3*1);
    }
    fctwidget->MainSplitter->setSizes(i);
    this->repaint();
  }
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

