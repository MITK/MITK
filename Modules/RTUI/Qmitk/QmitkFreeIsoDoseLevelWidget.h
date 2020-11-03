/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef QMITK_FREE_ISO_DOSE_LEVEL_WIDGET_H
#define QMITK_FREE_ISO_DOSE_LEVEL_WIDGET_H

#include "MitkRTUIExports.h"

#include "ui_QmitkFreeIsoDoseLevelWidget.h"
#include <QWidget>

#include "mitkIsoDoseLevel.h"

/**
* \class QmitkFreeIsoDoseLevelWidget
* \brief Widget that allows to show and edit the content of an mitk::IsoDoseLevel instance.
*/
class MITKRTUI_EXPORT QmitkFreeIsoDoseLevelWidget : public QWidget, private Ui::QmitkFreeIsoDoseLevelWidget
{
  Q_OBJECT

public:
  explicit QmitkFreeIsoDoseLevelWidget(QWidget* parent=nullptr);

  mitk::DoseValueAbs getReferenceDose() const;
  mitk::IsoDoseLevel* getIsoDoseLevel() const;

signals:
  void ValueChanged(mitk::IsoDoseLevel*, mitk::DoseValueRel oldValue);
  void ColorChanged(mitk::IsoDoseLevel*);
  void VisualizationStyleChanged(mitk::IsoDoseLevel*);

  public Q_SLOTS:
    /**
    * \brief Slot that can be used to set the reference dose.
    */
    void setReferenceDose(double newReferenceDose);

    /**
    * \brief Slot that can be used to set the dose level instance that should be handled by the widget.
    */
    void setIsoDoseLevel(mitk::IsoDoseLevel* level);

    void OnRelValueChanged(double newValue);
    void OnAbsValueChanged(double newValue);
    void OnSliderChanged(int newValue);
    void OnVisibleClicked(bool checked);
    void OnColorChanged(QColor color);

protected:

  /**
  * \brief Updates the widget according to its current settings.
  */
  void update();
  void updateValue(mitk::DoseValueRel newDose);

  mitk::DoseValueAbs m_ReferenceDose;
  mitk::IsoDoseLevel::Pointer m_IsoDoseLevel;
  bool m_InternalUpdate;

};

#endif // QmitkFreeIsoDoseLevelWidget_H
