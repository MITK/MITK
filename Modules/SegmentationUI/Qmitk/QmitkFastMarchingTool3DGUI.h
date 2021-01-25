/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkFastMarchingTool3DGUI_h_Included
#define QmitkFastMarchingTool3DGUI_h_Included

#include "QmitkFastMarchingToolGUIBase.h"
#include <MitkSegmentationUIExports.h>

/**
\ingroup org_mitk_gui_qt_interactivesegmentation_internal
\brief GUI for mitk::FastMarchingTool3D.
\sa mitk::FastMarchingTool
*/
class MITKSEGMENTATIONUI_EXPORT QmitkFastMarchingTool3DGUI : public QmitkFastMarchingToolGUIBase
{
  Q_OBJECT

public:
  mitkClassMacro(QmitkFastMarchingTool3DGUI, QmitkFastMarchingToolGUIBase);
  itkFactorylessNewMacro(Self);
  itkCloneMacro(Self);

protected:
  QmitkFastMarchingTool3DGUI();
  ~QmitkFastMarchingTool3DGUI() = default;
};

#endif
