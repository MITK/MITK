/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef QmitkMultiLabelSegmentationPreferencePage_h_included
#define QmitkMultiLabelSegmentationPreferencePage_h_included

#include "berryIQtPreferencePage.h"
#include "org_mitk_gui_qt_multilabelsegmentation_Export.h"
#include <berryIPreferences.h>

class QWidget;
class QCheckBox;
class QRadioButton;
class QDoubleSpinBox;

class MITK_QT_SEGMENTATION QmitkMultiLabelSegmentationPreferencePage : public QObject, public berry::IQtPreferencePage
{
  Q_OBJECT
  Q_INTERFACES(berry::IPreferencePage)

public:

  QmitkMultiLabelSegmentationPreferencePage();
  ~QmitkMultiLabelSegmentationPreferencePage() override;

  void Init(berry::IWorkbench::Pointer workbench) override;

  void CreateQtControl(QWidget* widget) override;

  QWidget* GetQtControl() const override;

  ///
  /// \see IPreferencePage::PerformOk()
  ///
  bool PerformOk() override;

  ///
  /// \see IPreferencePage::PerformCancel()
  ///
  void PerformCancel() override;

  ///
  /// \see IPreferencePage::Update()
  ///
  void Update() override;

protected slots:

  void OnSmoothingCheckboxChecked(int);

protected:

  QWidget* m_MainControl;
  QRadioButton* m_RadioOutline;
  QRadioButton* m_RadioOverlay;
  QDoubleSpinBox* m_SmoothingSpinBox;
  QDoubleSpinBox* m_DecimationSpinBox;
  QCheckBox* m_SelectionModeCheckBox;

  bool m_Initializing;

  berry::IPreferences::Pointer m_SegmentationPreferencesNode;
};

#endif /* QMITKDATAMANAGERPREFERENCEPAGE_H_ */
