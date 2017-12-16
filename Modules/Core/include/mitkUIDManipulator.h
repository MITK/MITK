/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

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
