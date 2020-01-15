/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef BERRYCOMMANDTRACING_H_
#define BERRYCOMMANDTRACING_H_

#include <QString>

#include <org_blueberry_core_commands_Export.h>

namespace berry {

/**
 * <p>
 * A utility class for printing tracing output to the console.
 * </p>
 * <p>
 * Clients must not extend or instantiate this class.
 * </p>
 */
class BERRY_COMMANDS CommandTracing
{

public:

  /**
   * The separator to place between the component and the message.
   */
  static const QString SEPARATOR; // = " >>> ";

  /**
   * <p>
   * Prints a tracing message to standard out. The message is prefixed by a
   * component identifier and some separator. See the example below.
   * </p>
   *
   * <pre>
   *        BINDINGS &gt;&gt; There are 4 deletion markers
   * </pre>
   *
   * @param component
   *            The component for which this tracing applies; may be
   *            empty
   * @param message
   *            The message to print to standard out; may be empty
   */
  static void PrintTrace(const QString& component,
                         const QString& message);

private:

  /**
   * This class is not intended to be instantiated.
   */
  CommandTracing();
};

}

#endif /* BERRYCOMMANDTRACING_H_ */
