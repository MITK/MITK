#ifndef HELLOWORLDDIALOG_H_
#define HELLOWORLDDIALOG_H_

#include "IDialog.h"
#include "mitkQtHelloWorldDll.h"

class MITK_QT_HELLOWORLD HelloWorldDialog: public IDialog
{
  cherryClassMacro(HelloWorldDialog)

  public:
    virtual void show();
};

#endif /*HELLOWORLDDIALOG_H_*/