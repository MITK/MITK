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

QmitkLesionInfoWidget::QmitkLesionInfoWidget(QWidget* parent /*= nullptr*/)
  : QWidget(parent)
{
  // create GUI from the Qt Designer's .ui file
  m_Controls.setupUi(this);

  SetupConnections();
}

QmitkLesionInfoWidget::~QmitkLesionInfoWidget()
{
  // nothing here
}

void QmitkLesionInfoWidget::SetupConnections()
{
  // connect each list widget with our custom slots
  connect(m_Controls.lesionListWidget->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)), SLOT(OnLesionListSelectionChanged(const QItemSelection&, const QItemSelection&)));
  connect(m_Controls.segmentationListWidget->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)), SLOT(OnSegmentationListSelectionChanged(const QItemSelection&, const QItemSelection&)));
  connect(m_Controls.imageListWidget->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)), SLOT(OnImageListSelectionChanged(const QItemSelection&, const QItemSelection&)));
}

void QmitkLesionInfoWidget::SetDataStorage(mitk::DataStorage* dataStorage)
{
  if (m_DataStorage != dataStorage)
  {
    // set the new data storage
    m_DataStorage = dataStorage;
    m_SemanticRelations = std::make_unique<mitk::SemanticRelations>(m_DataStorage);
  }
}

void QmitkLesionInfoWidget::SetCurrentCaseID(const mitk::SemanticTypes::CaseID& caseID)
{
  m_CaseID = caseID;
}

void QmitkLesionInfoWidget::Update(const mitk::SemanticTypes::CaseID& caseID)
{
  if (nullptr == m_SemanticRelations)
  {
    return;
  }

  ClearLesionInfoWidget();

  // create list widget entries with the current lesions and their corresponding images and segmentations
  std::vector<mitk::SemanticTypes::Lesion> allLesionsOfCase = m_SemanticRelations->GetAllLesionsOfCase(caseID);
  for (const auto& lesion : allLesionsOfCase)
  {
    m_Controls.lesionListWidget->addItem(QString::fromStdString(lesion.UID));
  }
}

void QmitkLesionInfoWidget::OnLesionListSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
  QString lesionUID = m_Controls.lesionListWidget->currentItem()->text();
  if (lesionUID.isEmpty())
  {
    return;
  }

  if (nullptr == m_SemanticRelations)
  {
    return;
  }

  ClearSegmentationList();
  ClearImageList();

  mitk::SemanticTypes::Lesion selectedLesion;
  // only the UID is needed to identify a representing lesion
  selectedLesion.UID = lesionUID.toStdString();
  std::vector<mitk::DataNode::Pointer> allSegmentationsOfLesion = m_SemanticRelations->GetAllSegmentationsOfLesion(m_CaseID, selectedLesion);
  for (const auto& segmentation : allSegmentationsOfLesion)
  {
    m_Controls.segmentationListWidget->addItem(QString::fromStdString(segmentation->GetName()));
  }
}

void QmitkLesionInfoWidget::OnSegmentationListSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
  // TODO: load images in imageListWidget, that are connected to the selected segmentation
}

void QmitkLesionInfoWidget::OnImageListSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
  // TODO: not needed, since image selection does not filter any further?
}

void QmitkLesionInfoWidget::ClearLesionInfoWidget()
{
  while (m_Controls.lesionListWidget->count() > 0)
  {
    m_Controls.lesionListWidget->takeItem(0);
  }

  ClearSegmentationList();
  ClearImageList();
}

void QmitkLesionInfoWidget::ClearSegmentationList()
{
  while (m_Controls.segmentationListWidget->count() > 0)
  {
    m_Controls.segmentationListWidget->takeItem(0);
  }
}

void QmitkLesionInfoWidget::ClearImageList()
{
  while (m_Controls.imageListWidget->count() > 0)
  {
    m_Controls.imageListWidget->takeItem(0);
  }
}
