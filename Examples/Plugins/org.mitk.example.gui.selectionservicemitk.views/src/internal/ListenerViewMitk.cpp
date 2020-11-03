/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "ListenerViewMitk.h"

// Mitk includes
#include "mitkDataNodeObject.h"

const std::string ListenerViewMitk::VIEW_ID = "org.mitk.views.listenerviewmitk";

ListenerViewMitk::ListenerViewMitk() : m_Parent(nullptr)
{
}

void ListenerViewMitk::CreateQtPartControl(QWidget *parent)
{
  // create GUI widgets
  m_Parent = parent;
  m_Controls.setupUi(parent);

  m_Parent->setEnabled(true);
}

void ListenerViewMitk::ToggleRadioMethod(QString selectStr)
{
  // change the radio button state according to the name of the selected data node
  if (selectStr == "DataNode 1")
    m_Controls.radioButton->toggle();
  else if (selectStr == "DataNode 2")
    m_Controls.radioButton_2->toggle();
}

void ListenerViewMitk::SetFocus()
{
}

//! [MITK Selection Listener method implementation]
void ListenerViewMitk::OnSelectionChanged(berry::IWorkbenchPart::Pointer /*part*/,
                                          const QList<mitk::DataNode::Pointer> &nodes)
{
  // any nodes there?
  if (!nodes.empty())
  {
    // get the selected node (in the BlueBerry example there is always only one node)
    mitk::DataNode::Pointer dataNode = nodes.front();
    // pass the name of the selected node to method
    ToggleRadioMethod(QString::fromStdString(dataNode->GetName()));
  }
}
//! [MITK Selection Listener method implementation]
