/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkDropdownQuestionWidget_h
#define QmitkDropdownQuestionWidget_h

#include <QmitkQuestionWidget.h>
#include <mitkDropdownQuestion.h>

class QComboBox;

/** \brief Question widget for DropdownQuestion, displaying options in a QComboBox.
 *
 * The combo box shows a "Choose" placeholder item followed by a separator and
 * the actual options. Only a single option may be selected at a time.
 *
 * \sa mitk::Forms::DropdownQuestion, QmitkQuestionWidget
 */
class MITKFORMSUI_EXPORT QmitkDropdownQuestionWidget : public QmitkQuestionWidget
{
  Q_OBJECT

public:
  /** \brief Construct the widget.
   *
   * \param[in] parent The parent widget.
   */
  explicit QmitkDropdownQuestionWidget(QWidget* parent = nullptr);

  /** \brief Destructor.
   */
  ~QmitkDropdownQuestionWidget() override;

  /** \brief Create a new QmitkDropdownQuestionWidget instance.
   *
   * \param[in] parent The parent widget.
   * \return A pointer to the new widget. The caller takes ownership.
   */
  QmitkQuestionWidget* CreateAnother(QWidget* parent = nullptr) const override;

  /** \brief Get the associated DropdownQuestion.
   *
   * \return A pointer to the DropdownQuestion, or \c nullptr if not set.
   */
  mitk::Forms::Question* GetQuestion() const override;

  /** \brief Initialize the widget for a DropdownQuestion.
   *
   * \param[in] question Must be a DropdownQuestion. The widget does not take ownership.
   * \throw mitk::Exception If \p question is not a DropdownQuestion.
   */
  void SetQuestion(mitk::Forms::Question* question) override;

  /** \brief Reset the combo box selection and clear all responses.
   */
  void Reset() override;

private:
  void CreateWidgets();
  void RemoveWidgets();

  void OnCurrentIndexChanged(int index);

  mitk::Forms::DropdownQuestion* m_Question;

  QVBoxLayout* m_Layout;
  QComboBox* m_ComboBox;
};

#endif
