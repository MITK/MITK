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

#ifndef QmitkSmoothTool3DGUI_h_Included
#define QmitkSmoothTool3DGUI_h_Included

#include "QmitkToolGUI.h"
#include "SegmentationUIExports.h"
#include "mitkSmoothTool3D.h"

#include <QDoubleSpinBox>

class QSlider;
/**
  \ingroup org_mitk_gui_qt_interactivesegmentation_internal
  \brief GUI for mitk::SmoothTool3D.

  This GUI shows ...

  Last contributor: $Author$
*/
class SegmentationUI_EXPORT QmitkSmoothTool3DGUI : public QmitkToolGUI
{
  Q_OBJECT

  public:

    mitkClassMacro(QmitkSmoothTool3DGUI, QmitkToolGUI);
    itkNewMacro(QmitkSmoothTool3DGUI);

  signals:

    /// \brief Emitted when button "Run" is pressed
    void Run();

  public slots:

  protected slots:

    void OnNewToolAssociated(mitk::Tool*);
    void OnRun();

  protected:
    QmitkSmoothTool3DGUI();
    virtual ~QmitkSmoothTool3DGUI();

    void BusyStateChanged(bool);

    mitk::SmoothTool3D::Pointer m_SmoothTool3D;
};

#endif
