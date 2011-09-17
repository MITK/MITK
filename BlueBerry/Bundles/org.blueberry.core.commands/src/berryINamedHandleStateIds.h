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


#ifndef BERRYINAMEDHANDLESTATEIDS_H_
#define BERRYINAMEDHANDLESTATEIDS_H_

#include <string>

namespace berry {

/**
 * <p>
 * State identifiers that are understood by named handle objects that implement
 * {@link IObjectWithState}.
 * </p>
 * <p>
 * Clients may implement or extend this class.
 * </p>
 *
 */
struct INamedHandleStateIds {

  /**
   * The state id used for overriding the description of a named handle
   * object. This state's value must return a {@link String}.
   */
  static const std::string DESCRIPTION; // = "DESCRIPTION";

  /**
   * The state id used for overriding the name of a named handle object. This
   * state's value must return a {@link String}.
   */
  static const std::string NAME; // = "NAME";
};

}

#endif /* BERRYINAMEDHANDLESTATEIDS_H_ */
