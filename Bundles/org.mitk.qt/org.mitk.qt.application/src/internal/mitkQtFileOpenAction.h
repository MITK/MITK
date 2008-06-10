#ifndef CHERRYQTFILEOPENACTION_H_
#define CHERRYQTFILEOPENACTION_H_

#include <QAction>

#include <org.opencherry.ui.qt/cherryQtSimpleWorkbenchWindow.h>

class mitkQtFileOpenAction : public QAction
{
  Q_OBJECT
  
public:
  
  mitkQtFileOpenAction(cherry::QtWorkbenchWindow::Pointer window);
  
protected slots:
  
  void Run();
  
private:
  
  cherry::QtWorkbenchWindow::Pointer m_Window;
};


#endif /*CHERRYQTFILEOPENACTION_H_*/
