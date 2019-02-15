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

#ifndef QMITKUSNAVIGATIONSTEPCOMBINEDMODALITY_H
#define QMITKUSNAVIGATIONSTEPCOMBINEDMODALITY_H

#include "QmitkUSAbstractNavigationStep.h"

namespace Ui {
class QmitkUSNavigationStepCombinedModality;
}

/**
 * \brief Navigation step for creating and selecting a combined modality.
 * Already created combined modalities can be selected from a service list
 * widget and calibrations can be loaded for them. New combined modalities can
 * be created from two service list widgets of NavigationDataSources and
 * USDevices.
 */
class QmitkUSNavigationStepCombinedModality : public QmitkUSAbstractNavigationStep
{
  Q_OBJECT

protected slots:
  /**
   * \brief Triggered, when the selection in the service list widget has changed.
   */
  void OnDeviceSelectionChanged();

  /**
   * \brief Triggered, when the user has clicked "Load Calibration".
   * Opens a file open dialog and sets the selected calibration for the selected
   * combined modality.
   */
  void OnLoadCalibration();

  /**
   * \brief Triggered, when the button for creating a new combined modality was clicked.
   */
  void OnCombinedModalityCreateNewButtonClicked();

  /**
   * \brief Triggered, when the dialog for creating a new combined modality was closed.
   */
  void OnCombinedModalityCreationExit();

  void OnCombinedModalityEditExit();

  /**
   * \brief Triggered, when the button for deleting a combined modality was clicked.
   * Unregisters the combined modality from the microservice and removes it.
   */
  void OnDeleteButtonClicked();

  void OnCombinedModalityEditButtonClicked();

  void OnActivateButtonClicked();

  /**
   * \brief Triggered, when the button for disconnecting a combined modality was clicked.
   * The state of the combined modality is changed to 'initialized'.
   */
  void OnDisconnectButtonClicked();

public:
  explicit QmitkUSNavigationStepCombinedModality(QWidget *parent = 0);
  ~QmitkUSNavigationStepCombinedModality();

  virtual bool OnStartStep();
  virtual bool OnRestartStep();
  virtual bool OnFinishStep();

  virtual bool OnActivateStep();
  virtual void OnUpdate();

  virtual QString GetTitle();
  virtual bool GetIsRestartable();

  mitk::AbstractUltrasoundTrackerDevice::Pointer GetSelectedCombinedModality();

  void SetCalibrationLoadedNecessary(bool);

  void OnDevicePropertyChanged(const std::string&, const std::string&);

protected:
  void SetCombinedModalityCreateWidgetEnabled(bool enabled);
  void SetCombinedModalityEditWidgetEnabled(bool enabled);
  void CreateCombinedModalityResultAndSignalIt();

  bool UpdateCalibrationState();
  void UpdateTrackingToolNames();

  /**
   * \brief The Combined Modality which was selected by the user.
   */
  itk::SmartPointer<mitk::AbstractUltrasoundTrackerDevice> m_CombinedModality;

  std::string m_LoadedCalibration; //<<< Temporary saving point for loaded calibration data.
  std::string m_LastCalibrationFilename; //<<< saves the filename last loaded calibration

  bool m_CalibrationLoadedNecessary;

private:
  mitk::MessageDelegate2<QmitkUSNavigationStepCombinedModality, const std::string&, const std::string&> m_ListenerDeviceChanged;

  Ui::QmitkUSNavigationStepCombinedModality *ui;
};

#endif // QMITKUSNAVIGATIONSTEPCOMBINEDMODALITY_H
