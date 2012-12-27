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


#ifndef GROUPTAGPROPERTY_H_HEADER_INCLUDED_C1F4DF54
#define GROUPTAGPROPERTY_H_HEADER_INCLUDED_C1F4DF54

#include <mitkBaseProperty.h>

namespace mitk {

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable: 4522)
#endif

/*! @brief Property class that has no value.

  @ingroup DataManagement

    The GroupTag property is used to tag a datatree node to show, that it is member of a
    group of datatree nodes. This can be used to build groups of datatreenodes without the
    need to contain them in a specific hiearchic order in the datatree
*/
class MITK_CORE_EXPORT GroupTagProperty : public BaseProperty
{
  public:
    mitkClassMacro(GroupTagProperty, BaseProperty);
    itkNewMacro(GroupTagProperty);

    using BaseProperty::operator=;

  protected:
    GroupTagProperty();

  private:

    // purposely not implemented
    GroupTagProperty(const GroupTagProperty&);
    GroupTagProperty& operator=(const GroupTagProperty&);

    virtual bool IsEqual(const BaseProperty& property) const;
    virtual bool Assign(const BaseProperty& property);
};

#ifdef _MSC_VER
# pragma warning(pop)
#endif

} // namespace mitk



#endif /* GROUPTAGPROPERTY_H_HEADER_INCLUDED_C1F4DF54 */
