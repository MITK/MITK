/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkScreenshotQuestionWidget_h
#define QmitkScreenshotQuestionWidget_h

#include <QmitkQuestionWidget.h>
#include <mitkScreenshotQuestion.h>

class QmitkScreenshotWidget;

class QMenu;
class QPushButton;

/** \brief Question widget for ScreenshotQuestion, allowing users to take screenshots of render windows.
 *
 * A "Take Screenshot" button shows a popup menu with axial, sagittal, and coronal
 * view options. Taken screenshots appear as thumbnail widgets that can be removed.
 *
 * \sa mitk::Forms::ScreenshotQuestion, QmitkQuestionWidget
 */
class MITKFORMSUI_EXPORT QmitkScreenshotQuestionWidget : public QmitkQuestionWidget
{
  Q_OBJECT

public:
  /** \brief Construct the widget.
   *
   * \param[in] parent The parent widget.
   */
  explicit QmitkScreenshotQuestionWidget(QWidget* parent = nullptr);

  /** \brief Destructor.
   */
  ~QmitkScreenshotQuestionWidget() override;

  /** \brief Create a new QmitkScreenshotQuestionWidget instance.
   *
   * \param[in] parent The parent widget.
   * \return A pointer to the new widget. The caller takes ownership.
   */
  QmitkQuestionWidget* CreateAnother(QWidget* parent = nullptr) const override;

  /** \brief Get the associated ScreenshotQuestion.
   *
   * \return A pointer to the ScreenshotQuestion, or \c nullptr if not set.
   */
  mitk::Forms::Question* GetQuestion() const override;

  /** \brief Initialize the widget for a ScreenshotQuestion.
   *
   * \param[in] question Must be a ScreenshotQuestion. The widget does not take ownership.
   * \throw mitk::Exception If \p question is not a ScreenshotQuestion.
   */
  void SetQuestion(mitk::Forms::Question* question) override;

  /** \brief Remove all screenshot widgets and clear all responses.
   */
  void Reset() override;

private:
  void OnTakeScreenshotButtonClicked();
  void OnPopupMenuTriggered(const QString& action);
  void AddScreenshotWidget(const QString& screenshotFileName);
  void OnRemoveScreenshot(QmitkScreenshotWidget* sender);

  mitk::Forms::ScreenshotQuestion* m_Question;

  QVBoxLayout* m_Layout;
  QPushButton* m_TakeScreenshotButton;
  QMenu* m_PopupMenu;
};

#endif
