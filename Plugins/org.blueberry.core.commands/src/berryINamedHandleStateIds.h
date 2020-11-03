/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef BERRYINAMEDHANDLESTATEIDS_H_
#define BERRYINAMEDHANDLESTATEIDS_H_

#include <QString>

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
  static const QString DESCRIPTION; // = "DESCRIPTION";

  /**
   * The state id used for overriding the name of a named handle object. This
   * state's value must return a {@link String}.
   */
  static const QString NAME; // = "NAME";
};

}

#endif /* BERRYINAMEDHANDLESTATEIDS_H_ */
