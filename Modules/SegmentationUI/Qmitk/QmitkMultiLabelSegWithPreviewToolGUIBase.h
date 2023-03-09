/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkMultiLabelSegWithPreviewToolGUIBase_h
#define QmitkMultiLabelSegWithPreviewToolGUIBase_h

#include "QmitkSegWithPreviewToolGUIBase.h"

#include "QmitkSimpleLabelSetListWidget.h"

#include <MitkSegmentationUIExports.h>

/**
  \ingroup org_mitk_gui_qt_interactivesegmentation_internal
  \brief GUI for tools based on mitk::AutoMLSegmentationWithPreviewTool.

  This GUI offers an additional list to select the label that should be confirmed.
*/
class MITKSEGMENTATIONUI_EXPORT QmitkMultiLabelSegWithPreviewToolGUIBase : public QmitkSegWithPreviewToolGUIBase
{
  Q_OBJECT

public:
  mitkClassMacro(QmitkMultiLabelSegWithPreviewToolGUIBase, QmitkSegWithPreviewToolGUIBase);

protected slots :

  void OnLabelSelectionChanged(const QmitkSimpleLabelSetListWidget::LabelVectorType& selectedLabels);
  void OnRadioTransferAllClicked(bool checked);

protected:
  QmitkMultiLabelSegWithPreviewToolGUIBase();
  ~QmitkMultiLabelSegWithPreviewToolGUIBase() = default;

  void InitializeUI(QBoxLayout* mainLayout) override;

  void EnableWidgets(bool enabled) override;
  void SetLabelSetPreview(const mitk::LabelSetImage* preview);
  void ActualizePreviewLabelVisibility();

private:
  QmitkSimpleLabelSetListWidget* m_LabelSelectionList = nullptr;
  QWidget* m_RadioTransferAll = nullptr;
  QWidget* m_RadioTransferSelected = nullptr;
};

#endif
