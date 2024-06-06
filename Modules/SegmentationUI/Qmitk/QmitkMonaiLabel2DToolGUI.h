/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkMonaiLabelTool2DGUI_h
#define QmitkMonaiLabelTool2DGUI_h

#include <MitkSegmentationUIExports.h>
#include "QmitkMonaiLabelToolGUI.h"

class MITKSEGMENTATIONUI_EXPORT QmitkMonaiLabel2DToolGUI : public QmitkMonaiLabelToolGUI
{
  Q_OBJECT

public:
  mitkClassMacro(QmitkMonaiLabel2DToolGUI, QmitkMonaiLabelToolGUI);
  itkFactorylessNewMacro(Self);
  itkCloneMacro(Self);

protected:
  QmitkMonaiLabel2DToolGUI();
  ~QmitkMonaiLabel2DToolGUI() = default;
};

#endif
