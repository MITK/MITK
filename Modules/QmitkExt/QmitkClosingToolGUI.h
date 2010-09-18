#ifndef QMITKCLOSINGTOOLGUI_H
#define QMITKCLOSINGTOOLGUI_H

#include "QmitkMorphologicToolGUI.h"
#include "QmitkExtExports.h"

class /*QmitkExt_EXPORTS*/ QmitkClosingToolGUI : public QmitkMorphologicToolGUI
{
  Q_OBJECT
public:

  mitkClassMacro(QmitkClosingToolGUI, QmitkMorphologicToolGUI);
  itkNewMacro(QmitkClosingToolGUI);

protected:

  virtual ~QmitkClosingToolGUI();
  QmitkClosingToolGUI();

};

#endif