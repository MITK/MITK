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
   * Provides a universally unique identifier (UID) for each instance. Other
   * classes can inherit from Identifiable to gain this capability.
   *
   * The UID is unique at creation time but is not content-dependent:
   * - An instance may change its data over its lifetime and still keep the same UID.
   * - If an instance is persisted and loaded multiple times, several in-memory
   *   objects may share the same UID.
   * - UIDs are analogous to git paths (identifying an entity whose state may
   *   change) rather than git hashes. They abstract from memory addresses,
   *   enabling serialization.
   * - It is up to the application to ensure appropriate UID usage within its scope.
   *
   * \note Identifiable itself is not inherently persistable. Derived classes decide
   *       if and how to persist their UID.
   * \note To change the UID after construction, use mitk::UIDManipulator. This
   *       indirection exists to discourage casual UID modification; it is intended
   *       for data readers only.
   *
   * \sa UIDGenerator, UIDManipulator
   * \ingroup Core
   */
  class MITKCORE_EXPORT Identifiable
  {
  public:
    using UIDType = std::string; ///< Type alias for unique identifiers.

    /** \brief Default constructor. Generates a new random UID. */
    Identifiable();

    /**
     * \brief Construct with a pre-existing UID.
     * \param[in] uid The unique identifier to assign.
     */
    explicit Identifiable(const UIDType &uid);

    /** \brief Copy construction is deleted to prevent UID duplication. */
    Identifiable(const Identifiable &) = delete;

    /** \brief Move constructor. Transfers ownership of the UID. */
    Identifiable(Identifiable &&) noexcept;

    /** \brief Virtual destructor. */
    virtual ~Identifiable();

    /** \brief Copy assignment is deleted to prevent UID duplication. */
    Identifiable & operator =(const Identifiable &) = delete;

    /**
     * \brief Move assignment operator. Transfers ownership of the UID.
     * \param[in] other The source object (left in a valid but unspecified state).
     * \return Reference to this object.
     */
    Identifiable & operator =(Identifiable &&other) noexcept;

    /**
     * \brief Get the unique identifier of this object.
     * \return The UID string. May be empty if the object has no UID.
     */
    virtual UIDType GetUID() const;

    /**
     * \brief Get a runtime-unique ID for this object instance.
     *
     * Unlike GetUID(), this ID is never persisted and never manipulated by
     * readers. It is guaranteed unique for every distinct in-memory instance,
     * derived from the object's memory address at construction time.
     *
     * Thread-safe: written once in the constructor, read-only thereafter.
     *
     * \note Do not rely on this value after a move operation -- the moved-to
     *       object inherits the original address.
     *
     * \return A string representation of the construction-time memory address.
     */
    UIDType GetRuntimeUID() const noexcept;

  protected:
    virtual void SetUID(const UIDType& uid);

  private:
    friend class UIDManipulator;

    struct Impl;
    Impl *m_Impl;
  };
}

#endif
