/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkIQuestionFactory_h
#define mitkIQuestionFactory_h

#include <mitkServiceInterface.h>
#include <MitkFormsExports.h>

#include <string>

namespace mitk::Forms
{
  class Question;

  /** \brief Register subclasses of Question to create instances of them based on their type string.
   *
   * This is a service interface. Obtain a pointer to its single instance via GetInstance().
   *
   * Each Question subclass must be registered by calling Register(), which is typically done
   * in the module activator class. After Question subclasses are registered, instances of them
   * can be created with Create() based on their type string. This mechanism is primarily used
   * for the deserialization of questions.
   */
  class MITKFORMS_EXPORT IQuestionFactory
  {
  public:
    /** \brief Obtain a pointer to the single instance of this service.
     */
    static IQuestionFactory* GetInstance();

    virtual ~IQuestionFactory();

    /** \brief Register a Question subclass for the instance creation based on its type string.
     *
     * The service takes over ownership of the passed Question pointer.
     *
     * \sa Question::GetType()
     */
    virtual void Register(Question* question) = 0;

    /** \brief Create an instance of a Question subclass based on its type string.
     *
     * \sa Question::GetType(), Question::CreateAnother()
     */
    virtual Question* Create(const std::string& type) const = 0;
  };
}

MITK_DECLARE_SERVICE_INTERFACE(mitk::Forms::IQuestionFactory, "org.mitk.Forms.IQuestionFactory")

#endif
