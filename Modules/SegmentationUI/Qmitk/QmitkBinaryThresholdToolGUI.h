/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkBinaryThresholdToolGUI_h
#define QmitkBinaryThresholdToolGUI_h

#include "QmitkBinaryThresholdToolGUIBase.h"
#include "mitkBinaryThresholdTool.h"
#include <MitkSegmentationUIExports.h>

#include "ctkSliderWidget.h"
#include <qcheckbox.h>

/**
  \ingroup org_mitk_gui_qt_interactivesegmentation_internal
  \brief GUI for mitk::BinaryThresholdTool.

  This GUI shows a slider to change the tool's threshold and an OK button to accept a preview for actual thresholding.

  There is only a slider for INT values in QT. So, if the working image has a float/double pixeltype, we need to convert
  the original float intensity into a respective int value for the slider. The slider range is then between 0 and 99.

  If the pixeltype is INT, then we do not need any conversion.

  Last contributor: $Author$
*/
class MITKSEGMENTATIONUI_EXPORT QmitkBinaryThresholdToolGUI : public QmitkBinaryThresholdToolGUIBase
{
  Q_OBJECT

public:
  mitkClassMacro(QmitkBinaryThresholdToolGUI, QmitkBinaryThresholdToolGUIBase);
  itkFactorylessNewMacro(Self);
  itkCloneMacro(Self);

protected:
  QmitkBinaryThresholdToolGUI();
  ~QmitkBinaryThresholdToolGUI() override;
};

#endif
