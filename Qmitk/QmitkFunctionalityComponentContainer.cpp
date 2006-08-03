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
#include "QmitkFunctionalityComponentContainer.h"
#include "QmitkFunctionalityComponentContainerGUI.h"

#include "QmitkTreeNodeSelector.h"


QmitkFunctionalityComponentContainer::QmitkFunctionalityComponentContainer(QObject *parent, const char *name, mitk::DataTreeIteratorBase* it)
: QmitkBaseFunctionalityComponent(it),m_GUI(NULL),m_Name(name)
{
  SetAvailability(true);
//  CreateSelector(parent);

}

QmitkFunctionalityComponentContainer::~QmitkFunctionalityComponentContainer()
{

}

QString QmitkFunctionalityComponentContainer::GetFunctionalityName()
{
 return m_Name;
}

void QmitkFunctionalityComponentContainer::SetFunctionalityName(QString name)
{
  m_Name = name;
}

void QmitkFunctionalityComponentContainer::TreeChanged()
{
  m_GUI->m_TreeNodeSelector->SetDataTreeNodeIterator(this->GetDataTreeIterator());
}

QAction * QmitkFunctionalityComponentContainer::CreateAction(QActionGroup* m_FunctionalityComponentActionGroup)
{
    QAction* action;
    action = NULL;
//    action = new QAction( tr( "Simple Example" ), QPixmap((const char**)slicer_xpm), tr( "&Simple Example" ), 0, parent, "simple example" );
    return action;
}

void QmitkFunctionalityComponentContainer::CreateConnections()
{
  if ( m_GUI )
  {
    connect( (QObject*)(m_GUI->m_TreeNodeSelector), SIGNAL(Activated(mitk::DataTreeIteratorClone)),(QObject*) this, SLOT(ImageSelected(mitk::DataTreeIteratorClone)) );
  }
}


QString QmitkFunctionalityComponentContainer::GetFunctionalityComponentName()
{
 return m_Name;
}

void QmitkFunctionalityComponentContainer::ImageSelected(mitk::DataTreeIteratorClone imageIt)
{
  //std::string name;
  //if (imageIt->Get()->GetName(name))
  //{
  //  std::cout << "Tree node selected with name '" << name << "'" << std::endl;
  //}
}

void QmitkFunctionalityComponentContainer::Activated()
{
  QmitkBaseFunctionalityComponent::Activated();
}

/******************************************************************************************/
void QmitkFunctionalityComponentContainer::CreateSelector(QWidget* parent)
{
   if (m_GUI == NULL)
  {
    m_GUI = new QmitkFunctionalityComponentContainerGUI(parent);
  }
}

