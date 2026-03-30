/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkParagraphQuestion_h
#define mitkParagraphQuestion_h

#include <mitkTextQuestion.h>

namespace mitk::Forms
{
  /** \brief A Question whose possible response is multi-line free text.
   *
   * ParagraphQuestion is intended for longer text answers. For single-line short text,
   * use ShortAnswerQuestion instead.
   *
   * \sa ShortAnswerQuestion, TextQuestion
   */
  class MITKFORMS_EXPORT ParagraphQuestion : public TextQuestion
  {
  public:
    /** \brief Virtual destructor.
     */
    ~ParagraphQuestion() override;

    /** \brief Return the type string "Paragraph".
     *
     * \return The string "Paragraph".
     */
    std::string GetType() const override;

    /** \brief Create a new default-constructed ParagraphQuestion instance.
     *
     * \return A pointer to a new ParagraphQuestion. The caller takes ownership.
     */
    Question* CreateAnother() const override;

    /** \brief Deserialize this ParagraphQuestion from JSON.
     *
     * \param[in] j The JSON object to deserialize from.
     */
    void FromJSON(const nlohmann::ordered_json& j) override;

    /** \brief Serialize this ParagraphQuestion to JSON.
     *
     * \param[out] j The JSON object to serialize into.
     */
    void ToJSON(nlohmann::ordered_json& j) const override;
  };

  /** \brief Deserialize a ParagraphQuestion from JSON.
   *
   * \param[in] j The JSON object to deserialize from.
   * \param[in,out] q The ParagraphQuestion to populate.
   *
   * \sa to_json(nlohmann::ordered_json&, const ParagraphQuestion&)
   */
  MITKFORMS_EXPORT void from_json(const nlohmann::ordered_json& j, ParagraphQuestion& q);

  /** \brief Serialize a ParagraphQuestion to JSON.
   *
   * \param[out] j The JSON object to serialize into.
   * \param[in] q The ParagraphQuestion to serialize.
   *
   * \sa from_json(const nlohmann::ordered_json&, ParagraphQuestion&)
   */
  MITKFORMS_EXPORT void to_json(nlohmann::ordered_json& j, const ParagraphQuestion& q);
}

#endif
