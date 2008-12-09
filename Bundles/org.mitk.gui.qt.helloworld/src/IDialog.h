#ifndef QMITKIDIALOG_H_
#define QMITKIDIALOG_H_

#include "cherryMacros.h"
#include <QMessageBox>
#include "mitkQtHelloWorldDll.h"

struct MITK_QT_HELLOWORLD IDialog
{
  cherryManifestMacro(IDialog, );
  virtual void show() = 0;
};

#endif /*QMITKIDIALOG_H_*/