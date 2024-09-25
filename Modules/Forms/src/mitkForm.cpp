/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkForm.h>

#include <mitkExceptionMacro.h>
#include <mitkIQuestionFactory.h>
#include <mitkQuestion.h>

#include <nlohmann/json.hpp>

#include <ctime>
#include <fstream>

using namespace mitk::Forms;
using namespace nlohmann;

namespace
{
  std::string GetCurrentISO8601DateTime()
  {
    std::timespec ts;
    std::timespec_get(&ts, TIME_UTC);
    std::array<char, 21> buffer{}; // YYYY-MM-DDThh:mm:ssZ

    std::strftime(buffer.data(), buffer.size(), "%FT%TZ", std::gmtime(&ts.tv_sec));

    return buffer.data();
  }
}

Form::Section::Section(const std::string& title, const std::string& description)
  : m_Title(title),
    m_Description(description)
{
}

Form::Section::~Section() = default;

Form::Section::Section(Section&& other) noexcept = default;

Form::Section& Form::Section::operator=(Section&& other) noexcept = default;

std::string Form::Section::GetTitle() const
{
  return m_Title;
}

void Form::Section::SetTitle(const std::string& title)
{
  m_Title = title;
}

std::string Form::Section::GetDescription() const
{
  return m_Description;
}

void Form::Section::SetDescription(const std::string& description)
{
  m_Description = description;
}

std::vector<Question*> Form::Section::GetQuestions() const
{
  std::vector<Question*> questions;
  questions.reserve(m_Questions.size());

  for (const auto& question : m_Questions)
    questions.push_back(question.get());

  return questions;
}

void Form::Section::AddQuestion(Question* question)
{
  m_Questions.emplace_back(question);
}

Form::Form(const std::string& title, const std::string& description)
{
  m_Sections.emplace_back(title, description);
}

Form::~Form() = default;

Form::Form(Form&& other) noexcept = default;

Form& Form::operator=(Form&& other) noexcept = default;

Form::Section& Form::AddSection(const std::string& title, const std::string& description)
{
  return m_Sections.emplace_back(title, description);
}

int Form::GetNumberOfSections() const
{
  return static_cast<int>(m_Sections.size());
}

Form::Section& Form::GetSection(int index)
{
  return m_Sections.at(index);
}

const Form::Section& Form::GetSection(int index) const
{
  return m_Sections.at(index);
}

std::string Form::GetTitle() const
{
  return m_Sections[0].GetTitle();
}

void Form::SetTitle(const std::string& title)
{
  m_Sections[0].SetTitle(title);
}

std::string Form::GetDescription() const
{
  return m_Sections[0].GetDescription();
}

void Form::SetDescription(const std::string& description)
{
  m_Sections[0].SetDescription(description);
}

std::vector<Question*> Form::GetQuestions() const
{
  return m_Sections[0].GetQuestions();
}

void Form::AddQuestion(Question* question)
{
  m_Sections[0].AddQuestion(question);
}

void Form::AddSupplement(const std::string& key)
{
  if (m_Supplements.find(key) == m_Supplements.end())
    m_Supplements[key] = "";
}

bool Form::SetSupplement(const std::string& key, const std::string& value)
{
  if (m_Supplements.find(key) == m_Supplements.end())
    return false;

  m_Supplements[key] = value;

  return true;
}

const std::map<std::string, std::string>& Form::GetSupplements() const
{
  return m_Supplements;
}

std::vector<Form::Section>::const_iterator Form::begin() const
{
  return m_Sections.begin();
}

std::vector<Form::Section>::const_iterator Form::end() const
{
  return m_Sections.end();
}

std::vector<Form::Section>::iterator Form::begin()
{
  return m_Sections.begin();
}

std::vector<Form::Section>::iterator Form::end()
{
  return m_Sections.end();
}

void mitk::Forms::SubmitToCSV(const Form& form, const fs::path& csvPath)
{
  std::ofstream csvFile;

  if (fs::exists(csvPath))
  {
    csvFile.open(csvPath, std::ofstream::app);

    if (!csvFile.is_open())
      mitkThrow() << "Could not open file \"" << csvPath << "\"!";
  }
  else
  {
    csvFile.open(csvPath);

    if (!csvFile.is_open())
      mitkThrow() << "Could not create file \"" << csvPath << "\"!";

    csvFile << "\"Timestamp\"";

    for (const auto& [key, value] : form.GetSupplements())
      csvFile << ",\"" << key << '"';

    for (const auto& section : form)
    {
      for (const auto* question : section.GetQuestions())
      {
        csvFile << ",\"" << question->GetQuestionText() << '"';
      }
    }

    csvFile << '\n';
  }

  csvFile << '"' << GetCurrentISO8601DateTime() << '"';

  for (const auto& [key, value] : form.GetSupplements())
    csvFile << ",\"" << value << '"';

  for (const auto& section : form)
  {
    for (const auto* question : section.GetQuestions())
    {
      csvFile << ",\"";
      bool isFirstResponse = true;

      std::vector<std::string> responses;

      if (question->HasFileResponses())
      {
        auto paths = question->SubmitFileResponses(csvPath.parent_path());

        for (const auto& path : paths)
        {
          auto posixPath = path.string();
          std::replace(posixPath.begin(), posixPath.end(), '\\', '/');
          responses.push_back(posixPath);
        }
      }
      else
      {
        responses = question->GetResponsesAsStrings();
      }

      for (const auto& response : responses)
      {
        if (!isFirstResponse)
          csvFile << ';';

        csvFile << response;

        isFirstResponse = false;
      }

      csvFile << '"';
    }
  }

  csvFile << std::endl;
}

void mitk::Forms::from_json(const nlohmann::ordered_json& j, Form& f)
{
  if (!j.contains("FileFormat") || j["FileFormat"] != "MITK Form")
    mitkThrow() << "Expected \"FileFormat\" field to be \"MITK Form\"!";

  if (!j.contains("Version") || j["Version"] != 1)
    mitkThrow() << "Expected \"Version\" field to be 1!";;

  const auto* questionFactory = IQuestionFactory::GetInstance();

  if (j.contains("Sections"))
  {
    bool isFirstSection = true;

    for (const auto& jSection : j["Sections"])
    {
      std::string title;
      std::string description;

      if (jSection.contains("Title"))
        title = jSection["Title"];

      if (jSection.contains("Description"))
        description = jSection["Description"];

      auto& section = isFirstSection
        ? f.GetSection(0)
        : f.AddSection(title, description);

      if (isFirstSection)
      {
        section.SetTitle(title);
        section.SetDescription(description);

        isFirstSection = false;
      }

      if (jSection.contains("Questions"))
      {
        for (const auto& jQuestion : jSection["Questions"])
        {
          auto question = questionFactory->Create(jQuestion["Type"]);
          question->FromJSON(jQuestion);

          section.AddQuestion(question);
        }
      }
    }
  }
}

void mitk::Forms::to_json(nlohmann::ordered_json& j, const Form& f)
{
  j["FileFormat"] = "MITK Form";
  j["Version"] = 1;

  const int numberOfSections = f.GetNumberOfSections();

  for (int index = 0; index < numberOfSections; ++index)
  {
    const auto& section = f.GetSection(index);
    ordered_json jSection;

    if (auto title = section.GetTitle(); !title.empty())
      jSection["Title"] = title;

    if (auto description = section.GetDescription(); !description.empty())
      jSection["Description"] = description;

    for (const auto* question : section.GetQuestions())
    {
      ordered_json jQuestion;
      question->ToJSON(jQuestion);

      jSection["Questions"].push_back(jQuestion);
    }

    j["Sections"].push_back(jSection);
  }
}
