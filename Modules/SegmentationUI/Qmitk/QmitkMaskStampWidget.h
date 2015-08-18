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

#ifndef QmitkMaskStampWidget_h_Included
#define QmitkMaskStampWidget_h_Included

#include "MitkSegmentationUIExports.h"
#include "mitkDataNode.h"

#include <QWidget>

#include "ui_QmitkMaskStampWidgetGUIControls.h"

namespace mitk {
  class ToolManager;
}

/**
  \brief GUI for mask stamp functionality

  \ingroup ToolManagerEtAl
  \ingroup Widgets
*/

class MITKSEGMENTATIONUI_EXPORT QmitkMaskStampWidget : public QWidget
{
  Q_OBJECT

  public:

    QmitkMaskStampWidget(QWidget* parent = 0, const char* name = 0);
    virtual ~QmitkMaskStampWidget();

    void SetDataStorage( mitk::DataStorage* storage );

  protected slots:

    void OnShowInformation(bool);

    void OnStamp();

  private:

    mitk::ToolManager* m_ToolManager;

    mitk::DataStorage* m_DataStorage;

    Ui::QmitkMaskStampWidgetGUIControls m_Controls;

};

#endif
