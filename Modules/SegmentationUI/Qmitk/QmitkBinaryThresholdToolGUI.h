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

#ifndef QmitkBinaryThresholdToolGUI_h_Included
#define QmitkBinaryThresholdToolGUI_h_Included

#include "QmitkToolGUI.h"
#include "SegmentationUIExports.h"
#include "mitkBinaryThresholdTool.h"

#include "ui_QmitkBinaryThresholdToolGUIControls.h"

namespace mitk {
  class BinaryThresholdTool;
}

/**
  \ingroup org_mitk_gui_qt_interactivesegmentation_internal
  \brief GUI for mitk::BinaryThresholdTool.

  This GUI shows a slider to change the tool's threshold ...
*/
class SegmentationUI_EXPORT QmitkBinaryThresholdToolGUI : public QmitkToolGUI
{
  Q_OBJECT

  public:

    mitkClassMacro(QmitkBinaryThresholdToolGUI, QmitkToolGUI);
    itkNewMacro(QmitkBinaryThresholdToolGUI);

    void OnThresholdingIntervalBordersChanged(double lower, double upper, bool isFloat);
    void OnThresholdingValueChanged(double current);

  protected slots:

    void OnNewToolAssociated(mitk::Tool*);
    void OnCancel();
    void OnAcceptPreview();
    void OnInvertPreview();
    void OnNewLabel();
    void OnShowInformation(bool);
    void OnShowAdvancedControls(bool);

    /// \brief Called when the Slider value has changed.
    void OnThresholdSliderValueChanged(double value);

  protected:
    QmitkBinaryThresholdToolGUI();
    virtual ~QmitkBinaryThresholdToolGUI();

    /// \brief changes the cursor icon
    void BusyStateChanged(bool);

    /// \brief is image float or int?
    bool m_isFloat;

    /// \brief the GUI controls
    Ui::QmitkBinaryThresholdToolGUIControls m_Controls;

    /// \brief the tool this GUI is made for
    mitk::BinaryThresholdTool* m_BinaryThresholdTool;
};

#endif
