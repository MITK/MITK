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

  class MITKFORMS_EXPORT IQuestionFactory
  {
  public:
    static IQuestionFactory* GetInstance();

    virtual ~IQuestionFactory();

    virtual void Register(Question* question) = 0;
    virtual Question* Create(const std::string& type) const = 0;
  };
}

MITK_DECLARE_SERVICE_INTERFACE(mitk::Forms::IQuestionFactory, "org.mitk.Forms.IQuestionFactory")

#endif
