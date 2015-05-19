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

#ifndef MITKWORKBENCHCOMMANDCONSTANTS_H
#define MITKWORKBENCHCOMMANDCONSTANTS_H

#include <org_mitk_gui_common_Export.h>

#include <berryIWorkbenchCommandConstants.h>

namespace mitk {

class MITK_GUI_COMMON_PLUGIN WorkbenchCommandConstants : public berry::IWorkbenchCommandConstants
{
public:

  // Project Category

  /**
   * Id for command "Close Project" in category "Project"
   * (value is <code>"org.mitk.ui.project.close"</code>).
   */
  static const QString PROJECT_CLOSE; // = "org.mitk.ui.project.close";

  /**
   * Id for command "Save Project" in category "Project"
   * (value is <code>"org.mitk.ui.project.save"</code>).
   */
  static const QString PROJECT_SAVE; // = "org.mitk.ui.project.save";

};

}


#endif // MITKWORKBENCHCOMMANDCONSTANTS_H
