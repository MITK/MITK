/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef QmitkAICPRegistrationView_h
#define QmitkAICPRegistrationView_h

#include <berryISelectionListener.h>

#include <QmitkAbstractView.h>
#include <QWidget>
#include "ui_QmitkAICPRegistrationViewControls.h"

// forwarddeclaration
class AICPRegistrationViewData;

/**
  * \brief Implemenation of a worker thread class.
  *
  *        Worker class that runs the registration
  *        in a seperate QThread to prevent the registration from blocking the
  *        GUI.
  */
class UIWorker : public QObject
{
  Q_OBJECT

  private:
    /** Pimpl with the registration data.*/
    AICPRegistrationViewData* d;

  public slots:

    /** Method that runs the registration algorithm in a seperate QThread.*/
    void RegistrationThreadFunc();

  signals:

    /** Signal emitted when the registration was successful.*/
    void RegistrationFinished();

  public:

    /** Set the data used for the registration.*/
    void SetRegistrationData(AICPRegistrationViewData* data);
};

/**
  \brief QmitkAICPRegistrationView provides a simple UI to register two
         surfaces with the AnisotropicIterativeClosestPointRegistration
         algorithm.

  \sa QmitkAbstractView
  \ingroup ${plugin_target}_internal
*/
class QmitkAICPRegistrationView : public QmitkAbstractView
{
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

  public:

    static const std::string VIEW_ID;

    QmitkAICPRegistrationView();

    ~QmitkAICPRegistrationView() override;

  protected slots:

    /** Starts the registration. When the method is called a seperate UIWorker
      * thread will be run in the background to prevent blocking the GUI.
      */
    void OnStartRegistration();

    /** Enables/disables the calculation of the Target Registration Error (TRE).
      */
    void OnEnableTreCalculation();

    /** Enables/disables the trimmed version of the A-ICP algorithm.*/
    void OnEnableTrimming();

  public slots:

    /** Method called when the algorithm is finishes. This method will setup
      * the GUI.
      */
    void OnRegistrationFinished();

  protected:

    void CreateQtPartControl(QWidget *parent) override;

    void SetFocus() override;

    Ui::QmitkAICPRegistrationViewControls m_Controls;

  private:

    AICPRegistrationViewData* d;

    /** Check for the correct input data.*/
    bool CheckInput();

};

#endif // QmitkAICPRegistrationView_h
