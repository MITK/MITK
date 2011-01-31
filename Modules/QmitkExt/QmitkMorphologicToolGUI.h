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
#ifndef QMITKMORPHOLOGICTOOLGUI_H
#define QMITKMORPHOLOGICTOOLGUI_H

#include "QmitkToolGUI.h"
#include "mitkMorphologicTool.h"
#include "QmitkExtExports.h"
#include <QSlider>
#include <QSpinBox>
#include <QCheckBox>

class /*QmitkExt_EXPORTS*/ QmitkMorphologicToolGUI : public QmitkToolGUI
{
  Q_OBJECT
public:

  mitkClassMacro(QmitkMorphologicToolGUI, QmitkToolGUI);
  itkNewMacro(QmitkMorphologicToolGUI);

  protected slots:

    void OnNewToolAssociated(mitk::Tool*);
    void OnSliderValueChanged(int value);
    void OnSpinBoxValueChanged();
    void OnCheckStateChanged(int state);
    void OnStructElementChanged(int id);
    void OnAcceptPreview();

protected:

  virtual ~QmitkMorphologicToolGUI();
  QmitkMorphologicToolGUI();

  mitk::MorphologicTool::Pointer m_MorphologicTool;
  QSlider* m_Slider;
  QSpinBox* m_SpinBox;
  QCheckBox* m_CheckBox;

};

#endif