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

#ifndef MITKCOREEXTCONSTANTS_H_
#define MITKCOREEXTCONSTANTS_H_

#include <string>

#include <org_mitk_core_ext_Export.h>

namespace mitk 
{
  /**
  * The CoreExt Constants contains a list of unique ids in the following form: <br>
  * "org.mitk.mybundle.mytype.propername" <br>
  * 
  * This ids have the purpose of connecting the plugin.xml of each bundle to the 
  * appropriate classes. 
  *
  * Additionally it includes names of tags, which are used in XML files <br>
  * regarding any input device.
  *
  * @ingroup org_mitk_core_ext
  */
  struct MITKCOREEXT_EXPORT CoreExtConstants
  {
    static const std::string INPUTDEVICE_SERVICE;
    static const std::string INPUTDEVICE_PREFERENCES;
    static const std::string INPUTDEVICE_EXTENSION_NAME;
    static const std::string INPUTDEVICE_XMLATTRIBUTE_CLASS;
    static const std::string INPUTDEVICE_XMLATTRIBUTE_DESCRIPTION;
    static const std::string INPUTDEVICE_XMLATTRIBUTE_NAME;
    static const std::string INPUTDEVICE_XMLATTRIBUTE_ID;
    static const std::string WIIMOTE_SURFACEINTERACTION;
    static const std::string WIIMOTE_HEADTRACKING;
    static const std::string WIIMOTE_XMLATTRIBUTE_NAME;
  };
}

#endif /*MITKCOREEXTCONSTANTS_H_*/
