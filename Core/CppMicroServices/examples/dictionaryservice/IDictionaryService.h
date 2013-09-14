/*=============================================================================

  Library: CppMicroServices

  Copyright (c) German Cancer Research Center,
    Division of Medical and Biological Informatics

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

=============================================================================*/

#ifndef IDICTIONARYSERVICE_H
#define IDICTIONARYSERVICE_H

//! [service]
#include <usServiceInterface.h>

#include <string>

#ifdef Example_dictionaryservice_EXPORTS
  #define DICTIONAYSERVICE_EXPORT US_ABI_EXPORT
#else
  #define DICTIONAYSERVICE_EXPORT US_ABI_IMPORT
#endif

/**
 * A simple service interface that defines a dictionary service.
 * A dictionary service simply verifies the existence of a word.
 **/
struct DICTIONAYSERVICE_EXPORT IDictionaryService
{
  // Out-of-line virtual desctructor for proper dynamic cast
  // support with older versions of gcc.
  virtual ~IDictionaryService();

  /**
   * Check for the existence of a word.
   * @param word the word to be checked.
   * @return true if the word is in the dictionary,
   *         false otherwise.
   **/
  virtual bool CheckWord(const std::string& word) = 0;
};

US_DECLARE_SERVICE_INTERFACE(IDictionaryService, "IDictionaryService/1.0")
//! [service]

#endif // DICTIONARYSERVICE_H
