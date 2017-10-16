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

// semantic relations module
#include <mitkNodePredicates.h>

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
  // connect each list widget with a custom slots
  connect(m_Controls.lesionListWidget, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)), SLOT(OnCurrentLesionItemChanged(QListWidgetItem*, QListWidgetItem*)));
  connect(m_Controls.lesionListWidget, SIGNAL(itemDoubleClicked(QListWidgetItem*)), SLOT(OnLesionItemDoubleClicked(QListWidgetItem*)));
  connect(m_Controls.segmentationListWidget, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)), SLOT(OnCurrentSegmentationItemChanged(QListWidgetItem*, QListWidgetItem*)));
  connect(m_Controls.segmentationListWidget, SIGNAL(itemDoubleClicked(QListWidgetItem*)), SLOT(OnSegmentationItemDoubleClicked(QListWidgetItem*)));
  connect(m_Controls.imageListWidget, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)), SLOT(OnCurrentImageItemChanged(QListWidgetItem*, QListWidgetItem*)));
  connect(m_Controls.imageListWidget, SIGNAL(itemDoubleClicked(QListWidgetItem*)), SLOT(OnImageItemDoubleClicked(QListWidgetItem*)));
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

  // if the case ID of updated instance is equal to the currently active caseID
  if (caseID == m_CaseID)
  {
    ClearLesionInfoWidget();
    // create list widget entries with the current lesions
    std::vector<mitk::SemanticTypes::Lesion> allLesionsOfCase = m_SemanticRelations->GetAllLesionsOfCase(caseID);
    for (const auto& lesion : allLesionsOfCase)
    {
      m_Controls.lesionListWidget->addItem(QString::fromStdString(lesion.UID));
    }
  }
}

void QmitkLesionInfoWidget::OnCurrentLesionItemChanged(QListWidgetItem* current, QListWidgetItem* /*previous*/)
{
  if (nullptr == m_SemanticRelations)
  {
    return;
  }

  if (nullptr == current)
  {
    // no item clicked; cannot retrieve the current lesion
    return;
  }

  QString lesionUID = current->text();
  // only the UID is needed to identify a representing lesion
  m_CurrentLesion.UID = lesionUID.toStdString();
  if (lesionUID.isEmpty())
  {
    return;
  }

  ClearSegmentationList();
  // create list widget entries with the corresponding segmentations of the current lesion
  std::vector<mitk::DataNode::Pointer> allSegmentationsOfLesion = m_SemanticRelations->GetAllSegmentationsOfLesion(m_CaseID, m_CurrentLesion);
  if (allSegmentationsOfLesion.empty())
  {
    m_Controls.segmentationListWidget->addItem("No segmentation found");
    return;
  }
  for (const auto& segmentation : allSegmentationsOfLesion)
  {
    m_Controls.segmentationListWidget->addItem(QString::fromStdString(segmentation->GetName()));
  }
}

void QmitkLesionInfoWidget::OnLesionItemDoubleClicked(QListWidgetItem* clickedItem)
{
  if (nullptr == m_SemanticRelations)
  {
    return;
  }

  if (nullptr == clickedItem)
  {
    // no item clicked; cannot retrieve the current lesion
    return;
  }
}

void QmitkLesionInfoWidget::OnCurrentSegmentationItemChanged(QListWidgetItem* current, QListWidgetItem* /*previous*/)
{
  if (nullptr == m_SemanticRelations)
  {
    return;
  }

  if (nullptr == current)
  {
    // no item clicked; cannot retrieve the current segmentation
    return;
  }

  QString segmentationName = current->text();
  m_CurrentSegmentation = m_DataStorage->GetNamedNode(segmentationName.toStdString());
  if (nullptr == m_CurrentSegmentation)
  {
    return;
  }

  m_Controls.imageListWidget->clear();
  // create list widget entry with the parent image of the current segmentation
  mitk::DataStorage::SetOfObjects::ConstPointer parentNodes = m_DataStorage->GetSources(m_CurrentSegmentation, mitk::NodePredicates::GetImagePredicate(), false);
  mitk::DataNode::ConstPointer parentImageOfSegmentation = parentNodes->front();
  m_Controls.imageListWidget->addItem(QString::fromStdString(parentImageOfSegmentation->GetName()));
}

void QmitkLesionInfoWidget::OnSegmentationItemDoubleClicked(QListWidgetItem* clickedItem)
{
  if (nullptr == m_SemanticRelations)
  {
    return;
  }

  if (nullptr == clickedItem)
  {
    // no item clicked; cannot retrieve the current segmentation
    return;
  }
}

void QmitkLesionInfoWidget::OnCurrentImageItemChanged(QListWidgetItem* current, QListWidgetItem* /*previous*/)
{
  if (nullptr == m_SemanticRelations)
  {
    return;
  }

  if (nullptr == current)
  {
    // no item clicked; cannot retrieve the current image
    return;
  }

  QString imageName = current->text();
  m_CurrentImage = m_DataStorage->GetNamedNode(imageName.toStdString());
  if (nullptr == m_CurrentImage)
  {
    return;
  }
}

void QmitkLesionInfoWidget::OnImageItemDoubleClicked(QListWidgetItem* clickedItem)
{
  if (nullptr == m_SemanticRelations)
  {
    return;
  }

  if (nullptr == clickedItem)
  {
    // no item clicked; cannot retrieve the current image
    return;
  }
}

void QmitkLesionInfoWidget::ClearLesionInfoWidget()
{
  m_Controls.lesionListWidget->clear();
  m_CurrentLesion.UID = "";
  ClearSegmentationList();
}

void QmitkLesionInfoWidget::ClearSegmentationList()
{
  m_Controls.segmentationListWidget->clear();
  m_Controls.imageListWidget->clear();
}
