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
#include "ui_QmitkSimpleRegistrationViewControls.h"

#include <mitkImage.h>
#include <QmitkRegistrationJob.h>

typedef short DiffusionPixelType;

/*!
\brief View for diffusion image registration / head motion correction
*/

// Forward Qt class declarations

class QmitkSimpleRegistrationView : public QmitkAbstractView
{

  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

public:

  typedef itk::Image< float, 3 > ItkFloatImageType;
  typedef itk::Image<DiffusionPixelType, 3> ITKDiffusionVolumeType;
  typedef itk::VectorImage< short, 3 >   ItkDwiType;
  static const std::string VIEW_ID;

  QmitkSimpleRegistrationView();
  virtual ~QmitkSimpleRegistrationView();

  virtual void CreateQtPartControl(QWidget *parent) override;
  void SetFocus() override;

protected slots:

  void MovingImageChanged();
  void FixedImageChanged();
  void TractoChanged();
  void StartRegistration();
  void StartTractoRegistration();
  void OnRegResultIsAvailable(mitk::MAPRegistrationWrapper::Pointer spResultRegistration, const QmitkRegistrationJob* pRegJob);

protected:

  /// \brief called by QmitkAbstractView when DataManager's selection has changed
  virtual void OnSelectionChanged(berry::IWorkbenchPart::Pointer part, const QList<mitk::DataNode::Pointer>& nodes) override;

  Ui::QmitkSimpleRegistrationViewControls* m_Controls;
  mitk::DataNode::Pointer  m_MovingImageNode;
  int  m_RegistrationType;

private:

};
