/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkFastMarchingToolGUIBase_h_Included
#define QmitkFastMarchingToolGUIBase_h_Included

#include "QmitkAutoSegmentationToolGUIBase.h"
#include "mitkFastMarchingTool3D.h"
#include <MitkSegmentationUIExports.h>

class ctkSliderWidget;
class ctkRangeWidget;
class QPushButton;

/**
\ingroup org_mitk_gui_qt_interactivesegmentation_internal
\brief Base GUI for mitk::FastMarchingTool (2D and 3D).
\sa mitk::FastMarchingTool
*/
class MITKSEGMENTATIONUI_EXPORT QmitkFastMarchingToolGUIBase : public QmitkAutoSegmentationToolGUIBase
{
  Q_OBJECT

public:
  mitkClassMacro(QmitkFastMarchingToolGUIBase, QmitkAutoSegmentationToolGUIBase);

protected slots:

  void OnThresholdChanged(double, double);
  void OnAlphaChanged(double);
  void OnBetaChanged(double);
  void OnSigmaChanged(double);
  void OnStoppingValueChanged(double);
  void OnClearSeeds();

protected:
  QmitkFastMarchingToolGUIBase(bool mode2D);
  ~QmitkFastMarchingToolGUIBase() override;

  void ConnectNewTool(mitk::AutoSegmentationWithPreviewTool* newTool) override;
  void InitializeUI(QBoxLayout* mainLayout) override;

  void EnableWidgets(bool) override;

  void Update();

  ctkRangeWidget *m_slwThreshold;
  ctkSliderWidget *m_slStoppingValue;
  ctkSliderWidget *m_slSigma;
  ctkSliderWidget *m_slAlpha;
  ctkSliderWidget *m_slBeta;

  QPushButton *m_btClearSeeds;

private:
};

#endif
