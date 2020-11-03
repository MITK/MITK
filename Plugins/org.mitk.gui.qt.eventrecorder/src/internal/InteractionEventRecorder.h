/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef InteractionEventRecorder_h
#define InteractionEventRecorder_h

#include <berryISelectionListener.h>

#include <QmitkAbstractView.h>

#include "ui_InteractionEventRecorderControls.h"

#include "mitkInteractionEventObserver.h"
#include "mitkEventRecorder.h"

/**
  \brief InteractionEventRecorder

  Demontrates the use of InteractionOversers.

  Allows to record all mouse interaction in the renderwindows save it as XML file. And also replay the interaction.

  \sa QmitkAbstractView
  \ingroup ${plugin_target}_internal
*/
class InteractionEventRecorder : public QmitkAbstractView
{
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

  public:

    static const std::string VIEW_ID;

  protected slots:

    /// \brief Called when the user clicks the GUI button

    void StartRecording();
    void StopRecording();
    void OpenFile();
    void RotatePlanes();
    void RotateView();
    void Play();

  protected:

    void CreateQtPartControl(QWidget *parent) override;

    void SetFocus() override;



    Ui::InteractionEventRecorderControls m_Controls;

 private:
    mitk::EventRecorder* m_CurrentObserver;

    us::ServiceRegistration<mitk::InteractionEventObserver> m_ServiceRegistration;

};

#endif // InteractionEventRecorder_h
