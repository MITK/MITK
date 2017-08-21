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

#include <QFuture>
#include <QFutureWatcher>
#include <qthread.h>

// Photoacoustics Hardware
#include <mitkQuantelLaser.h>
#include <mitkGalilMotor.h>
//#include <mitkOphirPyro.h>

#include <vector>

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
    void GetState();

    void InitLaser(bool successLaser, bool successMotor);
    void ResetLaser(bool successLaser, bool successMotor);
    void InitResetLaser();
    void TuneWavelength();
    void StartFastTuning();

    void ShutterCountDown();
    void EnableLaser();
    void ShowEnergy();

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
    bool m_PumpLaserConnected;
    bool m_OPOConnected;
    bool m_PyroConnected;
    bool m_SyncFromSpinBox;
    bool m_SyncFromSlider;
    double m_CurrentPulseEnergy;

    QFutureWatcher<void> m_ShutterWatcher;
    QFutureWatcher<void> m_EnergyWatcher;

    mitk::QuantelLaser::Pointer m_PumpLaserController;
    mitk::GalilMotor::Pointer m_OPOMotor;
    //mitk::OphirPyro::Pointer m_Pyro;

};

class InitThread : public QThread
{
  Q_OBJECT
    void run() Q_DECL_OVERRIDE
    {
      emit result(m_PumpLaserController->OpenConnection("OpotekPhocusMobile"), m_OPOMotor->OpenConnection("OpotekPhocusMobile"));
    }
  signals:
    void result(bool, bool);
  public:
    InitThread(mitk::QuantelLaser::Pointer ql, mitk::GalilMotor::Pointer gm)
    {
      m_PumpLaserController = ql;
      m_OPOMotor = gm;
    }
    mitk::QuantelLaser::Pointer m_PumpLaserController;
    mitk::GalilMotor::Pointer m_OPOMotor;
};

class ResetThread : public QThread
{
  Q_OBJECT
    void run() Q_DECL_OVERRIDE
  {
    emit result(m_PumpLaserController->CloseConnection(), m_OPOMotor->CloseConnection());
  }
signals:
  void result(bool, bool);
public:
  ResetThread(mitk::QuantelLaser::Pointer ql, mitk::GalilMotor::Pointer gm)
  {
    m_PumpLaserController = ql;
    m_OPOMotor = gm;
  }
  mitk::QuantelLaser::Pointer m_PumpLaserController;
  mitk::GalilMotor::Pointer m_OPOMotor;
};

#endif // OPOLaserControl_h

