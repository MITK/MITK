/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkTextQuestion.h>

using namespace mitk::Forms;

TextQuestion::~TextQuestion() = default;

std::vector<std::string> TextQuestion::GetResponsesAsStrings() const
{
  return { m_Response };
}

void TextQuestion::ClearResponses()
{
  m_Response.clear();
}

bool TextQuestion::IsComplete() const
{
  return !m_Response.empty();
}

std::string TextQuestion::GetResponse() const
{
  return m_Response;
}

void TextQuestion::SetResponse(const std::string& response)
{
  m_Response = response;
}
