/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkBinaryThresholdULToolGUI_h
#define QmitkBinaryThresholdULToolGUI_h

#include "QmitkBinaryThresholdToolGUIBase.h"

#include <MitkSegmentationUIExports.h>

/**
  \ingroup org_mitk_gui_qt_interactivesegmentation_internal
  \brief GUI for mitk::BinaryThresholdTool.

  This GUI shows a slider to change the tool's threshold and an OK button to accept a preview for actual thresholding.

  Last contributor: $Author$
*/
class MITKSEGMENTATIONUI_EXPORT QmitkBinaryThresholdULToolGUI : public QmitkBinaryThresholdToolGUIBase
{
  Q_OBJECT

public:
  mitkClassMacro(QmitkBinaryThresholdULToolGUI, QmitkBinaryThresholdToolGUIBase);
  itkFactorylessNewMacro(Self);
  itkCloneMacro(Self);

protected:
  QmitkBinaryThresholdULToolGUI();
  ~QmitkBinaryThresholdULToolGUI() override;
};

#endif
