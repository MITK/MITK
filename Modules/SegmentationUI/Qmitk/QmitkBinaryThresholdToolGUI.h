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

#include <QDoubleSpinBox>

class QSlider;
/**
  \ingroup org_mitk_gui_qt_interactivesegmentation_internal
  \brief GUI for mitk::BinaryThresholdTool.

  This GUI shows a slider to change the tool's threshold and an OK button to accept a preview for actual thresholding.

  There is only a slider for INT values in QT. So, if the working image has a float/double pixeltype, we need to convert
  the original float intensity into a respective int value for the slider. The slider range is then between 0 and 99.

  If the pixeltype is INT, then we do not need any conversion.

  Last contributor: $Author$
*/
class SegmentationUI_EXPORT QmitkBinaryThresholdToolGUI : public QmitkToolGUI
{
  Q_OBJECT

  public:

    mitkClassMacro(QmitkBinaryThresholdToolGUI, QmitkToolGUI);
    itkNewMacro(QmitkBinaryThresholdToolGUI);

    void OnThresholdingIntervalBordersChanged(double lower, double upper, bool isFloat);
    void OnThresholdingValueChanged(double current);

  signals:

    /// \brief Emitted when threshold Accepted
    void thresholdAccepted();

    /// \brief Emitted when threshold Canceled
    void thresholdCanceled();

  public slots:

  protected slots:

    void OnNewToolAssociated(mitk::Tool*);
    void OnAcceptThresholdPreview();

    /// \brief Called when Spinner value has changed. Consider: Spinner contains DOUBLE values
    void OnSpinnerValueChanged();

    /// \brief Called when Slider value has changed. Consider: Slider contains INT values
    void OnSliderValueChanged(int value);

  protected:
    QmitkBinaryThresholdToolGUI();
    virtual ~QmitkBinaryThresholdToolGUI();

    /// \brief When Slider (int value) has changed, we need to convert it to a respective double value for the spinner
    double SliderIntToDouble(int val);

    /// \brief When Spinner (double value) has changed, we need to convert it to a respective int value for the slider
    int DoubleToSliderInt(double val);

    QSlider* m_Slider;
    QDoubleSpinBox* m_Spinner;

    /// \brief is image float or int?
    bool m_isFloat;

    double m_RangeMin;
    double m_RangeMax;
    double m_Range;

    /// \brief helper bool values to find out, which of the GUI elements has been touched by the user.
    bool m_ChangingSlider, m_ChangingSpinner;

    mitk::BinaryThresholdTool::Pointer m_BinaryThresholdTool;
};

#endif
