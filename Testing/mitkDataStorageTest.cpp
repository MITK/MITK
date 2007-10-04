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
#include "mitkReferenceCountWatcher.h"
#include "mitkDataTreeHelper.h"

#include "mitkDataStorage.h"
#include "mitkNodePredicateProperty.h"
#include "mitkNodePredicateDataType.h"
#include "mitkNodePredicateDimension.h"
#include "mitkNodePredicateData.h"
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
int CheckDataStorage(int, char*[], bool manageCompleteTree)
{
  int returnValue = EXIT_SUCCESS;

  // create some datatree nodes to fill the ds
  mitk::DataTreeNode::Pointer n1 = mitk::DataTreeNode::New();   // node with image and name property
  mitk::Image::Pointer image = mitk::Image::New();
  unsigned int imageDimensions[] = { 10, 10, 10 };
  mitk::PixelType pt(typeid(int));
  image->Initialize( pt, 3, imageDimensions );
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

  mitk::DataTreeNode::Pointer n5 = mitk::DataTreeNode::New();   // extra node
  n5->SetProperty("name", new mitk::StringProperty("Node 5"));

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

  /* Adding a node without parents */
  std::cout << "Adding a node without parents: " << std::flush;
  try 
  {
    ds->Add(n5);
    if ((tree->Count() == objectsInTree + 1) && (tree->Contains(n5)))
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
    std::cout<<"[FAILED] - Exception thrown" << std::endl;
    returnValue = EXIT_FAILURE;
  }

  /* Requesting all Objects */
  std::cout << "Requesting all Objects: " << std::flush;
  try 
  {
    const mitk::DataStorage::SetOfObjects::ConstPointer all = ds->GetAll();
    std::vector<mitk::DataTreeNode::Pointer> stlAll = all->CastToSTLConstContainer();
    if (ds->GetManageCompleteTree() == true)
      if (   (stlAll.size() == (unsigned int) tree->Count())  // check if all tree nodes are in resultset
          && (std::find(stlAll.begin(), stlAll.end(), n1) != stlAll.end()) && (std::find(stlAll.begin(), stlAll.end(), n2) != stlAll.end())
          && (std::find(stlAll.begin(), stlAll.end(), n3) != stlAll.end()) && (std::find(stlAll.begin(), stlAll.end(), n4) != stlAll.end())
          && (std::find(stlAll.begin(), stlAll.end(), n5) != stlAll.end()))
      {
        std::cout<<"[PASSED]"<<std::endl;
      }
      else
      {
        std::cout << "[FAILED]" << std::endl;
        returnValue = EXIT_FAILURE;
      }
    else
      if (   (stlAll.size() == 5)  // check if all added nodes are in resultset
          && (std::find(stlAll.begin(), stlAll.end(), n1) != stlAll.end()) && (std::find(stlAll.begin(), stlAll.end(), n2) != stlAll.end())
          && (std::find(stlAll.begin(), stlAll.end(), n3) != stlAll.end()) && (std::find(stlAll.begin(), stlAll.end(), n4) != stlAll.end())
          && (std::find(stlAll.begin(), stlAll.end(), n5) != stlAll.end()))
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
    if (ds->GetManageCompleteTree() == true)
      if (   (stlAll.size() == (unsigned int) tree->Count())  // check if all tree nodes are in resultset
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
      if (   (stlAll.size() == 5)  // check if all added nodes are in resultset
          && (std::find(stlAll.begin(), stlAll.end(), n1) != stlAll.end()) && (std::find(stlAll.begin(), stlAll.end(), n2) != stlAll.end())
          && (std::find(stlAll.begin(), stlAll.end(), n3) != stlAll.end()) && (std::find(stlAll.begin(), stlAll.end(), n4) != stlAll.end())
          && (std::find(stlAll.begin(), stlAll.end(), n5) != stlAll.end()))
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
    mitk::StringProperty s("Node 2 - Surface Node");
    mitk::NodePredicateProperty predicate("name", &s);
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

  /* Requesting objects of specific dimension */
  std::cout << "Requesting objects of specific dimension: " << std::flush;
  try 
  {
    mitk::NodePredicateDimension predicate( 3 );
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

  /* Requesting objects with specific data object */
  std::cout << "Requesting objects with specific data object : " << std::flush;
  try 
  {
    mitk::NodePredicateData predicate(image);
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

  /* Requesting objects with NULL data */
  std::cout << "Requesting objects with NULL data: " << std::flush;
  try 
  {
    mitk::NodePredicateData predicate(NULL);
    mitk::DataStorage::SetOfObjects::ConstPointer all = ds->GetSubset(predicate);
    if (manageCompleteTree)   // more than n1-n5 is returned and the extra nodes do not have a dataobject
    {
      if ((all->Size() == (unsigned int) tree->Count() - 2)  // we want to get all but n1 and n2
          && (std::find(all->begin(), all->end(), n3) != all->end()) // and especially n3-n5
          && (std::find(all->begin(), all->end(), n4) != all->end())
          && (std::find(all->begin(), all->end(), n5) != all->end()))
      {
        std::cout<<"[PASSED]"<<std::endl;
      }
      else
      {
        std::cout<<"[FAILED]"<<std::endl;
        returnValue = EXIT_FAILURE;
      }

    }
    else
    {
      if ((all->Size() == 3)
          && (std::find(all->begin(), all->end(), n3) != all->end()) 
          && (std::find(all->begin(), all->end(), n4) != all->end())
          && (std::find(all->begin(), all->end(), n5) != all->end()))
      {
        std::cout<<"[PASSED]"<<std::endl;
      }
      else
      {
        std::cout<<"[FAILED]"<<std::endl;
        returnValue = EXIT_FAILURE;
      }
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

  /* Requesting objects that meet a disjunction criteria */
  std::cout << "Requesting objects that meet a disjunction criteria: " << std::flush;
  try 
  {
    mitk::NodePredicateDataType p1("Image");
    mitk::NodePredicateProperty p2("color", new mitk::ColorProperty(color));
    mitk::NodePredicateOR predicate;
    predicate.AddPredicate(p1);
    predicate.AddPredicate(p2);  // objects must be of datatype "Surface" or have red color (= n1, n2, n4)
    const mitk::DataStorage::SetOfObjects::ConstPointer all = ds->GetSubset(predicate);
    if ((all->Size() == 3) 
        && (std::find(all->begin(), all->end(), n1) != all->end()) 
        && (std::find(all->begin(), all->end(), n2) != all->end())
        && (std::find(all->begin(), all->end(), n4) != all->end()))
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
    if (ds->GetManageCompleteTree() == true)
      expectedCount = initialObjectsInTree + 3 + 1;  // all from init time, n1, n3, n5 and the directly added node
    else
      expectedCount = 3;  // n1, n3, n5

    if (   (all->Size() == (unsigned int) expectedCount) // check if correct objects are in resultset
        && (std::find(stlAll.begin(), stlAll.end(), n1) != stlAll.end()) 
        && (std::find(stlAll.begin(), stlAll.end(), n3) != stlAll.end())
        && (std::find(stlAll.begin(), stlAll.end(), n5) != stlAll.end()))
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
    const mitk::DataStorage::SetOfObjects::ConstPointer all = ds->GetSources(n3, NULL, true); // Get direct parents of n3 (=n2)
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
    const mitk::DataStorage::SetOfObjects::ConstPointer all = ds->GetSources(n3, NULL, false); // Get all parents of n3 (= n1 + n2)
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
    const mitk::DataStorage::SetOfObjects::ConstPointer all = ds->GetSources(n4, NULL, false); // Get all parents of n4 (= n1 + n2 + n3)
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

  /* Requesting *direct* derived objects */
  std::cout << "Requesting *direct* derived objects: " << std::flush;
  try
  {
    const mitk::DataStorage::SetOfObjects::ConstPointer all = ds->GetDerivations(n1, NULL, true); // Get direct childs of n1 (=n2)
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

  /* Requesting *direct* derived objects with multiple parents/derivations */
  std::cout << "Requesting *direct* derived objects with multiple parents/derivations: " << std::flush;
  try
  {
    const mitk::DataStorage::SetOfObjects::ConstPointer all = ds->GetDerivations(n2, NULL, true); // Get direct childs of n2 (=n3 + n4)
    std::vector<mitk::DataTreeNode::Pointer> stlAll = all->CastToSTLConstContainer(); 
    if ((all->Size() == 2) 
      && (std::find(stlAll.begin(), stlAll.end(), n3) != stlAll.end()) // check if n3 is the resultset
      && (std::find(stlAll.begin(), stlAll.end(), n4) != stlAll.end()) )// check if n4 is the resultset
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

  /* Requesting *all* derived objects */
  std::cout << "Requesting *all* derived objects: " << std::flush;
  try
  {
    const mitk::DataStorage::SetOfObjects::ConstPointer all = ds->GetDerivations(n1, NULL, false); // Get all childs of n1 (=n2, n3, n4)
    std::vector<mitk::DataTreeNode::Pointer> stlAll = all->CastToSTLConstContainer(); 
    if ((all->Size() == 3) 
      && (std::find(stlAll.begin(), stlAll.end(), n2) != stlAll.end())
      && (std::find(stlAll.begin(), stlAll.end(), n3) != stlAll.end())
      && (std::find(stlAll.begin(), stlAll.end(), n4) != stlAll.end()))
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
    const mitk::DataStorage::SetOfObjects::ConstPointer all = ds->GetSources(n4, NULL, false); // Get all parents of n4 (= n1 + n2 + n3, not n4 itself and not multiple versions of the nodes!)
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

  /* Checking for circular derivation relationships can not be performed, because the internal derivations datastructure 
     can not be accessed from the outside. (Therefore it should not be possible to create these circular relations */

  /* Checking GroupTagProperty */
  std::cout << "Checking GroupTagProperty 1: " << std::flush;
  try
  {
    mitk::GroupTagProperty::Pointer tp = new mitk::GroupTagProperty();
    mitk::NodePredicateProperty pred("Resection Proposal 1", tp);

    const mitk::DataStorage::SetOfObjects::ConstPointer all = ds->GetSubset(pred);
    std::vector<mitk::DataTreeNode::Pointer> stlAll = all->CastToSTLConstContainer();
    if (   (all->Size() == 2) // check if n2 and n3 are in resultset
        && (std::find(stlAll.begin(), stlAll.end(), n2) != stlAll.end()) && (std::find(stlAll.begin(), stlAll.end(), n3) != stlAll.end()))
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

  /* Checking GroupTagProperty 2 */
  std::cout << "Checking GroupTagProperty 2: " << std::flush;
  try
  {
    mitk::GroupTagProperty::Pointer tp = new mitk::GroupTagProperty();
    mitk::NodePredicateProperty pred("Resection Proposal 2", tp);

    const mitk::DataStorage::SetOfObjects::ConstPointer all = ds->GetSubset(pred);
    std::vector<mitk::DataTreeNode::Pointer> stlAll = all->CastToSTLConstContainer();
    if (   (all->Size() == 2) // check if n3 and n4 are in resultset
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

  /* Checking direct sources with condition */
  std::cout << "Checking direct sources with condition: " << std::flush;
  try
  {
    mitk::NodePredicateDataType pred("Surface");

    const mitk::DataStorage::SetOfObjects::ConstPointer all = ds->GetSources(n4, &pred, true);
    std::vector<mitk::DataTreeNode::Pointer> stlAll = all->CastToSTLConstContainer();
    if (   (all->Size() == 1) // check if n2 is in resultset
        && (std::find(stlAll.begin(), stlAll.end(), n2) != stlAll.end()))
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
  
  /* Checking all sources with condition */
  std::cout << "Checking all sources with condition: " << std::flush;
  try
  {
    
    mitk::NodePredicateDataType pred("Image");

    const mitk::DataStorage::SetOfObjects::ConstPointer all = ds->GetSources(n4, &pred, false);
    std::vector<mitk::DataTreeNode::Pointer> stlAll = all->CastToSTLConstContainer();
    if (   (all->Size() == 1) // check if n1 is in resultset
        && (std::find(stlAll.begin(), stlAll.end(), n1) != stlAll.end()))
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

  /* Checking all sources with condition with empty resultset */
  std::cout << "Checking all sources with condition with empty resultset: " << std::flush;
  try
  { 
    mitk::NodePredicateDataType pred("VesselTree");

    const mitk::DataStorage::SetOfObjects::ConstPointer all = ds->GetSources(n4, &pred, false);
    std::vector<mitk::DataTreeNode::Pointer> stlAll = all->CastToSTLConstContainer();
    if (all->Size() == 0) // check if resultset is empty
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

  /* Checking direct derivations with condition */
  std::cout << "Checking direct derivations with condition: " << std::flush;
  try
  {
    mitk::NodePredicateProperty pred("color");

    const mitk::DataStorage::SetOfObjects::ConstPointer all = ds->GetDerivations(n1, &pred, true);
    std::vector<mitk::DataTreeNode::Pointer> stlAll = all->CastToSTLConstContainer();
    if (   (all->Size() == 1) // check if n2 is in resultset
        && (std::find(stlAll.begin(), stlAll.end(), n2) != stlAll.end()))
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

  /* Checking all derivations with condition */
  std::cout << "Checking all derivations with condition: " << std::flush;
  try
  {
    mitk::NodePredicateProperty pred("color");

    const mitk::DataStorage::SetOfObjects::ConstPointer all = ds->GetDerivations(n1, &pred, false);
    std::vector<mitk::DataTreeNode::Pointer> stlAll = all->CastToSTLConstContainer();
    if (   (all->Size() == 2) // check if n2 and n4 are in resultset
        && (std::find(stlAll.begin(), stlAll.end(), n2) != stlAll.end())
        && (std::find(stlAll.begin(), stlAll.end(), n4) != stlAll.end()))
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

  /* Checking named node method */
  std::cout << "Checking named node method: " << std::flush;
  try
  {
    
    if (ds->GetNamedNode("Node 2 - Surface Node") == n2)
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

  /* Checking named node method with wrong name */
  std::cout << "Checking named node method with wrong name: " << std::flush;
  try
  {
    
    if (ds->GetNamedNode("This name does not exist") == NULL)
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

  /* Checking named object method */
  std::cout << "Checking named object method: " << std::flush;
  try
  {
    
    if (ds->GetNamedObject<mitk::Image>("Node 1 - Image Node") == image)
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

  /* Checking named object method with wrong DataType */
  std::cout << "Checking named object method with wrong DataType: " << std::flush;
  try
  {
    
    if (ds->GetNamedObject<mitk::Surface>("Node 1 - Image Node") == NULL)
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

  /* Checking named object method with wrong name */
  std::cout << "Checking named object method with wrong name: " << std::flush;
  try
  {
    
    if (ds->GetNamedObject<mitk::Image>("This name does not exist") == NULL)
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

  /* Checking GetNamedDerivedNode with valid name and direct derivation only */
  std::cout << "Checking GetNamedDerivedNode with valid name and direct derivation only: " << std::flush;
  try
  {
    if (ds->GetNamedDerivedNode("Node 2 - Surface Node", n1, true) == n2)
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

  /* Checking GetNamedDerivedNode with invalid Name and direct derivation only */
  std::cout << "Checking GetNamedDerivedNode with invalid Name and direct derivation only: " << std::flush;
  try
  {
    if (ds->GetNamedDerivedNode("wrong name", n1, true) == NULL)
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

  /* Checking GetNamedDerivedNode with invalid Name and direct derivation only */
  std::cout << "Checking GetNamedDerivedNode with valid Name and complete derivation: " << std::flush;
  try
  {
    if (ds->GetNamedDerivedNode("Node 3 - Empty Node", n1, false) == n3)
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

  /* Checking GetNamedDerivedNode with valid Name but direct derivation only */
  std::cout << "Checking GetNamedDerivedNode with valid Name and complete derivation: " << std::flush;
  try
  {
    if (ds->GetNamedDerivedNode("Node 3 - Empty Node", n1, true) == NULL)
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

  /* Checking removal of a node without relations */
  std::cout << "Checking removal of a node without relations: " << std::flush;
  try
  {
    mitk::DataTreeNode::Pointer extra = mitk::DataTreeNode::New();
    extra->SetProperty("name", new mitk::StringProperty("extra"));
   
    mitk::ReferenceCountWatcher::Pointer watcher = new mitk::ReferenceCountWatcher(extra);
    int refCountbeforeDS = watcher->GetReferenceCount();
    ds->Add(extra);
    if ((tree->Contains(extra) == false) 
      && (ds->GetNamedNode("extra") != extra))
    {
      std::cout << "[FAILED] - could not add extra node for this test" << std::endl;
      returnValue = EXIT_FAILURE;
    }
    else
    {
      ds->Remove(extra);
      if ((ds->GetNamedNode("extra") == NULL) && (refCountbeforeDS == watcher->GetReferenceCount()))
      {
        std::cout<<"[PASSED]"<<std::endl;
      }
      else
      {
        std::cout << "[FAILED] - extra node is still in Tree/DataStorage" << std::endl;
        returnValue = EXIT_FAILURE;
      }
      extra = NULL;
    }
  }
  catch(...)
  {
    std::cout<<"[FAILED] - Exception thrown" << std::endl;
    returnValue = EXIT_FAILURE;
  }

  /* Checking removal of a node with a parent */
  std::cout << "Checking removal of a node with a parent: " << std::flush;
  try
  {
    mitk::DataTreeNode::Pointer extra = mitk::DataTreeNode::New();
    extra->SetProperty("name", new mitk::StringProperty("extra"));
   
    mitk::ReferenceCountWatcher::Pointer watcher = new mitk::ReferenceCountWatcher(extra);
    int refCountbeforeDS = watcher->GetReferenceCount();
    ds->Add(extra, n1);   // n1 is parent of extra
    
    if ((ds->GetNamedNode("extra") != extra)
      && (ds->GetDerivations(n1)->Size() != 2))   // n2 and extra should be derived from n1
    {
      std::cout << "[FAILED] - could not add extra node for this test" << std::endl;
      returnValue = EXIT_FAILURE;
    }
    else
    {
      ds->Remove(extra);
      
      if ( (ds->GetNamedNode("extra") == NULL)
        && (refCountbeforeDS == watcher->GetReferenceCount())
        && (ds->GetDerivations(n1)->Size() == 1))   // after remove, only n2 should be derived from n1
      {
        std::cout<<"[PASSED]"<<std::endl;
      }
      else
      {
        std::cout << "[FAILED] - extra node is still in Tree/DataStorage" << std::endl;
        returnValue = EXIT_FAILURE;
      }
      extra = NULL;
    }
  }
  catch(...)
  {
    std::cout<<"[FAILED] - Exception thrown" << std::endl;
    returnValue = EXIT_FAILURE;
  }  

 /* Checking removal of a node with two parents */
  std::cout << "Checking removal of a node with two parents: " << std::flush;
  try
  {
    mitk::DataTreeNode::Pointer extra = mitk::DataTreeNode::New();
    extra->SetProperty("name", new mitk::StringProperty("extra"));
   
    mitk::ReferenceCountWatcher::Pointer watcher = new mitk::ReferenceCountWatcher(extra);
    int refCountbeforeDS = watcher->GetReferenceCount();
    mitk::DataStorage::SetOfObjects::Pointer p = mitk::DataStorage::SetOfObjects::New();
    p->push_back(n1);
    p->push_back(n2);
    ds->Add(extra, p);   // n1 and n2 are parents of extra
    
    if ( (ds->GetNamedNode("extra") != extra)
      && (ds->GetDerivations(n1)->Size() != 2)    // n2 and extra should be derived from n1
      && (ds->GetDerivations(n2)->Size() != 3))   // n3, n4 and extra should be derived from n2
    {
      std::cout << "[FAILED] - could not add extra node for this test" << std::endl;
      returnValue = EXIT_FAILURE;
    }
    else
    {
      ds->Remove(extra);
      
      if ( (ds->GetNamedNode("extra") == NULL)
        && (refCountbeforeDS == watcher->GetReferenceCount())
        && (ds->GetDerivations(n1)->Size() == 1)   // after remove, only n2 should be derived from n1
        && (ds->GetDerivations(n2)->Size() == 2))   // after remove, only n3 and n4 should be derived from n2
      {
        std::cout<<"[PASSED]"<<std::endl;
      }
      else
      {
        std::cout << "[FAILED] - extra node is still in Tree/DataStorage" << std::endl;
        returnValue = EXIT_FAILURE;
      }
      extra = NULL;
    }
  }
  catch(...)
  {
    std::cout<<"[FAILED] - Exception thrown" << std::endl;
    returnValue = EXIT_FAILURE;
  }  

 /* Checking removal of a node with two derived nodes */
  std::cout << "Checking removal of a node with two derived nodes: " << std::flush;
  try
  {
    mitk::DataTreeNode::Pointer extra = mitk::DataTreeNode::New();
    extra->SetProperty("name", new mitk::StringProperty("extra"));
    mitk::ReferenceCountWatcher::Pointer watcher = new mitk::ReferenceCountWatcher(extra);
    int refCountbeforeDS = watcher->GetReferenceCount();
    ds->Add(extra);
    
    mitk::DataTreeNode::Pointer d1 = mitk::DataTreeNode::New();
    d1->SetProperty("name", new mitk::StringProperty("d1"));
    ds->Add(d1, extra);
    mitk::DataTreeNode::Pointer d2 = mitk::DataTreeNode::New();
    d2->SetProperty("name", new mitk::StringProperty("d2"));    
    ds->Add(d2, extra);
    

    if ( (ds->GetNamedNode("extra") != extra)
      && (ds->GetNamedNode("d1") != d1)
      && (ds->GetNamedNode("d2") != d2)
      && (ds->GetSources(d1)->Size() != 1)    // extra should be source of d1
      && (ds->GetSources(d2)->Size() != 1)    // extra should be source of d2
      && (ds->GetDerivations(extra)->Size() != 2))    // d1 and d2 should be derived from extra
    {
      std::cout << "[FAILED] - could not add nodes for this test" << std::endl;
      returnValue = EXIT_FAILURE;
    }
    else
    {
      ds->Remove(extra);
      
      if ( (ds->GetNamedNode("extra") == NULL)
        && (ds->GetNamedNode("d1") == d1)                   // HIER GEHTS WEITER
        && (ds->GetNamedNode("d2") == d2)
        && (refCountbeforeDS == watcher->GetReferenceCount())
        && (ds->GetSources(d1)->Size() == 0)   // after remove, d1 should not have a source anymore
        && (ds->GetSources(d2)->Size() == 0))   // after remove, d2 should not have a source anymore
      {
        std::cout<<"[PASSED]"<<std::endl;
      }
      else
      {
        std::cout << "[FAILED] - extra node is still in Tree/DataStorage" << std::endl;
        returnValue = EXIT_FAILURE;
      }
      extra = NULL;
    }
  }
  catch(...)
  {
    std::cout<<"[FAILED] - Exception thrown" << std::endl;
    returnValue = EXIT_FAILURE;
  }  

 /* Checking removal of a node with two parents and two derived nodes */
  std::cout << "Checking removal of a node with two parents and two derived nodes: " << std::flush;
  try
  {
    mitk::DataTreeNode::Pointer extra = mitk::DataTreeNode::New();
    extra->SetProperty("name", new mitk::StringProperty("extra"));
    mitk::ReferenceCountWatcher::Pointer watcher = new mitk::ReferenceCountWatcher(extra);
    mitk::ReferenceCountWatcher::Pointer n1watcher = new mitk::ReferenceCountWatcher(n1);
    int refCountbeforeDS = watcher->GetReferenceCount();
    // int n1refCountbeforeDS = n1watcher->GetReferenceCount();

    mitk::DataStorage::SetOfObjects::Pointer p = mitk::DataStorage::SetOfObjects::New();
    p->push_back(n1);
    p->push_back(n2);
    ds->Add(extra, p);   // n1 and n2 are parents of extra

    mitk::DataTreeNode::Pointer d1 = mitk::DataTreeNode::New();
    d1->SetProperty("name", new mitk::StringProperty("d1x"));
    ds->Add(d1, extra);
    mitk::DataTreeNode::Pointer d2 = mitk::DataTreeNode::New();
    d2->SetProperty("name", new mitk::StringProperty("d2x"));    
    ds->Add(d2, extra);

    if ( (ds->GetNamedNode("extra") != extra)
      && (ds->GetNamedNode("d1x") != d1)
      && (ds->GetNamedNode("d2x") != d2)
      && (ds->GetSources(d1)->Size() != 1)    // extra should be source of d1
      && (ds->GetSources(d2)->Size() != 1)    // extra should be source of d2
      && (ds->GetDerivations(n1)->Size() != 2)    // n2 and extra should be derived from n1
      && (ds->GetDerivations(n2)->Size() != 3)   // n3, n4 and extra should be derived from n2
      && (ds->GetDerivations(extra)->Size() != 2))    // d1 and d2 should be derived from extra
    {
      std::cout << "[FAILED] - could not add nodes for this test" << std::endl;
      returnValue = EXIT_FAILURE;
    }
    else
    {
      ds->Remove(extra);

      if ( (ds->GetNamedNode("extra") == NULL)
        && (ds->GetNamedNode("d1x") == d1)
        && (ds->GetNamedNode("d2x") == d2)
        && (refCountbeforeDS == watcher->GetReferenceCount())
        && (ds->GetDerivations(n1)->Size() == 1)    // after remove, only n2 should be derived from n1
        && (ds->GetDerivations(n2)->Size() == 2)    // after remove, only n3 and n4 should be derived from n2
        && (ds->GetSources(d1)->Size() == 0)        // after remove, d1 should not have a source anymore
        && (ds->GetSources(d2)->Size() == 0))       // after remove, d2 should not have a source anymore
      {
        std::cout<<"[PASSED]"<<std::endl;
      }
      else
      {
        std::cout << "[FAILED] - extra node is still in Tree/DataStorage" << std::endl;
        returnValue = EXIT_FAILURE;
      }
      extra = NULL;
    }
  }
  catch(...)
  {
    std::cout<<"[FAILED] - Exception thrown" << std::endl;
    returnValue = EXIT_FAILURE;
  }  

 /* Checking for node is it's own parent exception */
  std::cout << "Checking for 'node is it's own parent' exception: " << std::flush;
  try
  {
    mitk::DataTreeNode::Pointer extra = mitk::DataTreeNode::New();
    extra->SetProperty("name", new mitk::StringProperty("extra"));

    mitk::DataStorage::SetOfObjects::Pointer p = mitk::DataStorage::SetOfObjects::New();
    p->push_back(n1);
    p->push_back(extra); // extra is parent of extra!!!
    ds->Add(extra, p); 

    if ( (ds->GetNamedNode("extra") == extra) && (ds->GetNamedDerivedNode("extra", extra) == extra))
    {
      std::cout << "[FAILED] - node was added, no exception was thrown " << std::endl;
      returnValue = EXIT_FAILURE;
    }
    else
    {
      std::cout << "[FAILED] - node was not added but no exception was thrown " << std::endl;
      returnValue = EXIT_FAILURE;
    }
  }
  catch(...)
  {
    std::cout<<"[PASSED] - Exception thrown as expected" << std::endl;
  }


 /* Checking reference count of node after add and remove */
  std::cout << "Checking reference count of node after add and remove: " << std::flush;
  try
  {
    mitk::DataTreeNode::Pointer extra = mitk::DataTreeNode::New();
    mitk::ReferenceCountWatcher::Pointer watcher = new mitk::ReferenceCountWatcher(extra);
    extra->SetProperty("name", new mitk::StringProperty("extra"));
    mitk::DataStorage::SetOfObjects::Pointer p = mitk::DataStorage::SetOfObjects::New();
    p->push_back(n1);
    p->push_back(n3); 
    ds->Add(extra, p); 
    extra = NULL;
    ds->Remove(ds->GetNamedNode("extra"));

    if (watcher->GetReferenceCount() == 0)
    {
      std::cout<<"[PASSED]"<<std::endl;
    }
    else
    {
      std::cout << "[FAILED] - reference count is " << watcher->GetReferenceCount() << ", but 0 was expected" << std::endl;
      returnValue = EXIT_FAILURE;
    }
  }
  catch(...)
  {
    std::cout<<"[FAILED] - Exception thrown" << std::endl;
    returnValue = EXIT_FAILURE;
  }  

 /* Checking GetGrouptags() */
  std::cout << "Checking GetGrouptags(): " << std::flush;
  try
  {
    const std::set<std::string> groupTags = ds->GetGroupTags();

    if ( (groupTags.size() == 2)
      && (std::find(groupTags.begin(), groupTags.end(), "Resection Proposal 1") != groupTags.end())
      && (std::find(groupTags.begin(), groupTags.end(), "Resection Proposal 2") != groupTags.end()))
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


 /* Checking DataTree Delete Oberver funtionality */
  std::cout << "Checking DataTree Delete Oberver funtionality: " << std::flush;
  try
  {
    mitk::DataTreeNode::Pointer extra = mitk::DataTreeNode::New();
    mitk::ReferenceCountWatcher::Pointer watcher = new mitk::ReferenceCountWatcher(extra);
    ds->Add(extra, n5); // add extra to DataStorage. Reference count should be 5 (extra smartpointer, tree, sources map, derivations map, derivations list of n5)
    mitk::DataTreeIteratorClone it = mitk::DataTreeHelper::FindIteratorToNode(tree, extra); // remove extra directly from tree
    it->Disconnect(); // delete node directly from tree. the observer mechanism should delete it from the internal relations too
    extra = NULL; // delete last reference to the node. its memory should be freed now

    if (watcher->GetReferenceCount() == 0)
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
