/*=========================================================================

Program:   BlueBerry Platform
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

#ifndef BERRYIVIEWCATEGORY_H_
#define BERRYIVIEWCATEGORY_H_

#include "berryIViewDescriptor.h"

#include <berryMacros.h>

#include <vector>

namespace berry {

/**
 * \ingroup org_blueberry_ui
 *
 */
struct BERRY_UI IViewCategory : public Object
{
  berryInterfaceMacro(IViewCategory, berry)

  virtual const std::string& GetId() const = 0;
  virtual std::string GetLabel() const = 0;
  virtual std::vector<std::string> GetPath() const = 0;
  virtual const std::vector<IViewDescriptor::Pointer>& GetViews() const = 0;

  virtual ~IViewCategory() {}
};

}

#endif /*BERRYIVIEWCATEGORY_H_*/
