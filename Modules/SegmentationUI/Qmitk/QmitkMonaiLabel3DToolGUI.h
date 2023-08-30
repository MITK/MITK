/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef QmitkMonaiLabelTool3DGUI_h_Included
#define QmitkMonaiLabelTool3DGUI_h_Included

#include <MitkSegmentationUIExports.h>
#include "QmitkMonaiLabelToolGUI.h"

class MITKSEGMENTATIONUI_EXPORT QmitkMonaiLabel3DToolGUI : public QmitkMonaiLabelToolGUI
{
  Q_OBJECT

public:
  mitkClassMacro(QmitkMonaiLabel3DToolGUI, QmitkMonaiLabelToolGUI);
  itkFactorylessNewMacro(Self);
  itkCloneMacro(Self);

protected:
  QmitkMonaiLabel3DToolGUI();
  ~QmitkMonaiLabel3DToolGUI() = default;
};
#endif
