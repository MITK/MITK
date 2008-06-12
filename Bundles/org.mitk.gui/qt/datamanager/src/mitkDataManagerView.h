#ifndef MITKDATAMANAGERVIEW_H_
#define MITKDATAMANAGERVIEW_H_

#include <cherryViewPart.h>
#include <cherryIPartListener.h>

#include "QmitkStandardViews.h"

class mitkDataManagerView : public cherry::ViewPart
{
  
public:
  void* CreatePartControl(void* parent);
  void SetFocus();
  
  ~mitkDataManagerView();
  
private:
  
  struct StdMultiWidgetListener : public cherry::IPartListener
  {
    cherryClassMacro(StdMultiWidgetListener);
    
    StdMultiWidgetListener(QmitkStandardViews* standardViews);
    
    void PartActivated(cherry::IWorkbenchPartReference::Pointer partRef);
    void PartBroughtToTop(cherry::IWorkbenchPartReference::Pointer partRef);
    void PartClosed(cherry::IWorkbenchPartReference::Pointer partRef);
    void PartDeactivated(cherry::IWorkbenchPartReference::Pointer partRef);
    void PartOpened(cherry::IWorkbenchPartReference::Pointer partRef);
    void PartHidden(cherry::IWorkbenchPartReference::Pointer partRef);
    void PartVisible(cherry::IWorkbenchPartReference::Pointer partRef);
    void PartInputChanged(cherry::IWorkbenchPartReference::Pointer partRef);
  
    void SetStdMultiWidget(cherry::IWorkbenchPartReference::Pointer partRef);
    void SetStdMultiWidget(cherry::IWorkbenchPart::Pointer partRef);
    void ClearMultiWidget();
    
  private: 
    QmitkStandardViews* m_StandardViewsWidget;
  
  };
  
  StdMultiWidgetListener::Pointer m_MultiWidgetListener;
 
};

#endif /*MITKDATAMANAGERVIEW_H_*/
