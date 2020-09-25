/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkSimpleLabelSetListWidget.h"

#include "mitkMessage.h"

#include <qlayout.h>

QmitkSimpleLabelSetListWidget::QmitkSimpleLabelSetListWidget(QWidget* parent) : QWidget(parent), m_LabelList(nullptr)
{
  QGridLayout* layout = new QGridLayout(this);
  this->setContentsMargins(0, 0, 0, 0);

  m_LabelList = new QListWidget(this);
  m_LabelList->setSelectionMode(QAbstractItemView::MultiSelection);
  m_LabelList->setResizeMode(QListView::Adjust);
  m_LabelList->setAutoScrollMargin(0);
  layout->addWidget(m_LabelList);

  connect(m_LabelList, SIGNAL(itemSelectionChanged()), this, SLOT(OnLabelSelectionChanged()));
}

QmitkSimpleLabelSetListWidget::~QmitkSimpleLabelSetListWidget()
{
  if (m_LabelSetImage.IsNotNull())
  {
    m_LabelSetImage->BeforeChangeLayerEvent -= mitk::MessageDelegate<QmitkSimpleLabelSetListWidget>(
      this, &QmitkSimpleLabelSetListWidget::OnLooseLabelSetConnection);
    m_LabelSetImage->AfterChangeLayerEvent -= mitk::MessageDelegate<QmitkSimpleLabelSetListWidget>(
      this, &QmitkSimpleLabelSetListWidget::OnEstablishLabelSetConnection);
    OnLooseLabelSetConnection();
  }
}

QmitkSimpleLabelSetListWidget::LabelVectorType QmitkSimpleLabelSetListWidget::SelectedLabels() const
{
  auto selectedItems = m_LabelList->selectedItems();
  LabelVectorType result;

  QList<QListWidgetItem*>::Iterator it;
  for (it = selectedItems.begin(); it != selectedItems.end(); ++it)
  {
    auto labelValue = (*it)->data(Qt::UserRole).toUInt();


    auto activeLayerID = m_LabelSetImage->GetActiveLayer();
    auto labelSet = m_LabelSetImage->GetLabelSet(activeLayerID);
    
    result.push_back(labelSet->GetLabel(labelValue));
  }

  return result;
}

const mitk::LabelSetImage* QmitkSimpleLabelSetListWidget::GetLabelSetImage() const
{
  return m_LabelSetImage;
}

void QmitkSimpleLabelSetListWidget::SetLabelSetImage(const mitk::LabelSetImage* image)
{
  if (image != m_LabelSetImage)
  {
    if (m_LabelSetImage.IsNotNull())
    {
      m_LabelSetImage->BeforeChangeLayerEvent -= mitk::MessageDelegate<QmitkSimpleLabelSetListWidget>(
        this, &QmitkSimpleLabelSetListWidget::OnLooseLabelSetConnection);
      m_LabelSetImage->AfterChangeLayerEvent -= mitk::MessageDelegate<QmitkSimpleLabelSetListWidget>(
        this, &QmitkSimpleLabelSetListWidget::OnLayerChanged);
      this->OnLooseLabelSetConnection();
    }

    m_LabelSetImage = image;

    if (m_LabelSetImage.IsNotNull())
    {
      m_LabelSetImage->BeforeChangeLayerEvent += mitk::MessageDelegate<QmitkSimpleLabelSetListWidget>(
        this, &QmitkSimpleLabelSetListWidget::OnLooseLabelSetConnection);
      m_LabelSetImage->AfterChangeLayerEvent += mitk::MessageDelegate<QmitkSimpleLabelSetListWidget>(
        this, &QmitkSimpleLabelSetListWidget::OnLayerChanged);
      this->OnLayerChanged();
    }
  }
}

void QmitkSimpleLabelSetListWidget::OnLooseLabelSetConnection()
{
  if (m_LabelSetImage.IsNull())
    return;

  auto activeLayerID = m_LabelSetImage->GetActiveLayer();
  auto labelSet = m_LabelSetImage->GetLabelSet(activeLayerID);

  // Reset LabelSetWidget Events
  labelSet->AddLabelEvent -= mitk::MessageDelegate<QmitkSimpleLabelSetListWidget>(
    this, &QmitkSimpleLabelSetListWidget::OnLabelChanged);
  labelSet->RemoveLabelEvent -= mitk::MessageDelegate<QmitkSimpleLabelSetListWidget>(
    this, &QmitkSimpleLabelSetListWidget::OnLabelChanged);
  labelSet->ModifyLabelEvent -= mitk::MessageDelegate<QmitkSimpleLabelSetListWidget>(
    this, &QmitkSimpleLabelSetListWidget::OnLabelChanged);
}

void QmitkSimpleLabelSetListWidget::OnEstablishLabelSetConnection()
{
  if (m_LabelSetImage.IsNull())
    return;

  auto activeLayerID = m_LabelSetImage->GetActiveLayer();
  auto labelSet = m_LabelSetImage->GetLabelSet(activeLayerID);

  // Reset LabelSetWidget Events
  labelSet->AddLabelEvent += mitk::MessageDelegate<QmitkSimpleLabelSetListWidget>(
    this, &QmitkSimpleLabelSetListWidget::OnLabelChanged);
  labelSet->RemoveLabelEvent += mitk::MessageDelegate<QmitkSimpleLabelSetListWidget>(
    this, &QmitkSimpleLabelSetListWidget::OnLabelChanged);
  labelSet->ModifyLabelEvent += mitk::MessageDelegate<QmitkSimpleLabelSetListWidget>(
    this, &QmitkSimpleLabelSetListWidget::OnLabelChanged);
}

void QmitkSimpleLabelSetListWidget::OnLayerChanged()
{
  this->OnEstablishLabelSetConnection();
  this->ResetList();
  emit ActiveLayerChanged();
  emit SelectedLabelsChanged(this->SelectedLabels());
}

void QmitkSimpleLabelSetListWidget::OnLabelChanged()
{
  this->ResetList();
  emit ActiveLayerChanged();
  emit SelectedLabelsChanged(this->SelectedLabels());
}

void QmitkSimpleLabelSetListWidget::OnLabelSelectionChanged()
{
  emit SelectedLabelsChanged(this->SelectedLabels());
}

void QmitkSimpleLabelSetListWidget::ResetList()
{
  m_LabelList->clear();
  
  auto activeLayerID = m_LabelSetImage->GetActiveLayer();
  auto labelSet = m_LabelSetImage->GetLabelSet(activeLayerID);

  auto iter = labelSet->IteratorConstBegin();
  for (; iter != labelSet->IteratorConstEnd(); ++iter)
  {
    auto color = iter->second->GetColor();
    QPixmap pixmap(10, 10);
    pixmap.fill(QColor(color[0] * 255, color[1] * 255, color[2] * 255));
    QIcon icon(pixmap);

    QListWidgetItem* item = new QListWidgetItem(icon, QString::fromStdString(iter->second->GetName()));
    item->setData(Qt::UserRole, QVariant(iter->second->GetValue()));
    m_LabelList->addItem(item);
  }
}

void QmitkSimpleLabelSetListWidget::SetSelectedLabels(const LabelVectorType& selectedLabels)
{
  for (int i = 0; i < m_LabelList->count(); ++i)
  {
    QListWidgetItem* item = m_LabelList->item(i);
    auto labelValue = item->data(Qt::UserRole).toUInt();

    auto finding = std::find_if(selectedLabels.begin(), selectedLabels.end(), [labelValue](const mitk::Label* label) {return label->GetValue() == labelValue; });
    item->setSelected(finding != selectedLabels.end());
  }
}

