/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

// semantic relations UI module
#include "QmitkLesionInfoWidget.h"

#include "QmitkCustomVariants.h"

// qt
#include <QListWidget>

QmitkLesionInfoWidget::QmitkLesionInfoWidget(std::shared_ptr<mitk::SemanticRelations> semanticRelations, QWidget* parent /*= nullptr*/)
  : QWidget(parent)
  , m_SemanticRelations(semanticRelations)
{
  // create GUI from the Qt Designer's .ui file
  m_Controls.setupUi(this);
}

QmitkLesionInfoWidget::~QmitkLesionInfoWidget()
{
  // nothing here
}

void QmitkLesionInfoWidget::SetCurrentCaseID(const mitk::SemanticTypes::CaseID& caseID)
{
  m_CaseID = caseID;
  UpdateLesionInfoWidget();
}

void QmitkLesionInfoWidget::UpdateLesionInfoWidget()
{
  ClearLesionInfoWidget();

  // create new toolbox items with the current lesions and their corresponding segmentations
  std::vector<mitk::SemanticTypes::Lesion> allLesionsOfCase = m_SemanticRelations->GetAllLesionsOfCase(m_CaseID);
  for (const auto& lesion : allLesionsOfCase)
  {
    // get the corresponding segmentation of the lesion
    std::vector<mitk::DataNode::Pointer> allSegmentations = m_SemanticRelations->GetAllSegmentationsOfLesion(m_CaseID, lesion);

    QListWidget* lesionListWidget = new QListWidget(m_Controls.lesionToolBox);
    for (const auto& segmentation : allSegmentations)
    {
      lesionListWidget->addItem(QString::fromStdString(segmentation->GetName()));
    }

    // connect each widget with our custom slots
    connect(lesionListWidget, SIGNAL(clicked(const QModelIndex&)), this, SLOT(OnLesionListItemClicked(const QModelIndex&)));
    connect(lesionListWidget->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)), SLOT(OnLesionListSelectionChanged(const QItemSelection&, const QItemSelection&)));

    m_Controls.lesionToolBox->addItem(lesionListWidget, QString::fromStdString(lesion.UID));
  }
}

void QmitkLesionInfoWidget::ClearLesionInfoWidget()
{
  // remove all existing toolbox items
  while (m_Controls.lesionToolBox->count() > 0)
  {
    m_Controls.lesionToolBox->removeItem(0);
  }
}

void QmitkLesionInfoWidget::OnLesionListItemClicked(const QModelIndex& selectedIndex)
{
  if (!selectedIndex.isValid())
  {
    return;
  }

  // TODO: item has been clicked
}

void QmitkLesionInfoWidget::OnLesionListSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
  // TODO
}
