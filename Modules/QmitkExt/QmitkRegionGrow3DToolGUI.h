/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: 28959 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef QMITKREGIONGROW3DTOOLGUI_H
#define QMITKREGIONGROW3DTOOLGUI_H

#include "QmitkToolGUI.h"
#include "mitkRegionGrow3DTool.h"
#include "QmitkPointListWidget.h"

#include <QSpinBox>
#include <QSlider>
#include <QLabel>
#include <QRadioButton>

class QmitkRegionGrow3DToolGUI : public QmitkToolGUI
{
  Q_OBJECT

public:
  mitkClassMacro(QmitkRegionGrow3DToolGUI, QmitkToolGUI);

  itkNewMacro(QmitkRegionGrow3DToolGUI);

  protected slots:
    void OnNewToolAssociated(mitk::Tool*);

    void OnSeedButtonToggled(bool);
    void DecreaseSlider();
    void IncreaseSlider();
    void SliderValueChanged(int);
    void OnConfirmButtonClicked();
    void OnGrowingDirectionChanged(bool);

protected:

  virtual ~QmitkRegionGrow3DToolGUI();
  QmitkRegionGrow3DToolGUI();

  void OnLowerThresholdValueChanged(int);
  void OnUpperThresholdValueChanged(int);

  mitk::RegionGrow3DTool::Pointer m_RegionGrow3DTool;

  QPushButton* m_SetSeedButton;
  QPushButton* m_DecreaseTHButton;
  QPushButton* m_IncreaseTHButton;
  QPushButton* m_ConfirmButton;
  QRadioButton* m_brightRadio;
  QRadioButton* m_darkRadio;
  QSlider* m_ThSlider;
  QLabel* m_ThValue;

};//class

#endif
#ifndef QMITKREGIONGROW3DTOOLGUI_H
#define QMITKREGIONGROW3DTOOLGUI_H

#include "QmitkToolGUI.h"
#include "mitkRegionGrow3DTool.h"
#include "QmitkPointListWidget.h"

#include <QSpinBox>
#include <QSlider>
#include <QLabel>
#include <QRadioButton>

class QmitkRegionGrow3DToolGUI : public QmitkToolGUI
{
  Q_OBJECT

public:
  mitkClassMacro(QmitkRegionGrow3DToolGUI, QmitkToolGUI);

  itkNewMacro(QmitkRegionGrow3DToolGUI);

  protected slots:
    void OnNewToolAssociated(mitk::Tool*);

    void OnSeedButtonToggled(bool);
    void DecreaseSlider();
    void IncreaseSlider();
    void SliderValueChanged(int);
    void OnConfirmButtonClicked();
    void OnGrowingDirectionChanged(bool);

protected:

  virtual ~QmitkRegionGrow3DToolGUI();
  QmitkRegionGrow3DToolGUI();

  void OnLowerThresholdValueChanged(int);
  void OnUpperThresholdValueChanged(int);

  mitk::RegionGrow3DTool::Pointer m_RegionGrow3DTool;

  QPushButton* m_SetSeedButton;
  QPushButton* m_DecreaseTHButton;
  QPushButton* m_IncreaseTHButton;
  QPushButton* m_ConfirmButton;
  QRadioButton* m_brightRadio;
  QRadioButton* m_darkRadio;
  QSlider* m_ThSlider;
  QLabel* m_ThValue;

};//class

#endif
