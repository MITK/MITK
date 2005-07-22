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

QmitkFunctionalityTemplate::QmitkFunctionalityTemplate(QObject *parent, const char *name, QmitkStdMultiWidget *mitkStdMultiWidget, mitk::DataTreeIteratorBase* it) 
: QmitkFunctionality(parent, name, it) ,m_Controls(NULL), m_MultiWidget(mitkStdMultiWidget)
{
  SetAvailability(true);
}

QmitkFunctionalityTemplate::~QmitkFunctionalityTemplate()
{
}

QWidget * QmitkFunctionalityTemplate::CreateMainWidget(QWidget *parent)
{  
  return NULL;
}

QWidget * QmitkFunctionalityTemplate::CreateControlWidget(QWidget *parent)
{  
  if (m_Controls == NULL)  
  {    
    m_Controls = new QmitkFunctionalityTemplateControls(parent);          
  }  
  return m_Controls;
}

void QmitkFunctionalityTemplate::CreateConnections()
{  
  if ( m_Controls )   
  {    
    //connect( ... );  
  }
}

QAction * QmitkFunctionalityTemplate::CreateAction(QActionGroup *parent)
{  
  QAction* action;  
  action = new QAction( tr( "ToolTip" ), QPixmap((const char**)icon_xpm), tr( "MenueEintrag" ), CTRL + Key_G, parent, "FunctionalityTemplate" );
  return action;
}

void QmitkFunctionalityTemplate::TreeChanged()
{
}

void QmitkFunctionalityTemplate::Activated()
{  
  QmitkFunctionality::Activated();
}
