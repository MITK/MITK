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
#ifndef _QMITKDotModelParametersWidget_H_INCLUDED
#define _QMITKDotModelParametersWidget_H_INCLUDED

//QT headers
#include <QWidget>
#include <QString>
#include "ui_QmitkDotModelParametersWidgetControls.h"
#include <org_mitk_gui_qt_diffusionimaging_Export.h>

class QmitkStdMultiWidget;

/** @brief
  */
class DIFFUSIONIMAGING_EXPORT QmitkDotModelParametersWidget : public QWidget
{
    //this is needed for all Qt objects that should have a MOC object (everything that derives from QObject)
    Q_OBJECT

public:

    static const std::string VIEW_ID;

    QmitkDotModelParametersWidget (QWidget* parent = 0, Qt::WindowFlags f = 0);
    virtual ~QmitkDotModelParametersWidget();

    virtual void CreateQtPartControl(QWidget *parent);

    void SetT2(double t2){ m_Controls->m_T2box->setValue(t2); }
    unsigned int GetT2(){ return m_Controls->m_T2box->value(); }

public slots:

protected:
    // member variables
    Ui::QmitkDotModelParametersWidgetControls* m_Controls;

private:

};

#endif // _QMITKDotModelParametersWidget_H_INCLUDED

