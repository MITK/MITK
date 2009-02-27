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

#include "mitkNavigationDataDisplacementFilter.h"
#include "mitkNavigationData.h"

#include "mitkTestingMacros.h"

#include <iostream>
#include "mitkPropertyList.h"
#include "mitkProperties.h"

/**Documentation
 *  test for the class "NavigationDataDisplacementFilter".
 */
int mitkNavigationDataDisplacementFilterTest(int /* argc */, char* /*argv*/[])
{
  MITK_TEST_BEGIN("NavigationDataDisplacementFilter")

  // let's create an object of our class  
  mitk::NavigationDataDisplacementFilter::Pointer myFilter = mitk::NavigationDataDisplacementFilter::New();
  
  // first test: did this work?
  // using MITK_TEST_CONDITION_REQUIRED makes the test stop after failure, since
  // it makes no sense to continue without an object.
  MITK_TEST_CONDITION_REQUIRED(myFilter.IsNotNull(),"Testing instantiation");

  /* create helper objects: navigation data with position as origin, zero quaternion, zero error and data valid */
  mitk::NavigationData::PositionType initialPos;
  mitk::FillVector3D(initialPos, 0.0, 0.0, 0.0);
  mitk::NavigationData::OrientationType initialOri(0.0, 0.0, 0.0, 0.0);
  mitk::NavigationData::ErrorType initialError(0.0);
  bool initialValid(true);
  
  mitk::NavigationData::Pointer nd1 = mitk::NavigationData::New();
  nd1->SetPosition(initialPos);
  nd1->SetOrientation(initialOri);
  nd1->SetError(initialError);
  nd1->SetDataValid(initialValid);

  myFilter->SetInput(nd1);
  MITK_TEST_CONDITION(myFilter->GetInput() == nd1, "testing Set-/GetInput()");

  /* create filter parameter */
  mitk::Vector3D offset; 
  mitk::FillVector3D(offset, 1.0, 1.0, 1.0);

  myFilter->SetOffset(offset);
  MITK_TEST_CONDITION(myFilter->GetOffset() == offset, "testing Set-/GetOffset()");

  mitk::NavigationData* output = myFilter->GetOutput();
  MITK_TEST_CONDITION_REQUIRED(output != NULL, "Testing GetOutput()");

  output->Update(); // execute filter

  MITK_TEST_CONDITION(output->GetPosition() == (initialPos + offset), "Testing offset calculation");
  MITK_TEST_CONDITION(output->GetOrientation() == initialOri, "Testing if Orientation remains unchanged");
  MITK_TEST_CONDITION(output->GetError() == initialError, "Testing if Error remains unchanged");
  MITK_TEST_CONDITION(output->IsDataValid() == initialValid, "Testing if DataValid remains unchanged");

  /* now test the filter with multiple inputs */
  mitk::NavigationData::PositionType initialPos2;
  mitk::FillVector3D(initialPos2, -1000.0, 567.89, 0.0);
  mitk::NavigationData::Pointer nd2 = mitk::NavigationData::New();
  nd2->SetPosition(initialPos2);
  nd2->SetOrientation(initialOri);
  nd2->SetError(initialError);
  nd2->SetDataValid(initialValid);

  myFilter = NULL;
  myFilter = mitk::NavigationDataDisplacementFilter::New();
  myFilter->SetOffset(offset);
  myFilter->SetInput(0, nd1);
  myFilter->SetInput(1, nd2);

  MITK_TEST_CONDITION(((myFilter->GetInput(0) == nd1) 
                    && (myFilter->GetInput(1) == nd2)), "testing Set-/GetInput(index, data)");
  
  output = myFilter->GetOutput(0);
  mitk::NavigationData* output2 = myFilter->GetOutput(1);
  output2->Update(); // execute filter pipeline. this should update both outputs!
  MITK_TEST_CONDITION(((output != NULL) 
                    && (output2 != NULL)), "testing GetOutput(index)");

  MITK_TEST_CONDITION(output->GetPosition() == (initialPos + offset), "Testing offset calculation for output 0");
  MITK_TEST_CONDITION(output2->GetPosition() == (initialPos2 + offset), "Testing offset calculation for output 1");
  
  MITK_TEST_CONDITION(output2->GetOrientation() == initialOri, "Testing if Orientation remains unchanged for output 1");
  MITK_TEST_CONDITION(output2->GetError() == initialError, "Testing if Error remains unchanged for output 1");
  MITK_TEST_CONDITION(output2->IsDataValid() == initialValid, "Testing if DataValid remains unchanged for output 1");

  /* test if anything changes on second ->Update() */
  output->Update(); // nothing should change, since inputs remain unchanged
  MITK_TEST_CONDITION((output->GetPosition() == (initialPos + offset))
                    &&(output2->GetPosition() == (initialPos2 + offset)), "Testing offset calculation after second update()");

  /* change an input, see if output changes too */
  mitk::NavigationData::PositionType pos2;
  mitk::FillVector3D(pos2, 123.456, -234.567, 789.987);
  nd1->SetPosition(pos2);  // nd1 is modified, but nothing should change until pipe gets updated
  MITK_TEST_CONDITION((output->GetPosition() == (initialPos + offset))
    &&(output2->GetPosition() == (initialPos2 + offset)), "Testing offset calculation after input change, before update()");
  output->Update(); // update pipeline. should recalculate positions, because input has changed
  MITK_TEST_CONDITION((output->GetPosition() == (pos2 + offset))
    &&(output2->GetPosition() == (initialPos2 + offset)), "Testing offset calculation after input change, after update()");

  /* now, change filter parameter and check if pipeline updates correctly */
  mitk::Vector3D offset2;
  mitk::FillVector3D(offset2, -1.2, 3.45, -6.789);
  myFilter->SetOffset(offset2);
  MITK_TEST_CONDITION((output->GetPosition() == (pos2 + offset))
    &&(output2->GetPosition() == (initialPos2 + offset)), "Testing offset calculation after offset parameter change, before update()");
  output->Update(); // update pipeline. should recalculate positions, because input has changed
  MITK_TEST_CONDITION((output->GetPosition() == (pos2 + offset2))
    &&(output2->GetPosition() == (initialPos2 + offset2)), "Testing offset calculation after offset parameter , after update()");

  /* Now test the PropertyList handling of the GetParameters() and SetParameters() methods */
  mitk::PropertyList::ConstPointer p = myFilter->GetParameters();
  MITK_TEST_CONDITION(p.IsNotNull(), "Testing GetParameters() has valid return type");
  mitk::Vector3D v;
  MITK_TEST_CONDITION(p->GetPropertyValue<mitk::Vector3D>("NavigationDataDisplacementFilter_Offset", v) == true, "PropertyList contains correct key?");
  MITK_TEST_CONDITION(v == offset2, "Offset Property contains correct value?");
  MITK_TEST_CONDITION(p->GetMap()->size() == 1, "PropertyList contains correct number of items?");

  mitk::PropertyList::Pointer p2 = mitk::PropertyList::New();
  myFilter->SetParameters(p2);
  MITK_TEST_CONDITION(myFilter->GetOffset() == offset2, "Offset unchanged on SetParameters() with empty PropertyList?");
  p2->SetProperty("NavigationDataDisplacementFilter_WRONGNAME", mitk::Vector3DProperty::New(v));  // fill PropertyList with incorrect parameter
  MITK_TEST_CONDITION(myFilter->GetOffset() == offset2, "Offset unchanged on SetParameters() with wrong propertyname?");

  mitk::FillVector3D(v, -111.1, 0.0, 111.1);
  p2->SetProperty("NavigationDataDisplacementFilter_Offset", mitk::Vector3DProperty::New(v));  // fill PropertyList with correct parameter
  myFilter->SetParameters(p2);
  MITK_TEST_CONDITION(myFilter->GetOffset() == v, "Offset updated correctly by SetParameters()?");
  
  p2 = mitk::PropertyList::New();
  mitk::Vector3D v2;
  mitk::FillVector3D(v2, 0.0, 111.1, 0.0);
  p2->SetProperty("NavigationDataDisplacementFilter_Offset", mitk::Vector3DProperty::New(v2));  // change parameter in PropertyList
  myFilter->SetParameters(p2);
  MITK_TEST_CONDITION(myFilter->GetOffset() == v2, "Offset updated correctly by SetParameters()?");
  //MITK_TEST_CONDITION(myFilter->GetParameters() == p2, "GetParameters() returns an equal PropertyList as the one set with SetParameters?");  // not possible, since PropertyLists don't have a == operator

  v = myFilter->GetOffset();
  myFilter->SetParameters(myFilter->GetParameters());
  MITK_TEST_CONDITION(myFilter->GetOffset() == v, "Offset unchanged after SetParameters(GetParamters())");

  // always end with this!
  MITK_TEST_END();
}

