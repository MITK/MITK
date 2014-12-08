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

#include "QmitkPropertiesTableEditor.h"

#include "QmitkPropertiesTableModel.h"
#include "QmitkPropertyDelegate.h"

#include "mitkBaseRenderer.h"
#include "mitkFocusManager.h"
#include "mitkGlobalInteraction.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QTableView>
#include <QLineEdit>
#include <QLabel>

#include <vtkRenderWindow.h>

QmitkPropertiesTableEditor::QmitkPropertiesTableEditor(QWidget* parent
                                                       , Qt::WindowFlags f,mitk::DataNode::Pointer  /*_Node*/)
: QWidget(parent, f)
, m_NodePropertiesTableView(0)
, m_Model(0)
{
  // set up empty gui elements
  this->init();

  // set up model
  m_Model = new QmitkPropertiesTableModel(m_NodePropertiesTableView, 0);
  m_NodePropertiesTableView->setModel(m_Model);
}

QmitkPropertiesTableEditor::~QmitkPropertiesTableEditor()
{
}

void QmitkPropertiesTableEditor::SetPropertyList( mitk::PropertyList::Pointer _List )
{
  if(_List.IsNotNull())
  {
    m_Model->SetPropertyList(_List);
    m_NodePropertiesTableView->resizeColumnsToContents();
    m_NodePropertiesTableView->resizeRowsToContents();
    m_NodePropertiesTableView->horizontalHeader()->setStretchLastSection(true);
    m_NodePropertiesTableView->setEditTriggers(QAbstractItemView::CurrentChanged);
  }
  else
  {
    m_Model->SetPropertyList(0);
  }
}

QmitkPropertiesTableModel* QmitkPropertiesTableEditor::getModel() const
{
  return m_Model;
}

void QmitkPropertiesTableEditor::init()
{
  // read/ dim
  QVBoxLayout* _NodePropertiesLayout = new QVBoxLayout;
  QWidget* _PropertyFilterKeyWordPane = new QWidget(QWidget::parentWidget());
  QHBoxLayout* _PropertyFilterKeyWordLayout = new QHBoxLayout;
  QLabel* _LabelPropertyFilterKeyWord = new QLabel("Filter: ",_PropertyFilterKeyWordPane);
  m_TxtPropertyFilterKeyWord = new QLineEdit(_PropertyFilterKeyWordPane);
  m_NodePropertiesTableView = new QTableView(QWidget::parentWidget());

  // write
  setLayout(_NodePropertiesLayout);

  _PropertyFilterKeyWordPane->setLayout(_PropertyFilterKeyWordLayout);

  _PropertyFilterKeyWordLayout->setMargin(0);
  _PropertyFilterKeyWordLayout->addWidget(_LabelPropertyFilterKeyWord);
  _PropertyFilterKeyWordLayout->addWidget(m_TxtPropertyFilterKeyWord);

  _NodePropertiesLayout->setMargin(0);
  _NodePropertiesLayout->addWidget(_PropertyFilterKeyWordPane);
  _NodePropertiesLayout->addWidget(m_NodePropertiesTableView);

  m_NodePropertiesTableView->setSelectionMode( QAbstractItemView::SingleSelection );
  m_NodePropertiesTableView->setSelectionBehavior( QAbstractItemView::SelectItems );
  m_NodePropertiesTableView->verticalHeader()->hide();
  m_NodePropertiesTableView->setItemDelegate(new QmitkPropertyDelegate(this));
  m_NodePropertiesTableView->setAlternatingRowColors(true);
  m_NodePropertiesTableView->setSortingEnabled(true);
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
  m_NodePropertiesTableView->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
#else
  m_NodePropertiesTableView->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
#endif

  QObject::connect( m_TxtPropertyFilterKeyWord, SIGNAL( textChanged(const QString &) )
    , this, SLOT( PropertyFilterKeyWordTextChanged(const QString &) ) );

}

void QmitkPropertiesTableEditor::PropertyFilterKeyWordTextChanged( const QString &  /*text*/ )
{
  m_Model->SetFilterPropertiesKeyWord(m_TxtPropertyFilterKeyWord->text().toStdString());
}

QTableView* QmitkPropertiesTableEditor::getTable() const
{
  return m_NodePropertiesTableView;
}
