/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef MITKCOREEXTCONSTANTS_H_
#define MITKCOREEXTCONSTANTS_H_

#include <QString>

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
    static const QString INPUTDEVICE_SERVICE;
    static const QString INPUTDEVICE_PREFERENCES;
    static const QString INPUTDEVICE_EXTENSION_NAME;
    static const QString INPUTDEVICE_XMLATTRIBUTE_CLASS;
    static const QString INPUTDEVICE_XMLATTRIBUTE_DESCRIPTION;
    static const QString INPUTDEVICE_XMLATTRIBUTE_NAME;
    static const QString INPUTDEVICE_XMLATTRIBUTE_ID;
    static const QString WIIMOTE_SURFACEINTERACTION;
    static const QString WIIMOTE_HEADTRACKING;
    static const QString WIIMOTE_XMLATTRIBUTE_NAME;
  };
}

#endif /*MITKCOREEXTCONSTANTS_H_*/
