/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkModuleActivator.h"
#include "mitkQuestionWidgetFactory.h"

#include <QmitkCheckboxesQuestionWidget.h>
#include <QmitkDropdownQuestionWidget.h>
#include <QmitkLinearScaleQuestionWidget.h>
#include <QmitkMultipleChoiceQuestionWidget.h>
#include <QmitkParagraphQuestionWidget.h>
#include <QmitkScreenshotQuestionWidget.h>
#include <QmitkShortAnswerQuestionWidget.h>

#include <usModuleContext.h>

using namespace mitk::Forms::UI;

ModuleActivator::ModuleActivator()
  : m_QuestionWidgetFactory(std::make_unique<QuestionWidgetFactory>())
{
}

ModuleActivator::~ModuleActivator() = default;

void ModuleActivator::Load(us::ModuleContext* context)
{
  context->RegisterService<IQuestionWidgetFactory>(m_QuestionWidgetFactory.get());

  this->RegisterQuestionWidget<CheckboxesQuestion, QmitkCheckboxesQuestionWidget>();
  this->RegisterQuestionWidget<DropdownQuestion, QmitkDropdownQuestionWidget>();
  this->RegisterQuestionWidget<LinearScaleQuestion, QmitkLinearScaleQuestionWidget>();
  this->RegisterQuestionWidget<MultipleChoiceQuestion, QmitkMultipleChoiceQuestionWidget>();
  this->RegisterQuestionWidget<ParagraphQuestion, QmitkParagraphQuestionWidget>();
  this->RegisterQuestionWidget<ScreenshotQuestion, QmitkScreenshotQuestionWidget>();
  this->RegisterQuestionWidget<ShortAnswerQuestion, QmitkShortAnswerQuestionWidget>();
}

void ModuleActivator::Unload(us::ModuleContext*)
{
}

US_EXPORT_MODULE_ACTIVATOR(ModuleActivator)
