#include <cherryISelectionProvider.h>
#include <cherryIWorkbenchWindow.h>
#include <cherryISelectionService.h>
#include "QmitkPropertyListView.h"
#include <QmitkPropertiesTableEditor.h>
#include <mitkDataTreeNodeObject.h>
#include <mbilog.h>
#include <QVBoxLayout>

const std::string QmitkPropertyListView::VIEW_ID = "org.mitk.views.propertylistview";

QmitkPropertyListView::QmitkPropertyListView()
{
}

QmitkPropertyListView::~QmitkPropertyListView()
{
  this->GetSite()->GetWorkbenchWindow()->GetSelectionService()
    ->RemoveSelectionListener(cherry::ISelectionListener::Pointer(this));
}

void QmitkPropertyListView::CreateQtPartControl( QWidget* parent )
{
  m_NodePropertiesTableEditor = new QmitkPropertiesTableEditor;

  QVBoxLayout* layout = new QVBoxLayout;
  layout->addWidget(m_NodePropertiesTableEditor);

  parent->setLayout(layout);

  this->GetSite()->GetWorkbenchWindow()->GetSelectionService()
    ->AddSelectionListener(cherry::ISelectionListener::Pointer(this));
}

void QmitkPropertyListView::SelectionChanged( cherry::IWorkbenchPart::Pointer  /*part*/ , cherry::ISelection::ConstPointer selection )
{
  mitk::DataTreeNodeSelection::ConstPointer _DataTreeNodeSelection 
    = selection.Cast<const mitk::DataTreeNodeSelection>();

  if(_DataTreeNodeSelection.IsNotNull())
  {
    std::vector<mitk::DataTreeNode*> selectedNodes;
    mitk::DataTreeNodeObject* _DataTreeNodeObject = 0;

    for(mitk::DataTreeNodeSelection::iterator it = _DataTreeNodeSelection->Begin();
      it != _DataTreeNodeSelection->End(); ++it)
    {
      _DataTreeNodeObject = dynamic_cast<mitk::DataTreeNodeObject*>((*it).GetPointer());
      if(_DataTreeNodeObject)
        selectedNodes.push_back( _DataTreeNodeObject->GetDataTreeNode() );
    }

    if(selectedNodes.size() > 0)
    {
      m_NodePropertiesTableEditor->SetPropertyList(selectedNodes.back()->GetPropertyList());
    }
  }
}