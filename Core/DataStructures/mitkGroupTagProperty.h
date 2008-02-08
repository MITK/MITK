/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef GROUPTAGPROPERTY_H_HEADER_INCLUDED_C1F4DF54
#define GROUPTAGPROPERTY_H_HEADER_INCLUDED_C1F4DF54

#include <mitkBaseProperty.h>

namespace mitk {

/*! @brief Property class that has no value. 
    
  @ingroup DataTree

    The GroupTag property is used to tag a datatree node to show, that it is member of a 
    group of datatree nodes. This can be used to build groups of datatreenodes without the
    need to contain them in a specific hiearchic order in the datatree
*/
class MITK_CORE_EXPORT GroupTagProperty : public BaseProperty
{
  public:
    GroupTagProperty();
    virtual ~GroupTagProperty();

    mitkClassMacro(GroupTagProperty, BaseProperty);

    virtual bool operator==(const BaseProperty& property) const;
    virtual bool Assignable(const BaseProperty& other) const;
};

} // namespace mitk



#endif /* GROUPTAGPROPERTY_H_HEADER_INCLUDED_C1F4DF54 */
