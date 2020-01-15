/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#include "org_mitk_gui_qt_fit_inspector_Activator.h"

#include "ModelFitInspectorView.h"
#include "mitkModelFitConstants.h"

#include <QmitkNodeDescriptorManager.h>
#include <mitkNodePredicateProperty.h>
#include <mitkStringProperty.h>
#include <mitkModelFitConstants.h>

void org_mitk_gui_qt_fit_inspector_Activator::start(ctkPluginContext* context)
	{
		BERRY_REGISTER_EXTENSION_CLASS(ModelFitInspectorView, context)

  QmitkNodeDescriptorManager* manager = QmitkNodeDescriptorManager::GetInstance();

  mitk::StringProperty::Pointer parameterProp = mitk::StringProperty::New(mitk::ModelFitConstants::PARAMETER_TYPE_VALUE_PARAMETER().c_str());
  mitk::NodePredicateProperty::Pointer isModelFitParameter = mitk::NodePredicateProperty::New(mitk::ModelFitConstants::PARAMETER_TYPE_PROPERTY_NAME().c_str(), parameterProp);
  manager->AddDescriptor(new QmitkNodeDescriptor(tr("Image"), QString(":/ModelFitVisualization/parameter_image.png"), isModelFitParameter, manager));

  mitk::StringProperty::Pointer derivedProp = mitk::StringProperty::New(mitk::ModelFitConstants::PARAMETER_TYPE_VALUE_DERIVED_PARAMETER().c_str());
  mitk::NodePredicateProperty::Pointer isModelFitDerived = mitk::NodePredicateProperty::New(mitk::ModelFitConstants::PARAMETER_TYPE_PROPERTY_NAME().c_str(), derivedProp);
  manager->AddDescriptor(new QmitkNodeDescriptor(tr("Image"), QString(":/ModelFitVisualization/derived_image.png"), isModelFitDerived, manager));

  mitk::StringProperty::Pointer criterionProp = mitk::StringProperty::New(mitk::ModelFitConstants::PARAMETER_TYPE_VALUE_CRITERION().c_str());
  mitk::NodePredicateProperty::Pointer isModelFitCriterion = mitk::NodePredicateProperty::New(mitk::ModelFitConstants::PARAMETER_TYPE_PROPERTY_NAME().c_str(), criterionProp);
  manager->AddDescriptor(new QmitkNodeDescriptor(tr("Image"), QString(":/ModelFitVisualization/criterion_image.png"), isModelFitCriterion, manager));

  mitk::StringProperty::Pointer evalProp = mitk::StringProperty::New(mitk::ModelFitConstants::PARAMETER_TYPE_VALUE_EVALUATION_PARAMETER().c_str());
  mitk::NodePredicateProperty::Pointer isModelFitEvaluation = mitk::NodePredicateProperty::New(mitk::ModelFitConstants::PARAMETER_TYPE_PROPERTY_NAME().c_str(), evalProp);
  manager->AddDescriptor(new QmitkNodeDescriptor(tr("Image"), QString(":/ModelFitVisualization/eval_image.png"), isModelFitEvaluation, manager));

	}

	void org_mitk_gui_qt_fit_inspector_Activator::stop(ctkPluginContext* context)
	{
		Q_UNUSED(context)
	}
