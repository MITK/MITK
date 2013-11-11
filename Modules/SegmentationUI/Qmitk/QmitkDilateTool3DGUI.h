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

#ifndef QmitkDilateTool3DGUI_h_Included
#define QmitkDilateTool3DGUI_h_Included

#include "QmitkToolGUI.h"
#include "SegmentationUIExports.h"
#include "mitkDilateTool3D.h"

#include <QDoubleSpinBox>

class QSlider;
/**
  \ingroup org_mitk_gui_qt_interactivesegmentation_internal
  \brief GUI for mitk::DilateTool3D

  This GUI shows ...

  Last contributor: $Author$
*/
class SegmentationUI_EXPORT QmitkDilateTool3DGUI : public QmitkToolGUI
{
  Q_OBJECT

  public:

    mitkClassMacro(QmitkDilateTool3DGUI, QmitkToolGUI);
    itkNewMacro(QmitkDilateTool3DGUI);

  signals:

    /// \brief Emitted when button "Run" is pressed
    void Run();

  public slots:

  protected slots:

    void OnNewToolAssociated(mitk::Tool*);
    void OnRun();

  protected:
    QmitkDilateTool3DGUI();
    virtual ~QmitkDilateTool3DGUI();

    void BusyStateChanged(bool);

    mitk::DilateTool3D::Pointer m_DilateTool3D;
};

#endif
