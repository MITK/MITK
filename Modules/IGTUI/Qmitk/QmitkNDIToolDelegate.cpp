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


QmitkNDIToolDelegate::QmitkNDIToolDelegate(QObject * parent) : QStyledItemDelegate(parent),
m_Types(), m_DataStorage(NULL), m_Predicate(NULL), m_TagProperty(NULL), m_TagPropertyName()
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
        QString fileName = index.data(/*mitk::FileNameRole*/).value<QString>();
        fileName = QFileDialog::getOpenFileName(editor, "Open SROM file", fileName, "SROM files (*.rom)");
        QLabel* l = qobject_cast<QLabel*>(editor);
        l->setText(fileName);
      }
      return;
    }
  case TypeCol:
    {
      QString type = index.data(/*mitk::TypeRole*/).value<QString>();
      QComboBox* c = qobject_cast<QComboBox*>(editor);
      c->setCurrentIndex(c->findText(type));
      connect(c, SIGNAL(currentIndexChanged(int)), this, SLOT(ComboBoxCurrentIndexChanged(int)));
      return;
    }
  case NodeCol:
    {
      mitk::DataNode::Pointer n = index.data(/*mitk::OrganNodeRole*/).value<mitk::DataNode::Pointer>();
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
        model->setData(index, qVariantFromValue(dsc->GetSelectedNode()), OrganNodeRole);
        //model->setData(index, QString::fromStdString(dsc->GetSelectedNode()->GetName()), Qt::DisplayRole);
        model->setData(index, QString::fromStdString(dsc->GetSelectedNode()->GetName()));
        if ((m_TagProperty.IsNotNull()) && (m_TagPropertyName.empty() == false))  // tag this node as selected
          dsc->GetSelectedNode()->SetProperty(m_TagPropertyName.c_str(), m_TagProperty);
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


void QmitkNDIToolDelegate::SetTagProperty(mitk::BaseProperty::Pointer prop)
{
  m_TagProperty = prop;
}

void QmitkNDIToolDelegate::SetTagPropertyName( const std::string& name )
{
  m_TagPropertyName = name;
}
