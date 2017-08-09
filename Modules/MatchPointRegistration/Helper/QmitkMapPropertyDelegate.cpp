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

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "QmitkMapPropertyDelegate.h"

#include <QmitkCustomVariants.h>

#include <mitkRenderingManager.h>

#include <bitset>
#include <QPainter>
#include <QApplication>
#include <QCheckBox>
#include <QLabel>
#include <QPushButton>
#include <QColorDialog>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QStringList>
#include <QMessageBox>
#include <QPen>

QmitkMapPropertyDelegate::QmitkMapPropertyDelegate(QObject * /*parent*/)
{
}

void QmitkMapPropertyDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option
    , const QModelIndex &index) const
{

    QVariant data = index.data(Qt::DisplayRole);

    QString name = data.value<QString>();

    QStyledItemDelegate::paint(painter, option, index);

}

QWidget* QmitkMapPropertyDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option
    , const QModelIndex &index) const
{
    QVariant data = index.data(Qt::EditRole);
    QVariant displayData = index.data(Qt::DisplayRole);
    QString name = index.model()->data(index.model()->index(index.row(), index.column() - 1)).value<QString>();

    if (data.isValid())
    {

        QWidget* editorWidget = nullptr;

        if (data.type() == QVariant::Int)
        {
            QSpinBox* spinBox = new QSpinBox(parent);
            spinBox->setSingleStep(1);
            spinBox->setMinimum(std::numeric_limits<int>::min());
            spinBox->setMaximum(std::numeric_limits<int>::max());
            editorWidget = spinBox;
        }
        // see qt documentation. cast is correct, it would be obsolete if we
        // store doubles
        else if (static_cast<QMetaType::Type>(data.type()) == QMetaType::Float)
        {
            QDoubleSpinBox* spinBox = new QDoubleSpinBox(parent);
            spinBox->setDecimals(5);
            spinBox->setSingleStep(0.1);
            spinBox->setMinimum(std::numeric_limits<float>::min());
            spinBox->setMaximum(std::numeric_limits<float>::max());

            editorWidget = spinBox;
        }
        else if (data.type() == QVariant::StringList)
        {
            QStringList entries = data.value<QStringList>();
            QComboBox* comboBox = new QComboBox(parent);
            comboBox->setEditable(false);
            comboBox->addItems(entries);

            editorWidget = comboBox;
        }
        else
        {
            editorWidget = QStyledItemDelegate::createEditor(parent, option, index);
        }

        if (editorWidget)
        {
            // install event filter
            editorWidget->installEventFilter(const_cast<QmitkMapPropertyDelegate*>(this));
        }

        return editorWidget;

    }
    else
        return new QLabel(displayData.toString(), parent);

}

void QmitkMapPropertyDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{

    QVariant data = index.data(Qt::EditRole);
    QVariant displayData = index.data(Qt::DisplayRole);

    std::cout << "Set EDITOR DATA : " << data.toDouble() << std::endl;

    if (data.isValid())
    {


        if (data.type() == QVariant::Int)
        {
            QSpinBox* spinBox = qobject_cast<QSpinBox *>(editor);
            spinBox->setValue(data.toInt());
        }
        // see qt documentation. cast is correct, it would be obsolete if we
        // store doubles
        else if (static_cast<QMetaType::Type>(data.type()) == QMetaType::Float)
        {
            QDoubleSpinBox* spinBox = qobject_cast<QDoubleSpinBox *>(editor);
            spinBox->setValue(data.toDouble());

            std::cout << "Set EDITOR DATA : " << spinBox->value() << std::endl;
        }

        else if (data.type() == QVariant::StringList)
        {
            QComboBox* comboBox = qobject_cast<QComboBox *>(editor);
            QString displayString = displayData.value<QString>();
            comboBox->setCurrentIndex(comboBox->findData(displayString));
        }

        else
            return QStyledItemDelegate::setEditorData(editor, index);
    }
}

void QmitkMapPropertyDelegate::setModelData(QWidget *editor, QAbstractItemModel* model
    , const QModelIndex &index) const
{
    QVariant data = index.data(Qt::EditRole);
    QVariant displayData = index.data(Qt::DisplayRole);

    if (data.isValid())
    {

        if (data.type() == QVariant::Color)
        {
            QWidget *colorBtn = qobject_cast<QWidget *>(editor);
            QVariant colorVariant;
            colorVariant.setValue<QColor>(colorBtn->palette().color(QPalette::Button));
            model->setData(index, colorVariant);
        }

        else if (data.type() == QVariant::Int)
        {
            QSpinBox* spinBox = qobject_cast<QSpinBox *>(editor);
            int intValue = spinBox->value();

            QVariant intValueVariant;
            intValueVariant.setValue<float>(static_cast<float>(intValue));
            model->setData(index, intValueVariant);
        }

        else if (static_cast<QMetaType::Type>(data.type()) == QMetaType::Float)
        {
            QDoubleSpinBox* spinBox = qobject_cast<QDoubleSpinBox *>(editor);
            double doubleValue = spinBox->value();

            std::cout << "SET MODEL DATA << FLOAT : " << doubleValue << std::endl;

            QVariant doubleValueVariant;
            doubleValueVariant.setValue<float>(static_cast<float>(doubleValue));
            std::cout << "SET MODEL DATA << Variant : " << doubleValue << std::endl;
            model->setData(index, doubleValueVariant);
        }

        else if (data.type() == QVariant::StringList)
        {
            QString displayData = data.value<QString>();

            QComboBox* comboBox = qobject_cast<QComboBox *>(editor);
            QString comboBoxValue = comboBox->currentText();

            QVariant comboBoxValueVariant;
            comboBoxValueVariant.setValue<QString>(comboBoxValue);
            model->setData(index, comboBoxValueVariant);
        }

        else
            QStyledItemDelegate::setModelData(editor, model, index);
    }

}

void QmitkMapPropertyDelegate::commitAndCloseEditor()
{
    QWidget* editor = 0;
    if (QPushButton *pushBtn = qobject_cast<QPushButton *>(sender()))
    {
        editor = pushBtn;
    }

    if (editor)
    {
        emit commitData(editor);
        emit closeEditor(editor);
    }

}

void QmitkMapPropertyDelegate::updateEditorGeometry(QWidget *editor,
    const QStyleOptionViewItem &option,
    const QModelIndex & /*index*/) const
{
    editor->setGeometry(option.rect);
}

void QmitkMapPropertyDelegate::ComboBoxCurrentIndexChanged(int  /*index*/)
{
    if (QComboBox *comboBox = qobject_cast<QComboBox *>(sender()))
    {
        emit commitData(comboBox);
        emit closeEditor(comboBox);
    }
}

void QmitkMapPropertyDelegate::SpinBoxValueChanged(const QString&  /*value*/)
{
    QAbstractSpinBox *spinBox = 0;
    if ((spinBox = qobject_cast<QSpinBox *>(sender()))
        || (spinBox = qobject_cast<QDoubleSpinBox *>(sender())))
    {
        emit commitData(spinBox);
        emit closeEditor(spinBox);
    }
}

void QmitkMapPropertyDelegate::showColorDialog()
{

}

bool QmitkMapPropertyDelegate::eventFilter(QObject *o, QEvent *e)
{
    // filter all kind of events on our editor widgets
    // when certain events occur, repaint all render windows, because rendering relevant properties might have changed
    switch (e->type())
    {
    case QEvent::KeyRelease:
    case QEvent::MouseButtonRelease:
    case QEvent::MouseButtonDblClick:
    case QEvent::Wheel:
    case QEvent::FocusIn:
    {
        if (QWidget* editor = dynamic_cast<QWidget*>(o))
        {
            emit commitData(editor);
        }
        break;
    }
    default:
    {
        break;
    }
    }

    return false;
}
