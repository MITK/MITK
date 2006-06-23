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

#include "@FUNCTIONALITY_NAME@.h"
#include "@FUNCTIONALITY_NAME@Controls.h"
#include <qaction.h>
#include "icon.xpm"
#include "QmitkTreeNodeSelector.h"

@FUNCTIONALITY_NAME@::@FUNCTIONALITY_NAME@(QObject *parent, const char *name, QmitkStdMultiWidget *mitkStdMultiWidget, mitk::DataTreeIteratorBase* it)
    : QmitkFunctionality(parent, name, it) ,m_Controls(NULL), m_MultiWidget(mitkStdMultiWidget)
{
  SetAvailability(true);
}

@FUNCTIONALITY_NAME@::~@FUNCTIONALITY_NAME@()
{}

QWidget * @FUNCTIONALITY_NAME@::CreateMainWidget(QWidget *parent)
{
  return NULL;
}

QWidget * @FUNCTIONALITY_NAME@::CreateControlWidget(QWidget *parent)
{
  if (m_Controls == NULL)
  {
    m_Controls = new @FUNCTIONALITY_NAME@Controls(parent);
  }
  return m_Controls;
}

void @FUNCTIONALITY_NAME@::CreateConnections()
{
  if ( m_Controls )
  {
    connect( (QObject*)(m_Controls->m_TreeNodeSelector), SIGNAL(Activated(mitk::DataTreeIteratorClone)),(QObject*) this, SLOT(ImageSelected(mitk::DataTreeIteratorClone)) );
  }
}

QAction * @FUNCTIONALITY_NAME@::CreateAction(QActionGroup *parent)
{
  QAction* action;
  action = new QAction( tr( "ToolTip" ), QPixmap((const char**)icon_xpm), tr( "MenueEintrag" ), 0, parent, "@FUNCTIONALITY_NAME@" );
  return action;
}

void @FUNCTIONALITY_NAME@::TreeChanged()
{
  m_Controls->m_TreeNodeSelector->SetDataTreeNodeIterator(this->GetDataTreeIterator());
}

void @FUNCTIONALITY_NAME@::Activated()
{
  QmitkFunctionality::Activated();
}

void @FUNCTIONALITY_NAME@::ImageSelected(mitk::DataTreeIteratorClone imageIt)
{
  std::string name;
  if (imageIt->Get()->GetName(name))
  {
    std::cout << "Tree node selected with name '" << name << "'" << std::endl;
  }
}
