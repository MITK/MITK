/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkShortAnswerQuestionWidget_h
#define QmitkShortAnswerQuestionWidget_h

#include <QmitkQuestionWidget.h>
#include <mitkShortAnswerQuestion.h>

class QLineEdit;

/** \brief Question widget for ShortAnswerQuestion, displaying a single-line text input.
 *
 * The widget uses a QLineEdit with a "Your answer" placeholder text.
 *
 * \sa mitk::Forms::ShortAnswerQuestion, QmitkQuestionWidget
 */
class MITKFORMSUI_EXPORT QmitkShortAnswerQuestionWidget : public QmitkQuestionWidget
{
  Q_OBJECT

public:
  /** \brief Construct the widget.
   *
   * \param[in] parent The parent widget.
   */
  explicit QmitkShortAnswerQuestionWidget(QWidget* parent = nullptr);

  /** \brief Destructor.
   */
  ~QmitkShortAnswerQuestionWidget() override;

  /** \brief Create a new QmitkShortAnswerQuestionWidget instance.
   *
   * \param[in] parent The parent widget.
   * \return A pointer to the new widget. The caller takes ownership.
   */
  QmitkQuestionWidget* CreateAnother(QWidget* parent = nullptr) const override;

  /** \brief Get the associated ShortAnswerQuestion.
   *
   * \return A pointer to the ShortAnswerQuestion, or \c nullptr if not set.
   */
  mitk::Forms::Question* GetQuestion() const override;

  /** \brief Initialize the widget for a ShortAnswerQuestion.
   *
   * \param[in] question Must be a ShortAnswerQuestion. The widget does not take ownership.
   * \throw mitk::Exception If \p question is not a ShortAnswerQuestion.
   */
  void SetQuestion(mitk::Forms::Question* question) override;

  /** \brief Clear the line edit and reset all responses.
   */
  void Reset() override;

private:
  void OnTextEdited(const QString& text);

  mitk::Forms::ShortAnswerQuestion* m_Question;

  QVBoxLayout* m_Layout;
  QLineEdit* m_LineEdit;
};

#endif
