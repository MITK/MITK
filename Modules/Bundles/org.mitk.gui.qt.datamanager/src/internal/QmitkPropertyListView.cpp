#include <berryISelectionProvider.h>
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>
#include <berryISelectionService.h>
#include "QmitkPropertyListView.h"
#include <QmitkPropertiesTableEditor.h>
#include <mitkDataTreeNodeObject.h>
#include <mitkLogMacros.h>
#include <QVBoxLayout>

const std::string QmitkPropertyListView::VIEW_ID = "org.mitk.views.propertylistview";

QmitkPropertyListView::QmitkPropertyListView()
{
}

QmitkPropertyListView::~QmitkPropertyListView()
{
}

void QmitkPropertyListView::CreateQtPartControl( QWidget* parent )
{
  m_NodePropertiesTableEditor = new QmitkPropertiesTableEditor;

  QVBoxLayout* layout = new QVBoxLayout;
  layout->addWidget(m_NodePropertiesTableEditor);

  parent->setLayout(layout);
}

void QmitkPropertyListView::OnSelectionChanged( std::vector<mitk::DataTreeNode*> nodes )
{
  if (nodes.empty() || (nodes.front() == NULL)) return;
      
  m_NodePropertiesTableEditor->SetPropertyList(nodes.front()->GetPropertyList());
}

bool QmitkPropertyListView::IsExclusiveFunctionality() const
{
  return false;
}