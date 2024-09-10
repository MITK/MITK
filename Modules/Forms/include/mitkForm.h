/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkForm_h
#define mitkForm_h

#include <mitkFileSystem.h>
#include <MitkFormsExports.h>

#include <memory>
#include <string>
#include <vector>

#include <nlohmann/json_fwd.hpp>

namespace mitk::Forms
{
  class Question;

  /** A form consisting of questions possibly divided into multiple sections/pages.
   *
   * A form always has at least a single section, which is also used to define the form's general
   * title and description. Helper methods like SetTitle() or SetDescription() can be used to
   * conveniently set these properties of the first section.
   */
  class MITKFORMS_EXPORT Form
  {
  public:
    class MITKFORMS_EXPORT Section
    {
    public:
      explicit Section(const std::string& title = "", const std::string& description = "");
      ~Section();

      Section(Section&& other) noexcept;
      Section& operator=(Section&& other) noexcept;

      std::string GetTitle() const;
      void SetTitle(const std::string& title);

      std::string GetDescription() const;
      void SetDescription(const std::string& description);

      std::vector<Question*> GetQuestions() const;
      void AddQuestion(Question* question);

    private:
      std::string m_Title;
      std::string m_Description;
      std::vector<std::unique_ptr<Question>> m_Questions;
    };

    explicit Form(const std::string& title = "", const std::string& description = "");
    ~Form();

    Form(Form&& other) noexcept;
    Form& operator=(Form&& other) noexcept;

    Section& AddSection(const std::string& title = "", const std::string& description = "");
    int GetNumberOfSections() const;

    Section& GetSection(int index);
    const Section& GetSection(int index) const;

    std::string GetTitle() const;
    void SetTitle(const std::string& title);

    std::string GetDescription() const;
    void SetDescription(const std::string& description);

    std::vector<Question*> GetQuestions() const;
    void AddQuestion(Question* question);

    std::vector<Section>::const_iterator begin() const;
    std::vector<Section>::const_iterator end() const;

    std::vector<Section>::iterator begin();
    std::vector<Section>::iterator end();

  private:
    std::vector<Section> m_Sections;
  };

  MITKFORMS_EXPORT void SubmitToCSV(const Form& form, const fs::path& csvPath);

  MITKFORMS_EXPORT void from_json(const nlohmann::ordered_json& j, Form& f);
  MITKFORMS_EXPORT void to_json(nlohmann::ordered_json& j, const Form& f);
}

#endif
