/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkBinaryThresholdToolGUI_h
#define QmitkBinaryThresholdToolGUI_h

#include <QmitkBinaryThresholdToolGUIBase.h>
#include <mitkBinaryThresholdTool.h>
#include <MitkSegmentationUIExports.h>

/**
  \brief GUI for mitk::BinaryThresholdTool providing a single-threshold slider.

  This GUI shows a slider to change the tool's threshold and an OK button to accept
  a preview for actual thresholding. It operates in single-threshold mode (lower bound only).

  For images with float/double pixel types, the original float intensity is converted
  to an integer slider value in the range 0 to 99. For integer pixel types, no conversion
  is needed.

  \sa mitk::BinaryThresholdTool
  \sa QmitkBinaryThresholdToolGUIBase
  \sa QmitkBinaryThresholdULToolGUI
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
