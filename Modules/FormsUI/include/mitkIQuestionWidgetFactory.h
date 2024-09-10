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
       */
      static IQuestionWidgetFactory* GetInstance();

      virtual ~IQuestionWidgetFactory();

      /** \brief Register a QmitkQuestionWidget subclass for a certain Question type string.
       *
       * The service takes over ownership of the passed QmitkQuestionWidget pointer.
       *
       * \sa Question::GetType()
       */
      virtual void Register(const std::string& questionType, QmitkQuestionWidget* widgetPrototype) = 0;

      /** \brief Create an instance of a matching QmitkQuestionWidget subclass for a certain question.
       *
       * The given question is passed to QmitkQuestionWidget::SetQuestion().
       *
       * \sa QmitkQuestionWidget::CreateAnother()
       */
      virtual QmitkQuestionWidget* Create(Question* question, QWidget* parent = nullptr) const = 0;
    };
  }
}

MITK_DECLARE_SERVICE_INTERFACE(mitk::Forms::UI::IQuestionWidgetFactory, "org.mitk.Forms.UI.IQuestionWidgetFactory")

#endif
