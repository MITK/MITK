/*=========================================================================
Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date$
Version:   $Revision$
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
=========================================================================*/
#include "QmitkFunctionalityTemplate.h"
#include "QmitkFunctionalityTemplateControls.h"
#include <qaction.h>
#include "icon.xpm"
#include <mitkEventMapper.h>
#include <mitkGlobalInteraction.h>
#include <mitkBaseRenderer.h>
#include "QmitkRenderWindow.h"
#include "QmitkSelectableGLWidget.h"
#include "QmitkStdMultiWidget.h"
#include <QmitkStepperAdapter.h>
#include "qpushbutton.h"
// for stereo setting
#include <mitkOpenGLRenderer.h>
#include <mitkVtkRenderWindow.h>
#include <vtkRenderWindow.h>
// for zoom/pan
#include <mitkDisplayCoordinateOperation.h>
#include <mitkDisplayVectorInteractor.h>
#include <mitkDisplayInteractor.h>
#include <mitkInteractionConst.h>


QmitkFunctionalityTemplate::QmitkFunctionalityTemplate(QObject *parent, const char *name, QmitkStdMultiWidget *mitkStdMultiWidget, mitk::DataTreeIteratorBase* it) : QmitkFunctionality(parent, name, it) ,
controls(NULL), multiWidget(mitkStdMultiWidget)
{
    setAvailability(true);
}
QmitkFunctionalityTemplate::~QmitkFunctionalityTemplate()
{
}
QWidget * QmitkFunctionalityTemplate::createMainWidget(QWidget *parent)
{
		return NULL;
}
QWidget * QmitkFunctionalityTemplate::createControlWidget(QWidget *parent)
{
    if (controls == NULL)
    {
        controls = new QmitkFunctionalityTemplateControls(parent);        
    }
    return controls;
}

void QmitkFunctionalityTemplate::createConnections()
{
  if ( controls ) 
  {
    //connect( ... );
  }
}
QAction * QmitkFunctionalityTemplate::createAction(QActionGroup *parent)
{
    QAction* action;
    action = new QAction( tr( "Functionality Template" ), QPixmap((const char**)icon_xpm), tr( "&Functionality Template" ), CTRL + Key_G, parent, "functionality template" );
    return action;
}

void QmitkFunctionalityTemplate::treeChanged()
{
}
void QmitkFunctionalityTemplate::activated()
{
  QmitkFunctionality::activated();
}
