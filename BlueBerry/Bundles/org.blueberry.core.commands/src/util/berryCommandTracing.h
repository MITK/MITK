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


#ifndef BERRYCOMMANDTRACING_H_
#define BERRYCOMMANDTRACING_H_

#include <string>

namespace berry {

/**
 * <p>
 * A utility class for printing tracing output to the console.
 * </p>
 * <p>
 * Clients must not extend or instantiate this class.
 * </p>
 *
 * @since 3.2
 */
class CommandTracing {

public:

  /**
   * The separator to place between the component and the message.
   */
  static const std::string SEPARATOR; // = " >>> ";

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
  static void PrintTrace(const std::string& component,
      const std::string& message);

private:

  /**
   * This class is not intended to be instantiated.
   */
  CommandTracing();
};

}

#endif /* BERRYCOMMANDTRACING_H_ */
