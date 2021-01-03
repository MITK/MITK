/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkAutoMLSegmentationToolGUIBase_h_Included
#define QmitkAutoMLSegmentationToolGUIBase_h_Included

#include "QmitkAutoSegmentationToolGUIBase.h"

#include "QmitkSimpleLabelSetListWidget.h"

#include <MitkSegmentationUIExports.h>

/**
  \ingroup org_mitk_gui_qt_interactivesegmentation_internal
  \brief GUI for tools based on mitk::AutoMLSegmentationWithPreviewTool.

  This GUI offers an additional list to select the label that should be confirmed.
*/
class MITKSEGMENTATIONUI_EXPORT QmitkAutoMLSegmentationToolGUIBase : public QmitkAutoSegmentationToolGUIBase
{
  Q_OBJECT

public:
  mitkClassMacro(QmitkAutoMLSegmentationToolGUIBase, QmitkAutoSegmentationToolGUIBase);

protected slots :

  void OnLabelSelectionChanged(const QmitkSimpleLabelSetListWidget::LabelVectorType& selectedLabels);

protected:
  QmitkAutoMLSegmentationToolGUIBase();
  ~QmitkAutoMLSegmentationToolGUIBase() = default;

  void InitializeUI(QBoxLayout* mainLayout) override;
  void ConnectNewTool(mitk::AutoSegmentationWithPreviewTool* newTool) override;

  void EnableWidgets(bool enabled) override;
  void SetLabelSetPreview(const mitk::LabelSetImage* preview);

private:
  QmitkSimpleLabelSetListWidget* m_LabelSelectionList = nullptr;
};

#endif
