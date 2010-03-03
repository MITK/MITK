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
: m_SelectionListener(0)
{
}

QmitkPropertyListView::~QmitkPropertyListView()
{
  berry::ISelectionService* s = GetSite()->GetWorkbenchWindow()->GetSelectionService();
  if(s)
    s->RemoveSelectionListener(m_SelectionListener);

}

void QmitkPropertyListView::CreateQtPartControl( QWidget* parent )
{
  m_NodePropertiesTableEditor = new QmitkPropertiesTableEditor;

  QVBoxLayout* layout = new QVBoxLayout;
  layout->addWidget(m_NodePropertiesTableEditor);

  parent->setLayout(layout);

  m_SelectionListener = new berry::SelectionChangedAdapter<QmitkPropertyListView>
    (this, &QmitkPropertyListView::SelectionChanged);
  berry::ISelectionService* s = GetSite()->GetWorkbenchWindow()->GetSelectionService();
  if(s)
    s->AddSelectionListener(m_SelectionListener);

  berry::ISelection::ConstPointer selection( this->GetSite()->GetWorkbenchWindow()->GetSelectionService()->GetSelection("org.mitk.views.datamanager"));
  if(selection.IsNotNull())
    this->SelectionChanged(berry::IWorkbenchPart::Pointer(0), selection);

}

void QmitkPropertyListView::SelectionChanged( berry::IWorkbenchPart::Pointer, berry::ISelection::ConstPointer selection )
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

bool QmitkPropertyListView::IsExclusiveFunctionality() const
{
  return false;
}