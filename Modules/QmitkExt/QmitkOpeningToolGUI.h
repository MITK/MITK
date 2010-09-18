#ifndef QMITKOPENINGTOOLGUI_H
#define QMITKOPENINGTOOLGUI_H

#include "QmitkMorphologicToolGUI.h"
#include "QmitkExtExports.h"

class /*QmitkExt_EXPORTS*/ QmitkOpeningToolGUI : public QmitkMorphologicToolGUI
{
  Q_OBJECT
public:

  mitkClassMacro(QmitkOpeningToolGUI, QmitkMorphologicToolGUI);
  itkNewMacro(QmitkOpeningToolGUI);

protected:

  virtual ~QmitkOpeningToolGUI();
  QmitkOpeningToolGUI();

};

#endif