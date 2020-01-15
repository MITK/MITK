/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef PASimulator_h
#define PASimulator_h

#include <berryISelectionListener.h>

#include <QmitkAbstractView.h>

#include "ui_PASimulatorControls.h"

#include "mitkPATissueGenerator.h"
#include "mitkPATissueGeneratorParameters.h"
#include "mitkPAInSilicoTissueVolume.h"
#include "mitkPAPropertyCalculator.h"
#include "mitkPASimulationBatchGenerator.h"

/**
  \brief PASimulator

  \warning  This class is not yet documented. Use "git blame" and ask the author to provide basic documentation.

  \sa QmitkAbstractView
  \ingroup ${plugin_target}_internal
*/
class PASimulator : public QmitkAbstractView
{
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

public:

  static const std::string VIEW_ID;

  protected slots:

  /// \brief Called when the user clicks the GUI button
  void DoImageProcessing();
  void ClickedCheckboxFixedSeed();
  void ClickedRandomizePhysicalParameters();
  void OpenFolder();
  void OpenBinary();
  void UpdateVisibilityOfBatchCreation();
  void UpdateParametersAccordingToWavelength();

protected:

  void CreateQtPartControl(QWidget *parent) override;

  void SetFocus() override;

  Ui::PASimulatorControls m_Controls;

  mitk::pa::PropertyCalculator::Pointer m_PhotoacousticPropertyCalculator;

private:

  mitk::pa::TissueGeneratorParameters::Pointer GetParametersFromUIInput();
};

#endif // PASimulator_h
