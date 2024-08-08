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
    class MITKFORMSUI_EXPORT IQuestionWidgetFactory
    {
    public:
      static IQuestionWidgetFactory* GetInstance();

      virtual ~IQuestionWidgetFactory();

      virtual void Register(const std::string& questionType, QmitkQuestionWidget* widgetPrototype) = 0;
      virtual QmitkQuestionWidget* Create(Question* question, QWidget* parent = nullptr) const = 0;
    };
  }
}

MITK_DECLARE_SERVICE_INTERFACE(mitk::Forms::UI::IQuestionWidgetFactory, "org.mitk.Forms.UI.IQuestionWidgetFactory")

#endif
