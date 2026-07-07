/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkModuleActivator_h
#define mitkModuleActivator_h

#include <mitkIQuestionWidgetFactory.h>
#include <usModuleActivator.h>
#include <memory>

namespace mitk::Forms::UI
{
  /**
   * \brief Module activator for the FormsUI module.
   *
   * Registers all built-in question widget types with the IQuestionWidgetFactory
   * service when the module is loaded.
   */
  class ModuleActivator : public us::ModuleActivator
  {
  public:
    ModuleActivator();
    ~ModuleActivator() override;

    /** \brief Register all built-in question widget types with the widget factory. */
    void Load(us::ModuleContext* context) override;

    /** \brief Unregister the question widget factory service. */
    void Unload(us::ModuleContext* context) override;

  private:
    template <class TQuestion, class TWidget>
    void RegisterQuestionWidget()
    {
      auto question = std::make_unique<TQuestion>();
      m_QuestionWidgetFactory->Register(question->GetType(), new TWidget);
    }

    std::unique_ptr<IQuestionWidgetFactory> m_QuestionWidgetFactory;
  };
}

#endif
