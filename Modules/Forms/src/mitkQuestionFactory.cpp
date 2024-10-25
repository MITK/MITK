/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkQuestionFactory.h"

#include <mitkExceptionMacro.h>
#include <mitkQuestion.h>

using namespace mitk::Forms;

QuestionFactory::QuestionFactory() = default;

QuestionFactory::~QuestionFactory()
{
}

void QuestionFactory::Register(Question* question)
{
  auto type = question->GetType();

  if (m_Prototypes.find(type) != m_Prototypes.end())
    mitkThrow() << "A prototype is already registered for questions of type \"" << type << "\"!";

  m_Prototypes[type].reset(question);
}

Question* QuestionFactory::Create(const std::string& type) const
{
  try
  {
    return m_Prototypes.at(type)->CreateAnother();
  }
  catch (const std::out_of_range&)
  {
  }

  return nullptr;
}
