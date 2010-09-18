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
    void OnSpinBoxValueChanged(int value);
    void OnCheckStateChanged(int state);
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