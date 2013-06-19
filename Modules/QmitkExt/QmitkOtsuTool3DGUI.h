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

#ifndef QmitkOtsuTool3DGUI_h_Included
#define QmitkOtsuTool3DGUI_h_Included

#include "QmitkToolGUI.h"
#include "QmitkExtExports.h"
#include "mitkOtsuTool3D.h"
#include <QListWidget.h>

class QSpinBox;
class QLabel;

/**
  \ingroup org_mitk_gui_qt_interactivesegmentation_internal
  \brief GUI for mitk::.
  \sa mitk::

  This GUI shows ...

  Last contributor: $Author$
*/
class QmitkExt_EXPORT QmitkOtsuTool3DGUI : public QmitkToolGUI
{
  Q_OBJECT

  public:
    mitkClassMacro(QmitkOtsuTool3DGUI, QmitkToolGUI);
    itkNewMacro(QmitkOtsuTool3DGUI);

  signals:

  public slots:

  protected slots:

    void OnNewToolAssociated(mitk::Tool*);

    void OnSpinboxValueAccept();

    void OnSegmentationRegionAccept();

    void OnItemSelectionChanged();

    void OnVolumePreviewChecked(int);

  protected:

    QmitkOtsuTool3DGUI();
    virtual ~QmitkOtsuTool3DGUI();

    QSpinBox* m_Spinbox;
    QLabel* m_SizeLabel;
    QListWidget* m_selectionListWidget;

    mitk::OtsuTool3D::Pointer m_OtsuTool3DTool;
};

#endif
