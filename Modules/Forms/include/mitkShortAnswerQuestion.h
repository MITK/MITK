/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkShortAnswerQuestion_h
#define mitkShortAnswerQuestion_h

#include <mitkTextQuestion.h>

namespace mitk::Forms
{
  /** \brief A Question whose possible response is a short single-line free text.
   *
   * ShortAnswerQuestion is intended for brief, single-line text answers. For longer multi-line
   * text, use ParagraphQuestion instead.
   *
   * \sa ParagraphQuestion, TextQuestion
   */
  class MITKFORMS_EXPORT ShortAnswerQuestion : public TextQuestion
  {
  public:
    /** \brief Virtual destructor.
     */
    ~ShortAnswerQuestion() override;

    /** \brief Return the type string "Short answer".
     *
     * \return The string "Short answer".
     */
    std::string GetType() const override;

    /** \brief Create a new default-constructed ShortAnswerQuestion instance.
     *
     * \return A pointer to a new ShortAnswerQuestion. The caller takes ownership.
     */
    Question* CreateAnother() const override;

    /** \brief Deserialize this ShortAnswerQuestion from JSON.
     *
     * \param[in] j The JSON object to deserialize from.
     */
    void FromJSON(const nlohmann::ordered_json& j) override;

    /** \brief Serialize this ShortAnswerQuestion to JSON.
     *
     * \param[out] j The JSON object to serialize into.
     */
    void ToJSON(nlohmann::ordered_json& j) const override;
  };

  /** \brief Deserialize a ShortAnswerQuestion from JSON.
   *
   * \param[in] j The JSON object to deserialize from.
   * \param[in,out] q The ShortAnswerQuestion to populate.
   *
   * \sa to_json(nlohmann::ordered_json&, const ShortAnswerQuestion&)
   */
  MITKFORMS_EXPORT void from_json(const nlohmann::ordered_json& j, ShortAnswerQuestion& q);

  /** \brief Serialize a ShortAnswerQuestion to JSON.
   *
   * \param[out] j The JSON object to serialize into.
   * \param[in] q The ShortAnswerQuestion to serialize.
   *
   * \sa from_json(const nlohmann::ordered_json&, ShortAnswerQuestion&)
   */
  MITKFORMS_EXPORT void to_json(nlohmann::ordered_json& j, const ShortAnswerQuestion& q);
}

#endif
