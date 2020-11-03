/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkMaskStampWidget_h_Included
#define QmitkMaskStampWidget_h_Included

#include "MitkSegmentationUIExports.h"
#include "mitkDataNode.h"

#include <QWidget>

#include "ui_QmitkMaskStampWidgetGUIControls.h"

namespace mitk
{
  class ToolManager;
}

/**
  \brief GUI for mask stamp functionality

  This class uses the mitk::ToolManagerProvider::MULTILABEL_SEGMENTATION context.

  \ingroup ToolManagerEtAl
  \ingroup Widgets
*/

class MITKSEGMENTATIONUI_EXPORT QmitkMaskStampWidget : public QWidget
{
  Q_OBJECT

public:
  QmitkMaskStampWidget(QWidget *parent = nullptr, const char *name = nullptr);
  ~QmitkMaskStampWidget() override;

  void SetDataStorage(mitk::DataStorage *storage);

protected slots:

  void OnShowInformation(bool);

  void OnStamp();

private:
  mitk::ToolManager *m_ToolManager;

  mitk::DataStorage *m_DataStorage;

  Ui::QmitkMaskStampWidgetGUIControls m_Controls;
};

#endif
