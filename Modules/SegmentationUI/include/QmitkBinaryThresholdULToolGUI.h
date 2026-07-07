/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkBinaryThresholdULToolGUI_h
#define QmitkBinaryThresholdULToolGUI_h

#include <QmitkBinaryThresholdToolGUIBase.h>

#include <MitkSegmentationUIExports.h>

/**
  \brief GUI for mitk::BinaryThresholdTool with upper and lower threshold range.

  This GUI shows a range slider to change both upper and lower threshold values and
  an OK button to accept a preview for actual thresholding. It operates in upper/lower
  threshold mode (dual bounds).

  \sa mitk::BinaryThresholdTool
  \sa QmitkBinaryThresholdToolGUIBase
  \sa QmitkBinaryThresholdToolGUI
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
