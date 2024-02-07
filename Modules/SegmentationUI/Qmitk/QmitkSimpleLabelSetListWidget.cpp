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

QmitkSimpleLabelSetListWidget::QmitkSimpleLabelSetListWidget(QWidget* parent) : QWidget(parent), m_LabelList(nullptr), m_Emmiting(false)
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
    m_LabelSetImage->AfterChangeLayerEvent -= mitk::MessageDelegate<QmitkSimpleLabelSetListWidget>(
      this, &QmitkSimpleLabelSetListWidget::OnLayerChanged);
    m_LabelSetImage->RemoveLabelAddedListener(mitk::MessageDelegate1<QmitkSimpleLabelSetListWidget, mitk::LabelSetImage::LabelValueType>(
      this, &QmitkSimpleLabelSetListWidget::OnLabelChanged));
    m_LabelSetImage->RemoveLabelModifiedListener(mitk::MessageDelegate1<QmitkSimpleLabelSetListWidget, mitk::LabelSetImage::LabelValueType>(
      this, &QmitkSimpleLabelSetListWidget::OnLabelChanged));
    m_LabelSetImage->RemoveLabelRemovedListener(mitk::MessageDelegate1<QmitkSimpleLabelSetListWidget, mitk::LabelSetImage::LabelValueType>(
      this, &QmitkSimpleLabelSetListWidget::OnLabelChanged));
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
    result.push_back(m_LabelSetImage->GetLabel(labelValue));
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
      m_LabelSetImage->AfterChangeLayerEvent -= mitk::MessageDelegate<QmitkSimpleLabelSetListWidget>(
        this, &QmitkSimpleLabelSetListWidget::OnLayerChanged);
      m_LabelSetImage->RemoveLabelAddedListener(mitk::MessageDelegate1<QmitkSimpleLabelSetListWidget, mitk::LabelSetImage::LabelValueType>(
        this, &QmitkSimpleLabelSetListWidget::OnLabelChanged));
      m_LabelSetImage->RemoveLabelModifiedListener(mitk::MessageDelegate1<QmitkSimpleLabelSetListWidget, mitk::LabelSetImage::LabelValueType>(
        this, &QmitkSimpleLabelSetListWidget::OnLabelChanged));
      m_LabelSetImage->RemoveLabelRemovedListener(mitk::MessageDelegate1<QmitkSimpleLabelSetListWidget, mitk::LabelSetImage::LabelValueType>(
        this, &QmitkSimpleLabelSetListWidget::OnLabelChanged));
    }

    m_LabelSetImage = image;

    if (m_LabelSetImage.IsNotNull())
    {
      m_LabelSetImage->AfterChangeLayerEvent += mitk::MessageDelegate<QmitkSimpleLabelSetListWidget>(
        this, &QmitkSimpleLabelSetListWidget::OnLayerChanged);
      m_LabelSetImage->AddLabelAddedListener(mitk::MessageDelegate1<QmitkSimpleLabelSetListWidget, mitk::LabelSetImage::LabelValueType>(
        this, &QmitkSimpleLabelSetListWidget::OnLabelChanged));
      m_LabelSetImage->AddLabelModifiedListener(mitk::MessageDelegate1<QmitkSimpleLabelSetListWidget, mitk::LabelSetImage::LabelValueType>(
        this, &QmitkSimpleLabelSetListWidget::OnLabelChanged));
      m_LabelSetImage->AddLabelRemovedListener(mitk::MessageDelegate1<QmitkSimpleLabelSetListWidget, mitk::LabelSetImage::LabelValueType>(
        this, &QmitkSimpleLabelSetListWidget::OnLabelChanged));
    }
  }
}

void QmitkSimpleLabelSetListWidget::OnLayerChanged()
{
  if (!this->m_Emmiting)
  {
    this->ResetList();

    this->m_Emmiting = true;
    emit ActiveLayerChanged();
    emit SelectedLabelsChanged(this->SelectedLabels());
    this->m_Emmiting = false;
  }
}

void QmitkSimpleLabelSetListWidget::OnLabelChanged(mitk::LabelSetImage::LabelValueType lv)
{
  if (!this->m_Emmiting && m_LabelSetImage->GetGroupIndexOfLabel(lv)==m_LabelSetImage->GetActiveLayer())
  {
    this->ResetList();

    this->m_Emmiting = true;
    emit ActiveLayerChanged();
    emit SelectedLabelsChanged(this->SelectedLabels());
    this->m_Emmiting = false;
  }
}

void QmitkSimpleLabelSetListWidget::OnLabelSelectionChanged()
{
  if (!this->m_Emmiting)
  {
    this->m_Emmiting = true;
    emit SelectedLabelsChanged(this->SelectedLabels());
    this->m_Emmiting = false;
  }
}

void QmitkSimpleLabelSetListWidget::ResetList()
{
  m_LabelList->clear();
  
  auto activeLayerID = m_LabelSetImage->GetActiveLayer();
  auto labels = m_LabelSetImage->GetConstLabelsByValue(m_LabelSetImage->GetLabelValuesByGroup(activeLayerID));

  for (auto& label : labels)
  {
    auto color = label->GetColor();
    QPixmap pixmap(10, 10);
    pixmap.fill(QColor(color[0] * 255, color[1] * 255, color[2] * 255));
    QIcon icon(pixmap);

    QListWidgetItem* item = new QListWidgetItem(icon, QString::fromStdString(label->GetName()));
    item->setData(Qt::UserRole, QVariant(label->GetValue()));
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

