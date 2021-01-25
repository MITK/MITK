/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkFastMarchingToolGUI_h_Included
#define QmitkFastMarchingToolGUI_h_Included

#include "QmitkFastMarchingToolGUIBase.h"
#include <MitkSegmentationUIExports.h>

/**
\ingroup org_mitk_gui_qt_interactivesegmentation_internal
\brief GUI for mitk::FastMarchingTool in 2D.
\sa mitk::FastMarchingTool
*/
class MITKSEGMENTATIONUI_EXPORT QmitkFastMarchingToolGUI : public QmitkFastMarchingToolGUIBase
{
  Q_OBJECT

public:
  mitkClassMacro(QmitkFastMarchingToolGUI, QmitkFastMarchingToolGUIBase);
  itkFactorylessNewMacro(Self);
  itkCloneMacro(Self);

protected:
  QmitkFastMarchingToolGUI();
  ~QmitkFastMarchingToolGUI() = default;
};

#endif
