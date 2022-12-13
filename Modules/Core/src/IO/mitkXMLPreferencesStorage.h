/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkXMLPreferencesStorage_h
#define mitkXMLPreferencesStorage_h

#include <mitkIPreferencesStorage.h>

namespace mitk
{
  /**
   * \brief See IPreferencesStorage.
   */
  class XMLPreferencesStorage : public IPreferencesStorage
  {
  public:
    explicit XMLPreferencesStorage(const std::filesystem::path& filename);
    ~XMLPreferencesStorage() override;

    void Flush() override;
  };
}

#endif
