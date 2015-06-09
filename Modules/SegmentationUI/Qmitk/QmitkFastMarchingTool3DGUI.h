/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef QmitkFastMarchingTool3DGUI_h_Included
#define QmitkFastMarchingTool3DGUI_h_Included

#include "QmitkToolGUI.h"
#include <MitkSegmentationUIExports.h>
#include "mitkFastMarchingTool3D.h"

class ctkSliderWidget;
class ctkRangeWidget;
class QPushButton;

#include "QmitkStepperAdapter.h"

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
  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)

  void OnThresholdChanged(int current);


  protected slots:

    void OnNewToolAssociated(mitk::Tool*);

    void OnThresholdChanged(double, double);
    void OnAlphaChanged(double);
    void OnBetaChanged(double);
    void OnSigmaChanged(double);
    void OnStoppingValueChanged(double);
    void OnConfirmSegmentation();
    void Refetch();
    void SetStepper(mitk::Stepper *);
    void OnClearSeeds();

protected:

  QmitkFastMarchingTool3DGUI();
  virtual ~QmitkFastMarchingTool3DGUI();

  void BusyStateChanged(bool) override;

  void Update();

  ctkRangeWidget*  m_slwThreshold;
  ctkSliderWidget* m_slStoppingValue;
  ctkSliderWidget* m_slSigma;
  ctkSliderWidget* m_slAlpha;
  ctkSliderWidget* m_slBeta;

  QPushButton* m_btConfirm;
  QPushButton* m_btClearSeeds;

  mitk::FastMarchingTool3D::Pointer m_FastMarchingTool;

  bool m_TimeIsConnected;
  mitk::Stepper::Pointer m_TimeStepper;

  void OnFastMarchingToolReady();
};

#endif
