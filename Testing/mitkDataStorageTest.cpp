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
#include "mitkColorProperty.h"
#include "mitkGroupTagProperty.h"
#include "mitkDataTreeNode.h"

#include "mitkDataStorage.h"
#include "mitkNodePredicateProperty.h"
#include "mitkNodePredicateDataType.h"
#include "mitkNodePredicateNOT.h"
#include "mitkNodePredicateAND.h"
#include "mitkNodePredicateOR.h"
#include "mitkNodePredicateSource.h"

int CheckDataStorage(int argc, char* argv[], bool manageCompleteTree);

int mitkDataStorageTest(int argc, char* argv[])
{
  
  std::cout << "1. Step: Testing DataStorage in 'Only manage added nodes' mode." << std::endl;
  int returnValue1 = CheckDataStorage(argc, argv, false); // test the data storage in both operation modes
  std::cout << "2. Step: Testing DataStorage in 'Manage complete datatree' mode." << std::endl;
  int returnValue2 = CheckDataStorage(argc, argv, true);
  
  return (returnValue1 && returnValue2);
}

//##Documentation
//## @brief Test for the DataStorage class and its associated classes (e.g. the predicate classes)
int CheckDataStorage(int argc, char* argv[], bool manageCompleteTree)
{
  int returnValue = EXIT_SUCCESS;

  // create some datatree nodes to fill the ds
  mitk::DataTreeNode::Pointer n1 = mitk::DataTreeNode::New();   // node with image and name property
  mitk::Image::Pointer image = mitk::Image::New();
  n1->SetData(image);
  n1->SetProperty("name", new mitk::StringProperty("Node 1 - Image Node"));
  mitk::DataStorage::SetOfObjects::Pointer parents1 = mitk::DataStorage::SetOfObjects::New();


  mitk::DataTreeNode::Pointer n2 = mitk::DataTreeNode::New();   // node with surface and name and color properties
  mitk::Surface::Pointer surface = mitk::Surface::New();
  n2->SetData(surface);
  n2->SetProperty("name", new mitk::StringProperty("Node 2 - Surface Node"));
  mitk::Color color;  color.Set(1.0f, 0.0f, 0.0f);
  n2->SetColor(color);
  n2->SetProperty("Resection Proposal 1", new mitk::GroupTagProperty());
  mitk::DataStorage::SetOfObjects::Pointer parents2 = mitk::DataStorage::SetOfObjects::New();
  parents2->InsertElement(0, n1);  // n1 (image node) is source of n2 (surface node)


  mitk::DataTreeNode::Pointer n3 = mitk::DataTreeNode::New();   // node without data but with name property
  n3->SetProperty("name", new mitk::StringProperty("Node 3 - Empty Node"));
  n3->SetProperty("Resection Proposal 1", new mitk::GroupTagProperty());
  n3->SetProperty("Resection Proposal 2", new mitk::GroupTagProperty());
  mitk::DataStorage::SetOfObjects::Pointer parents3 = mitk::DataStorage::SetOfObjects::New();
  parents3->InsertElement(0, n2);  // n2 is source of n3 
  
  mitk::DataTreeNode::Pointer n4 = mitk::DataTreeNode::New();   // node without data but with color property
  n4->SetColor(color);
  n4->SetProperty("Resection Proposal 2", new mitk::GroupTagProperty());
  mitk::DataStorage::SetOfObjects::Pointer parents4 = mitk::DataStorage::SetOfObjects::New();
  parents4->InsertElement(0, n2); 
  parents4->InsertElement(1, n3);  // n2 and n3 are sources of n4 

  /* Get Data Storage for given tree */
  std::cout << "Get Data Storage for given tree : " << std::flush;
  mitk::DataTree::Pointer tree = mitk::DataTree::New();
  int objectsInTree = tree->Count();
  int initialObjectsInTree = objectsInTree;
  mitk::DataStorage::Pointer ds;
  try 
  {
    ds = mitk::DataStorage::CreateInstance(tree);
    ds->SetManageCompleteTree(manageCompleteTree);
    if (ds.IsNotNull())
      std::cout<<"[PASSED]"<<std::endl;
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
    return returnValue;   // further tests not possible, if no datastorage object could be created
  }

  /* Add an object */
  std::cout << "Adding a new object: " << std::flush;
  try 
  {
    ds->Add(n1, parents1);
    if ((tree->Count() == objectsInTree + 1) && (tree->Contains(n1)))
    {
      std::cout<<"[PASSED]"<<std::endl;
      objectsInTree = tree->Count();
    }
    else
    {
      std::cout << "[FAILED] - node not found in tree" << std::endl;
      returnValue = EXIT_FAILURE;
    }
  } 
  catch(...)
  {
    std::cout << "[FAILED] - Exception thrown" << std::endl;
    returnValue = EXIT_FAILURE;
  }

  /* Check exception on adding the same object again */
  std::cout << "Check exception on adding the same object again: " << std::flush;
  try 
  {
    ds->Add(n1, parents1);
    std::cout << "[FAILED] - no exception thrown" << std::endl; // no exception thrown
    returnValue = EXIT_FAILURE;
  } 
  catch(...)
  {
    if ((tree->Count() == objectsInTree ) && (tree->Contains(n1)))
    {
      std::cout<<"[PASSED]"<<std::endl;  // exception thrown, no object added, as expected
    }
    else
    {
      std::cout << "[FAILED] - exception thrown, but object count in tree is different" << std::endl;
      returnValue = EXIT_FAILURE;
    }
  }

  /* Add an object that has a source object */
  std::cout << "Adding an object that has a source object: " << std::flush;
  try 
  {
    ds->Add(n2, parents2);
    if ((tree->Count() == objectsInTree + 1) && (tree->Contains(n2)))
    {
      std::cout<<"[PASSED]"<<std::endl;
      objectsInTree = tree->Count();
    }
    else
    {
      std::cout << "[FAILED] - node not found in tree" << std::endl;
      returnValue = EXIT_FAILURE;
    }
  } 
  catch(...)
  {
    std::cout << "[FAILED] - Exception thrown" << std::endl;
    returnValue = EXIT_FAILURE;
  }

  /* Add some more objects needed for further tests */
  std::cout << "Adding some more objects needed for further tests: " << std::flush;
  try 
  {
    ds->Add(n3, parents3);   // n3 object that has name property and one parent
    ds->Add(n4, parents4);   // n4 object that has color property
    if ((tree->Count() == objectsInTree + 2) && (tree->Contains(n3)) && (tree->Contains(n4)))
    {
      std::cout<<"[PASSED]"<<std::endl;
      objectsInTree = tree->Count();
    }
    else
    {
      std::cout << "[FAILED] - nodes not found in tree" << std::endl;
      returnValue = EXIT_FAILURE;
    }
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
    if (ds->GetManageCompleteTree())
      if (   (stlAll.size() == tree->Count())  // check if all tree nodes are in resultset
          && (std::find(stlAll.begin(), stlAll.end(), n1) != stlAll.end()) && (std::find(stlAll.begin(), stlAll.end(), n2) != stlAll.end())
          && (std::find(stlAll.begin(), stlAll.end(), n3) != stlAll.end()) && (std::find(stlAll.begin(), stlAll.end(), n4) != stlAll.end()))
      {
        std::cout<<"[PASSED]"<<std::endl;
      }
      else
      {
        std::cout << "[FAILED]" << std::endl;
        returnValue = EXIT_FAILURE;
      }
    else
      if (   (stlAll.size() == 4)  // check if all added nodes are in resultset
          && (std::find(stlAll.begin(), stlAll.end(), n1) != stlAll.end()) && (std::find(stlAll.begin(), stlAll.end(), n2) != stlAll.end())
          && (std::find(stlAll.begin(), stlAll.end(), n3) != stlAll.end()) && (std::find(stlAll.begin(), stlAll.end(), n4) != stlAll.end()))
      {
        std::cout<<"[PASSED]"<<std::endl;
      }
      else
      {
        std::cout << "[FAILED]" << std::endl;
        returnValue = EXIT_FAILURE;
      }
  } 
  catch(...)
  {
    std::cout<<"[FAILED] - Exception thrown" << std::endl;
    returnValue = EXIT_FAILURE;
  }
  
  /* Adding a node directly to the tree to test if the DataStorage can handle that */
  std::cout << "Adding a node directly to the tree to test if the DataStorage can handle that." << std::endl;
  mitk::DataTreePreOrderIterator it(tree);
  mitk::DataTreeNode::Pointer treeNode = mitk::DataTreeNode::New();   // node with image and name property
  treeNode->SetProperty("name", new mitk::StringProperty("TreeNode - not added by DataStorage"));  
  it.Add(treeNode);
  /* Requesting all Objects again to check for new tree node */
  std::cout << "Requesting all Objects again to check for new tree node: " << std::flush;
  try 
  {
    const mitk::DataStorage::SetOfObjects::ConstPointer all = ds->GetAll();
    std::vector<mitk::DataTreeNode::Pointer> stlAll = all->CastToSTLConstContainer();
    if (ds->GetManageCompleteTree())
      if (   (stlAll.size() == tree->Count())  // check if all tree nodes are in resultset
          && (std::find(stlAll.begin(), stlAll.end(), n1) != stlAll.end()) && (std::find(stlAll.begin(), stlAll.end(), n2) != stlAll.end())
          && (std::find(stlAll.begin(), stlAll.end(), n3) != stlAll.end()) && (std::find(stlAll.begin(), stlAll.end(), n4) != stlAll.end())
          && (std::find(stlAll.begin(), stlAll.end(), treeNode) != stlAll.end()))
      {
        std::cout<<"[PASSED]"<<std::endl;
      }
      else
      {
        std::cout << "[FAILED]" << std::endl;
        returnValue = EXIT_FAILURE;
      }
    else
      if (   (stlAll.size() == 4)  // check if all added nodes are in resultset
          && (std::find(stlAll.begin(), stlAll.end(), n1) != stlAll.end()) && (std::find(stlAll.begin(), stlAll.end(), n2) != stlAll.end())
          && (std::find(stlAll.begin(), stlAll.end(), n3) != stlAll.end()) && (std::find(stlAll.begin(), stlAll.end(), n4) != stlAll.end()))
      {
        std::cout<<"[PASSED]"<<std::endl;
      }
      else
      {
        std::cout << "[FAILED]" << std::endl;
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
    mitk::NodePredicateProperty predicate("name", new mitk::StringProperty("Node 2 - Surface Node"));
    mitk::DataStorage::SetOfObjects::ConstPointer all = ds->GetSubset(predicate);
    // delete nameProp;
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
    mitk::NodePredicateProperty p2("color", new mitk::ColorProperty(color));
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
    mitk::NodePredicateProperty proppred("color", cp);
    mitk::NodePredicateNOT predicate(proppred);

    const mitk::DataStorage::SetOfObjects::ConstPointer all = ds->GetSubset(predicate);
    std::vector<mitk::DataTreeNode::Pointer> stlAll = all->CastToSTLConstContainer();
    int expectedCount;
    if (ds->GetManageCompleteTree())
      expectedCount = initialObjectsInTree + 2 + 1;  // all from init time, n1, n3 and the directly added node
    else
      expectedCount = 2;  // n1, n3

    if (   (all->Size() == expectedCount) // check if correct objects are in resultset
        && (std::find(stlAll.begin(), stlAll.end(), n1) != stlAll.end()) && (std::find(stlAll.begin(), stlAll.end(), n3) != stlAll.end()))
    {
      std::cout<<"[PASSED]"<<std::endl;
    }
    else
    {
      std::cout << "[FAILED]" << std::endl;
      returnValue = EXIT_FAILURE;
    }
  }
  catch(...)
  {
    std::cout<<"[FAILED] - Exception thrown" << std::endl;
    returnValue = EXIT_FAILURE;
  }

  /* Requesting *direct* source objects */
  std::cout << "Requesting *direct* source objects: " << std::flush;
  try
  {
    const mitk::DataStorage::SetOfObjects::ConstPointer all = ds->GetSources(n3, true); // Get direct parents of n3 (=n2)
    std::vector<mitk::DataTreeNode::Pointer> stlAll = all->CastToSTLConstContainer(); 
    if ((all->Size() == 1) && (std::find(stlAll.begin(), stlAll.end(), n2) != stlAll.end()) )// check if n1 is the resultset
    {
      std::cout<<"[PASSED]"<<std::endl;
    }
    else
    {
      std::cout << "[FAILED]" << std::endl;
      returnValue = EXIT_FAILURE;
    }
  }
  catch(...)
  {
    std::cout<<"[FAILED] - Exception thrown" << std::endl;
    returnValue = EXIT_FAILURE;
  }

  /* Requesting *all* source objects */
  std::cout << "Requesting *all* source objects: " << std::flush;
  try
  {
    const mitk::DataStorage::SetOfObjects::ConstPointer all = ds->GetSources(n3, false); // Get all parents of n3 (= n1 + n2)
    std::vector<mitk::DataTreeNode::Pointer> stlAll = all->CastToSTLConstContainer(); 
    if ((all->Size() == 2) 
      && (std::find(stlAll.begin(), stlAll.end(), n1) != stlAll.end())
      && (std::find(stlAll.begin(), stlAll.end(), n2) != stlAll.end())) // check if n1 and n2 are the resultset
    {
      std::cout<<"[PASSED]"<<std::endl;
    }
    else
    {
      std::cout << "[FAILED]" << std::endl;
      returnValue = EXIT_FAILURE;
    }
  }
  catch(...)
  {
    std::cout<<"[FAILED] - Exception thrown" << std::endl;
    returnValue = EXIT_FAILURE;
  }

  /* Requesting *all* sources of object with multiple parents */
  std::cout << "Requesting *all* sources of object with multiple parents: " << std::flush;
  try
  {
    const mitk::DataStorage::SetOfObjects::ConstPointer all = ds->GetSources(n4, false); // Get all parents of n4 (= n1 + n2 + n3)
    std::vector<mitk::DataTreeNode::Pointer> stlAll = all->CastToSTLConstContainer(); 
    if ((all->Size() == 3)
      && (std::find(stlAll.begin(), stlAll.end(), n1) != stlAll.end())
      && (std::find(stlAll.begin(), stlAll.end(), n2) != stlAll.end())
      && (std::find(stlAll.begin(), stlAll.end(), n3) != stlAll.end())) // check if n1 and n2 and n3 are the resultset
    {
      std::cout<<"[PASSED]"<<std::endl;
    }
    else
    {
      std::cout << "[FAILED]" << std::endl;
      returnValue = EXIT_FAILURE;
    }
  }
  catch(...)
  {
    std::cout<<"[FAILED] - Exception thrown" << std::endl;
    returnValue = EXIT_FAILURE;
  }

  /* Checking for circular source relationships */
  std::cout << "Checking for circular source relationships: " << std::flush;
  try
  {
    parents1->InsertElement(0, n4);   // make n1 derived from n4 (which is derived from n2, which is derived from n1)
    const mitk::DataStorage::SetOfObjects::ConstPointer all = ds->GetSources(n4, false); // Get all parents of n4 (= n1 + n2 + n3, not n4 itself and not multiple versions of the nodes!)
    std::vector<mitk::DataTreeNode::Pointer> stlAll = all->CastToSTLConstContainer(); 
    if ((all->Size() == 3)
      && (std::find(stlAll.begin(), stlAll.end(), n1) != stlAll.end())
      && (std::find(stlAll.begin(), stlAll.end(), n2) != stlAll.end())
      && (std::find(stlAll.begin(), stlAll.end(), n3) != stlAll.end())) // check if n1 and n2 and n3 are the resultset
    {
      std::cout<<"[PASSED]"<<std::endl;
    }
    else
    {
      std::cout << "[FAILED]" << std::endl;
      returnValue = EXIT_FAILURE;
    }
  }
  catch(...)
  {
    std::cout<<"[FAILED] - Exception thrown" << std::endl;
    returnValue = EXIT_FAILURE;
  }


  /* finally return cumulated returnValue */
  return returnValue;
}

