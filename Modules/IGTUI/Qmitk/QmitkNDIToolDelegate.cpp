/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date: 2009-11-25 13:21:22 +0100 (Mi, 25 Nov 2009) $
Version:   $Revision: 20524 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#include "QmitkNDIToolDelegate.h"
#include <QStringList>
#include <QComboBox>
#include <QLabel>
#include <QFileDialog>

#include "QmitkEnums.h"
#include "QmitkDataStorageComboBox.h"
#include "QmitkCustomVariants.h"
#include "mitkDataStorage.h"
#include "mitkNodePredicateBase.h"
#include "QmitkDataStorageComboBox.h"
#include "mitkNDIPassiveTool.h"

Q_DECLARE_METATYPE(mitk::NDIPassiveTool*)

namespace mitk
{
  enum NDIItemDataRole
  {
    NDIToolRole  = 70,
    ToolIndexRole,
    FileNameRole,
    OrganNodeRole,
    TypeRole
  };
}

QmitkNDIToolDelegate::QmitkNDIToolDelegate(QObject * parent) : QStyledItemDelegate(parent), 
m_Types(), m_DataStorage(NULL), m_Predicate(NULL)
{
}

QWidget* QmitkNDIToolDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
  if (index.isValid() == false)
    return QStyledItemDelegate::createEditor(parent, option, index);

  switch (index.column())
  {
  case SROMCol:
    {   
      return new QLabel("", parent);
    }
  case TypeCol:
    {
      QComboBox* c = new QComboBox(parent);
      c->addItems(m_Types);
      return c;
    }
  case NodeCol:
    {
      return new QmitkDataStorageComboBox(m_DataStorage, m_Predicate, parent);
    }
  case IndexCol:
  case NameCol:
  case StatusCol:
  default:
    return QStyledItemDelegate::createEditor(parent, option, index);
  }
}


void QmitkNDIToolDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
  if (index.isValid() == false)
    return;

  switch (index.column())
  {
  case SROMCol:
    {
      QLabel* l = qobject_cast<QLabel*>(editor);
      if (l->text().isEmpty())
      {
        QString fileName = qVariantValue<QString>(index.data(/*mitk::FileNameRole*/));
        fileName = QFileDialog::getOpenFileName(editor, "Open SROM file", fileName, "SROM files (*.rom)");
        QLabel* l = qobject_cast<QLabel*>(editor);
        l->setText(fileName);
      }
      return;
    }
  case TypeCol:
    {
      QString type = qVariantValue<QString>(index.data(/*mitk::TypeRole*/));
      QComboBox* c = qobject_cast<QComboBox*>(editor);   
      c->setCurrentIndex(c->findText(type));
      connect(c, SIGNAL(currentIndexChanged(int)), this, SLOT(ComboBoxCurrentIndexChanged(int)));
      return;
    }
  case NodeCol:
    {
      mitk::DataTreeNode::Pointer n = qVariantValue<mitk::DataTreeNode::Pointer>(index.data(/*mitk::OrganNodeRole*/));
      if (n.IsNotNull())
      {
        QmitkDataStorageComboBox* dsc = qobject_cast<QmitkDataStorageComboBox*>(editor);
        dsc->setCurrentIndex(dsc->findText(QString::fromStdString(n->GetName())));
        connect(dsc, SIGNAL(currentIndexChanged(int)), this, SLOT(ComboBoxCurrentIndexChanged(int)));
      }
      return;
    }
  case IndexCol:
  case NameCol:
  case StatusCol:
  default:
    QStyledItemDelegate::setEditorData(editor, index);
  }
}


void QmitkNDIToolDelegate::setModelData(QWidget *editor, QAbstractItemModel* model, const QModelIndex &index) const
{
  if (index.isValid() == false)
    return;

  switch (index.column())
  {
  case SROMCol:
    {
      QLabel* l = qobject_cast<QLabel*>(editor);
      //model->setData(index, l->text(), mitk::FileNameRole);
      //model->setData(index, l->text(), Qt::DisplayRole);  // use for display too
      model->setData(index, l->text());  // use for display too
      return;
    }
  case TypeCol:
    {
      QComboBox* c = qobject_cast<QComboBox*>(editor);   
      //model->setData(index, c->currentText(), mitk::TypeRole);
      model->setData(index, c->currentText(), Qt::DisplayRole);
      return;
    }
  case NodeCol:
    {
      QmitkDataStorageComboBox* dsc = qobject_cast<QmitkDataStorageComboBox*>(editor);
      if (dsc->GetSelectedNode().IsNotNull())
      {
        //model->setData(index, qVariantFromValue(dsc->GetSelectedNode()), mitk::OrganNodeRole);
        //model->setData(index, QString::fromStdString(dsc->GetSelectedNode()->GetName()), Qt::DisplayRole);  
        model->setData(index, QString::fromStdString(dsc->GetSelectedNode()->GetName()));
      }
    }
    return;
  case IndexCol:
  case NameCol:
  case StatusCol:
  default:
    QStyledItemDelegate::setModelData(editor, model, index);
  }
}

void QmitkNDIToolDelegate::commitAndCloseEditor()
{
  //QWidget* editor = 0;
  //if(QPushButton *pushBtn = qobject_cast<QPushButton *>(sender()))
  //{

  //}

  //if(editor)
  //{
    //emit commitData(editor);
    //emit closeEditor(editor);
  //}

}


void QmitkNDIToolDelegate::ComboBoxCurrentIndexChanged( int  /*index*/ )
{
  if(QComboBox *comboBox = qobject_cast<QComboBox *>(sender()))
  {
    emit commitData(comboBox);
    emit closeEditor(comboBox);
  }
}


void QmitkNDIToolDelegate::SetTypes( const QStringList& types )
{
  m_Types = types;
}


void QmitkNDIToolDelegate::SetDataStorage(mitk::DataStorage* ds)
{
  m_DataStorage = ds;
}


void QmitkNDIToolDelegate::SetPredicate(mitk::NodePredicateBase::Pointer p)
{
  m_Predicate = p;
}
