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

#include "mitkNodePredicateSource.h"
#include "mitkTestingMacros.h"
#include "mitkStandaloneDataStorage.h"

  int mitkNodePredicateSourceTest(int /* argc */, char* /*argv*/[])
  {
    MITK_TEST_BEGIN("Testing nodePredicateSource")

      // Create a DataStorage
      mitk::DataStorage::Pointer myDataStorage(mitk::StandaloneDataStorage::New().GetPointer());
    mitk::DataNode::Pointer godfather, grandMother, mother, daughter;
    godfather = mitk::DataNode::New();
    grandMother = mitk::DataNode::New();
    mother = mitk::DataNode::New();
    daughter = mitk::DataNode::New();

    myDataStorage->Add(godfather);
    myDataStorage->Add(grandMother);
    myDataStorage->Add(mother, grandMother);
    myDataStorage->Add(daughter, mother);
    mitk::NodePredicateSource::Pointer testPredicate = mitk::NodePredicateSource::New(grandMother, false, myDataStorage);
    ;

    MITK_TEST_CONDITION_REQUIRED(true, "Testing instantiation");
    MITK_TEST_CONDITION_REQUIRED(testPredicate->CheckNode(mother), "Node is derivative");
    MITK_TEST_CONDITION_REQUIRED(!testPredicate->CheckNode(godfather), "Node is not derivative");
    MITK_TEST_CONDITION_REQUIRED(!testPredicate->CheckNode(daughter), "Node is derivative but only direct derivatives are wanted");

    testPredicate = NULL;
    testPredicate = mitk::NodePredicateSource::New(grandMother, true, myDataStorage);
    MITK_TEST_CONDITION_REQUIRED(testPredicate->CheckNode(daughter), "Node is not direct derivative and all derivatives are wanted ");
    MITK_TEST_CONDITION_REQUIRED(!testPredicate->CheckNode(grandMother), "Self is not a derivative!");

    MITK_TEST_END();
  }