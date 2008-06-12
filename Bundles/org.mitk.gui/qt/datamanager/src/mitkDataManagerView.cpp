#include "mitkDataManagerView.h"

#include <QPushButton>
#include <QDockWidget>

#include "QmitkStdMultiWidget.h"

#include <mitkStdMultiWidgetEditor.h>

#include <cherryIEditorPart.h>
#include <cherryIWorkbenchPage.h>

void* mitkDataManagerView::CreatePartControl(void* parent)
{
  QDockWidget* dock = static_cast<QDockWidget*>(parent);
  /*
  QPushButton* button = new QPushButton("Hi", static_cast<QWidget*>(parent));
  dock->setWidget(button);
  return button;
  */
  
  
  QmitkStandardViews* stdViews = new QmitkStandardViews(dock);
  dock->setWidget( stdViews );
  
  m_MultiWidgetListener = new StdMultiWidgetListener(stdViews);
  this->GetSite()->GetPage()->AddPartListener(m_MultiWidgetListener);
  
  cherry::IWorkbenchPart::Pointer editor = 
      this->GetSite()->GetPage()->GetActiveEditor();
  
  m_MultiWidgetListener->SetStdMultiWidget(editor);
  
  return stdViews;
}
  
void mitkDataManagerView::SetFocus()
{
  
}

mitkDataManagerView::~mitkDataManagerView()
{
  this->GetSite()->GetPage()->RemovePartListener(m_MultiWidgetListener);
}

mitkDataManagerView::
StdMultiWidgetListener::StdMultiWidgetListener(QmitkStandardViews* standardViews)
: m_StandardViewsWidget(standardViews)
{
  
}
    
void 
mitkDataManagerView::
StdMultiWidgetListener::PartActivated(cherry::IWorkbenchPartReference::Pointer partRef)
{
  std::cout << "PartActivated\n";
  this->SetStdMultiWidget(partRef);
}

void 
mitkDataManagerView::
StdMultiWidgetListener::PartBroughtToTop(cherry::IWorkbenchPartReference::Pointer partRef)
{
  std::cout << "PartBroughtToTop\n";
  this->SetStdMultiWidget(partRef);
}

void 
mitkDataManagerView::
StdMultiWidgetListener::PartClosed(cherry::IWorkbenchPartReference::Pointer partRef)
{
  std::cout << "PartClosed\n";
  this->ClearMultiWidget();
}

void 
mitkDataManagerView::
StdMultiWidgetListener::PartDeactivated(cherry::IWorkbenchPartReference::Pointer partRef)
{
  std::cout << "PartDeactivated\n";
  this->ClearMultiWidget();
}

void 
mitkDataManagerView::
StdMultiWidgetListener::PartOpened(cherry::IWorkbenchPartReference::Pointer partRef)
{
  std::cout << "PartOpened\n";
}

void 
mitkDataManagerView::
StdMultiWidgetListener::PartHidden(cherry::IWorkbenchPartReference::Pointer partRef)
{
  std::cout << "PartHidden\n";
  this->ClearMultiWidget();
}

void 
mitkDataManagerView::
StdMultiWidgetListener::PartVisible(cherry::IWorkbenchPartReference::Pointer partRef)
{
  std::cout << "PartVisible\n";
  this->SetStdMultiWidget(partRef);
}

void 
mitkDataManagerView::
StdMultiWidgetListener::PartInputChanged(cherry::IWorkbenchPartReference::Pointer partRef)
{
  std::cout << "PartInputChanged\n";
}

void 
mitkDataManagerView::
StdMultiWidgetListener::SetStdMultiWidget(cherry::IWorkbenchPartReference::Pointer partRef)
{
  if (partRef.IsNull()) return;
      
  cherry::IWorkbenchPart::Pointer part = partRef->GetPart(false);
  this->SetStdMultiWidget(part);
}

void 
mitkDataManagerView::
StdMultiWidgetListener::SetStdMultiWidget(cherry::IWorkbenchPart::Pointer part)
{
  if (part.IsNull()) return;
  
  QmitkStdMultiWidget* multiWidget = 0;
  if (part.Cast<mitkStdMultiWidgetEditor>().IsNotNull())
  {
    std::cout << "getting multi-widget...\n";
    multiWidget = part.Cast<mitkStdMultiWidgetEditor>()->GetStdMultiWidget();
  }
  
  if (multiWidget != 0)
  {
    std::cout << "setting camera controller\n";
    m_StandardViewsWidget->SetCameraController(multiWidget->mitkWidget4->GetRenderer()->GetCameraController());
  }
}

void 
mitkDataManagerView::
StdMultiWidgetListener::ClearMultiWidget()
{
  m_StandardViewsWidget->SetCameraController(0);
}
