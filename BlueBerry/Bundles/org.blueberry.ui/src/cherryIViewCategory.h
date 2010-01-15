/*=========================================================================

Program:   openCherry Platform
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

#ifndef CHERRYIVIEWCATEGORY_H_
#define CHERRYIVIEWCATEGORY_H_

#include "cherryIViewDescriptor.h"

#include <cherryMacros.h>

#include <vector>

namespace cherry {

/**
 * \ingroup org_opencherry_ui
 *
 */
struct CHERRY_UI IViewCategory : public Object
{
  cherryInterfaceMacro(IViewCategory, cherry)

  virtual const std::string& GetId() const = 0;
  virtual std::string GetLabel() const = 0;
  virtual std::vector<std::string> GetPath() const = 0;
  virtual const std::vector<IViewDescriptor::Pointer>& GetViews() const = 0;

  virtual ~IViewCategory() {}
};

}

#endif /*CHERRYIVIEWCATEGORY_H_*/
