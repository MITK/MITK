/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkIPropertyTransience_h
#define mitkIPropertyTransience_h

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
   * \brief Interface of the property transience service.
   *
   * This service manages which DataNode properties are transient, i.e. hold
   * runtime or UI state (like the node "selected" flag) that must not be
   * persisted to scene files. Although such properties live on the DataNode,
   * transience is declared per BaseData type: a rule registered for a type
   * applies to every node whose data is an instance of that type (or a
   * subclass). Register against mitk::BaseData to cover all data types.
   *
   * At serialization time the scene serializer queries IsTransient() with the
   * node's data to decide whether a property is excluded.
   */
  class MITKCORE_EXPORT IPropertyTransience
  {
  public:
    virtual ~IPropertyTransience();

    /** \brief Declare a property transient for nodes whose data is-a TData.
     *
     * \tparam TData BaseData or a subclass the rule applies to. Use
     *         mitk::BaseData to mark the property transient for any data type.
     * \param[in] propertyName Name of the transient property.
     */
    template <class TData, typename = std::enable_if_t<std::is_base_of_v<BaseData, TData>>>
    void AddTransient(const std::string &propertyName)
    {
      this->InternalAddTransient(propertyName,
                                 [](const BaseData *data) { return dynamic_cast<const TData *>(data) != nullptr; });
    }

    /** \brief Check whether a property is transient for the given node data.
     *
     * \param[in] data The node's BaseData. May be null, in which case no rule
     *            matches.
     * \param[in] propertyName Name of the property to check.
     * \return True if any registered rule for propertyName matches the data type.
     */
    virtual bool IsTransient(const BaseData *data, const std::string &propertyName) const = 0;

  protected:
    /** \brief Register a transient property with a data-type matcher.
     *
     * Called by the public AddTransient() template.
     *
     * \param[in] propertyName Name of the transient property.
     * \param[in] dataTypeMatcher Predicate returning true if the rule applies to
     *            the passed BaseData.
     */
    virtual void InternalAddTransient(const std::string &propertyName,
                                      std::function<bool(const BaseData *)> dataTypeMatcher) = 0;
  };
}

MITK_DECLARE_SERVICE_INTERFACE(mitk::IPropertyTransience, "org.mitk.IPropertyTransience")

#endif
