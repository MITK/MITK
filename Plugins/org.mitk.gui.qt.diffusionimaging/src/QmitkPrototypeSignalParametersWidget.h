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
#ifndef _QmitkPrototypeSignalParametersWidget_H_INCLUDED
#define _QmitkPrototypeSignalParametersWidget_H_INCLUDED

//QT headers
#include <QWidget>
#include <QString>
#include "ui_QmitkPrototypeSignalParametersWidgetControls.h"
#include <org_mitk_gui_qt_diffusionimaging_Export.h>

class QmitkStdMultiWidget;

/** @brief
  */
class DIFFUSIONIMAGING_EXPORT QmitkPrototypeSignalParametersWidget : public QWidget
{
    //this is needed for all Qt objects that should have a MOC object (everything that derives from QObject)
    Q_OBJECT

public:

    static const std::string VIEW_ID;

    QmitkPrototypeSignalParametersWidget (QWidget* parent = 0, Qt::WindowFlags f = 0);
    virtual ~QmitkPrototypeSignalParametersWidget();

    virtual void CreateQtPartControl(QWidget *parent);

    void SetMinAdc(double min){ m_Controls->m_MinAdcBox->setValue(min); }
    void SetMaxAdc(double max){ m_Controls->m_MaxAdcBox->setValue(max); }
    void SetMinFa(double min){ m_Controls->m_MinFaBox->setValue(min); }
    void SetMaxFa(double max){ m_Controls->m_MaxFaBox->setValue(max); }
    void SetNumberOfSamples(int n){ m_Controls->m_NumSamplesBox->setValue(n); }

    double GetMinAdc(){ return m_Controls->m_MinAdcBox->value(); }
    double GetMaxAdc(){ return m_Controls->m_MaxAdcBox->value(); }
    double GetMinFa(){ return m_Controls->m_MinFaBox->value(); }
    double GetMaxFa(){ return m_Controls->m_MaxFaBox->value(); }
    double GetNumberOfSamples(){ return m_Controls->m_NumSamplesBox->value(); }

public slots:

protected:
    // member variables
    Ui::QmitkPrototypeSignalParametersWidgetControls* m_Controls;

private:

};

#endif // _QmitkPrototypeSignalParametersWidget_H_INCLUDED

