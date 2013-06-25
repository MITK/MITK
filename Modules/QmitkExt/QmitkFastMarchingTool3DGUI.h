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
#include "QmitkExtExports.h"
#include "mitkFastMarchingTool3D.h"

class ctkSliderWidget;
class QLabel;
class QFrame;
class QPushButton;
class QCheckBox;


#include "QmitkStepperAdapter.h"

/**
\ingroup org_mitk_gui_qt_interactivesegmentation_internal
\brief GUI for mitk::FastMarchingTool.
\sa mitk::FastMarchingTool
*/
class QmitkExt_EXPORT QmitkFastMarchingTool3DGUI : public QmitkToolGUI
{
  Q_OBJECT

public:
  mitkClassMacro(QmitkFastMarchingTool3DGUI, QmitkToolGUI);
  itkNewMacro(QmitkFastMarchingTool3DGUI);

  void OnThresholdChanged(int current);


  protected slots:

    void OnNewToolAssociated(mitk::Tool*);

    void OnUpperThresholdChanged(double);
    void OnLowerThresholdChanged(double);
    void OnAlphaChanged(double);
    void OnBetaChanged(double);
    void OnStoppingValueChanged(double);
    void OnConfirmSegmentation();
    void OnLivePreviewCheckBoxChanged(int value);
    void OnStepperRefetch();
    void OnClearSeeds();

protected:

  QmitkFastMarchingTool3DGUI();
  virtual ~QmitkFastMarchingTool3DGUI();

  ctkSliderWidget* m_UpperThresholdSlider;
  ctkSliderWidget* m_LowerThresholdSlider;
  ctkSliderWidget* m_StoppingValueSlider;
  ctkSliderWidget* m_AlphaSlider;
  ctkSliderWidget* m_BetaSlider;

  QPushButton* m_btConfirm;
  QPushButton* m_btClearSeeds;
  QCheckBox* m_cbxLivePreview;

  mitk::FastMarchingTool3D::Pointer m_FastMarchingTool;

  bool m_TimeIsConnected;
  QmitkStepperAdapter* m_TimeStepper;
};

#endif
