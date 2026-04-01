/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkScreenshotQuestion_h
#define mitkScreenshotQuestion_h

#include <mitkQuestion.h>
#include <mitkFileSystem.h>

#include <set>

namespace mitk::Forms
{
  /** \brief A Question whose responses are screenshot image files.
   *
   * Users can take screenshots of render windows as responses. The screenshots
   * are stored as temporary PNG files. During submission, the files are copied
   * to a "screenshots" subdirectory relative to the CSV output path.
   *
   * \sa Question
   */
  class MITKFORMS_EXPORT ScreenshotQuestion : public Question
  {
  public:
    /** \brief Virtual destructor.
     */
    ~ScreenshotQuestion() override;

    /** \brief Return the type string "Screenshot".
     *
     * \return The string "Screenshot".
     */
    std::string GetType() const override;

    /** \brief Create a new default-constructed ScreenshotQuestion instance.
     *
     * \return A pointer to a new ScreenshotQuestion. The caller takes ownership.
     */
    Question* CreateAnother() const override;

    /** \brief Deserialize this ScreenshotQuestion from JSON.
     *
     * \param[in] j The JSON object to deserialize from.
     */
    void FromJSON(const nlohmann::ordered_json& j) override;

    /** \brief Serialize this ScreenshotQuestion to JSON.
     *
     * \param[out] j The JSON object to serialize into.
     */
    void ToJSON(nlohmann::ordered_json& j) const override;

    /** \brief Return the screenshot file paths as strings.
     *
     * \return A vector of file path strings for each screenshot.
     */
    std::vector<std::string> GetResponsesAsStrings() const override;

    /** \brief Clear all screenshot responses.
     */
    void ClearResponses() override;

    /** \brief Check whether at least one screenshot has been taken.
     *
     * \return \c true if at least one screenshot path is stored, \c false otherwise.
     */
    bool IsComplete() const override;

    /** \brief Always returns \c true since responses are file paths.
     *
     * \return \c true.
     */
    bool HasFileResponses() const override;

    /** \brief Copy screenshot files to a "screenshots" subdirectory and return relative paths.
     *
     * Creates a "screenshots" subdirectory under \p basePath if it does not exist,
     * copies the screenshot files there, and returns paths relative to \p basePath.
     *
     * \param[in] basePath The base directory for the submission output.
     * \return A vector of relative paths to the copied screenshot files, or an empty
     *         vector if no screenshots exist.
     */
    std::vector<fs::path> SubmitFileResponses(const fs::path& basePath) const override;

    /** \brief Add a screenshot file path as a response.
     *
     * The screenshot is only added if the file at the given path exists.
     *
     * \param[in] path The file path of the screenshot image.
     */
    void AddScreenshot(const fs::path& path);

    /** \brief Remove a screenshot response and delete the file from disk.
     *
     * If the path was among the stored responses and the file exists on disk,
     * the file is deleted. Filesystem errors during deletion are silently ignored.
     *
     * \param[in] path The file path of the screenshot to remove.
     */
    void RemoveScreenshot(const fs::path& path);

  private:
    std::set<fs::path> m_Responses;
  };

  /** \brief Deserialize a ScreenshotQuestion from JSON.
   *
   * \param[in] j The JSON object to deserialize from.
   * \param[in,out] q The ScreenshotQuestion to populate.
   *
   * \sa to_json(nlohmann::ordered_json&, const ScreenshotQuestion&)
   */
  MITKFORMS_EXPORT void from_json(const nlohmann::ordered_json& j, ScreenshotQuestion& q);

  /** \brief Serialize a ScreenshotQuestion to JSON.
   *
   * \param[out] j The JSON object to serialize into.
   * \param[in] q The ScreenshotQuestion to serialize.
   *
   * \sa from_json(const nlohmann::ordered_json&, ScreenshotQuestion&)
   */
  MITKFORMS_EXPORT void to_json(nlohmann::ordered_json& j, const ScreenshotQuestion& q);
}

#endif
