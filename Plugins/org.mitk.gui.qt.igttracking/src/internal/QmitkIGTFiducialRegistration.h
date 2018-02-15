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

#ifndef QmitkIGTFiducialRegistration_h
#define QmitkIGTFiducialRegistration_h

#include <berryISelectionListener.h>

#include <QmitkAbstractView.h>

#include "ui_QmitkIGTFiducialRegistrationControls.h"


#include <ctkServiceEvent.h>

/*!
\brief QmitkIGTFiducialRegistration

\sa QmitkFunctionality
\ingroup ${plugin_target}_internal
*/
class QmitkIGTFiducialRegistration : public QmitkAbstractView
{
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

  public:

    virtual void SetFocus() override;

    static const std::string VIEW_ID;

    virtual void CreateQtPartControl(QWidget *parent) override;

    QmitkIGTFiducialRegistration();
    virtual ~QmitkIGTFiducialRegistration();

  public slots:


  protected slots:

  void PointerSelectionChanged();
  void ImageSelectionChanged();


  protected:

    void InitializeRegistration();

    Ui::IGTFiducialRegistrationControls m_Controls;

    mitk::NavigationData::Pointer m_TrackingPointer;


};

#endif // IGTFiducialRegistration_h
