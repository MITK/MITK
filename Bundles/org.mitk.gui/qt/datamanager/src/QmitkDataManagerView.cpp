#include "QmitkDataManagerView.h"

#include <QPushButton>
#include <QDockWidget>
#include <QVBoxLayout>

#include <mitkDataStorageEditorInput.h>
#include <mitkIDataStorageReference.h>

#include "QmitkStdMultiWidget.h"
#include "QmitkDataTreeComboBox.h"

#include <QmitkStdMultiWidgetEditor.h>

#include <cherryIEditorPart.h>
#include <cherryIWorkbenchPage.h>

void QmitkDataManagerView::CreateQtPartControl(QWidget* parent)
{
  QVBoxLayout* layout = new QVBoxLayout(parent);
  layout->setContentsMargins(0,0,0,0);
  
  cherry::IEditorPart::Pointer editor = 
      this->GetSite()->GetPage()->GetActiveEditor();
  
  // To get hold of the DataStorage hold by the MultiWidget editor, 
  // you can do the following:
  //
  // cherry::IEditorInput::Pointer input = editor->GetEditorInput();
  // if (input.Cast<mitk::DataStorageEditorInput>().IsNotNull())
  // {
  //   mitk::IDataStorageReference::Pointer dataStorageRef = input.Cast<DataStorageEditorInput>()->GetDataStorageReference();
  //   DataStorage::Pointer dataStorage = dataStorageRef->GetDataStorage();
  //   ...
  // }
  
  // If you do not have a part instance to do GetSite(), you can call
  // cherry::PlatforumUI::GetWorkbench()->GetActiveWorkbenchWindow()->GetActivePage()
  // in order to get an IWorkbenchPage object

  
  mitk::DataTree::Pointer dataTree;
  
  if (editor.IsNotNull())
  {
    cherry::IEditorInput::Pointer input = editor->GetEditorInput();
    if (input.Cast<mitk::DataStorageEditorInput>().IsNotNull())
    {
      mitk::IDataStorageReference::Pointer dataStorageRef = input.Cast<mitk::DataStorageEditorInput>()->GetDataStorageReference();
      dataTree = dataStorageRef->GetDataTree();
    }
  }
  
  QmitkDataTreeComboBox* comboBox = new QmitkDataTreeComboBox(dataTree, parent);
  layout->addWidget(comboBox);
    
  QmitkStandardViews* stdViews = new QmitkStandardViews(parent);
  layout->addWidget(stdViews);
  
  m_MultiWidgetListener = new StdMultiWidgetListener(stdViews);
  m_MultiWidgetListener->SetStdMultiWidget(editor.Cast<IWorkbenchPart>());
  this->GetSite()->GetPage()->AddPartListener(m_MultiWidgetListener);
  
}
  
void QmitkDataManagerView::SetFocus()
{
  
}

QmitkDataManagerView::~QmitkDataManagerView()
{
  this->GetSite()->GetPage()->RemovePartListener(m_MultiWidgetListener);
}

QmitkDataManagerView::
StdMultiWidgetListener::StdMultiWidgetListener(QmitkStandardViews* standardViews)
: m_StandardViewsWidget(standardViews)
{
  
}
    
void 
QmitkDataManagerView::
StdMultiWidgetListener::PartActivated(cherry::IWorkbenchPartReference::Pointer partRef)
{
  std::cout << "PartActivated\n";
  this->SetStdMultiWidget(partRef);
}

void 
QmitkDataManagerView::
StdMultiWidgetListener::PartBroughtToTop(cherry::IWorkbenchPartReference::Pointer partRef)
{
  std::cout << "PartBroughtToTop\n";
  this->SetStdMultiWidget(partRef);
}

void 
QmitkDataManagerView::
StdMultiWidgetListener::PartClosed(cherry::IWorkbenchPartReference::Pointer partRef)
{
  std::cout << "PartClosed\n";
  this->ClearStdMultiWidget(partRef);
}

void 
QmitkDataManagerView::
StdMultiWidgetListener::PartDeactivated(cherry::IWorkbenchPartReference::Pointer partRef)
{
  std::cout << "PartDeactivated\n";
  //this->ClearStdMultiWidget(partRef);
}

void 
QmitkDataManagerView::
StdMultiWidgetListener::PartOpened(cherry::IWorkbenchPartReference::Pointer partRef)
{
  std::cout << "PartOpened\n";
}

void 
QmitkDataManagerView::
StdMultiWidgetListener::PartHidden(cherry::IWorkbenchPartReference::Pointer partRef)
{
  std::cout << "PartHidden\n";
  this->ClearStdMultiWidget(partRef);
}

void 
QmitkDataManagerView::
StdMultiWidgetListener::PartVisible(cherry::IWorkbenchPartReference::Pointer partRef)
{
  std::cout << "PartVisible\n";
  this->SetStdMultiWidget(partRef);
}

void 
QmitkDataManagerView::
StdMultiWidgetListener::PartInputChanged(cherry::IWorkbenchPartReference::Pointer partRef)
{
  std::cout << "PartInputChanged\n";
}

void 
QmitkDataManagerView::
StdMultiWidgetListener::SetStdMultiWidget(cherry::IWorkbenchPartReference::Pointer partRef)
{
  if (partRef.IsNull()) return;
      
  cherry::IWorkbenchPart::Pointer part = partRef->GetPart(false);
  this->SetStdMultiWidget(part);
}

void 
QmitkDataManagerView::
StdMultiWidgetListener::SetStdMultiWidget(cherry::IWorkbenchPart::Pointer part)
{
  if (part.IsNull()) return;
  
  QmitkStdMultiWidget* multiWidget = 0;
  if (part.Cast<QmitkStdMultiWidgetEditor>().IsNotNull())
  {
    std::cout << "getting multi-widget...\n";
    multiWidget = part.Cast<QmitkStdMultiWidgetEditor>()->GetStdMultiWidget();
  }
  
  if (multiWidget != 0)
  {
    std::cout << "setting camera controller\n";
    m_StandardViewsWidget->SetCameraController(multiWidget->mitkWidget4->GetRenderer()->GetCameraController());
  }
}

void 
QmitkDataManagerView::
StdMultiWidgetListener::ClearStdMultiWidget(cherry::IWorkbenchPartReference::Pointer partRef)
{
  if (partRef.IsNull()) return;
  
  if (partRef->GetPart(false).Cast<QmitkStdMultiWidgetEditor>().IsNotNull())
    m_StandardViewsWidget->SetCameraController(0);
}
