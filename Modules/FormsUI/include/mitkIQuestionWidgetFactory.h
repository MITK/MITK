/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkIQuestionWidgetFactory_h
#define mitkIQuestionWidgetFactory_h

#include <mitkServiceInterface.h>
#include <MitkFormsUIExports.h>

#include <QmitkQuestionWidget.h>

namespace mitk::Forms
{
  class Question;

  namespace UI
  {
    /** \brief Register widgets for questions.
     *
     * This is a service interface. Obtain a pointer to its single instance via GetInstance().
     *
     * Each QmitkQuestionWidget subclass must be registered by calling Register(), which is typically done
     * in the module activator class. After QmitkQuestionWidget subclasses are registered, instances of them
     * can be created with Create() based on their matching type of Question.
     */
    class MITKFORMSUI_EXPORT IQuestionWidgetFactory
    {
    public:
      /** \brief Obtain a pointer to the single instance of this service.
       *
       * \return A pointer to the IQuestionWidgetFactory service, or \c nullptr if unavailable.
       */
      static IQuestionWidgetFactory* GetInstance();

      /** \brief Virtual destructor.
       */
      virtual ~IQuestionWidgetFactory();

      /** \brief Register a QmitkQuestionWidget subclass for a certain Question type string.
       *
       * The service takes over ownership of the passed QmitkQuestionWidget pointer.
       *
       * \param[in] questionType The type string that identifies the Question subclass (as returned
       *            by Question::GetType()).
       * \param[in] widgetPrototype A prototype widget instance. Ownership is transferred to the service.
       *
       * \sa Question::GetType()
       */
      virtual void Register(const std::string& questionType, QmitkQuestionWidget* widgetPrototype) = 0;

      /** \brief Create an instance of a matching QmitkQuestionWidget subclass for a certain question.
       *
       * The widget is created via QmitkQuestionWidget::CreateAnother() and the given question
       * is passed to QmitkQuestionWidget::SetQuestion().
       *
       * \param[in] question The question for which to create a widget. The widget does not take ownership.
       * \param[in] parent The parent widget for the new widget.
       * \return A pointer to the new widget. The caller takes ownership.
       *
       * \sa QmitkQuestionWidget::CreateAnother(), QmitkQuestionWidget::SetQuestion()
       */
      virtual QmitkQuestionWidget* Create(Question* question, QWidget* parent = nullptr) const = 0;
    };
  }
}

MITK_DECLARE_SERVICE_INTERFACE(mitk::Forms::UI::IQuestionWidgetFactory, "org.mitk.Forms.UI.IQuestionWidgetFactory")

#endif
