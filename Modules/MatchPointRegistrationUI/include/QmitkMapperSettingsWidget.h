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

#include <QWidget>
#include <memory>

struct QmitkMappingJobSettings;

namespace Ui
{
  class QmitkMapperSettingsWidget;
}

/**
 * \class QmitkMapperSettingsWidget
 * \brief Widget for configuring image mapping / geometry refinement settings.
 *
 * This widget allows the user to configure how images are mapped using a registration,
 * including interpolation method, handling of undefined and unregistered pixels,
 * padding/error values, and optional super/sub-sampling factors (with linked or
 * independent axis control).
 *
 * The widget supports a "mask mode" that restricts interpolation to nearest-neighbour
 * only, which is required when mapping segmentation masks to preserve label integrity.
 *
 * Use ConfigureJobSettings() to transfer the current widget state into a
 * QmitkMappingJobSettings instance before starting a mapping job.
 *
 * \sa QmitkMappingJobSettings, QmitkMappingJob, QmitkFramesRegistrationJob
 */
class MITKMATCHPOINTREGISTRATIONUI_EXPORT QmitkMapperSettingsWidget : public QWidget
{
  Q_OBJECT

public:
  /**
   * \brief Constructs the mapper settings widget.
   * \param[in] parent Optional parent widget.
   */
  QmitkMapperSettingsWidget(QWidget *parent = nullptr);

  /** \brief Destructor. */
  ~QmitkMapperSettingsWidget() override;

  /**
   * \brief Populates a QmitkMappingJobSettings instance from the current widget state.
   *
   * Copies the interpolation type, undefined/unregistered pixel handling, padding value,
   * and error value from the UI controls into the provided settings object.
   *
   * \param[in,out] settings Pointer to the settings instance to configure.
   * \pre \p settings must not be \c nullptr.
   */
  void ConfigureJobSettings(QmitkMappingJobSettings *settings);

public Q_SLOTS:
  /**
   * \brief Enables or disables mask mode.
   *
   * In mask mode, the interpolation method is forced to nearest-neighbour and
   * error/padding values are set to 0. This is required when mapping segmentation
   * masks to avoid interpolation artifacts.
   *
   * \param[in] activeMask If true, activates mask mode; if false, deactivates it
   *   and resets interpolation to linear.
   */
  void SetMaskMode(bool activeMask);

  /**
   * \brief Enables or disables the super/sub-sampling controls.
   *
   * When disabled, the sampling factor group is hidden from the UI.
   *
   * \param[in] allow If true, sampling controls are shown and available;
   *   if false, they are hidden.
   */
  void AllowSampling(bool allow);

protected Q_SLOTS:
  void OnLinkSampleFactorChecked();

  void OnXFactorChanged(double d);

protected:
  std::unique_ptr<Ui::QmitkMapperSettingsWidget> m_Controls;
  bool m_MaskMode;
  bool m_allowSampling;
};

#endif
