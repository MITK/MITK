/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkModuleActivator_h
#define mitkModuleActivator_h

#include <mitkIQuestionFactory.h>
#include <usModuleActivator.h>
#include <memory>

namespace mitk::Forms
{
  /**
   * \brief Module activator for the Forms module.
   *
   * Registers all built-in question types with the IQuestionFactory service
   * when the module is loaded.
   */
  class ModuleActivator : public us::ModuleActivator
  {
  public:
    ModuleActivator();
    ~ModuleActivator() override;

    /** \brief Register all built-in question types with the question factory. */
    void Load(us::ModuleContext* context) override;

    /** \brief Unregister the question factory service. */
    void Unload(us::ModuleContext* context) override;

  private:
    template <class TQuestion>
    void RegisterQuestion()
    {
      m_QuestionFactory->Register(new TQuestion);
    }

    std::unique_ptr<IQuestionFactory> m_QuestionFactory;
  };
}

#endif
