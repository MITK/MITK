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

#include "ui_QmitkSurfaceRegistrationViewControls.h"

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

  protected slots:

    void OnRunRegistration();

    void OnEnableTreCalculation();

  protected:

    virtual void CreateQtPartControl(QWidget *parent);

    virtual void SetFocus();

    Ui::QmitkSurfaceRegistrationViewControls m_Controls;
};

#endif // QmitkSurfaceRegistrationView_h
