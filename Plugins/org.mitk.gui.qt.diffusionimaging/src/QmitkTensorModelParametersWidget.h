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
#ifndef _QMITKTensorModelParametersWidget_H_INCLUDED
#define _QMITKTensorModelParametersWidget_H_INCLUDED

//QT headers
#include <QWidget>
#include <QString>
#include "ui_QmitkTensorModelParametersWidgetControls.h"
#include <org_mitk_gui_qt_diffusionimaging_Export.h>

class QmitkStdMultiWidget;

/** @brief
  */
class DIFFUSIONIMAGING_EXPORT QmitkTensorModelParametersWidget : public QWidget
{
    //this is needed for all Qt objects that should have a MOC object (everything that derives from QObject)
    Q_OBJECT

public:

    static const std::string VIEW_ID;

    QmitkTensorModelParametersWidget (QWidget* parent = 0, Qt::WindowFlags f = 0);
    virtual ~QmitkTensorModelParametersWidget();

    virtual void CreateQtPartControl(QWidget *parent);

    double GetD1(){ return m_Controls->m_D1box->value(); }
    double GetD2(){ return m_Controls->m_D2box->value(); }
    double GetD3(){ return m_Controls->m_D3box->value(); }
    unsigned int GetT2(){ return m_Controls->m_T2box->value(); }

public slots:

    void DChanged( double value );

protected:
    // member variables
    Ui::QmitkTensorModelParametersWidgetControls* m_Controls;
    void UpdateUi();

private:

};

#endif // _QMITKTensorModelParametersWidget_H_INCLUDED

