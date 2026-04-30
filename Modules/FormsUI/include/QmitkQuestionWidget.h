/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkQuestionWidget_h
#define QmitkQuestionWidget_h

#include <MitkFormsUIExports.h>
#include <QFrame>

class QVBoxLayout;
class QLabel;

namespace mitk::Forms
{
  class Question;
}

/** \brief Abstract base class for all types of question widgets used in a QmitkForm.
 *
 * This class manages GUI elements common to all questions like labels for the question text and
 * a reminder that a response might be required. All specific GUI elements of derived classes
 * must be put into a layout and inserted by calling InsertLayout().
 *
 * Please make sure to read the full documentation of the pure virtual functions in particular to
 * fully understand implications and requirements.
 */
class MITKFORMSUI_EXPORT QmitkQuestionWidget : public QFrame
{
  Q_OBJECT

public:
  /** \brief Construct the question widget with common GUI elements.
   *
   * Sets up the question text label and the requirement reminder label (hidden by default).
   *
   * \param[in] parent The parent widget.
   */
  explicit QmitkQuestionWidget(QWidget* parent = nullptr);

  /** \brief Destructor.
   */
  ~QmitkQuestionWidget() override;

  /** \name Pure virtual functions
   *
   * QmitkQuestionWidget is an abstract base class. Derive from this class to add a widget for a
   * certain type of Question and override the following pure virtual functions. Please read the
   * full documentation for all of these functions to fully understand implications and requirements.
   *
   * Do not forget to register any new question widget by calling mitk::Forms::UI::IQuestionWidgetFactory::Register()
   * like it is done in this module's activator class.
   *
   * \sa mitk::Forms::UI::IQuestionWidgetFactory
   * \{
   */

  /** \brief Create a new instance of the derived question widget class type.
   *
   * This method is mainly used by mitk::Forms::UI::IQuestionWidgetFactory to create
   * new instances from registered prototype instances.
   *
   * \code{.cpp}
   * QmitkQuestionWidget* QmitkRhetoricalQuestionWidget::CreateAnother(QWidget* parent) const
   * {
   *   return new RhetoricalQuestionWidget(parent);
   * }
   * \endcode
   *
   * \param[in] parent The parent widget for the new instance.
   * \return A pointer to the new widget. The caller takes ownership.
   */
  virtual QmitkQuestionWidget* CreateAnother(QWidget* parent = nullptr) const = 0;

  /** \brief Get the question associated with this widget.
   *
   * \code{.cpp}
   * Question* QmitkRhetoricalQuestionWidget::GetQuestion() const
   * {
   *   // In class declaration: mitk::Forms::RhetoricalQuestion* m_Question;
   *   return m_Question;
   * }
   * \endcode
   *
   * \return A pointer to the associated Question, or \c nullptr if none is set.
   *
   * \sa SetQuestion()
   */
  virtual mitk::Forms::Question* GetQuestion() const = 0;

  /** \brief Initialize the widget based on the given question.
   *
   * This method is rarely used explicitly since it is automatically called by
   * mitk::Forms::UI::IQuestionWidgetFactory::Create().
   *
   * You are excepted to throw an mitk::Exception if the Question type does not match
   * the expectations of the widget.
   *
   * \note It is required to call this base class method at the beginning of the derived
   * method.
   *
   * \code{.cpp}
   * void QmitkRhetoricalQuestionWidget::SetQuestion(Question* question)
   * {
   *   QmitkQuestionWidget::SetQuestion(question);
   *
   *   auto rhetoricalQuestion = dynamic_cast<RhetoricalQuestion*>(question);
   *
   *   if (rhetoricalQuestion == nullptr)
   *     mitkThrow() << "QmitkRhetoricalQuestionWidget only accepts RhetoricalQuestion as question type!";
   *
   *   m_Question = rhetoricalQuestion;
   * }
   * \endcode
   *
   * \param[in] question The question to associate with this widget. Must not be \c nullptr.
   *
   * \throw mitk::Exception If \p question is \c nullptr or of an incompatible type.
   */
  virtual void SetQuestion(mitk::Forms::Question* question) = 0;

  /** \brief Reset the state of the GUI as if no interaction would have been happened yet.
   *
   * \note It is required to call this base class method at the end of the derived method.
   *
   * \code{.cpp}
   * void QmitkRhetoricalQuestionWidget::Reset()
   * {
   *   // It's a rhetorical question... nothing specific to be reset.
   *   QmitkQuestionWidget::Reset(); // Nevertheless, this is required at the end!
   * }
   * \endcode
   */
  virtual void Reset() = 0;

  /**\}*/

  /** \brief Show or hide the requirement reminder.
   *
   * \param[in] visible If \c true, show the requirement indicator; if \c false, hide it.
   */
  void SetRequirementVisible(bool visible);

  /** \brief Show the requirement reminder label and highlight the widget border in red.
   */
  void ShowRequirement();

  /** \brief Hide the requirement reminder label and reset the widget border style.
   */
  void HideRequirement();

protected:
  /** \brief Insert a layout containing all GUI elements specific to the derived question widget type.
   *
   * This method is typically called from the constructor of a derived class after all GUI elements
   * have been set up and organized in a layout.
   *
   * \code{.cpp}
   * QmitkRhetoricalQuestionWidget::QmitkRhetoricalQuestionWidget(QWidget* parent)
   *   : QmitkQuestionWidget(parent),
   *     m_Question(nullptr),
   *     m_Layout(new QVBoxLayout),
   *     m_Label(new QLabel)
   * {
   *   m_Label->setText("You know the answer... we all do.");
   *   m_Layout->addWidget(m_Label);
   *
   *   this->InsertLayout(m_Layout);
   * }
   * \endcode
   *
   * \param[in] layout The layout to insert between the question label and the requirement label.
   */
  void InsertLayout(QLayout* layout);

private:
  QVBoxLayout* m_Layout;
  QLabel* m_QuestionLabel;
  QLabel* m_RequiredLabel;
};

#endif
