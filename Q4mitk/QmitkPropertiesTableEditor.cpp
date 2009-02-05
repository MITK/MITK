#include "QmitkPropertiesTableEditor.h"

#include "QmitkPropertiesTableModel.h"

#include <QHBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QTableView>

QmitkPropertiesTableEditor::QmitkPropertiesTableEditor(mitk::DataTreeNode::Pointer _Node, QWidget* parent
                                                       , Qt::WindowFlags f)
: QWidget(parent, f)
, m_NodePropertiesTableView(0)
, m_Model(0)
{
  // set up empty gui elements
  this->init();

  // set up model
  m_Model = new QmitkPropertiesTableModel(0, m_NodePropertiesTableView);    
  m_NodePropertiesTableView->setModel(m_Model);
}

QmitkPropertiesTableEditor::~QmitkPropertiesTableEditor()
{
}

void QmitkPropertiesTableEditor::setNode(mitk::DataTreeNode::Pointer _Node)
{
  m_Model->setNode(_Node);
}

mitk::DataTreeNode::Pointer QmitkPropertiesTableEditor::getNode() const
{
  return m_Model->getNode();
}

QmitkPropertiesTableModel* QmitkPropertiesTableEditor::getModel() const
{
  return m_Model;
}

void QmitkPropertiesTableEditor::init()
{
  // read
  QHBoxLayout* _NodePropertiesLayout = new QHBoxLayout;  
  m_NodePropertiesTableView = new QTableView(QWidget::parentWidget());
  
  // write
  setLayout(_NodePropertiesLayout);
  _NodePropertiesLayout->addWidget(m_NodePropertiesTableView);
  m_NodePropertiesTableView->setSelectionMode( QAbstractItemView::SingleSelection );
  m_NodePropertiesTableView->setSelectionBehavior( QAbstractItemView::SelectItems );
  m_NodePropertiesTableView->horizontalHeader()->setStretchLastSection ( true );
  m_NodePropertiesTableView->verticalHeader()->hide();
}