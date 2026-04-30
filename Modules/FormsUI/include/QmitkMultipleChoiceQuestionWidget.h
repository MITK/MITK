/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkMultipleChoiceQuestionWidget_h
#define QmitkMultipleChoiceQuestionWidget_h

#include <QmitkQuestionWidget.h>
#include <mitkMultipleChoiceQuestion.h>

class QButtonGroup;
class QGridLayout;
class QLineEdit;
class QPushButton;

/** \brief Question widget for MultipleChoiceQuestion, displaying radio buttons with an optional "Other" field.
 *
 * Each option is shown as a radio button. If the question has an "Other" option enabled,
 * an additional radio button with a text field is shown. A "Clear selection" button
 * appears after a selection is made (unless the question is required).
 *
 * \sa mitk::Forms::MultipleChoiceQuestion, QmitkQuestionWidget
 */
class MITKFORMSUI_EXPORT QmitkMultipleChoiceQuestionWidget : public QmitkQuestionWidget
{
  Q_OBJECT

public:
  /** \brief Construct the widget.
   *
   * \param[in] parent The parent widget.
   */
  explicit QmitkMultipleChoiceQuestionWidget(QWidget* parent = nullptr);

  /** \brief Destructor.
   */
  ~QmitkMultipleChoiceQuestionWidget() override;

  /** \brief Create a new QmitkMultipleChoiceQuestionWidget instance.
   *
   * \param[in] parent The parent widget.
   * \return A pointer to the new widget. The caller takes ownership.
   */
  QmitkQuestionWidget* CreateAnother(QWidget* parent = nullptr) const override;

  /** \brief Get the associated MultipleChoiceQuestion.
   *
   * \return A pointer to the MultipleChoiceQuestion, or \c nullptr if not set.
   */
  mitk::Forms::Question* GetQuestion() const override;

  /** \brief Initialize the widget for a MultipleChoiceQuestion.
   *
   * \param[in] question Must be a MultipleChoiceQuestion. The widget does not take ownership.
   * \throw mitk::Exception If \p question is not a MultipleChoiceQuestion.
   */
  void SetQuestion(mitk::Forms::Question* question) override;

  /** \brief Reset the radio button selection, clear the "Other" text, and clear all responses.
   */
  void Reset() override;

private:
  void CreateWidgets();
  void RemoveWidgets();

  void OnIdClicked(int id);
  void OnTextEdited(const QString& text);
  void OnEditingFinished();
  void OnClearButtonClicked();

  mitk::Forms::MultipleChoiceQuestion* m_Question;

  QGridLayout* m_Layout;
  QButtonGroup* m_ButtonGroup;
  QLineEdit* m_OtherLineEdit;
  int m_OtherId;
  QPushButton* m_ClearButton;
};

#endif
