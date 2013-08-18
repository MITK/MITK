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
#ifndef QMITKPIXELMANIPULATIONTOOLGUI_H
#define QMITKPIXELMANIPULATIONTOOLGUI_H

#include "QmitkToolGUI.h"
#include "mitkPixelManipulationTool.h"
#include "QmitkDataStorageComboBox.h"
#include <QSlider>
#include <QSpinBox>

class QmitkPixelManipulationToolGUI : public QmitkToolGUI
{
  Q_OBJECT
public:

  mitkClassMacro(QmitkPixelManipulationToolGUI, QmitkToolGUI);
  itkNewMacro(QmitkPixelManipulationToolGUI);

  protected slots:

    void OnNewToolAssociated(mitk::Tool*);
    void OnSliderValueChanged(int);
    void OnSpinBoxChanged();
    void OnOkButtonClicked();
    void SetFixedValueOn(bool);
    void SetFixedValueOff(bool);

protected:

  QmitkPixelManipulationToolGUI();
  virtual ~QmitkPixelManipulationToolGUI();

  mitk::PixelManipulationTool::Pointer m_PixelManipulationTool;
  QSlider* m_Slider;
  QSpinBox* m_Spinner;

};//class
#endif
