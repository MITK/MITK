/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkIdentifiable_h
#define mitkIdentifiable_h

#include <string>
#include <MitkCoreExports.h>

namespace mitk
{
  /**
   * \brief Base class of identifiable objects.
   *
   * If you want to change the unique ID after creation time, you can use
   * the mitk::UIDManipulator class. The reationale behind this pattern is
   * to ensure that you think twice before doing this. It is intended to be
   * used by data readers if necessary at all.
   */
  class MITKCORE_EXPORT Identifiable
  {
  public:
    using UIDType = std::string;

    Identifiable();
    explicit Identifiable(const UIDType &uid);
    Identifiable(const Identifiable &) = delete;
    Identifiable(Identifiable &&) noexcept;
    virtual ~Identifiable();

    Identifiable & operator =(const Identifiable &) = delete;
    Identifiable & operator =(Identifiable &&other) noexcept;

    /**
     * \brief Get unique ID of an object.
     *
     * \return Empty string if an object has no unique ID.
     */
    UIDType GetUID() const;

  private:
    friend class UIDManipulator;

    void SetUID(const UIDType &uid);

    struct Impl;
    Impl *m_Impl;
  };
}

#endif
