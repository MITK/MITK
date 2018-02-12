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

#include "mitkTrackingTool.h"

#include "mitkTestingMacros.h"

#include <iostream>
#include <time.h>

/**Documentation
* NDIPassiveTool has a protected constructor and a protected itkFactorylessNewMacro
* so that only it's friend class NDITrackingDevice is able to instantiate
* tool objects. Therefore, we derive from NDIPassiveTool and add a
* public itkFactorylessNewMacro, so that we can instantiate and test the class
*/
class InternalTrackingToolTestClass : public mitk::TrackingTool
{
public:
  mitkClassMacro(InternalTrackingToolTestClass, TrackingTool);
  /** make a public constructor, so that the test is able
  *   to instantiate NDIPassiveTool
  */
  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)
protected:
  InternalTrackingToolTestClass() : mitk::TrackingTool()
  {
  }


};

/**
 *  Simple example for a test for the class "TrackingTool".
 *
 *  argc and argv are the command line parameters which were passed to
 *  the ADD_TEST command in the CMakeLists.txt file. For the automatic
 *  tests, argv is either empty for the simple tests or contains the filename
 *  of a test image for the image tests (see CMakeLists.txt).
 */
int mitkInternalTrackingToolTest(int /* argc */, char* /*argv*/[])
{
  // always start with this!
  MITK_TEST_BEGIN("TrackingTool")

  // always end with this!
  MITK_TEST_END();
}

