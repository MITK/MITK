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
#include <map>
#include <vector>

#include <nlohmann/json_fwd.hpp>

namespace mitk::Forms
{
  class Question;

  /** \brief A form consisting of questions possibly divided into multiple sections/pages.
   *
   * A form always has at least a single section, which is also used to define the form's general
   * title and description. Helper methods like SetTitle() or SetDescription() can be used to
   * conveniently set these properties of the first section.
   *
   * Forms can be serialized to and deserialized from JSON and submitted to CSV files.
   *
   * \sa Question, SubmitToCSV()
   */
  class MITKFORMS_EXPORT Form
  {
  public:
    /** \brief A section within a Form, grouping related questions together.
     *
     * Each section has its own title and description, and holds a list of Question objects.
     * The Section takes ownership of any Question pointers added to it.
     *
     * \sa Form
     */
    class MITKFORMS_EXPORT Section
    {
    public:
      /** \brief Construct a section with optional title and description.
       *
       * \param[in] title The section title.
       * \param[in] description The section description.
       */
      explicit Section(const std::string& title = "", const std::string& description = "");

      /** \brief Destructor.
       */
      ~Section();

      /** \brief Move constructor.
       */
      Section(Section&& other) noexcept;

      /** \brief Move assignment operator.
       */
      Section& operator=(Section&& other) noexcept;

      /** \brief Get the title of this section.
       *
       * \return The section title.
       */
      std::string GetTitle() const;

      /** \brief Set the title of this section.
       *
       * \param[in] title The section title.
       */
      void SetTitle(const std::string& title);

      /** \brief Get the description of this section.
       *
       * \return The section description.
       */
      std::string GetDescription() const;

      /** \brief Set the description of this section.
       *
       * \param[in] description The section description.
       */
      void SetDescription(const std::string& description);

      /** \brief Get all questions belonging to this section.
       *
       * \return A vector of non-owning pointers to the questions.
       */
      std::vector<Question*> GetQuestions() const;

      /** \brief Add a question to this section.
       *
       * The section takes ownership of the pointer.
       *
       * \param[in] question A pointer to the question to add. Ownership is transferred.
       */
      void AddQuestion(Question* question);

    private:
      std::string m_Title;
      std::string m_Description;
      std::vector<std::unique_ptr<Question>> m_Questions;
    };

    /** \brief Construct a form with an optional title and description for its first section.
     *
     * A form always starts with at least one section.
     *
     * \param[in] title The form title (stored as the first section's title).
     * \param[in] description The form description (stored as the first section's description).
     */
    explicit Form(const std::string& title = "", const std::string& description = "");

    /** \brief Destructor.
     */
    ~Form();

    /** \brief Move constructor.
     */
    Form(Form&& other) noexcept;

    /** \brief Move assignment operator.
     */
    Form& operator=(Form&& other) noexcept;

    /** \brief Add a new section to the form.
     *
     * \param[in] title The title of the new section.
     * \param[in] description The description of the new section.
     * \return A reference to the newly added section.
     */
    Section& AddSection(const std::string& title = "", const std::string& description = "");

    /** \brief Get the total number of sections in this form.
     *
     * \return The number of sections (always at least 1).
     */
    int GetNumberOfSections() const;

    /** \brief Get a section by its index.
     *
     * \param[in] index The zero-based section index.
     * \return A reference to the requested section.
     * \throw std::out_of_range If \p index is out of bounds.
     */
    Section& GetSection(int index);

    /** \brief Get a section by its index (const overload).
     *
     * \param[in] index The zero-based section index.
     * \return A const reference to the requested section.
     * \throw std::out_of_range If \p index is out of bounds.
     */
    const Section& GetSection(int index) const;

    /** \brief Convenience method to get the title of the first section (the form title).
     *
     * \return The form title.
     */
    std::string GetTitle() const;

    /** \brief Convenience method to set the title of the first section (the form title).
     *
     * \param[in] title The form title.
     */
    void SetTitle(const std::string& title);

    /** \brief Convenience method to get the description of the first section (the form description).
     *
     * \return The form description.
     */
    std::string GetDescription() const;

    /** \brief Convenience method to set the description of the first section (the form description).
     *
     * \param[in] description The form description.
     */
    void SetDescription(const std::string& description);

    /** \brief Convenience method to get all questions from the first section.
     *
     * \return A vector of non-owning pointers to the questions in the first section.
     */
    std::vector<Question*> GetQuestions() const;

    /** \brief Convenience method to add a question to the first section.
     *
     * The first section takes ownership of the pointer.
     *
     * \param[in] question A pointer to the question to add. Ownership is transferred.
     */
    void AddQuestion(Question* question);

    /** \brief Add extra data that is submitted together with the responses.
     *
     * Supplements are a rudimentary and delicate feature to add extra
     * data to submissions, i.e., extra columns for CSV submissions.
     *
     * Supplements are key-value pairs supposed to be completely set up before
     * the first submission. Afterwards, do not add any new supplements.
     * Values of existing supplements can be safely modified, though.
     *
     * If a supplement with the given key already exists, the call has no effect.
     *
     * \param[in] key The key (column header) for the supplement.
     *
     * \note Do not add new supplements after the first submission.
     * \note Submissions do not clear any supplements or their values.
     */
    void AddSupplement(const std::string& key);

    /** \brief Set the value of a certain existing supplement.
     *
     * \param[in] key The key of the supplement to update.
     * \param[in] value The new value for the supplement.
     * \return \c true if the supplement exists and was updated, \c false otherwise.
     *
     * \sa AddSupplement()
     */
    bool SetSupplement(const std::string& key, const std::string& value);

    /** \brief Get all supplements as a key-value map.
     *
     * \return A const reference to the internal supplements map.
     *
     * \sa AddSupplement(), SetSupplement()
     */
    const std::map<std::string, std::string>& GetSupplements() const;

    /** \brief Get a const iterator to the first section (for range-based for loops).
     */
    std::vector<Section>::const_iterator begin() const;

    /** \brief Get a const iterator past the last section (for range-based for loops).
     */
    std::vector<Section>::const_iterator end() const;

    /** \brief Get an iterator to the first section (for range-based for loops).
     */
    std::vector<Section>::iterator begin();

    /** \brief Get an iterator past the last section (for range-based for loops).
     */
    std::vector<Section>::iterator end();

  private:
    std::vector<Section> m_Sections;
    std::map<std::string, std::string> m_Supplements;
  };

  /** \brief Submit form responses to a CSV file.
   *
   * If the CSV file does not exist, it is created with a header row containing a timestamp column,
   * supplement columns, and a column for each question. If the file already exists, a new row of
   * responses is appended.
   *
   * Questions whose HasFileResponses() returns \c true have their files copied via
   * SubmitFileResponses() and the resulting relative paths are written instead.
   *
   * \param[in] form The form whose responses are submitted.
   * \param[in] csvPath The path to the CSV file.
   *
   * \throw mitk::Exception If the CSV file cannot be opened or created.
   *
   * \sa Form, Question::HasFileResponses(), Question::SubmitFileResponses()
   */
  MITKFORMS_EXPORT void SubmitToCSV(const Form& form, const fs::path& csvPath);

  /** \brief Deserialize a Form from JSON.
   *
   * Expects the JSON to contain a "FileFormat" field set to "MITK Form" and a "Version" field
   * set to 1. Questions are created via IQuestionFactory based on their "Type" field.
   *
   * \param[in] j The JSON object to deserialize from.
   * \param[in,out] f The Form object to populate.
   *
   * \throw mitk::Exception If the file format or version is invalid.
   *
   * \sa to_json(nlohmann::ordered_json&, const Form&)
   */
  MITKFORMS_EXPORT void from_json(const nlohmann::ordered_json& j, Form& f);

  /** \brief Serialize a Form to JSON.
   *
   * \param[out] j The JSON object to serialize into.
   * \param[in] f The Form object to serialize.
   *
   * \sa from_json(const nlohmann::ordered_json&, Form&)
   */
  MITKFORMS_EXPORT void to_json(nlohmann::ordered_json& j, const Form& f);
}

#endif
