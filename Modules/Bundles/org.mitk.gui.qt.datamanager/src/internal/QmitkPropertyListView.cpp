#include <berryISelectionProvider.h>
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>
#include <berryISelectionService.h>
#include "QmitkPropertyListView.h"
#include <QmitkPropertiesTableEditor.h>
#include <mitkDataNodeObject.h>
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
  mitk::DataNodeSelection::ConstPointer _DataNodeSelection 
    = selection.Cast<const mitk::DataNodeSelection>();

  if(_DataNodeSelection.IsNotNull())
  {
    std::vector<mitk::DataNode*> selectedNodes;
    mitk::DataNodeObject* _DataNodeObject = 0;

    for(mitk::DataNodeSelection::iterator it = _DataNodeSelection->Begin();
      it != _DataNodeSelection->End(); ++it)
    {
      _DataNodeObject = dynamic_cast<mitk::DataNodeObject*>((*it).GetPointer());
      if(_DataNodeObject)
        selectedNodes.push_back( _DataNodeObject->GetDataNode() );
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