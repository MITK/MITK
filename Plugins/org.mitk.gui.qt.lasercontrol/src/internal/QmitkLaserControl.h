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


#ifndef OPOLaserControl_h
#define OPOLaserControl_h

#include <berryISelectionListener.h>

#include <QmitkAbstractView.h>

#include "ui_QmitkLaserControlControls.h"

#include <mitkOpotekPumpLaserController.h>
#include <mitkOpotekLaser.h>
#include <mitkGalilMotor.h>
/**
  \brief OPOLaserControl

  \warning  This class is not yet documented. Use "git blame" and ask the author why he is a lazy fuck.

  \sa QmitkAbstractView
  \ingroup ${plugin_target}_internal
*/
class OPOLaserControl : public QmitkAbstractView
{
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

  public:

    static const std::string VIEW_ID;

  protected slots:

    /// \brief Called when the user clicks the GUI button
    void ConnectToLaser();
    void GetStatus();
    void SendCustomMessage();


    void InitLaser();
    void TuneWavelength();
    void StartFastTuning();
    void ToggleFlashlamp();
    void ToggleQSwitch();
    void SyncWavelengthSetBySlider();
    void SyncWavelengthSetBySpinBox();

  protected:

    virtual void CreateQtPartControl(QWidget *parent) override;

    virtual void SetFocus() override;

    /// \brief called by QmitkFunctionality when DataManager's selection has changed
    virtual void OnSelectionChanged( berry::IWorkbenchPart::Pointer source,
                                     const QList<mitk::DataNode::Pointer>& nodes ) override;

    Ui::OPOLaserControlControls m_Controls;
    bool m_LaserSystemConnected;
    bool m_SyncFromSpinBox;
    bool m_SyncFromSlider;

    mitk::OpotekPumpLaserController::Pointer m_PumpLaserController;
    mitk::OpotekLaser::Pointer m_OpotekLaserSystem;
    mitk::GalilMotor::Pointer m_OPOMotor;

};

#endif // OPOLaserControl_h
