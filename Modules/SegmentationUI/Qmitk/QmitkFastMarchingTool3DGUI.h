/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkFastMarchingTool3DGUI_h_Included
#define QmitkFastMarchingTool3DGUI_h_Included

#include "QmitkToolGUI.h"
#include "mitkFastMarchingTool3D.h"
#include <MitkSegmentationUIExports.h>

class ctkSliderWidget;
class ctkRangeWidget;
class QPushButton;
class QCheckBox;

/**
\ingroup org_mitk_gui_qt_interactivesegmentation_internal
\brief GUI for mitk::FastMarchingTool.
\sa mitk::FastMarchingTool
*/
class MITKSEGMENTATIONUI_EXPORT QmitkFastMarchingTool3DGUI : public QmitkToolGUI
{
  Q_OBJECT

public:
  mitkClassMacro(QmitkFastMarchingTool3DGUI, QmitkToolGUI);
  itkFactorylessNewMacro(Self);
  itkCloneMacro(Self);

    void OnThresholdChanged(int current);

protected slots:

  void OnNewToolAssociated(mitk::Tool *);

  void OnThresholdChanged(double, double);
  void OnAlphaChanged(double);
  void OnBetaChanged(double);
  void OnSigmaChanged(double);
  void OnStoppingValueChanged(double);
  void OnConfirmSegmentation();
  void OnClearSeeds();

protected:
  QmitkFastMarchingTool3DGUI();
  ~QmitkFastMarchingTool3DGUI() override;

  void BusyStateChanged(bool) override;

  void Update();

  ctkRangeWidget *m_slwThreshold;
  ctkSliderWidget *m_slStoppingValue;
  ctkSliderWidget *m_slSigma;
  ctkSliderWidget *m_slAlpha;
  ctkSliderWidget *m_slBeta;

  QPushButton *m_btConfirm;
  QPushButton *m_btClearSeeds;

  QCheckBox* m_CheckProcessAll = nullptr;
  QCheckBox* m_CheckCreateNew = nullptr;

  mitk::FastMarchingTool3D::Pointer m_FastMarchingTool;

private:
  void EnableWidgets(bool);
};

#endif
