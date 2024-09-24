/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkModuleActivator.h"
#include "mitkQuestionFactory.h"

#include <mitkCheckboxesQuestion.h>
#include <mitkDropdownQuestion.h>
#include <mitkLinearScaleQuestion.h>
#include <mitkMultipleChoiceQuestion.h>
#include <mitkParagraphQuestion.h>
#include <mitkScreenshotQuestion.h>
#include <mitkShortAnswerQuestion.h>

#include <usModuleContext.h>

using namespace mitk::Forms;

ModuleActivator::ModuleActivator()
  : m_QuestionFactory(std::make_unique<QuestionFactory>())
{
}

ModuleActivator::~ModuleActivator() = default;

void ModuleActivator::Load(us::ModuleContext* context)
{
  context->RegisterService<IQuestionFactory>(m_QuestionFactory.get());

  this->RegisterQuestion<CheckboxesQuestion>();
  this->RegisterQuestion<DropdownQuestion>();
  this->RegisterQuestion<LinearScaleQuestion>();
  this->RegisterQuestion<MultipleChoiceQuestion>();
  this->RegisterQuestion<ParagraphQuestion>();
  this->RegisterQuestion<ScreenshotQuestion>();
  this->RegisterQuestion<ShortAnswerQuestion>();
}

void ModuleActivator::Unload(us::ModuleContext*)
{
}

US_EXPORT_MODULE_ACTIVATOR(ModuleActivator)
