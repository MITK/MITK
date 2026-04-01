/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkLinearScaleQuestionWidget_h
#define QmitkLinearScaleQuestionWidget_h

#include <QmitkQuestionWidget.h>
#include <mitkLinearScaleQuestion.h>

class QButtonGroup;
class QGridLayout;
class QPushButton;

/** \brief Question widget for LinearScaleQuestion, displaying radio buttons on a numeric scale.
 *
 * The widget renders a row of radio buttons between the range bounds with optional
 * low-end and high-end labels. A "Clear selection" button appears after an option
 * is selected (unless the question is required).
 *
 * \sa mitk::Forms::LinearScaleQuestion, QmitkQuestionWidget
 */
class MITKFORMSUI_EXPORT QmitkLinearScaleQuestionWidget : public QmitkQuestionWidget
{
  Q_OBJECT

public:
  /** \brief Construct the widget.
   *
   * \param[in] parent The parent widget.
   */
  explicit QmitkLinearScaleQuestionWidget(QWidget* parent = nullptr);

  /** \brief Destructor.
   */
  ~QmitkLinearScaleQuestionWidget() override;

  /** \brief Create a new QmitkLinearScaleQuestionWidget instance.
   *
   * \param[in] parent The parent widget.
   * \return A pointer to the new widget. The caller takes ownership.
   */
  QmitkQuestionWidget* CreateAnother(QWidget* parent = nullptr) const override;

  /** \brief Get the associated LinearScaleQuestion.
   *
   * \return A pointer to the LinearScaleQuestion, or \c nullptr if not set.
   */
  mitk::Forms::Question* GetQuestion() const override;

  /** \brief Initialize the widget for a LinearScaleQuestion.
   *
   * \param[in] question Must be a LinearScaleQuestion. The widget does not take ownership.
   * \throw mitk::Exception If \p question is not a LinearScaleQuestion.
   */
  void SetQuestion(mitk::Forms::Question* question) override;

  /** \brief Reset the radio button selection and clear all responses.
   */
  void Reset() override;

private:
  void CreateWidgets();
  void RemoveWidgets();

  void OnIdClicked(int id);
  void OnClearButtonClicked();

  mitk::Forms::LinearScaleQuestion* m_Question;

  QGridLayout* m_Layout;
  QButtonGroup* m_ButtonGroup;
  QPushButton* m_ClearButton;
};

#endif
