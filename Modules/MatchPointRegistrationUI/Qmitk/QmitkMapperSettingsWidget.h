/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkMapperSettingsWidget_h
#define QmitkMapperSettingsWidget_h

#include <MitkMatchPointRegistrationUIExports.h>

#include "ui_QmitkMapperSettingsWidget.h"
#include <QWidget>

struct QmitkMappingJobSettings;

/**
 * \class QmitkMapperSettingsWidget
 * \brief Widget that views the information and profile of an algorithm stored in an DLLInfo object.
 */
class MITKMATCHPOINTREGISTRATIONUI_EXPORT QmitkMapperSettingsWidget : public QWidget,
                                                                      private Ui::QmitkMapperSettingsWidget
{
  Q_OBJECT

public:
  QmitkMapperSettingsWidget(QWidget *parent = nullptr);

  /**
   * Configures the passed settings according to the current state of the
   * widget.
   * \param settings to a instance based on QmitkMappingJobSettings.
   * \pre settings must point to a valid instance..
   */
  void ConfigureJobSettings(QmitkMappingJobSettings *settings);

public Q_SLOTS:
  /**
    * \brief Slot that can be used to set the mode for the mapping settings.
    * Mask mode allows only nearest neighbour interpolation. It is needed for exmample
    * when mapping segmentations.*/
  void SetMaskMode(bool activeMask);

  /**
  * \brief Slot that can be used to set the widget to a mode where super/sub sampling
  * is allowed (true) or not (false).*/
  void AllowSampling(bool allow);

protected Q_SLOTS:
  void OnLinkSampleFactorChecked();

  void OnXFactorChanged(double d);

protected:
  bool m_MaskMode;
  bool m_allowSampling;
};

#endif
