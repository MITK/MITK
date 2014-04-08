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


#ifndef QmitkSurfaceRegistrationView_h
#define QmitkSurfaceRegistrationView_h

#include <berryISelectionListener.h>

#include <QmitkAbstractView.h>
#include <QWidget>
#include "ui_QmitkSurfaceRegistrationViewControls.h"

// forwarddeclaration
class SurfaceRegistrationViewData;

class UIWorker : public QObject
{
  Q_OBJECT

  private:
    SurfaceRegistrationViewData* d;

  public slots:

    void RegistrationThreadFunc();

  signals:

    void RegistrationFinished();

  public:
    void SetRegistrationData(SurfaceRegistrationViewData* data);
};

/**
  \brief QmitkSurfaceRegistrationView

  \warning  This class is not yet documented. Use "git blame" and ask the author to provide basic documentation.

  \sa QmitkAbstractView
  \ingroup ${plugin_target}_internal
*/
class QmitkSurfaceRegistrationView : public QmitkAbstractView
{
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

  public:

    static const std::string VIEW_ID;

    QmitkSurfaceRegistrationView();

    ~QmitkSurfaceRegistrationView();

  protected slots:

    void OnRunRegistration();

    void OnEnableTreCalculation();

    void OnEnableTrimming();

  public slots:

    void OnRegistrationFinished();

  protected:

    virtual void CreateQtPartControl(QWidget *parent);

    virtual void SetFocus();

    Ui::QmitkSurfaceRegistrationViewControls m_Controls;

  private:

    SurfaceRegistrationViewData* d;
};

#endif // QmitkSurfaceRegistrationView_h
