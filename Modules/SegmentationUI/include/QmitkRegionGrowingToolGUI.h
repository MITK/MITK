/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkRegionGrowingToolGUI_h
#define QmitkRegionGrowingToolGUI_h

#include <QmitkToolGUI.h>
#include <MitkSegmentationUIExports.h>

/**
  \brief GUI for mitk::RegionGrowingTool.
  \sa mitk::RegionGrowingTool

  Explains the mouse interaction.
*/
class MITKSEGMENTATIONUI_EXPORT QmitkRegionGrowingToolGUI : public QmitkToolGUI
{
  Q_OBJECT

public:
  mitkClassMacro(QmitkRegionGrowingToolGUI, QmitkToolGUI);
  itkFactorylessNewMacro(Self);
  itkCloneMacro(Self);

protected:
  QmitkRegionGrowingToolGUI();
  ~QmitkRegionGrowingToolGUI() override;
};

#endif
