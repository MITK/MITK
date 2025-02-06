/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitknnInteractiveToolGUI_h
#define QmitknnInteractiveToolGUI_h

#include "QmitkSegWithPreviewToolGUIBase.h"

namespace Ui
{
  class QmitknnInteractiveToolGUI;
}

class MITKSEGMENTATIONUI_EXPORT QmitknnInteractiveToolGUI : public QmitkSegWithPreviewToolGUIBase
{
  Q_OBJECT

public:
  mitkClassMacro(QmitknnInteractiveToolGUI, QmitkSegWithPreviewToolGUIBase);
  itkFactorylessNewMacro(Self);

protected:
  QmitknnInteractiveToolGUI();
  ~QmitknnInteractiveToolGUI() override;

  void InitializeUI(QBoxLayout* mainLayout) override;

private:
  Ui::QmitknnInteractiveToolGUI* m_Ui;
};

#endif
