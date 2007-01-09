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

#include "QmitkVolumeVisualization.h"
#include "QmitkVolumeVisualizationControls.h"
#include "QmitkTransferFunctionWidget.h"
#include <qaction.h>
#include <qcheckbox.h>
#include "icon.xpm"
#include "QmitkDataTreeComboBox.h"
#include <mitkDataTreeNode.h>
#include <mitkProperties.h>

  QmitkVolumeVisualization::QmitkVolumeVisualization(QObject *parent, const char *name, QmitkStdMultiWidget *mitkStdMultiWidget, mitk::DataTreeIteratorBase* it)
: QmitkFunctionality(parent, name, it) , m_MultiWidget(mitkStdMultiWidget) ,m_Controls(NULL)
{
  SetAvailability(true);
}

QmitkVolumeVisualization::~QmitkVolumeVisualization()
{}

QWidget * QmitkVolumeVisualization::CreateMainWidget(QWidget*)
{
  return NULL;
}

QWidget * QmitkVolumeVisualization::CreateControlWidget(QWidget *parent)
{
  if (m_Controls == NULL)
  {
    m_Controls = new QmitkVolumeVisualizationControls(parent);
  }
  return m_Controls;
}

void QmitkVolumeVisualization::CreateConnections()
{
  if ( m_Controls )
  {
    connect( (QObject*)(m_Controls->m_TreeNodeSelector), SIGNAL(activated(const mitk::DataTreeFilter::Item*)),(QObject*) this, SLOT(ImageSelected(const mitk::DataTreeFilter::Item*)) );
    m_Controls->m_TreeNodeSelector->SetDataTree(this->GetDataTreeIterator());
    connect( (QObject*)(m_Controls), SIGNAL(EnableRenderingToggled(bool)),(QObject*) this, SLOT(EnableRendering(bool)));
  }
}

QAction * QmitkVolumeVisualization::CreateAction(QActionGroup *parent)
{
  QAction* action;
  action = new QAction( tr( "VolumeVisualization" ), QPixmap((const char**)icon_xpm), tr( "VolumeVisualization" ), 0, parent, "VolumeVisualization" );
  return action;
}

void QmitkVolumeVisualization::TreeChanged()
{
  m_Controls->m_TreeNodeSelector->Update();
}

void QmitkVolumeVisualization::Activated()
{
  QmitkFunctionality::Activated();
}
void QmitkVolumeVisualization::ImageSelected(const mitk::DataTreeFilter::Item* item)
{
  mitk::DataTreeNode* node = const_cast<mitk::DataTreeNode*>(item->GetNode());
  bool enabled = false; 
  if (node) {
    node->GetBoolProperty("volumerendering",enabled);
  }
  m_Controls->m_EnableRenderingCB->setChecked(enabled);
  if (enabled) {
m_Controls->m_TransferFunctionWidget->SetDataTreeNode(node);
}
}
void QmitkVolumeVisualization::EnableRendering(bool state) {
  std::cout << "EnableRendering:" << state << std::endl;
  const mitk::DataTreeFilter::Item* item = m_Controls->m_TreeNodeSelector->GetFilter()->GetSelectedItem();
  if (item)
  {
    mitk::DataTreeNode* node = const_cast<mitk::DataTreeNode*>(item->GetNode());
    if (state && node) {
      node->SetProperty("volumerendering",new mitk::BoolProperty(true));
      mitk::Image* image = dynamic_cast<mitk::Image*>(node->GetData());
      if (!image) return;
      m_Controls->m_TransferFunctionWidget->SetDataTreeNode(node);
    } else if (!state && node) {
      node->SetProperty("volumerendering",new mitk::BoolProperty(false));
    }
  }
}

