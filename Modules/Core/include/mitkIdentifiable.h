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
   * Offers an interface to query a UID for the instance. Can be inherited
   * by other classes to provide this capability.\n\n
   * What does the UID stand for/what is its scope?\n
   * - It is unique in its creation, but it is not a content specific unique ID. Therefore:
   *  - A class instance, associated with a UID, may change its values over its lifetime,
        but still have the same UID.
      - If a class instance gets persisted and loaded multiple times, then their could
        be several instances with the same UID.
      - UIDs are therefore more simelar to git paths than to git hashes. They identify something, but the state of
        something can change (stream of commits). The difference of the UID compared to using e.g. plain instance
        pointers to identify an object is that UIDs allow the feature of serialization as they abstract from the
        memory location of the current runtime environment.
     - It is up to the application that builds upon that feature to ensure appropriate usage within
       the application scope.
   * @remark It is not a feature of mitk::Identifiable per se to be persistable. It depends on classes that derive
   * from mitk::Identifiable, if and how they implement a persistence mechanism for their MITK UID.
   * @remark If you want to change the unique ID after creation time, you can use
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
    virtual UIDType GetUID() const;

  protected:
    virtual void SetUID(const UIDType& uid);

  private:
    friend class UIDManipulator;

    struct Impl;
    Impl *m_Impl;
  };
}

#endif
