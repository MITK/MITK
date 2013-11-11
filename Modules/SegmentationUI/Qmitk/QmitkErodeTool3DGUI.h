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

#ifndef QmitkErodeTool3DGUI_h_Included
#define QmitkErodeTool3DGUI_h_Included

#include "QmitkToolGUI.h"
#include "SegmentationUIExports.h"
#include "mitkErodeTool3D.h"

#include <QDoubleSpinBox>

class QSlider;
/**
  \ingroup org_mitk_gui_qt_interactivesegmentation_internal
  \brief GUI for mitk::ErodeTool3D.

  This GUI shows ...

  Last contributor: $Author$
*/
class SegmentationUI_EXPORT QmitkErodeTool3DGUI : public QmitkToolGUI
{
  Q_OBJECT

  public:

    mitkClassMacro(QmitkErodeTool3DGUI, QmitkToolGUI);
    itkNewMacro(QmitkErodeTool3DGUI);

  signals:

    /// \brief Emitted when button "Run" is pressed
    void Run();

  public slots:

  protected slots:

    void OnNewToolAssociated(mitk::Tool*);
    void OnRun();

  protected:
    QmitkErodeTool3DGUI();
    virtual ~QmitkErodeTool3DGUI();

    void BusyStateChanged(bool);

    mitk::ErodeTool3D::Pointer m_ErodeTool3D;
};

#endif
