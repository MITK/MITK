#ifndef QMITKHELLOWORLDVIEW_H_
#define QMITKHELLOWORLDVIEW_H_

#include <cherryIPartListener.h>

#include <QmitkViewPart.h>
#include <QmitkStandardViews.h>

#include "mitkQtHelloWorldDll.h"

class MITK_QT_HELLOWORLD QmitkHelloWorldView : public QmitkViewPart
{

public:

  void SetFocus();

  ~QmitkHelloWorldView();

protected:

  void CreateQtPartControl(QWidget* parent);

};

#endif /*QMITKHELLOWORLDVIEW_H_*/
