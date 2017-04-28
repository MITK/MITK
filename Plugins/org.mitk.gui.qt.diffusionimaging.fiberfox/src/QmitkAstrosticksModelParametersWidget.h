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
#ifndef _QMITKAstrosticksModelParametersWidget_H_INCLUDED
#define _QMITKAstrosticksModelParametersWidget_H_INCLUDED

//QT headers
#include <QWidget>
#include <QString>
#include "ui_QmitkAstrosticksModelParametersWidgetControls.h"
#include <org_mitk_gui_qt_diffusionimaging_fiberfox_Export.h>

/** @brief
  */
class DIFFUSIONIMAGING_FIBERFOX_EXPORT QmitkAstrosticksModelParametersWidget : public QWidget
{
    //this is needed for all Qt objects that should have a MOC object (everything that derives from QObject)
    Q_OBJECT

public:

    static const std::string VIEW_ID;

    QmitkAstrosticksModelParametersWidget (QWidget* parent = nullptr, Qt::WindowFlags f = nullptr);
    virtual ~QmitkAstrosticksModelParametersWidget();

    virtual void CreateQtPartControl(QWidget *parent);

    void SetT1(double t1){ m_Controls->m_T1box->setValue(t1); }
    void SetT2(double t2){ m_Controls->m_T2box->setValue(t2); }
    void SetD(double d){ m_Controls->m_D1box->setValue(d); }
    void SetRandomizeSticks(bool r){ m_Controls->m_RandomCheck->setChecked(r); }

    double GetD(){ return m_Controls->m_D1box->value(); }
    unsigned int GetT2(){ return m_Controls->m_T2box->value(); }
    unsigned int GetT1(){ return m_Controls->m_T1box->value(); }
    bool GetRandomizeSticks(){ return m_Controls->m_RandomCheck->isChecked(); }

public slots:

protected:
    // member variables
    Ui::QmitkAstrosticksModelParametersWidgetControls* m_Controls;

private:

};

#endif // _QMITKAstrosticksModelParametersWidget_H_INCLUDED

