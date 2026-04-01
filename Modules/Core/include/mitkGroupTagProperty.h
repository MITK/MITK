/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkGroupTagProperty_h
#define mitkGroupTagProperty_h

#include <mitkBaseProperty.h>

namespace mitk
{
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4522)
#endif

  /**
   * \brief Property class that carries no value, used as a tag for group membership.
   *
   * The GroupTag property is used to tag a data tree node to indicate that it is
   * a member of a group of data tree nodes. This enables building groups of nodes
   * without requiring a specific hierarchical order in the data tree. Its mere
   * presence on a node signals group membership.
   *
   * \ingroup DataManagement
   *
   * \sa BaseProperty
   */
  class MITKCORE_EXPORT GroupTagProperty : public BaseProperty
  {
  public:
    mitkClassMacro(GroupTagProperty, BaseProperty);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    /**
     * \brief Serialize the group tag to JSON.
     *
     * Since GroupTagProperty has no value, this writes a null JSON value.
     *
     * \param[out] j The JSON value to write into.
     * \return Always \c true.
     */
    bool ToJSON(nlohmann::json& j) const override;

    /**
     * \brief Deserialize the group tag from JSON.
     *
     * Since GroupTagProperty has no value, this is essentially a no-op.
     *
     * \param[in] j The JSON value to read from.
     * \return Always \c true.
     */
    bool FromJSON(const nlohmann::json& j) override;

    using BaseProperty::operator=;

  protected:
    GroupTagProperty();
    GroupTagProperty(const GroupTagProperty &);

    mitkCloneMacro(GroupTagProperty);

  private:
    // purposely not implemented
    GroupTagProperty &operator=(const GroupTagProperty &);

    bool IsEqual(const BaseProperty &property) const override;
    bool Assign(const BaseProperty &property) override;
  };

#ifdef _MSC_VER
#pragma warning(pop)
#endif

} // namespace mitk

#endif
