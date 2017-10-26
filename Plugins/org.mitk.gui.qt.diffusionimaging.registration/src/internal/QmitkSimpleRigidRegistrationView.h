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



#include <berryISelectionListener.h>
#include <berryIStructuredSelection.h>
#include <QmitkAbstractView.h>
#include "ui_QmitkSimpleRigidRegistrationViewControls.h"

#include <mitkImage.h>
#include <QmitkRegistrationJob.h>
#include <mitkMultiModalRigidDefaultRegistrationAlgorithm.h>

typedef short DiffusionPixelType;

/*!
\brief View for diffusion image registration / head motion correction
*/

// Forward Qt class declarations

class QmitkSimpleRigidRegistrationView : public QmitkAbstractView
{

  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

public:

  typedef itk::VectorImage< short, 3 >   ItkDwiType;
  static const std::string VIEW_ID;

  QmitkSimpleRigidRegistrationView();
  virtual ~QmitkSimpleRigidRegistrationView();

  virtual void CreateQtPartControl(QWidget *parent) override;
  void SetFocus() override;

protected slots:

  void MovingImageChanged();
  void FixedImageChanged();
  void StartRegistration();
  void OnRegResultIsAvailable(mitk::MAPRegistrationWrapper::Pointer spResultRegistration, const QmitkRegistrationJob* pRegJob);

protected:

  /// \brief called by QmitkAbstractView when DataManager's selection has changed
  virtual void OnSelectionChanged(berry::IWorkbenchPart::Pointer part, const QList<mitk::DataNode::Pointer>& nodes) override;

  Ui::QmitkSimpleRigidRegistrationViewControls* m_Controls;
  mitk::DataNode::Pointer  m_MovingImageNode;

private:

};
