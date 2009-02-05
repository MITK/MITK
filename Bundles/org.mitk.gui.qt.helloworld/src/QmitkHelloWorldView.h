#ifndef QMITKHELLOWORLDVIEW_H_
#define QMITKHELLOWORLDVIEW_H_

#include <cherryIPartListener.h>

#include <QmitkFunctionality.h>
#include <QPushButton>
#include <QmitkStandardViews.h>

#include "mitkQtHelloWorldDll.h"

class MITK_QT_HELLOWORLD QmitkHelloWorldView : public QObject, public QmitkFunctionality
{
  Q_OBJECT;

public:
  void SetFocus();
  void StdMultiWidgetAvailable();
  void StdMultiWidgetNotAvailable();

  ~QmitkHelloWorldView();
protected slots:
    void buttonClicked();

protected:

  void CreateQtPartControl(QWidget* parent);
  QPushButton* m_ButtonHelloWorld;
  QWidget* m_Parent;

};

#endif /*QMITKHELLOWORLDVIEW_H_*/
