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


#ifndef BERRYIRUNTIMECONSTANTS_H_
#define BERRYIRUNTIMECONSTANTS_H_

#include <string>

namespace berry {

struct IRuntimeConstants {

  /**
   * The unique identifier constant (value "<code>org.blueberry.core.runtime</code>")
   * of the Core Runtime (pseudo-) plug-in.
   */
  static const std::string& PI_RUNTIME(); // = "org.blueberry.core.runtime";

  /**
   * Name of this bundle.
   */
  static const std::string& PI_COMMON(); // = "org.blueberry.solstice.common";

  /**
   * Status code constant (value 2) indicating an error occurred while running a plug-in.
   */
  static const int PLUGIN_ERROR; // = 2;

  /**
   * Status code constant (value 5) indicating the platform could not write
   * some of its metadata.
   */
  static const int FAILED_WRITE_METADATA; // = 5;

};

}

#endif /* BERRYIRUNTIMECONSTANTS_H_ */
