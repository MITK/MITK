/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkParagraphQuestionWidget_h
#define QmitkParagraphQuestionWidget_h

#include <QmitkQuestionWidget.h>
#include <mitkParagraphQuestion.h>

class QTextEdit;

/** \brief Question widget for ParagraphQuestion, displaying a multi-line text editor.
 *
 * The widget uses a QTextEdit with auto-adjusting height based on the entered text content.
 *
 * \sa mitk::Forms::ParagraphQuestion, QmitkQuestionWidget
 */
class MITKFORMSUI_EXPORT QmitkParagraphQuestionWidget : public QmitkQuestionWidget
{
  Q_OBJECT

public:
  /** \brief Construct the widget.
   *
   * \param[in] parent The parent widget.
   */
  explicit QmitkParagraphQuestionWidget(QWidget* parent = nullptr);

  /** \brief Destructor.
   */
  ~QmitkParagraphQuestionWidget() override;

  /** \brief Create a new QmitkParagraphQuestionWidget instance.
   *
   * \param[in] parent The parent widget.
   * \return A pointer to the new widget. The caller takes ownership.
   */
  QmitkQuestionWidget* CreateAnother(QWidget* parent = nullptr) const override;

  /** \brief Get the associated ParagraphQuestion.
   *
   * \return A pointer to the ParagraphQuestion, or \c nullptr if not set.
   */
  mitk::Forms::Question* GetQuestion() const override;

  /** \brief Initialize the widget for a ParagraphQuestion.
   *
   * \param[in] question Must be a ParagraphQuestion. The widget does not take ownership.
   * \throw mitk::Exception If \p question is not a ParagraphQuestion.
   */
  void SetQuestion(mitk::Forms::Question* question) override;

  /** \brief Clear the text editor and reset all responses.
   */
  void Reset() override;

private:
  void AdjustHeight();
  void OnTextChanged();

  mitk::Forms::ParagraphQuestion* m_Question;

  QVBoxLayout* m_Layout;
  QTextEdit* m_TextEdit;
};

#endif
