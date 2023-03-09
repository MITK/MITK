/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkLassoToolGUI_h
#define QmitkLassoToolGUI_h

#include "QmitkEditableContourToolGUIBase.h"
#include <MitkSegmentationUIExports.h>

/**
\ingroup org_mitk_gui_qt_interactivesegmentation_internal
\brief GUI for mitk::NewAddTool.
\sa mitk::LassoTool
*/
class MITKSEGMENTATIONUI_EXPORT QmitkLassoToolGUI : public QmitkEditableContourToolGUIBase
{
  Q_OBJECT

public:
  mitkClassMacro(QmitkLassoToolGUI, QmitkEditableContourToolGUIBase);
  itkFactorylessNewMacro(Self);
  itkCloneMacro(Self);

protected:
  QmitkLassoToolGUI();
  ~QmitkLassoToolGUI() override;
};

#endif
