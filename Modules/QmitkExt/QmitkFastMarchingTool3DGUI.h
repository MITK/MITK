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
class QmitkExt_EXPORT QmitkFastMarchingTool3DGUI : public QmitkToolGUI
{
  Q_OBJECT

public:
  mitkClassMacro(QmitkFastMarchingTool3DGUI, QmitkToolGUI);
  itkNewMacro(QmitkFastMarchingTool3DGUI);

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

protected:

  QmitkFastMarchingTool3DGUI();
  virtual ~QmitkFastMarchingTool3DGUI();

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
  QCheckBox* m_LivePreviewCheckBox;

  mitk::FastMarchingTool3D::Pointer m_FastMarchingTool;
};

#endif
