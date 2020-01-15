/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkUIDManipulator_h
#define mitkUIDManipulator_h

#include <mitkIdentifiable.h>
#include <MitkCoreExports.h>

namespace mitk
{
  class Identifiable;

  /**
   * \brief Change the unique ID of an mitk::Identifiable.
   *
   * Think twice before doing this. Intended for data readers.
   */
  class MITKCORE_EXPORT UIDManipulator final
  {
  public:
    explicit UIDManipulator(Identifiable *identifiable);
    ~UIDManipulator();

    /**
     * \brief Set unique ID of an identifiable object.
     */
    void SetUID(const Identifiable::UIDType &uid);

  private:
    Identifiable *m_Identifiable;
  };
}

#endif
