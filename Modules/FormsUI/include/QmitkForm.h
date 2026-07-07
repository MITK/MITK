/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkForm_h
#define QmitkForm_h

#include <mitkForm.h>

#include <MitkFormsUIExports.h>

#include <QWidget>
#include <memory>

namespace Ui
{
  class QmitkForm;
}

/** \brief Qt widget that renders and manages an interactive mitk::Forms::Form.
 *
 * QmitkForm presents a multi-section form to the user, handles navigation between
 * sections, validates required questions, and submits responses to a CSV file.
 * It creates QmitkQuestionWidget instances for each question via the
 * mitk::Forms::UI::IQuestionWidgetFactory service.
 *
 * If no responses path is set, the user is prompted with a file dialog on submission.
 *
 * \sa mitk::Forms::Form, QmitkQuestionWidget, mitk::Forms::UI::IQuestionWidgetFactory
 */
class MITKFORMSUI_EXPORT QmitkForm : public QWidget
{
  Q_OBJECT

public:
  /** \brief Construct the widget without a form.
   *
   * Call SetForm() afterwards to associate a form.
   *
   * \param[in] parent The parent widget.
   */
  explicit QmitkForm(QWidget* parent = nullptr);

  /** \brief Construct the widget and associate it with the given form.
   *
   * \param[in] form A pointer to the Form to display. The widget does not take ownership.
   * \param[in] parent The parent widget.
   *
   * \pre \p form must not be \c nullptr.
   */
  explicit QmitkForm(mitk::Forms::Form* form, QWidget* parent = nullptr);

  /** \brief Destructor.
   */
  ~QmitkForm() override;

  /** \brief Get the currently associated form.
   *
   * \return A pointer to the form, or \c nullptr if none is set.
   */
  mitk::Forms::Form* GetForm() const;

  /** \brief Set the form to display.
   *
   * This recreates all question widgets and resets the form state.
   *
   * \param[in] form A pointer to the Form to display. Must not be \c nullptr.
   *
   * \throw mitk::Exception If \p form is \c nullptr.
   */
  void SetForm(mitk::Forms::Form* form);

  /** \brief Get the path where form responses are saved.
   *
   * \return The CSV file path, or an empty path if not set.
   */
  fs::path GetResponsesPath() const;

  /** \brief Set the path where form responses will be saved.
   *
   * If this is set before submission, the file dialog is skipped and responses
   * are written directly to this path.
   *
   * \param[in] csvPath The file path for CSV output.
   */
  void SetResponsesPath(const fs::path& csvPath);

signals:
  /** \brief Emitted when the user clicks the submit button and all required questions
   *         in the final section are validated.
   */
  void Submit();

private:
  void CreateQuestionWidgets();
  bool ValidateCurrentSection();
  void Reset();

  void Update();
  void UpdateFormHeader();
  void UpdateSubmittedHeader();
  void UpdateSectionHeader();
  void UpdateQuestionWidgets();
  void UpdateFormButtons();

  void OnBackButtonClicked();
  void OnNextButtonClicked();
  void OnSubmitButtonClicked();
  void OnClearButtonClicked();
  void OnSubmitAnotherButtonClicked();

  std::unique_ptr<Ui::QmitkForm> m_Ui;
  mitk::Forms::Form* m_Form;
  fs::path m_ResponsesPath;
  bool m_HasBeenSubmitted;
};

#endif
