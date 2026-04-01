/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkLinearScaleQuestion_h
#define mitkLinearScaleQuestion_h

#include <mitkQuestion.h>
#include <optional>
#include <utility>

namespace mitk::Forms
{
  /** \brief A Question whose possible responses are represented by a discrete linear scale.
   *
   * The question can have a single response, which is one of the numbers within the allowed
   * range. The range must start with 0 or 1 and end with any number between 2 and 10 (1 to 5
   * by default). Optional labels can be assigned to the low and high ends of the scale.
   *
   * \sa Question
   */
  class MITKFORMS_EXPORT LinearScaleQuestion : public Question
  {
  public:
    /** \brief Constructor.
     *
     * Initializes the range to [1, 5] with no response and empty labels.
     */
    LinearScaleQuestion();

    /** \brief Virtual destructor.
     */
    ~LinearScaleQuestion() override;

    /** \brief Return the type string "Linear scale".
     *
     * \return The string "Linear scale".
     */
    std::string GetType() const override;

    /** \brief Create a new default-constructed LinearScaleQuestion instance.
     *
     * \return A pointer to a new LinearScaleQuestion. The caller takes ownership.
     */
    Question* CreateAnother() const override;

    /** \brief Deserialize this LinearScaleQuestion from JSON.
     *
     * \param[in] j The JSON object to deserialize from.
     */
    void FromJSON(const nlohmann::ordered_json& j) override;

    /** \brief Serialize this LinearScaleQuestion to JSON.
     *
     * \param[out] j The JSON object to serialize into.
     */
    void ToJSON(nlohmann::ordered_json& j) const override;

    /** \brief Return the numeric response as a single-element vector of strings.
     *
     * \return A vector containing the response as a string, or an empty vector if no
     *         response has been given.
     */
    std::vector<std::string> GetResponsesAsStrings() const override;

    /** \brief Clear the response.
     */
    void ClearResponses() override;

    /** \brief Check whether a response has been given.
     *
     * \return \c true if a response value is set, \c false otherwise.
     */
    bool IsComplete() const override;

    /** \brief Get the current numeric response.
     *
     * \return The response value, or \c std::nullopt if no response has been given.
     */
    std::optional<int> GetResponse() const;

    /** \brief Set the numeric response.
     *
     * \param[in] response The selected number on the scale.
     */
    void SetResponse(int response);

    /** \brief Get the allowed range.
     *
     * \return A pair where \c first is the lower bound and \c second is the upper bound.
     */
    std::pair<int, int> GetRange() const;

    /** \brief Set the allowed range of numbers for a valid response.
     *
     * A valid range must start with 0 or 1 and end with any number between 2 and 10 (1 to 5
     * by default).
     *
     * \param[in] range A pair where \c first is the lower bound (0 or 1) and \c second is
     *            the upper bound (2 to 10).
     *
     * \throw mitk::Exception If the range is invalid.
     */
    void SetRange(const std::pair<int, int>& range);

    /** \brief Get the labels for the low and high ends of the scale.
     *
     * \return A pair where \c first is the low-end label and \c second is the high-end label.
     */
    std::pair<std::string, std::string> GetRangeLabels() const;

    /** \brief Set the labels for the low and high ends of the scale.
     *
     * \param[in] labels A pair where \c first is the low-end label and \c second is the
     *            high-end label.
     */
    void SetRangeLabels(const std::pair<std::string, std::string>& labels);

  private:
    std::optional<int> m_Response;
    std::pair<int, int> m_Range;
    std::pair<std::string, std::string> m_Labels;
  };

  /** \brief Deserialize a LinearScaleQuestion from JSON.
   *
   * \param[in] j The JSON object to deserialize from.
   * \param[in,out] q The LinearScaleQuestion to populate.
   *
   * \sa to_json(nlohmann::ordered_json&, const LinearScaleQuestion&)
   */
  MITKFORMS_EXPORT void from_json(const nlohmann::ordered_json& j, LinearScaleQuestion& q);

  /** \brief Serialize a LinearScaleQuestion to JSON.
   *
   * \param[out] j The JSON object to serialize into.
   * \param[in] q The LinearScaleQuestion to serialize.
   *
   * \sa from_json(const nlohmann::ordered_json&, LinearScaleQuestion&)
   */
  MITKFORMS_EXPORT void to_json(nlohmann::ordered_json& j, const LinearScaleQuestion& q);
}

#endif
