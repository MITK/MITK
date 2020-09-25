/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkOtsuTool3DGUI_h_Included
#define QmitkOtsuTool3DGUI_h_Included

#include "QmitkToolGUI.h"
#include "mitkOtsuTool3D.h"

#include "ui_QmitkOtsuToolWidgetControls.h"

#include <MitkSegmentationUIExports.h>
#include <QPushButton>

/**
  \ingroup org_mitk_gui_qt_interactivesegmentation_internal
  \brief GUI for mitk::.
  \sa mitk::

  This GUI shows ...

  Last contributor: $Author$
*/
class MITKSEGMENTATIONUI_EXPORT QmitkOtsuTool3DGUI : public QmitkToolGUI
{
  Q_OBJECT

public:
  mitkClassMacro(QmitkOtsuTool3DGUI, QmitkToolGUI);
  itkFactorylessNewMacro(Self);
  itkCloneMacro(Self);

protected slots :

  void OnNewToolAssociated(mitk::Tool *);

  void OnSpinboxValueAccept();

  void OnSegmentationRegionAccept();

  void OnRegionSelectionChanged(const QmitkSimpleLabelSetListWidget::LabelVectorType& selectedLabels);

  void OnRegionSpinboxChanged(int);

private slots:

  void OnAdvancedSettingsButtonToggled(bool toggled);

protected:
  QmitkOtsuTool3DGUI();
  ~QmitkOtsuTool3DGUI() override;

  void BusyStateChanged(bool value) override;

  mitk::OtsuTool3D::Pointer m_OtsuTool3DTool;

  Ui_QmitkOtsuToolWidgetControls m_Controls;

  int m_NumberOfRegions;

  bool m_UseValleyEmphasis;

  int m_NumberOfBins;
};

#endif
