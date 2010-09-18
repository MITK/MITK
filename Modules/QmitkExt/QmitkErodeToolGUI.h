#ifndef QMITKERODETOOLGUI_H
#define QMITKERODETOOLGUI_H

#include "QmitkMorphologicToolGUI.h"
#include "QmitkExtExports.h"

class /*QmitkExt_EXPORTS*/ QmitkErodeToolGUI : public QmitkMorphologicToolGUI
{
  Q_OBJECT
public:

  mitkClassMacro(QmitkErodeToolGUI, QmitkMorphologicToolGUI);
  itkNewMacro(QmitkErodeToolGUI);

protected:

  virtual ~QmitkErodeToolGUI();
  QmitkErodeToolGUI();

};

#endif