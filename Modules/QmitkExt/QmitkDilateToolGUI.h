#ifndef QMITKDILATETOOLGUI_H
#define QMITKDILATETOOLGUI_H

#include "QmitkMorphologicToolGUI.h"
#include "QmitkExtExports.h"

class /*QmitkExt_EXPORTS*/ QmitkDilateToolGUI : public QmitkMorphologicToolGUI
{
  Q_OBJECT
public:

  mitkClassMacro(QmitkDilateToolGUI, QmitkMorphologicToolGUI);
  itkNewMacro(QmitkDilateToolGUI);

protected:

  virtual ~QmitkDilateToolGUI();
  QmitkDilateToolGUI();

};

#endif