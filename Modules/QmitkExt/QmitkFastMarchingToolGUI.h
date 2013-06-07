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

#ifndef QmitkFastMarchingToolGUI_h_Included
#define QmitkFastMarchingToolGUI_h_Included

#include "QmitkToolGUI.h"
#include "QmitkExtExports.h"
#include "mitkFastMarchingTool.h"

class QSlider;
class QLabel;
class QFrame;
class QPushButton;
#include <QCheckBox>;


/**
\ingroup org_mitk_gui_qt_interactivesegmentation_internal
\brief GUI for mitk::FastMarchingTool.
\sa mitk::FastMarchingTool
*/
class QmitkExt_EXPORT QmitkFastMarchingToolGUI : public QmitkToolGUI
{
  Q_OBJECT

public:
  mitkClassMacro(QmitkFastMarchingToolGUI, QmitkToolGUI);
  itkNewMacro(QmitkFastMarchingToolGUI);

  void OnThresholdChanged(int current);


  protected slots:

    void OnNewToolAssociated(mitk::Tool*);

    void OnUpperThresholdChanged(int value);
    void OnLowerThresholdChanged(int value);
    void OnMuChanged(int value);
    void OnStoppingValueChanged(int value);
    void OnStandardDeviationChanged(int value);
    void OnConfirmSegmentation();
    void OnLivePreviewCheckBoxChanged(int value);

    void OnClearSeeds();

protected:

  QmitkFastMarchingToolGUI();
  virtual ~QmitkFastMarchingToolGUI();

  QSlider* m_UpperThresholdSlider;
  QLabel* m_UpperThresholdLabel;

  QSlider* m_LowerThresholdSlider;
  QLabel* m_LowerThresholdLabel;

  QSlider* m_StoppingValueSlider;
  QLabel* m_StoppingValueLabel;

  QSlider* m_MuSlider;
  QLabel* m_MuLabel;

  QSlider* m_StandardDeviationSlider;
  QLabel* m_StandardDeviationLabel;

  QPushButton* m_ConfirmButton;
  QPushButton* m_ClearSeedsButton;
  QCheckBox* m_LivePreviewCheckBox;

  mitk::FastMarchingTool::Pointer m_FastMarchingTool;

};

#endif
