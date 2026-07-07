/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkCheckboxesQuestionWidget_h
#define QmitkCheckboxesQuestionWidget_h

#include <QmitkQuestionWidget.h>
#include <mitkCheckboxesQuestion.h>

class QButtonGroup;
class QGridLayout;
class QLineEdit;

/** \brief Question widget for CheckboxesQuestion, displaying checkboxes with an optional "Other" field.
 *
 * Each option is shown as a checkbox, allowing multiple selections. If the question
 * has an "Other" option enabled, an additional checkbox with a text field is shown.
 *
 * \sa mitk::Forms::CheckboxesQuestion, QmitkQuestionWidget
 */
class MITKFORMSUI_EXPORT QmitkCheckboxesQuestionWidget : public QmitkQuestionWidget
{
  Q_OBJECT

public:
  /** \brief Construct the widget.
   *
   * \param[in] parent The parent widget.
   */
  explicit QmitkCheckboxesQuestionWidget(QWidget* parent = nullptr);

  /** \brief Destructor.
   */
  ~QmitkCheckboxesQuestionWidget() override;

  /** \brief Create a new QmitkCheckboxesQuestionWidget instance.
   *
   * \param[in] parent The parent widget.
   * \return A pointer to the new widget. The caller takes ownership.
   */
  QmitkQuestionWidget* CreateAnother(QWidget* parent = nullptr) const override;

  /** \brief Get the associated CheckboxesQuestion.
   *
   * \return A pointer to the CheckboxesQuestion, or \c nullptr if not set.
   */
  mitk::Forms::Question* GetQuestion() const override;

  /** \brief Initialize the widget for a CheckboxesQuestion.
   *
   * \param[in] question Must be a CheckboxesQuestion. The widget does not take ownership.
   * \throw mitk::Exception If \p question is not a CheckboxesQuestion.
   */
  void SetQuestion(mitk::Forms::Question* question) override;

  /** \brief Uncheck all checkboxes, clear the "Other" text, and reset all responses.
   */
  void Reset() override;

private:
  void CreateWidgets();
  void RemoveWidgets();

  void OnIdToggled(int id, bool checked);
  void OnTextEdited(const QString& text);
  void OnEditingFinished();

  mitk::Forms::CheckboxesQuestion* m_Question;

  QGridLayout* m_Layout;
  QButtonGroup* m_ButtonGroup;
  QLineEdit* m_OtherLineEdit;
  int m_OtherId;
};

#endif
