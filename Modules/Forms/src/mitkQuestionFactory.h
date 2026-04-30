/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkQuestionFactory_h
#define mitkQuestionFactory_h

#include <mitkIQuestionFactory.h>

#include <memory>
#include <string>
#include <unordered_map>

namespace mitk::Forms
{
  /**
   * \brief Default implementation of IQuestionFactory using prototype-based creation.
   *
   * Stores question prototypes keyed by their type string and creates
   * new instances by cloning the matching prototype.
   */
  class QuestionFactory : public IQuestionFactory
  {
  public:
    QuestionFactory();
    ~QuestionFactory() override;

    /**
     * \brief Register a question prototype. Takes ownership of the pointer.
     * \param question The prototype question to register. Its type string is used as the key.
     */
    void Register(Question* question) override;

    /**
     * \brief Create a new question instance by cloning the registered prototype for the given type.
     * \param type The question type string.
     * \return A new Question instance, or nullptr if no prototype is registered for this type.
     */
    Question* Create(const std::string& type) const override;

  private:
    std::unordered_map<std::string, std::unique_ptr<Question>> m_Prototypes;
  };
}

#endif
