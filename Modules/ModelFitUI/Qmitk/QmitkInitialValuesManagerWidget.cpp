/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QMenu>

#include <mitkNodePredicateAnd.h>
#include <mitkNodePredicateDataType.h>
#include <mitkNodePredicateGeometry.h>
#include <mitkNodePredicateDimension.h>
#include <mitkImage.h>

#include "QmitkInitialValuesManagerWidget.h"
#include "QmitkInitialValuesModel.h"
#include "QmitkInitialValuesTypeDelegate.h"
#include "QmitkInitialValuesDelegate.h"

QmitkInitialValuesManagerWidget::QmitkInitialValuesManagerWidget(QWidget*)
{
  this->m_Controls.setupUi(this);

  m_InternalModel = new QmitkInitialValuesModel(this);
  m_TypeDelegate = new QmitkInitialValuesTypeDelegate(this);
  m_ValuesDelegate = new QmitkInitialValuesDelegate(this);

  this->m_Controls.initialsView->setModel(m_InternalModel);

  this->m_Controls.initialsView->setItemDelegateForColumn(1, m_TypeDelegate);
  this->m_Controls.initialsView->setItemDelegateForColumn(2, m_ValuesDelegate);

  connect(m_InternalModel, SIGNAL(modelReset()), this, SLOT(OnModelReset()));

  this->update();
}

void
QmitkInitialValuesManagerWidget::OnModelReset()
{
  emit initialValuesChanged();
};

QmitkInitialValuesManagerWidget::~QmitkInitialValuesManagerWidget()
{
  delete m_InternalModel;
}

void QmitkInitialValuesManagerWidget::setInitialValues(const
    mitk::ModelTraitsInterface::ParameterNamesType& names,
    const mitk::ModelTraitsInterface::ParametersType values)
{
  this->m_InternalModel->setInitialValues(names, values);
}

void QmitkInitialValuesManagerWidget::setInitialValues(const
    mitk::ModelTraitsInterface::ParameterNamesType& names)
{
  this->m_InternalModel->setInitialValues(names);
}

void QmitkInitialValuesManagerWidget::setDataStorage(mitk::DataStorage* storage)
{
  m_ValuesDelegate->setDataStorage(storage);
  this->m_InternalModel->resetInitialParameterImage();
}

void QmitkInitialValuesManagerWidget::setReferenceImageGeometry(mitk::BaseGeometry* refgeo)
{
  mitk::TNodePredicateDataType<mitk::Image>::Pointer isImage = mitk::TNodePredicateDataType<mitk::Image>::New();
  mitk::NodePredicateDimension::Pointer is3D = mitk::NodePredicateDimension::New(3);

  if (refgeo)
  {
    mitk::NodePredicateGeometry::Pointer hasGeo = mitk::NodePredicateGeometry::New(refgeo);
    mitk::NodePredicateAnd::Pointer isValidInitialImage = mitk::NodePredicateAnd::New(isImage, hasGeo, is3D);
    m_ValuesDelegate->setNodePredicate(isValidInitialImage);
  }
  else
  {
    mitk::NodePredicateAnd::Pointer isValidInitialImage = mitk::NodePredicateAnd::New(isImage, is3D);
    m_ValuesDelegate->setNodePredicate(isImage);
  }

  this->m_InternalModel->resetInitialParameterImage();
}

mitk::ModelTraitsInterface::ParametersType
QmitkInitialValuesManagerWidget::getInitialValues() const
{
  return this->m_InternalModel->getInitialValues();
};


mitk::InitialParameterizationDelegateBase::Pointer
QmitkInitialValuesManagerWidget::getInitialParametrizationDelegate() const
{
  return this->m_InternalModel->getInitialParametrizationDelegate();
};

bool QmitkInitialValuesManagerWidget::hasValidInitialValues() const
{
  return this->m_InternalModel->hasValidInitialValues();
};

