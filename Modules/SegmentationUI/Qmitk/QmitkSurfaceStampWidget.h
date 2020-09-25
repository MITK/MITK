/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkSurfaceStampWidget_h_Included
#define QmitkSurfaceStampWidget_h_Included

#include "MitkSegmentationUIExports.h"

#include <QWidget>

#include "ui_QmitkSurfaceStampWidgetGUIControls.h"

namespace mitk
{
  class ToolManager;
}

/**
  \brief GUI for surface-based interpolation.

  This class uses the mitk::ToolManagerProvider::MULTILABEL_SEGMENTATION context.

  \ingroup ToolManagerEtAl
  \ingroup Widgets
*/

class MITKSEGMENTATIONUI_EXPORT QmitkSurfaceStampWidget : public QWidget
{
  Q_OBJECT

public:
  QmitkSurfaceStampWidget(QWidget *parent = nullptr, const char *name = nullptr);
  ~QmitkSurfaceStampWidget() override;

  void SetDataStorage(mitk::DataStorage *storage);

protected slots:

  void OnShowInformation(bool);

  void OnStamp();

private:
  mitk::ToolManager *m_ToolManager;

  mitk::DataStorage *m_DataStorage;

  Ui::QmitkSurfaceStampWidgetGUIControls m_Controls;
};

#endif
