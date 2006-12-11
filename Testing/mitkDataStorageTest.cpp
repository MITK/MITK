/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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

#include <fstream>
#include <algorithm>

#include "mitkImage.h"
#include "mitkSurface.h"
#include "mitkStringProperty.h"
#include <mitkColorProperty.h>
#include "mitkDataTreeNode.h"

#include "mitkDataStorage.h"
#include "mitkNodePredicateProperty.h"
#include "mitkNodePredicateDataType.h"
#include "mitkNodePredicateNOT.h"
#include "mitkNodePredicateAND.h"
#include "mitkNodePredicateOR.h"
#include "mitkNodePredicateSource.h"


//##Documentation
//## @brief Test for the DataStorage class and its associated classes (e.g. the predicate classes)
int mitkDataStorageTest(int argc, char* argv[])
{
  int returnValue = EXIT_SUCCESS;

  // create some datatree nodes to fill the ds
  mitk::DataTreeNode::Pointer n1 = mitk::DataTreeNode::New();   // node with image and name property
  mitk::Image::Pointer image = mitk::Image::New();
  n1->SetData(image);
  n1->SetProperty("name", new mitk::StringProperty("Image Node"));
  
  mitk::DataTreeNode::Pointer n2 = mitk::DataTreeNode::New();   // node with surface and name and color properties
  mitk::Surface::Pointer surface = mitk::Surface::New();
  n2->SetData(surface);
  n2->SetProperty("name", new mitk::StringProperty("Surface Node"));
  mitk::Color color;  color.Set(1.0f, 0.0f, 0.0f);
  n2->SetColor(color);

  mitk::DataTreeNode::Pointer n3 = mitk::DataTreeNode::New();   // node without data but with name property
  n3->SetProperty("name", new mitk::StringProperty("Empty Node"));
  
  mitk::DataTreeNode::Pointer n4 = mitk::DataTreeNode::New();   // node without data but with color property
  n4->SetColor(color);
  
  /* Create a DataStorage object */
  std::cout << "Instantiating a mitk::DataStorage object: " << std::flush;
  mitk::DataStorage::Pointer ds = mitk::DataStorage::New();
  if (ds.IsNull())
  {
    std::cout<<"[FAILED]"<<std::endl;
    returnValue = EXIT_FAILURE;
    return returnValue; // makes no sense to continue testing the datastorage, if it does not exist
  }
  else
    std::cout<<"[PASSED]"<<std::endl;

  /* Initialize Data Storage */
  std::cout << "Initialize Data Storage : " << std::flush;
  mitk::DataTree::Pointer tree = mitk::DataTree::New();
  try 
  {
    ds->Initialize(tree.GetPointer());
    std::cout<<"[PASSED]"<<std::endl;
  } 
  catch(...)
  {
    std::cout<<"[FAILED] - Exception thrown" << std::endl;
    returnValue = EXIT_FAILURE;
  }

  /* Add an object */
  std::cout << "Adding a new object: " << std::flush;
  try 
  {
    ds->Add(n1);
    std::cout<<"[PASSED]"<<std::endl;
  } 
  catch(...)
  {
    std::cout<<"[FAILED] - Exception thrown" << std::endl;
    returnValue = EXIT_FAILURE;
  }

  /* Check exception on adding the same object again */
  std::cout << "Check exception on adding the same object again: " << std::flush;
  try 
  {
    ds->Add(n1);
    std::cout<<"[FAILED]"<<std::endl; // no exception thrown
    returnValue = EXIT_FAILURE;
  } 
  catch(...)
  {
    std::cout<<"[PASSED]"<<std::endl; // exception thrown, as expected
  }

  /* Add an object that has a source object */
  std::cout << "Adding an object that has a source object: " << std::flush;
  try 
  {
    mitk::DataStorage::SetOfObjects::Pointer parents = mitk::DataStorage::SetOfObjects::New();
    parents->InsertElement(0, n1);  // n1 (image node) is source of n2 (surface node)
    ds->Add(n2, parents.GetPointer());
    std::cout<<"[PASSED]"<<std::endl;
  } 
  catch(...)
  {
    std::cout<<"[FAILED] - Exception thrown" << std::endl;
    returnValue = EXIT_FAILURE;
  }

  /* Add some more objects needed for further tests */
  std::cout << "Adding some more objects needed for further tests: " << std::flush;
  try 
  {
    ds->Add(n3);   // 3. object that has name property
    ds->Add(n4);   // 2. object that has color property
    std::cout<<"[PASSED]"<<std::endl;
  } 
  catch(...)
  {
    std::cout<<"[FAILED] - Exception thrown" << std::endl;
    returnValue = EXIT_FAILURE;
  }


  /* Requesting all Objects */
  std::cout << "Requesting all Objects: " << std::flush;
  try 
  {
    const mitk::DataStorage::SetOfObjects::ConstPointer all = ds->GetAll();
    std::vector<mitk::DataTreeNode::Pointer> stlAll = all->CastToSTLConstContainer();
    //std::cout << "all Objects: " << all << std::flush;
    if (   (stlAll.size() == 4)  // check if all added nodes are in resultset
      && (std::find(stlAll.begin(), stlAll.end(), n1) != stlAll.end()) && (std::find(stlAll.begin(), stlAll.end(), n2) != stlAll.end())
        && (std::find(stlAll.begin(), stlAll.end(), n3) != stlAll.end()) && (std::find(stlAll.begin(), stlAll.end(), n4) != stlAll.end()))
    {
      std::cout<<"[PASSED]"<<std::endl;
    }
    else
    {
      std::cout << "[FAILED]" << std::endl;
      //std::cout << " returned elements: ";
      //for (mitk::DataStorage::SetOfObjects::ConstIterator i = all->Begin(); i != all->End(); i++)
      //  std::cout << i << ", ";
      //std::cout << std::endl;
      returnValue = EXIT_FAILURE;
    }
  } 
  catch(...)
  {
    std::cout<<"[FAILED] - Exception thrown" << std::endl;
    returnValue = EXIT_FAILURE;
  }

  /* Requesting a named object */
  std::cout << "Requesting a named object: " << std::flush;
  try 
  {
    mitk::StringProperty nameProp("Surface Node"); // build new property for the search criteria
    mitk::NodePredicateProperty predicate("name", nameProp);
    mitk::DataStorage::SetOfObjects::ConstPointer all = ds->GetSubset(predicate);
    if ((all->Size() == 1) && (all->GetElement(0) == n2))  // check if correct object is in resultset
    {
      std::cout<<"[PASSED]"<<std::endl;
    }
    else
    {
      std::cout<<"[FAILED]"<<std::endl;
      returnValue = EXIT_FAILURE;
    }
  } 
  catch(...)
  {
    std::cout<<"[FAILED] - Exception thrown" << std::endl;
    returnValue = EXIT_FAILURE;
  }

  /* Requesting objects of specific data type */
  std::cout << "Requesting objects of specific data type: " << std::flush;
  try 
  {
    mitk::NodePredicateDataType predicate("Image");
    mitk::DataStorage::SetOfObjects::ConstPointer all = ds->GetSubset(predicate);
    if ((all->Size() == 1) && (all->GetElement(0) == n1))  // check if correct object is in resultset
    {
      std::cout<<"[PASSED]"<<std::endl;
    }
    else
    {
      std::cout<<"[FAILED]"<<std::endl;
      returnValue = EXIT_FAILURE;
    }
  } 
  catch(...)
  {
    std::cout<<"[FAILED] - Exception thrown" << std::endl;
    returnValue = EXIT_FAILURE;
  }

  /* Requesting objects that meet a conjunction criteria */
  std::cout << "Requesting objects that meet a conjunction criteria: " << std::flush;
  try 
  {
    mitk::NodePredicateDataType p1("Surface");
    mitk::ColorProperty colorprop(color);
    mitk::NodePredicateProperty p2("color", colorprop);
    mitk::NodePredicateAND predicate;
    predicate.AddPredicate(p1);
    predicate.AddPredicate(p2);  // objects must be of datatype "Surface" and have red color (= n2)
    const mitk::DataStorage::SetOfObjects::ConstPointer all = ds->GetSubset(predicate);
    if ((all->Size() == 1) && (all->GetElement(0) == n2))  // check if correct object is in resultset
    {
      std::cout<<"[PASSED]"<<std::endl;
    }
    else
    {
      std::cout<<"[FAILED]"<<std::endl;
      returnValue = EXIT_FAILURE;
    }
  } 
  catch(...)
  {
    std::cout<<"[FAILED] - Exception thrown" << std::endl;
    returnValue = EXIT_FAILURE;
  }

  /* Requesting objects that do not meet a criteria */
  std::cout << "Requesting objects that do not meet a criteria: " << std::flush;
  try 
  {
    mitk::ColorProperty::Pointer cp = new mitk::ColorProperty(color);
    mitk::NodePredicateProperty proppred("color", *cp);
    mitk::NodePredicateNOT predicate(proppred);

    const mitk::DataStorage::SetOfObjects::ConstPointer all = ds->GetSubset(predicate);
    if (   (all->Size() == 2) // check if correct object is in resultset
        && (all->GetElement(0) == n1) && (all->GetElement(1) == n3))
    {
      std::cout<<"[PASSED]"<<std::endl;
    }
    else
    {
      std::cout<<"[FAILED]"<<std::endl;
      returnValue = EXIT_FAILURE;
    }
  } 
  catch(...)
  {
    std::cout<<"[FAILED] - Exception thrown" << std::endl;
    returnValue = EXIT_FAILURE;
  }
  return returnValue;
}

