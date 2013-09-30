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

#ifndef QmitkMedianTool3DGUI_h_Included
#define QmitkMedianTool3DGUI_h_Included

#include "QmitkToolGUI.h"
#include "SegmentationUIExports.h"
#include "mitkMedianTool3D.h"

#include <QDoubleSpinBox>

class QSlider;
/**
  \ingroup org_mitk_gui_qt_interactivesegmentation_internal
  \brief GUI for mitk::MedianTool3D.

  This GUI shows a slider to change the tool's threshold and an OK button to accept a preview for actual thresholding.

  There is only a slider for INT values in QT. So, if the working image has a float/double pixeltype, we need to convert
  the original float intensity into a respective int value for the slider. The slider range is then between 0 and 99.

  If the pixeltype is INT, then we do not need any conversion.

  Last contributor: $Author$
*/
class SegmentationUI_EXPORT QmitkMedianTool3DGUI : public QmitkToolGUI
{
  Q_OBJECT

  public:

    mitkClassMacro(QmitkMedianTool3DGUI, QmitkToolGUI);
    itkNewMacro(QmitkMedianTool3DGUI);

  signals:

    /// \brief Emitted when button "Run" is pressed
    void Run();

  public slots:

  protected slots:

    void OnNewToolAssociated(mitk::Tool*);
    void OnRun();

  protected:
    QmitkMedianTool3DGUI();
    virtual ~QmitkMedianTool3DGUI();

    void BusyStateChanged(bool);

    mitk::MedianTool3D::Pointer m_MedianTool3D;
};

#endif
