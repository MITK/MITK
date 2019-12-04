/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "SelectionView.h"

const std::string SelectionView::VIEW_ID = "org.mitk.views.selectionview";

SelectionView::SelectionView() : m_Parent(nullptr)
{
}

void SelectionView::CreateQtPartControl(QWidget *parent)
{
  // create GUI widgets
  m_Parent = parent;
  m_Controls.setupUi(parent);

  //! [Qt Selection Provider registration]
  // create new qt selection provider
  m_SelectionProvider = new berry::QtSelectionProvider();
  // set the item selection model to the model of the QListWidget
  m_SelectionProvider->SetItemSelectionModel(m_Controls.m_SelectionList->selectionModel());
  // register selection provider
  GetSite()->SetSelectionProvider(m_SelectionProvider);
  //! [Qt Selection Provider registration]

  // set selection mode to single selection
  m_Controls.m_SelectionList->setSelectionMode(QAbstractItemView::SingleSelection);

  // pre-select the first item of the list
  m_Controls.m_SelectionList->setCurrentRow(0);

  m_Parent->setEnabled(true);
}

void SelectionView::SetFocus()
{
}
