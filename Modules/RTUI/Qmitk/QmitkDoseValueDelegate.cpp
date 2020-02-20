/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkDoseValueDelegate.h"

#include <QPainter>
#include <QApplication>
#include <QLabel>
#include <QDoubleSpinBox>

QmitkDoseValueDelegate::QmitkDoseValueDelegate(QObject * /*parent*/)
{
}

void QmitkDoseValueDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option
  , const QModelIndex &index) const
{
  QVariant data = index.data(Qt::DisplayRole);

  QStyleOptionViewItem opt = option;
  initStyleOption(&opt, index);

  QStyle *style = QApplication::style();


  style->drawItemText(painter, opt.rect.adjusted(0,0,-5,0), Qt::AlignRight | Qt::AlignVCenter, opt.palette,true, data.toString());
}

QWidget* QmitkDoseValueDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &
  , const QModelIndex &index) const
{
  QVariant data = index.data(Qt::EditRole);
  QVariant displayData = index.data(Qt::DisplayRole);
  QVariant absoluteDose = index.data(Qt::UserRole+1);

  if(data.isValid())
  {
    QDoubleSpinBox* spinBox = new QDoubleSpinBox(parent);
    spinBox->setDecimals(2);
    if (absoluteDose.toBool())
    {
      spinBox->setSingleStep(0.5);
      spinBox->setSuffix(QString(" Gy"));
    }
    else
    {
      spinBox->setSingleStep(1.0);
      spinBox->setSuffix(QString(" %"));
    }

    spinBox->setMinimum(0.0);
    spinBox->setMaximum(9999.0);

    spinBox->installEventFilter( const_cast<QmitkDoseValueDelegate*>(this) );

    return spinBox;

  }
  else
    return new QLabel(displayData.toString(), parent);

}

void QmitkDoseValueDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
  QVariant data = index.data(Qt::EditRole);

  if(data.isValid())
  {

    QDoubleSpinBox* spinBox = qobject_cast<QDoubleSpinBox *>(editor);
    if (spinBox)
    {
      spinBox->setValue(data.toDouble());
    }
    else
    {
      QStyledItemDelegate::setEditorData(editor, index);
    }
  }
}

void QmitkDoseValueDelegate::setModelData(QWidget *editor, QAbstractItemModel* model
  , const QModelIndex &index) const
{
  QVariant data = index.data(Qt::EditRole);

  if(data.isValid())
  {
    QDoubleSpinBox* spinBox = qobject_cast<QDoubleSpinBox *>(editor);
    double doubleValue = spinBox->value();

    QVariant doubleValueVariant(doubleValue);
    model->setData(index, doubleValueVariant);
  }
  else
  {
    QStyledItemDelegate::setModelData(editor, model, index);
  }
}
