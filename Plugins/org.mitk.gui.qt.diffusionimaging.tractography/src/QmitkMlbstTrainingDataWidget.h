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
#ifndef _QmitkMlbstTrainingDataWidget_H_INCLUDED
#define _QmitkMlbstTrainingDataWidget_H_INCLUDED

//QT headers
#include <QWidget>
#include <QString>
#include "ui_QmitkMlbstTrainingDataWidgetControls.h"
#include <org_mitk_gui_qt_diffusionimaging_tractography_Export.h>
#include <mitkDataNode.h>
#include <mitkDataStorage.h>

/** @brief
  */
class DIFFUSIONIMAGING_TRACTOGRAPHY_EXPORT QmitkMlbstTrainingDataWidget : public QWidget
{
    //this is needed for all Qt objects that should have a MOC object (everything that derives from QObject)
    Q_OBJECT

public:

    static const std::string VIEW_ID;

    QmitkMlbstTrainingDataWidget (QWidget* parent = nullptr, Qt::WindowFlags f = nullptr);

    virtual ~QmitkMlbstTrainingDataWidget();

    virtual void CreateQtPartControl(QWidget *parent);

    mitk::DataNode::Pointer GetImage(){ return m_Controls->image->GetSelectedNode(); }
    mitk::DataNode::Pointer GetFibers(){ return m_Controls->fibers->GetSelectedNode(); }
    mitk::DataNode::Pointer GetMask(){ return m_Controls->mask->GetSelectedNode(); }
    mitk::DataNode::Pointer GetWhiteMatter(){ return m_Controls->whiteMatter->GetSelectedNode(); }
    void SetDataStorage(mitk::DataStorage::Pointer ds)
    {
        m_Controls->image->SetDataStorage(ds);
        m_Controls->fibers->SetDataStorage(ds);
        m_Controls->mask->SetDataStorage(ds);
        m_Controls->whiteMatter->SetDataStorage(ds);
        m_Controls->mask->SetZeroEntryText("--");
        m_Controls->whiteMatter->SetZeroEntryText("--");
    }

public slots:

protected:
    // member variables
    Ui::QmitkMlbstTrainingDataWidgetControls* m_Controls;

private:

};

#endif // _QmitkMlbstTrainingDataWidget_H_INCLUDED

