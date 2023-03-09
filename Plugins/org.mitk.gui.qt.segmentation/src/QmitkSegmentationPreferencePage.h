/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkSegmentationPreferencePage_h
#define QmitkSegmentationPreferencePage_h

#include "org_mitk_gui_qt_segmentation_Export.h"

#include <berryIQtPreferencePage.h>

class QWidget;

namespace Ui
{
  class QmitkSegmentationPreferencePageControls;
}

class MITK_QT_SEGMENTATION QmitkSegmentationPreferencePage : public QObject, public berry::IQtPreferencePage
{
  Q_OBJECT
  Q_INTERFACES(berry::IPreferencePage)

public:

  QmitkSegmentationPreferencePage();
  ~QmitkSegmentationPreferencePage() override;

  void Init(berry::IWorkbench::Pointer workbench) override;

  void CreateQtControl(QWidget* widget) override;

  QWidget* GetQtControl() const override;

  bool PerformOk() override;

  void PerformCancel() override;

  void Update() override;

protected Q_SLOTS:

  void OnLabelSetPresetButtonClicked();
  void OnSuggestionsButtonClicked();

protected:

  Ui::QmitkSegmentationPreferencePageControls* m_Ui;
  QWidget* m_Control;

  bool m_Initializing;
};

#endif
