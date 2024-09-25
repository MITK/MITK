/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkQuestion_h
#define mitkQuestion_h

#include <mitkFileSystem.h>
#include <MitkFormsExports.h>

#include <string>
#include <vector>

#include <nlohmann/json_fwd.hpp>

namespace mitk::Forms
{
  /** \brief Abstract base class for all types of questions used in a Form.
   *
   * Please make sure to read the full documentation of the pure virtual functions in particular to
   * fully understand implications and requirements.
   */
  class MITKFORMS_EXPORT Question
  {
  public:
    Question();
    virtual ~Question();

    /** \brief Get the literal question.
     */
    std::string GetQuestionText() const;

    /** \brief Set the literal question.
     */
    void SetQuestionText(const std::string& question);

    /** \brief Check whether a response to this question is required to complete a form.
     */
    bool IsRequired() const;

    /** \brief Set whether a resonse to this question is required to complete a form.
     *
     * A question is not required by default.
     */
    void SetRequired(bool required = true);

    /** \brief Get the text that should be displayed to clearly mark a question as required.
     *
     * This text is typically displayed as soon as a response to a required question is missing.
     * The method can be overridden to customize the default text, e.g. for more complex types
     * of questions, where specific guidance is beneficial.
     *
     * \sa IsComplete()
     */
    virtual std::string GetRequiredText() const;

    /** \brief Query whether the responses given to this question are file paths.
     *
     * Always returns \c false - override this method only if necessary. Typically used during
     * submission to determine if files should be attached.
     *
     * \sa SubmitFileResponses()
     */
    virtual bool HasFileResponses() const;

    /** \brief Attach response files to submission.
     *
     * This method is supposed to be called during submission only if HasFileResponses() returns
     * \c true. The given base path should then be used to copy any files. The returned paths
     * should be relative to the base path and are used for submission instead of the return
     * value of GetResponsesAsStrings().
     *
     * \sa HasFileResponses()
     */
    virtual std::vector<fs::path> SubmitFileResponses(const fs::path& basePath) const;

    /** \name Pure virtual functions
     *
     * Question is an abstract base class. Derive from this class to add a new type of question
     * and override the following pure virtual functions. Please read the full documentation for
     * all of these functions to fully understand implications and requirements.
     *
     * Do not forget to register any new type of question by calling IQuestionFactory::Register()
     * like it is done in this module's activator class.
     *
     * \sa IQuestionFactory
     * \{
     */

    /** \brief Return the type of a question as string, e.g. "Multiple choice" or "Drop-down".
     *
     * This method is essential for the deserialization of questions into their correct type, resp.
     * derived class. The type string is used by IQuestionFactory to look up a registered prototype
     * instance of a certain type to create another instance of it.
     *
     * The type string does not have to match the class name or follow any other convention except
     * for it must be unique amongst all question types. Prefer natural language like in the
     * examples above in case it is used in a user interface to display a question's type.
     *
     * \code{.cpp}
     * std::string RhetoricalQuestion::GetType() const
     * {
     *   return "Rhetorical";
     * }
     * \endcode
     *
     * \sa CreateAnother()
     */
    virtual std::string GetType() const = 0;

    /** \brief Create a new instance of the derived question class type.
     *
     * This method is mainly used by IQuestionFactory to create new instances from registered
     * prototype instances based on a type string.
     *
     * \code{.cpp}
     * Question* RhetoricalQuestion::CreateAnother() const
     * {
     *   return new RhetoricalQuestion;
     * }
     * \endcode
     *
     * \sa GetType()
     */
    virtual Question* CreateAnother() const = 0;

    /** \brief Return the question's response(s) as strings.
     *
     * This is the single common generic interface for retrieving responses from all types
     * of questions. It is typically used to store responses in text-based formats like CSV.
     *
     * It is implemented as vector since certain types of questions like a checkboxes question
     * may support multiple responses. Otherwise, return a vector with a single element.
     *
     * \code{.cpp}
     * std::vector<std::string> RhetoricalQuestion::GetResponsesAsStrings() const
     * {
     *   return { m_Response };
     * }
     * \endcode
     */
    virtual std::vector<std::string> GetResponsesAsStrings() const = 0;

    /** \brief Clear the/all response(s).
     */
    virtual void ClearResponses() = 0;

    /** \brief Check if a question is considered to be answered completely.
     *
     * This method is typically called when IsRequired() returns \c true to determine
     * whether the requirements are fulfilled.
     */
    virtual bool IsComplete() const = 0;

    /** \brief Deserialize from JSON.
     *
     * Polymorphism and the use of base class pointers make it necessary to implement serialization
     * through member functions. Using the pure native approach of the "JSON for Modern C++" library
     * via free functions would lead to partially serialized instances of derived classes.
     *
     * The actual implementation can and should still be located in a corresponding %from_json()
     * free function but we also need the indirection through this member function.
     *
     * \code{.cpp}
     * void RhetoricalQuestion::FromJSON(const nlohmann::ordered_json& j)
     * {
     *   from_json(j, *this);
     * }
     * \endcode
     *
     * \sa from_json(const nlohmann::ordered_json& j, Question& q)
     */
    virtual void FromJSON(const nlohmann::ordered_json& j) = 0;

    /** \brief Serialize to JSON.
     *
     * Polymorphism and the use of base class pointers make it necessary to implement serialization
     * through member functions. Using the pure native approach of the "JSON for Modern C++" library
     * via free functions would lead to partially serialized instances of derived classes.
     *
     * The actual implementation can and should still be located in a corresponding %to_json() free
     * function but we also need the indirection through this member function.
     *
     * \code{.cpp}
     * void RhetoricalQuestion::ToJSON(nlohmann::ordered_json& j) const
     * {
     *   to_json(j, *this);
     * }
     * \endcode
     *
     * \sa to_json(nlohmann::ordered_json& j, const Question& q)
     */
    virtual void ToJSON(nlohmann::ordered_json& j) const = 0;

    /**\}*/

  private:
    std::string m_QuestionText;
    bool m_IsRequired;
  };

  /** \brief Deserialize from JSON.
   *
   * Polymorphism and the use of base class pointers make it necessary to implement serialization
   * through member functions. Using the pure native approach of the "JSON for Modern C++" library
   * via free functions would lead to partially serialized instances of derived classes.
   *
   * The actual implementation can and should still be located in a %from_json() free function but we
   * also need the indirection through a corresponding member function.
   *
   * Overloaded functions for derived Question classes should reuse their base class versions of
   * the function.
   *
   * \code{.cpp}
   * void mitk::Forms::from_json(const ordered_json& j, RhetoricalQuestion& q)
   * {
   *   from_json(j, static_cast<Question&>(q));
   *   // TODO: Deserialization of members exclusive to the derived class...
   * }
   * \endcode
   *
   * \sa Question::FromJSON()
   */
  MITKFORMS_EXPORT void from_json(const nlohmann::ordered_json& j, Question& q);

  /** \brief Serialize to JSON.
   *
   * Polymorphism and the use of base class pointers make it necessary to implement serialization
   * through member functions. Using the pure native approach of the "JSON for Modern C++" library
   * via free functions would lead to partially serialized instances of derived classes.
   *
   * The actual implementation can and should still be located in a %to_json() free function but we
   * also need the indirection through a corresponding member function.
   *
   * Overloaded functions for derived Question classes should reuse their base class versions of
   * the function.
   *
   * \code{.cpp}
   * void mitk::Forms::to_json(nlohmann::ordered_json& j, const RhetoricalQuestion& q)
   * {
   *   to_json(j, static_cast<Question&>(q));
   *   // TODO: Serialization of members exclusive to the derived class...
   * }
   * \endcode
   *
   * \sa Question::ToJSON()
   */
  MITKFORMS_EXPORT void to_json(nlohmann::ordered_json& j, const Question& q);
}

#endif
