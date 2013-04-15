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

#include "QmitkPropertyItemDelegate.h"
#include <mitkBaseProperty.h>
#include <QComboBox>
#include <QPainter>
#include <QSpinBox>

static mitk::BaseProperty* GetBaseProperty(const QVariant& data)
{
  return data.isValid()
    ? reinterpret_cast<mitk::BaseProperty*>(data.value<void*>())
    : NULL;
}

QmitkPropertyItemDelegate::QmitkPropertyItemDelegate(QObject* parent)
  : QStyledItemDelegate(parent)
{
}

QmitkPropertyItemDelegate::~QmitkPropertyItemDelegate()
{
}

QWidget* QmitkPropertyItemDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  QVariant data = index.data(Qt::EditRole);

  if (data.isValid())
  {
    if (data.type() == QVariant::Int)
    {
      QSpinBox* spinBox = new QSpinBox(parent);

      connect(spinBox, SIGNAL(editingFinished()), this, SLOT(OnSpinBoxEditingFinished()));

      return spinBox;
    }

    if (data.type() == QVariant::Double || static_cast<QMetaType::Type>(data.type()) == QMetaType::Float)
    {
      QDoubleSpinBox* spinBox = new QDoubleSpinBox(parent);

      spinBox->setDecimals(4);
      spinBox->setSingleStep(0.1);

      QString name = index.model()->data(index.model()->index(index.row(), index.column() - 1)).toString();

      if (name == "opacity")
      {
        spinBox->setMinimum(0.0);
        spinBox->setMaximum(1.0);
      }

      connect(spinBox, SIGNAL(editingFinished()), this, SLOT(OnSpinBoxEditingFinished()));

      return spinBox;
    }

    if (data.type() == QVariant::StringList)
    {
      QComboBox* comboBox = new QComboBox(parent);

      comboBox->addItems(data.toStringList());

      connect(comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(OnComboBoxCurrentIndexChanged(int)));

      return comboBox;
    }
  }

  return QStyledItemDelegate::createEditor(parent, option, index);
}

void QmitkPropertyItemDelegate::OnComboBoxCurrentIndexChanged(int)
{
  QComboBox* comboBox = qobject_cast<QComboBox*>(sender());

  emit commitData(comboBox);
  emit closeEditor(comboBox);
}

void QmitkPropertyItemDelegate::OnSpinBoxEditingFinished()
{
  QAbstractSpinBox* spinBox = qobject_cast<QAbstractSpinBox*>(sender());

  emit commitData(spinBox);
  emit closeEditor(spinBox);
}

void QmitkPropertyItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  QVariant data = index.data();

  if (index.column() == 1 && data.type() == QVariant::Color)
  {
    painter->fillRect(option.rect, data.value<QColor>());
    return;
  }

  QStyledItemDelegate::paint(painter, option, index);
}

void QmitkPropertyItemDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
  QVariant data = index.data(Qt::EditRole);

  if (!data.isValid())
    return;

  if (data.type() == QVariant::StringList)
  {
    QComboBox* comboBox = qobject_cast<QComboBox*>(editor);
    comboBox->setCurrentIndex(comboBox->findText(index.data().toString()));
  }
  else
  {
    QStyledItemDelegate::setEditorData(editor, index);
  }
}

void QmitkPropertyItemDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
  QVariant data = index.data(Qt::EditRole);

  if (!data.isValid())
    return;

  if (data.type() == QVariant::Int)
  {
    QSpinBox* spinBox = qobject_cast<QSpinBox*>(editor);
    model->setData(index, spinBox->value());
  }
  else if (data.type() == QVariant::Double)
  {
    QDoubleSpinBox* spinBox = qobject_cast<QDoubleSpinBox*>(editor);
    model->setData(index, spinBox->value());
  }
  else if (static_cast<QMetaType::Type>(data.type()) == QMetaType::Float)
  {
    QDoubleSpinBox* spinBox = qobject_cast<QDoubleSpinBox*>(editor);
    model->setData(index, static_cast<float>(spinBox->value()));
  }
  else if (data.type() == QVariant::StringList)
  {
    QComboBox* comboBox = qobject_cast<QComboBox*>(editor);
    model->setData(index, comboBox->currentText());
  }
  else
  {
    QStyledItemDelegate::setModelData(editor, model, index);
  }
}

QSize QmitkPropertyItemDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  return QStyledItemDelegate::sizeHint(option, index);
}
