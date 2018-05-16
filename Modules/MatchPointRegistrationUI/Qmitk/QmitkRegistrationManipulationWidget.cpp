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

//Qmitk
#include "QmitkRegistrationManipulationWidget.h"

//MatchPoint
#include <mapRegistrationManipulator.h>
#include <mapPreCachedRegistrationKernel.h>
#include <mapCombinedRegistrationKernel.h>
#include <mapNullRegistrationKernel.h>
#include <mapRegistrationCombinator.h>

#include <itkCompositeTransform.h>

#include <boost/math/constants/constants.hpp>

QmitkRegistrationManipulationWidget::QmitkRegistrationManipulationWidget(QWidget *parent)
  : QWidget(parent), m_CenterOfRotationIsRelativeToTarget(false), m_internalUpdate(false)
{
  this->setupUi(this);

  connect(this->slideRotX, SIGNAL(valueChanged(int)), this, SLOT(OnRotXSlideChanged(int)));
  connect(this->sbRotX, SIGNAL(valueChanged(double)), this, SLOT(OnRotXChanged(double)));
  connect(this->slideRotY, SIGNAL(valueChanged(int)), this, SLOT(OnRotYSlideChanged(int)));
  connect(this->sbRotY, SIGNAL(valueChanged(double)), this, SLOT(OnRotYChanged(double)));
  connect(this->slideRotZ, SIGNAL(valueChanged(int)), this, SLOT(OnRotZSlideChanged(int)));
  connect(this->sbRotZ, SIGNAL(valueChanged(double)), this, SLOT(OnRotZChanged(double)));

  connect(this->slideTransX, SIGNAL(valueChanged(int)), this, SLOT(OnTransXSlideChanged(int)));
  connect(this->sbTransX, SIGNAL(valueChanged(double)), this, SLOT(OnTransXChanged(double)));
  connect(this->slideTransY, SIGNAL(valueChanged(int)), this, SLOT(OnTransYSlideChanged(int)));
  connect(this->sbTransY, SIGNAL(valueChanged(double)), this, SLOT(OnTransYChanged(double)));
  connect(this->slideTransZ, SIGNAL(valueChanged(int)), this, SLOT(OnTransZSlideChanged(int)));
  connect(this->sbTransZ, SIGNAL(valueChanged(double)), this, SLOT(OnTransZChanged(double)));

  this->groupScale->setVisible(false);
}

QmitkRegistrationManipulationWidget::~QmitkRegistrationManipulationWidget()
= default;

void QmitkRegistrationManipulationWidget::Initialize()
{
  this->ResetTransforms();
  this->InitControls();
};

void QmitkRegistrationManipulationWidget::Initialize(MAPRegistrationType* precedingRegistration)
{
  this->ResetTransforms();
  this->m_PreRegistration = precedingRegistration;

  ::map::core::RegistrationManipulator<MAPRegistrationType> manipulator(m_CurrentRegistration);
  ::map::core::PreCachedRegistrationKernel<3, 3>::Pointer kernel = ::map::core::PreCachedRegistrationKernel<3, 3>::New();

  const map::core::RegistrationKernel<3, 3>* preKernel = dynamic_cast<const map::core::RegistrationKernel<3, 3>*>(&this->m_PreRegistration->getInverseMapping());
  itk::CompositeTransform < ::map::core::continuous::ScalarType, 3>::Pointer compTransform = itk::CompositeTransform < ::map::core::continuous::ScalarType, 3>::New();
  compTransform->AddTransform(preKernel->getTransformModel()->Clone());
  compTransform->AddTransform(this->m_InverseCurrentTransform);

  kernel->setTransformModel(compTransform);
  manipulator.setInverseMapping(kernel);

  this->InitControls();
};

void QmitkRegistrationManipulationWidget::Initialize(const mitk::Point3D& movingReference, const mitk::Point3D& targetReference)
{
  this->ResetTransforms();

  auto offset = targetReference - movingReference;
  m_DirectCurrentTransform->SetOffset(offset);
  m_DirectCurrentTransform->GetInverse(m_InverseCurrentTransform);

  this->InitControls();
};

void QmitkRegistrationManipulationWidget::ResetTransforms()
{
  this->m_CenterOfRotation.Fill(0.0);
  this->m_PreRegistration = nullptr;


  this->m_InverseCurrentTransform = TransformType::New();
  this->m_InverseCurrentTransform->SetIdentity();
  this->m_DirectCurrentTransform = TransformType::New();
  this->m_DirectCurrentTransform->SetIdentity();

  m_CurrentRegistration = MAPRegistrationType::New();

  ::map::core::RegistrationManipulator<MAPRegistrationType> manipulator(m_CurrentRegistration);
  ::map::core::PreCachedRegistrationKernel<3, 3>::Pointer kernel = ::map::core::PreCachedRegistrationKernel<3, 3>::New();
  kernel->setTransformModel(m_InverseCurrentTransform);
  manipulator.setInverseMapping(kernel);

  manipulator.setDirectMapping(::map::core::NullRegistrationKernel < 3, 3 >::New());
};

void QmitkRegistrationManipulationWidget::SetCenterOfRotation(const mitk::Point3D& center)
{
  this->m_CenterOfRotation = center;
  this->ConfigureTransformCenter();
  this->UpdateTransformWidgets();
};

/** Sets the internal m_CenterOfRotationIsRelativeToTarget. see below.*/
void QmitkRegistrationManipulationWidget::SetCenterOfRotationIsRelativeToTarget(bool targetRelative)
{
  this->m_CenterOfRotationIsRelativeToTarget = targetRelative;
  this->ConfigureTransformCenter();
  this->UpdateTransformWidgets();
};

void QmitkRegistrationManipulationWidget::InitControls()
{
  this->ConfigureTransformCenter();

  //set bounds of the translation slider widget to have sensible ranges
  auto currenttrans = m_DirectCurrentTransform->GetTranslation();
  this->slideTransX->setMinimum(currenttrans[0] - 250);
  this->slideTransY->setMinimum(currenttrans[1] - 250);
  this->slideTransZ->setMinimum(currenttrans[2] - 250);
  this->slideTransX->setMaximum(currenttrans[0] + 250);
  this->slideTransY->setMaximum(currenttrans[1] + 250);
  this->slideTransZ->setMaximum(currenttrans[2] + 250);
};

void QmitkRegistrationManipulationWidget::UpdateTransformWidgets()
{
  this->m_internalUpdate = true;
  this->sbTransX->setValue(this->m_DirectCurrentTransform->GetTranslation()[0]);
  this->sbTransY->setValue(this->m_DirectCurrentTransform->GetTranslation()[1]);
  this->sbTransZ->setValue(this->m_DirectCurrentTransform->GetTranslation()[2]);
  this->slideTransX->setValue(this->m_DirectCurrentTransform->GetTranslation()[0]);
  this->slideTransY->setValue(this->m_DirectCurrentTransform->GetTranslation()[1]);
  this->slideTransZ->setValue(this->m_DirectCurrentTransform->GetTranslation()[2]);

  this->sbRotX->setValue(this->m_DirectCurrentTransform->GetAngleX()*(180 / boost::math::double_constants::pi));
  this->sbRotY->setValue(this->m_DirectCurrentTransform->GetAngleY()*(180 / boost::math::double_constants::pi));
  this->sbRotZ->setValue(this->m_DirectCurrentTransform->GetAngleZ()*(180 / boost::math::double_constants::pi));
  this->slideRotX->setValue(this->m_DirectCurrentTransform->GetAngleX()*(180 / boost::math::double_constants::pi));
  this->slideRotY->setValue(this->m_DirectCurrentTransform->GetAngleY()*(180 / boost::math::double_constants::pi));
  this->slideRotZ->setValue(this->m_DirectCurrentTransform->GetAngleZ()*(180 / boost::math::double_constants::pi));
  this->m_internalUpdate = false;
};

void QmitkRegistrationManipulationWidget::UpdateTransform(bool updateRotation)
{
  if (updateRotation)
  {
    if (this->m_CenterOfRotationIsRelativeToTarget)
    {
     ConfigureTransformCenter();
    }

    this->m_DirectCurrentTransform->SetRotation(this->sbRotX->value()*(boost::math::double_constants::pi / 180),
      this->sbRotY->value()*(boost::math::double_constants::pi / 180),
      this->sbRotZ->value()*(boost::math::double_constants::pi / 180));
  }
  else
  {
    TransformType::OutputVectorType trans;
    trans[0] = this->sbTransX->value();
    trans[1] = this->sbTransY->value();
    trans[2] = this->sbTransZ->value();

    this->m_DirectCurrentTransform->SetTranslation(trans);
  }

  this->m_DirectCurrentTransform->GetInverse(this->m_InverseCurrentTransform);

  this->UpdateTransformWidgets();

  emit RegistrationChanged(this->m_CurrentRegistration);
};

map::core::RegistrationBase* QmitkRegistrationManipulationWidget::GetInterimRegistration() const
{
  return this->m_CurrentRegistration.GetPointer();
};

map::core::RegistrationBase::Pointer QmitkRegistrationManipulationWidget::GenerateRegistration() const
{
  MAPRegistrationType::Pointer newReg = MAPRegistrationType::New();

  ::map::core::RegistrationManipulator<MAPRegistrationType> manipulator(newReg);

  ::map::core::PreCachedRegistrationKernel<3, 3>::Pointer kernel = ::map::core::PreCachedRegistrationKernel<3, 3>::New();
  kernel->setTransformModel(m_InverseCurrentTransform);

  ::map::core::PreCachedRegistrationKernel<3, 3>::Pointer kernel2 = ::map::core::PreCachedRegistrationKernel<3, 3>::New();
  kernel2->setTransformModel(m_InverseCurrentTransform->GetInverseTransform());

  manipulator.setInverseMapping(kernel);
  manipulator.setDirectMapping(kernel2);

  if (this->m_PreRegistration.IsNotNull())
  { //compine registration with selected pre registration as baseline
    typedef ::map::core::RegistrationCombinator<MAPRegistrationType, MAPRegistrationType> CombinatorType;
    CombinatorType::Pointer combinator = CombinatorType::New();
    newReg = combinator->process(*m_PreRegistration, *newReg);
  }

  return newReg.GetPointer();
};

void QmitkRegistrationManipulationWidget::OnRotXChanged(double x)
{
  if (!m_internalUpdate)
  {
    m_internalUpdate = true;
    this->slideRotX->setValue(x);
    m_internalUpdate = false;
    this->UpdateTransform(true);
  }
};

void QmitkRegistrationManipulationWidget::OnRotXSlideChanged(int x)
{
  if (!m_internalUpdate)
  {
    this->sbRotX->setValue(x);
  }
};

void QmitkRegistrationManipulationWidget::OnRotYChanged(double y)
{
  if (!m_internalUpdate)
  {
    m_internalUpdate = true;
    this->slideRotY->setValue(y);
    m_internalUpdate = false;
    this->UpdateTransform(true);
  }
};

void QmitkRegistrationManipulationWidget::OnRotYSlideChanged(int y)
{
  if (!m_internalUpdate)
  {
    this->sbRotY->setValue(y);
  }
};

void QmitkRegistrationManipulationWidget::OnRotZChanged(double z)
{
  if (!m_internalUpdate)
  {
    m_internalUpdate = true;
    this->slideRotZ->setValue(z);
    m_internalUpdate = false;
    this->UpdateTransform(true);
  }
};

void QmitkRegistrationManipulationWidget::OnRotZSlideChanged(int z)
{
  if (!m_internalUpdate)
  {
    this->sbRotZ->setValue(z);
  }
};

void QmitkRegistrationManipulationWidget::OnTransXChanged(double x)
{
  if (!m_internalUpdate)
  {
    m_internalUpdate = true;
    this->slideTransX->setValue(x);
    m_internalUpdate = false;
    this->UpdateTransform();
  }
};

void QmitkRegistrationManipulationWidget::OnTransXSlideChanged(int x)
{
  if (!m_internalUpdate)
  {
    this->sbTransX->setValue(x);
  }
};

void QmitkRegistrationManipulationWidget::OnTransYChanged(double y)
{
  if (!m_internalUpdate)
  {
    m_internalUpdate = true;
    this->slideTransY->setValue(y);
    m_internalUpdate = false;
    this->UpdateTransform();
  }
};

void QmitkRegistrationManipulationWidget::OnTransYSlideChanged(int y)
{
  if (!m_internalUpdate)
  {
    this->sbTransY->setValue(y);
  }
};

void QmitkRegistrationManipulationWidget::OnTransZChanged(double z)
{
  if (!m_internalUpdate)
  {
    m_internalUpdate = true;
    this->slideTransZ->setValue(z);
    m_internalUpdate = false;
    this->UpdateTransform();
  }
};

void QmitkRegistrationManipulationWidget::OnTransZSlideChanged(int z)
{
  if (!m_internalUpdate)
  {
    this->sbTransZ->setValue(z);
  }
};

void QmitkRegistrationManipulationWidget::ConfigureTransformCenter()
{
  auto offset = m_DirectCurrentTransform->GetOffset();


  if (this->m_CenterOfRotationIsRelativeToTarget)
  {
    auto newCenter = m_InverseCurrentTransform->TransformPoint(m_CenterOfRotation);
    m_DirectCurrentTransform->SetCenter(newCenter);
  }
  else
  {
    m_DirectCurrentTransform->SetCenter(m_CenterOfRotation);
  }

  m_DirectCurrentTransform->SetOffset(offset);
  m_DirectCurrentTransform->GetInverse(m_InverseCurrentTransform);
};
