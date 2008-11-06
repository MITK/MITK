#ifndef QMITKDATAMANAGERVIEW_H_
#define QMITKDATAMANAGERVIEW_H_

#include <cherryIPartListener.h>

#include <QmitkViewPart.h>
#include <QmitkStandardViews.h>

#include "mitkQtDataManagerDll.h"

class MITK_QT_DATAMANAGER QmitkDataManagerView : public QmitkViewPart
{

public:

  void SetFocus();

  ~QmitkDataManagerView();

protected:

  void CreateQtPartControl(QWidget* parent);

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
    void ClearStdMultiWidget(cherry::IWorkbenchPartReference::Pointer partRef);

  private:
    QmitkStandardViews* m_StandardViewsWidget;

  };

  StdMultiWidgetListener::Pointer m_MultiWidgetListener;

};

#endif /*QMITKDATAMANAGERVIEW_H_*/
