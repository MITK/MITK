/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkNewAddTool2DGUI_h_Included
#define QmitkNewAddTool2DGUI_h_Included

#include "QmitkEditableContourToolGUIBase.h"
#include "ui_QmitkNewAddTool2DGUIControls.h"
#include <MitkSegmentationUIExports.h>

/**
\ingroup org_mitk_gui_qt_interactivesegmentation_internal
\brief GUI for mitk::NewAddTool.
\sa mitk::NewAddTool2D
*/
class MITKSEGMENTATIONUI_EXPORT QmitkNewAddTool2DGUI : public QmitkEditableContourToolGUIBase
{
  Q_OBJECT

public:
  mitkClassMacro(QmitkNewAddTool2DGUI, QmitkEditableContourToolGUIBase);
  itkFactorylessNewMacro(Self);
  itkCloneMacro(Self);

protected:
  QmitkNewAddTool2DGUI();
  ~QmitkNewAddTool2DGUI() override;
};

#endif
