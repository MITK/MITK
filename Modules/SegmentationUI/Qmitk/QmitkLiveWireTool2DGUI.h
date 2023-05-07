/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkLiveWireTool2DGUI_h
#define QmitkLiveWireTool2DGUI_h

#include "QmitkEditableContourToolGUIBase.h"
#include <MitkSegmentationUIExports.h>

/**
\ingroup org_mitk_gui_qt_interactivesegmentation_internal
\brief GUI for mitk::LiveWireTool.
\sa mitk::LiveWireTool2D
*/
class MITKSEGMENTATIONUI_EXPORT QmitkLiveWireTool2DGUI : public QmitkEditableContourToolGUIBase
{
  Q_OBJECT

public:
  mitkClassMacro(QmitkLiveWireTool2DGUI, QmitkToolGUI);
  itkFactorylessNewMacro(Self);
  itkCloneMacro(Self);

protected:
  QmitkLiveWireTool2DGUI();
  ~QmitkLiveWireTool2DGUI() override;
};

#endif
