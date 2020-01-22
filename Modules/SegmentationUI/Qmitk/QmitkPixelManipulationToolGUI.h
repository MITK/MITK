/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef QMITKPIXELMANIPULATIONTOOLGUI_H
#define QMITKPIXELMANIPULATIONTOOLGUI_H

#include "QmitkDataStorageComboBox.h"
#include "QmitkToolGUI.h"
#include "mitkPixelManipulationTool.h"
#include <QSlider>
#include <QSpinBox>

class QmitkPixelManipulationToolGUI : public QmitkToolGUI
{
  Q_OBJECT
public:
  mitkClassMacro(QmitkPixelManipulationToolGUI, QmitkToolGUI);
  itkFactorylessNewMacro(Self);
  itkCloneMacro(Self);

    protected slots :

    void OnNewToolAssociated(mitk::Tool *);
  void OnSliderValueChanged(int);
  void OnSpinBoxChanged();
  void OnOkButtonClicked();
  void SetFixedValueOn(bool);
  void SetFixedValueOff(bool);

protected:
  QmitkPixelManipulationToolGUI();
  ~QmitkPixelManipulationToolGUI() override;

  mitk::PixelManipulationTool::Pointer m_PixelManipulationTool;
  QSlider *m_Slider;
  QSpinBox *m_Spinner;

}; // class
#endif
