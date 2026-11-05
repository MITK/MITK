/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkIPropertyMigration_h
#define mitkIPropertyMigration_h

#include <mitkBaseData.h>
#include <mitkServiceInterface.h>

#include <MitkCoreExports.h>

#include <functional>
#include <string>
#include <type_traits>

namespace mitk
{
  /**
   * \ingroup MicroServices_Interfaces
   * \brief Interface of the property migration service.
   *
   * This service upgrades properties read from files written by older versions
   * of MITK, for example after a mapper renamed, retyped or dropped a property.
   * A migration is registered for a legacy property key and a BaseData type. It
   * applies to every property list that contains the key and belongs to a node
   * whose data is an instance of that type (or a subclass). Register against
   * mitk::BaseData to cover all data types, including data-less nodes
   * (data == nullptr).
   *
   * Scene readers migrate every property list they read: the default and the
   * render-window-specific property lists of a node as well as the property
   * list of its data, before transient properties are stripped.
   */
  class MITKCORE_EXPORT IPropertyMigration
  {
  public:
    /**
     * \brief Moves a legacy property into its replacement.
     *
     * The function receives the node's data (may be null), the legacy property
     * and the property list it was read into, and may add or change properties
     * of that list, e.g. write a converted value under a new key. The legacy
     * property is removed from the list after the function returned, unless
     * the function changed the value under the legacy key, which converts a
     * value in place.
     */
    using MigrationFunction =
      std::function<void(const BaseData *data, const BaseProperty &legacyProperty, PropertyList &properties)>;

    virtual ~IPropertyMigration();

    /** \brief Register a migration of a legacy property for nodes whose data is-a TData.
     *
     * \tparam TData BaseData or a subclass the migration applies to. Use
     *         mitk::BaseData to apply it to any data type, including data-less
     *         nodes (data == nullptr).
     * \param[in] legacyKey Exact key of the legacy property.
     * \param[in] migration Function that writes the replacement, or an empty
     *            function to remove the legacy property only.
     */
    template <class TData, typename = std::enable_if_t<std::is_base_of_v<BaseData, TData>>>
    void AddMigration(const std::string &legacyKey, MigrationFunction migration)
    {
      if constexpr (std::is_same_v<TData, BaseData>)
      {
        // A rule for BaseData applies to every node, including data-less ones
        // where data is null and a dynamic_cast would fail. Match unconditionally.
        this->InternalAddMigration(legacyKey, [](const BaseData *) { return true; }, std::move(migration));
      }
      else
      {
        this->InternalAddMigration(legacyKey,
                                   [](const BaseData *data) { return dynamic_cast<const TData *>(data) != nullptr; },
                                   std::move(migration));
      }
    }

    /** \brief Register the removal of a legacy property for nodes whose data is-a TData.
     *
     * \tparam TData BaseData or a subclass the removal applies to.
     * \param[in] legacyKey Exact key of the legacy property.
     */
    template <class TData, typename = std::enable_if_t<std::is_base_of_v<BaseData, TData>>>
    void AddRemoval(const std::string &legacyKey)
    {
      this->AddMigration<TData>(legacyKey, MigrationFunction());
    }

    /** \brief Migrate the legacy properties of a property list.
     *
     * Migrations run in the order of their registration. A legacy property
     * removed by a migration is not seen by later ones; one converted in place
     * is passed on to later migrations of the same key.
     *
     * \param[in] data The node's BaseData. May be null (e.g. a data-less node);
     *            migrations registered for mitk::BaseData still apply,
     *            type-specific migrations do not.
     * \param[in,out] properties The property list to migrate.
     */
    virtual void Migrate(const BaseData *data, PropertyList &properties) const = 0;

  protected:
    /** \brief Register a migration with a data-type matcher.
     *
     * Called by the public AddMigration() template.
     *
     * \param[in] legacyKey Exact key of the legacy property.
     * \param[in] dataTypeMatcher Predicate returning true if the migration
     *            applies to the passed BaseData.
     * \param[in] migration Function that writes the replacement, may be empty.
     */
    virtual void InternalAddMigration(const std::string &legacyKey,
                                      std::function<bool(const BaseData *)> dataTypeMatcher,
                                      MigrationFunction migration) = 0;
  };
}

MITK_DECLARE_SERVICE_INTERFACE(mitk::IPropertyMigration, "org.mitk.IPropertyMigration")

#endif
