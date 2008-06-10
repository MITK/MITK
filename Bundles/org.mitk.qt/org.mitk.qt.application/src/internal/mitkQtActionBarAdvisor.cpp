#include "mitkQtActionBarAdvisor.h"

#include <QMenu>
#include <QMenuBar>

#include <org.opencherry.ui/src/cherryPlatformUI.h>

#include <org.opencherry.ui.qt/internal/cherryQtShowViewAction.h>
#include <org.opencherry.ui.qt/cherryQtSimpleWorkbenchWindow.h>

#include "mitkQtFileOpenAction.h"

mitkQtActionBarAdvisor::mitkQtActionBarAdvisor(cherry::IActionBarConfigurer::Pointer configurer)
 : cherry::ActionBarAdvisor(configurer)
{
  
}

void mitkQtActionBarAdvisor::FillMenuBar(void* menuBar)
{
  QMenuBar* qMenuBar = static_cast<QMenuBar*>(menuBar);
  cherry::QtWorkbenchWindow::Pointer window = this->GetActionBarConfigurer()->GetWindowConfigurer()->GetWindow().Cast<cherry::QtWorkbenchWindow>();
  
  QMenu* fileMenu = qMenuBar->addMenu("&File");
  
  fileMenu->addAction(new mitkQtFileOpenAction(window));
    
  cherry::IViewRegistry* viewRegistry = cherry::PlatformUI::GetWorkbench()->GetViewRegistry();
  const std::vector<cherry::IViewDescriptor::Pointer>& viewDescriptors = viewRegistry->GetViews();
  
  QMenu* viewMenu = qMenuBar->addMenu("Show &View");
  
  std::vector<cherry::IViewDescriptor::Pointer>::const_iterator iter;
  for (iter = viewDescriptors.begin(); iter != viewDescriptors.end(); ++iter)
  {
    cherry::QtShowViewAction* viewAction = new cherry::QtShowViewAction(window, *iter);
    //m_ViewActions.push_back(viewAction);
    viewMenu->addAction(viewAction);
  }
}
