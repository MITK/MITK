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

#include <fstream>
#include <algorithm>

#include "mitkImage.h"
#include "mitkSurface.h"
#include "mitkStringProperty.h"
#include "mitkColorProperty.h"
#include "mitkGroupTagProperty.h"
#include "mitkDataNode.h"
#include "mitkReferenceCountWatcher.h"

#include "mitkDataStorage.h"
#include "mitkStandaloneDataStorage.h"
#include "mitkNodePredicateProperty.h"
#include "mitkNodePredicateDataType.h"
#include "mitkNodePredicateDimension.h"
#include "mitkNodePredicateData.h"
#include "mitkNodePredicateNOT.h"
#include "mitkNodePredicateAND.h"
#include "mitkNodePredicateOR.h"
#include "mitkNodePredicateSource.h"
#include "mitkMessage.h"

#include "mitkTestingMacros.h"


void TestDataStorage(mitk::DataStorage* ds);

class DSEventReceiver // Helper class for event testing
{
public:
  const mitk::DataNode* m_NodeAdded;
  const mitk::DataNode* m_NodeRemoved;

  DSEventReceiver()
    : m_NodeAdded(NULL), m_NodeRemoved(NULL)
  {
  }

  void OnAdd(const mitk::DataNode* node)
  {
    m_NodeAdded = node;
  }

  void OnRemove(const mitk::DataNode* node)
  {
    m_NodeRemoved = node;
  }
};


//## Documentation
//## main testing method
//## NOTE: the current Singleton implementation of DataTreeStorage will lead to crashes if a testcase fails
//##       and therefore mitk::DataStorage::ShutdownSingleton() is not called.
int mitkDataStorageTest(int /*argc*/, char* /*argv*/[])
{
  MITK_TEST_BEGIN("DataStorageTest");

  /* Create StandaloneDataStorage */
  MITK_TEST_OUTPUT( << "Create StandaloneDataStorage : ");
  mitk::StandaloneDataStorage::Pointer sds;
  try
  {
    sds = mitk::StandaloneDataStorage::New();
    MITK_TEST_CONDITION_REQUIRED(sds.IsNotNull(), "Testing Instatiation");
  }
  catch (...)
  {
    MITK_TEST_FAILED_MSG( << "Exception during creation of StandaloneDataStorage");
  }

  MITK_TEST_OUTPUT( << "Testing StandaloneDataStorage: ");
  TestDataStorage(sds);
  // TODO: Add specific StandaloneDataStorage Tests here
  sds = NULL;

  MITK_TEST_END();
}

//##Documentation
//## @brief Test for the DataStorage class and its associated classes (e.g. the predicate classes)
//## This method will be called once for each subclass of DataStorage
void TestDataStorage( mitk::DataStorage* ds )
{
  /* DataStorage valid? */
  MITK_TEST_CONDITION_REQUIRED(ds != NULL, "DataStorage valid?");

  // create some DataNodes to fill the ds
  mitk::DataNode::Pointer n1 = mitk::DataNode::New();   // node with image and name property
  mitk::Image::Pointer image = mitk::Image::New();
  unsigned int imageDimensions[] = { 10, 10, 10, 10 };
  mitk::PixelType pt(typeid(int));
  image->Initialize( pt, 4, imageDimensions );
  n1->SetData(image);
  n1->SetProperty("name", mitk::StringProperty::New("Node 1 - Image Node"));
  mitk::DataStorage::SetOfObjects::Pointer parents1 = mitk::DataStorage::SetOfObjects::New();

  mitk::DataNode::Pointer n2 = mitk::DataNode::New();   // node with surface and name and color properties
  mitk::Surface::Pointer surface = mitk::Surface::New();
  n2->SetData(surface);
  n2->SetProperty("name", mitk::StringProperty::New("Node 2 - Surface Node"));
  mitk::Color color;  color.Set(1.0f, 1.0f, 0.0f);
  n2->SetColor(color);
  n2->SetProperty("Resection Proposal 1", mitk::GroupTagProperty::New());
  mitk::DataStorage::SetOfObjects::Pointer parents2 = mitk::DataStorage::SetOfObjects::New();
  parents2->InsertElement(0, n1);  // n1 (image node) is source of n2 (surface node)

  mitk::DataNode::Pointer n3 = mitk::DataNode::New();   // node without data but with name property
  n3->SetProperty("name", mitk::StringProperty::New("Node 3 - Empty Node"));
  n3->SetProperty("Resection Proposal 1", mitk::GroupTagProperty::New());
  n3->SetProperty("Resection Proposal 2", mitk::GroupTagProperty::New());
  mitk::DataStorage::SetOfObjects::Pointer parents3 = mitk::DataStorage::SetOfObjects::New();
  parents3->InsertElement(0, n2);  // n2 is source of n3

  mitk::DataNode::Pointer n4 = mitk::DataNode::New();   // node without data but with color property
  n4->SetColor(color);
  n4->SetProperty("Resection Proposal 2", mitk::GroupTagProperty::New());
  mitk::DataStorage::SetOfObjects::Pointer parents4 = mitk::DataStorage::SetOfObjects::New();
  parents4->InsertElement(0, n2);
  parents4->InsertElement(1, n3);  // n2 and n3 are sources of n4

  mitk::DataNode::Pointer n5 = mitk::DataNode::New();   // extra node
  n5->SetProperty("name", mitk::StringProperty::New("Node 5"));

  try /* adding objects */
  {
    /* Add an object */
    ds->Add(n1, parents1);
    MITK_TEST_CONDITION_REQUIRED((ds->GetAll()->Size() == 1) && (ds->GetAll()->GetElement(0) == n1), "Testing Adding a new object");

    /* Check exception on adding the same object again */
    MITK_TEST_OUTPUT( << "Check exception on adding the same object again: ");
    MITK_TEST_FOR_EXCEPTION(..., ds->Add(n1, parents1));
    MITK_TEST_CONDITION(ds->GetAll()->Size() == 1, "Test if object count is correct after exception");

    /* Add an object that has a source object */
    ds->Add(n2, parents2);
    MITK_TEST_CONDITION_REQUIRED(ds->GetAll()->Size() == 2, "Testing Adding an object that has a source object");

    /* Add some more objects needed for further tests */
    ds->Add(n3, parents3);   // n3 object that has name property and one parent
    ds->Add(n4, parents4);   // n4 object that has color property
    ds->Add(n5);             // n5 has no parents
    MITK_TEST_CONDITION_REQUIRED(ds->GetAll()->Size() == 5, "Adding some more objects needed for further tests");
  }
  catch(...)
  {
    MITK_TEST_FAILED_MSG( << "Exeption during object creation");
  }

  try  /* object retrieval methods */
  {
    /* Requesting all Objects */
    {
      const mitk::DataStorage::SetOfObjects::ConstPointer all = ds->GetAll();
      std::vector<mitk::DataNode::Pointer> stlAll = all->CastToSTLConstContainer();

      MITK_TEST_CONDITION(
        (stlAll.size() == 5)  // check if all tree nodes are in resultset
        && (std::find(stlAll.begin(), stlAll.end(), n1) != stlAll.end()) && (std::find(stlAll.begin(), stlAll.end(), n2) != stlAll.end())
        && (std::find(stlAll.begin(), stlAll.end(), n3) != stlAll.end()) && (std::find(stlAll.begin(), stlAll.end(), n4) != stlAll.end())
        && (std::find(stlAll.begin(), stlAll.end(), n5) != stlAll.end()),
        "Testing GetAll()"
        );
    }
    /* Requesting a named object */
    {
      mitk::NodePredicateProperty::Pointer predicate(mitk::NodePredicateProperty::New("name", mitk::StringProperty::New("Node 2 - Surface Node")));
      mitk::DataStorage::SetOfObjects::ConstPointer all = ds->GetSubset(predicate);
      MITK_TEST_CONDITION((all->Size() == 1) && (all->GetElement(0) == n2), "Requesting a named object");
    }

    /* Requesting objects of specific data type */
    {
      mitk::NodePredicateDataType::Pointer predicate(mitk::NodePredicateDataType::New("Image"));
      mitk::DataStorage::SetOfObjects::ConstPointer all = ds->GetSubset(predicate);
      MITK_TEST_CONDITION((all->Size() == 1) && (all->GetElement(0) == n1), "Requesting objects of specific data type")
    }
    /* Requesting objects of specific dimension */
    {
      mitk::NodePredicateDimension::Pointer predicate(mitk::NodePredicateDimension::New( 4 ));
      mitk::DataStorage::SetOfObjects::ConstPointer all = ds->GetSubset(predicate);
      MITK_TEST_CONDITION((all->Size() == 1) && (all->GetElement(0) == n1), "Requesting objects of specific dimension")
    }
    /* Requesting objects with specific data object */
    {
      mitk::NodePredicateData::Pointer predicate(mitk::NodePredicateData::New(image));
      mitk::DataStorage::SetOfObjects::ConstPointer all = ds->GetSubset(predicate);
      MITK_TEST_CONDITION((all->Size() == 1) && (all->GetElement(0) == n1), "Requesting objects with specific data object")
    }
    /* Requesting objects with NULL data */
    {
      mitk::NodePredicateData::Pointer predicate(mitk::NodePredicateData::New(NULL));
      mitk::DataStorage::SetOfObjects::ConstPointer all = ds->GetSubset(predicate);
      MITK_TEST_CONDITION(
        (all->Size() == 3)
        && (std::find(all->begin(), all->end(), n3) != all->end())
        && (std::find(all->begin(), all->end(), n4) != all->end())
        && (std::find(all->begin(), all->end(), n5) != all->end())
        , "Requesting objects with NULL data");
    }
    /* Requesting objects that meet a conjunction criteria */
    {
      mitk::NodePredicateDataType::Pointer p1 = mitk::NodePredicateDataType::New("Surface");
      mitk::NodePredicateProperty::Pointer p2 = mitk::NodePredicateProperty::New("color", mitk::ColorProperty::New(color));
      mitk::NodePredicateAND::Pointer predicate = mitk::NodePredicateAND::New();
      predicate->AddPredicate(p1);
      predicate->AddPredicate(p2);  // objects must be of datatype "Surface" and have red color (= n2)
      const mitk::DataStorage::SetOfObjects::ConstPointer all = ds->GetSubset(predicate);
      MITK_TEST_CONDITION((all->Size() == 1) && (all->GetElement(0) == n2), "Requesting objects that meet a conjunction criteria");
    }
    /* Requesting objects that meet a disjunction criteria */
    {
      mitk::NodePredicateDataType::Pointer p1(mitk::NodePredicateDataType::New("Image"));
      mitk::NodePredicateProperty::Pointer p2(mitk::NodePredicateProperty::New("color", mitk::ColorProperty::New(color)));
      mitk::NodePredicateOR::Pointer predicate = mitk::NodePredicateOR::New();
      predicate->AddPredicate(p1);
      predicate->AddPredicate(p2);  // objects must be of datatype "Surface" or have red color (= n1, n2, n4)
      const mitk::DataStorage::SetOfObjects::ConstPointer all = ds->GetSubset(predicate);
      MITK_TEST_CONDITION(
        (all->Size() == 3)
        && (std::find(all->begin(), all->end(), n1) != all->end())
        && (std::find(all->begin(), all->end(), n2) != all->end())
        && (std::find(all->begin(), all->end(), n4) != all->end()),
        "Requesting objects that meet a disjunction criteria");
    }
    /* Requesting objects that do not meet a criteria */
    {
      mitk::ColorProperty::Pointer cp = mitk::ColorProperty::New(color);
      mitk::NodePredicateProperty::Pointer proppred(mitk::NodePredicateProperty::New("color", cp));
      mitk::NodePredicateNOT::Pointer predicate(mitk::NodePredicateNOT::New(proppred));

      const mitk::DataStorage::SetOfObjects::ConstPointer all = ds->GetSubset(predicate);
      std::vector<mitk::DataNode::Pointer> stlAll = all->CastToSTLConstContainer();
      MITK_TEST_CONDITION(
        (all->Size() == 3) // check if correct objects are in resultset
        && (std::find(stlAll.begin(), stlAll.end(), n1) != stlAll.end())
        && (std::find(stlAll.begin(), stlAll.end(), n3) != stlAll.end())
        && (std::find(stlAll.begin(), stlAll.end(), n5) != stlAll.end()), "Requesting objects that do not meet a criteria");
    }

    /* Requesting *direct* source objects */
    {
      const mitk::DataStorage::SetOfObjects::ConstPointer all = ds->GetSources(n3, NULL, true); // Get direct parents of n3 (=n2)
      std::vector<mitk::DataNode::Pointer> stlAll = all->CastToSTLConstContainer();
      MITK_TEST_CONDITION(
        (all->Size() == 1) && (std::find(stlAll.begin(), stlAll.end(), n2) != stlAll.end()),
        "Requesting *direct* source objects");
    }

    /* Requesting *all* source objects */
    {
      const mitk::DataStorage::SetOfObjects::ConstPointer all = ds->GetSources(n3, NULL, false); // Get all parents of n3 (= n1 + n2)
      std::vector<mitk::DataNode::Pointer> stlAll = all->CastToSTLConstContainer();
      MITK_TEST_CONDITION(
        (all->Size() == 2)
        && (std::find(stlAll.begin(), stlAll.end(), n1) != stlAll.end())
        && (std::find(stlAll.begin(), stlAll.end(), n2) != stlAll.end()),
        "Requesting *all* source objects"); // check if n1 and n2 are the resultset
    }

    /* Requesting *all* sources of object with multiple parents */
    {
      const mitk::DataStorage::SetOfObjects::ConstPointer all = ds->GetSources(n4, NULL, false); // Get all parents of n4 (= n1 + n2 + n3)
      std::vector<mitk::DataNode::Pointer> stlAll = all->CastToSTLConstContainer();
      MITK_TEST_CONDITION(
        (all->Size() == 3)
        && (std::find(stlAll.begin(), stlAll.end(), n1) != stlAll.end())
        && (std::find(stlAll.begin(), stlAll.end(), n2) != stlAll.end())
        && (std::find(stlAll.begin(), stlAll.end(), n3) != stlAll.end()) // check if n1 and n2 and n3 are the resultset
        , "Requesting *all* sources of object with multiple parents");
    }

    /* Requesting *direct* derived objects */
    {
      const mitk::DataStorage::SetOfObjects::ConstPointer all = ds->GetDerivations(n1, NULL, true); // Get direct childs of n1 (=n2)
      std::vector<mitk::DataNode::Pointer> stlAll = all->CastToSTLConstContainer();
      MITK_TEST_CONDITION(
        (all->Size() == 1)
        && (std::find(stlAll.begin(), stlAll.end(), n2) != stlAll.end())// check if n1 is the resultset
        , "Requesting *direct* derived objects");

    }

    ///* Requesting *direct* derived objects with multiple parents/derivations */

    {
      const mitk::DataStorage::SetOfObjects::ConstPointer all = ds->GetDerivations(n2, NULL, true); // Get direct childs of n2 (=n3 + n4)
      std::vector<mitk::DataNode::Pointer> stlAll = all->CastToSTLConstContainer();
      MITK_TEST_CONDITION(
        (all->Size() == 2)
        && (std::find(stlAll.begin(), stlAll.end(), n3) != stlAll.end()) // check if n3 is the resultset
        && (std::find(stlAll.begin(), stlAll.end(), n4) != stlAll.end()) // check if n4 is the resultset
        , "Requesting *direct* derived objects with multiple parents/derivations");
    }

    //* Requesting *all* derived objects */
    {
      const mitk::DataStorage::SetOfObjects::ConstPointer all = ds->GetDerivations(n1, NULL, false); // Get all childs of n1 (=n2, n3, n4)
      std::vector<mitk::DataNode::Pointer> stlAll = all->CastToSTLConstContainer();
      MITK_TEST_CONDITION(
        (all->Size() == 3)
        && (std::find(stlAll.begin(), stlAll.end(), n2) != stlAll.end())
        && (std::find(stlAll.begin(), stlAll.end(), n3) != stlAll.end())
        && (std::find(stlAll.begin(), stlAll.end(), n4) != stlAll.end())
        , "Requesting *all* derived objects");
    }

    /* Checking for circular source relationships */
    {
      parents1->InsertElement(0, n4);   // make n1 derived from n4 (which is derived from n2, which is derived from n1)
      const mitk::DataStorage::SetOfObjects::ConstPointer all = ds->GetSources(n4, NULL, false); // Get all parents of n4 (= n1 + n2 + n3, not n4 itself and not multiple versions of the nodes!)
      std::vector<mitk::DataNode::Pointer> stlAll = all->CastToSTLConstContainer();
      MITK_TEST_CONDITION(
        (all->Size() == 3)
        && (std::find(stlAll.begin(), stlAll.end(), n1) != stlAll.end())
        && (std::find(stlAll.begin(), stlAll.end(), n2) != stlAll.end())
        && (std::find(stlAll.begin(), stlAll.end(), n3) != stlAll.end()) // check if n1 and n2 and n3 are the resultset
        , "Checking for circular source relationships");
    }

    ///* Checking for circular derivation relationships can not be performed, because the internal derivations datastructure
    //   can not be accessed from the outside. (Therefore it should not be possible to create these circular relations */

    //* Checking GroupTagProperty */
    {
      mitk::GroupTagProperty::Pointer tp = mitk::GroupTagProperty::New();
      mitk::NodePredicateProperty::Pointer pred(mitk::NodePredicateProperty::New("Resection Proposal 1", tp));
      const mitk::DataStorage::SetOfObjects::ConstPointer all = ds->GetSubset(pred);
      std::vector<mitk::DataNode::Pointer> stlAll = all->CastToSTLConstContainer();
      MITK_TEST_CONDITION(
        (all->Size() == 2) // check if n2 and n3 are in resultset
        && (std::find(stlAll.begin(), stlAll.end(), n2) != stlAll.end())
        && (std::find(stlAll.begin(), stlAll.end(), n3) != stlAll.end())
        , "Checking GroupTagProperty");
    }

    /* Checking GroupTagProperty 2 */
    {
      mitk::GroupTagProperty::Pointer tp = mitk::GroupTagProperty::New();
      mitk::NodePredicateProperty::Pointer pred(mitk::NodePredicateProperty::New("Resection Proposal 2", tp));
      const mitk::DataStorage::SetOfObjects::ConstPointer all = ds->GetSubset(pred);
      std::vector<mitk::DataNode::Pointer> stlAll = all->CastToSTLConstContainer();
      MITK_TEST_CONDITION(
        (all->Size() == 2) // check if n3 and n4 are in resultset
        && (std::find(stlAll.begin(), stlAll.end(), n3) != stlAll.end())
        && (std::find(stlAll.begin(), stlAll.end(), n4) != stlAll.end())
        , "Checking GroupTagProperty 2");

    }

    /* Checking direct sources with condition */
    {
      mitk::NodePredicateDataType::Pointer pred = mitk::NodePredicateDataType::New("Surface");
      const mitk::DataStorage::SetOfObjects::ConstPointer all = ds->GetSources(n4, pred, true);
      std::vector<mitk::DataNode::Pointer> stlAll = all->CastToSTLConstContainer();
      MITK_TEST_CONDITION(
        (all->Size() == 1) // check if n2 is in resultset
        && (std::find(stlAll.begin(), stlAll.end(), n2) != stlAll.end())
        , "checking direct sources with condition");
    }

    /* Checking all sources with condition */
    {
      mitk::NodePredicateDataType::Pointer pred = mitk::NodePredicateDataType::New("Image");
      const mitk::DataStorage::SetOfObjects::ConstPointer all = ds->GetSources(n4, pred, false);
      std::vector<mitk::DataNode::Pointer> stlAll = all->CastToSTLConstContainer();
      MITK_TEST_CONDITION(
        (all->Size() == 1) // check if n1 is in resultset
        && (std::find(stlAll.begin(), stlAll.end(), n1) != stlAll.end())
        , "Checking all sources with condition");
    }

    /* Checking all sources with condition with empty resultset */
    {
      mitk::NodePredicateDataType::Pointer pred = mitk::NodePredicateDataType::New("VesselTree");
      const mitk::DataStorage::SetOfObjects::ConstPointer all = ds->GetSources(n4, pred, false);
      MITK_TEST_CONDITION(all->Size() == 0 , "Checking all sources with condition with empty resultset"); // check if resultset is empty
    }

    /* Checking direct derivations with condition */
    {
      mitk::NodePredicateProperty::Pointer pred = mitk::NodePredicateProperty::New("color");
      const mitk::DataStorage::SetOfObjects::ConstPointer all = ds->GetDerivations(n1, pred, true);
      std::vector<mitk::DataNode::Pointer> stlAll = all->CastToSTLConstContainer();
      MITK_TEST_CONDITION(
        (all->Size() == 1) // check if n2 is in resultset
        && (std::find(stlAll.begin(), stlAll.end(), n2) != stlAll.end())
        , "Checking direct derivations with condition");
    }

    /* Checking all derivations with condition */
    {
      mitk::NodePredicateProperty::Pointer pred = mitk::NodePredicateProperty::New("color");

      const mitk::DataStorage::SetOfObjects::ConstPointer all = ds->GetDerivations(n1, pred, false);
      std::vector<mitk::DataNode::Pointer> stlAll = all->CastToSTLConstContainer();
      MITK_TEST_CONDITION(
        (all->Size() == 2) // check if n2 and n4 are in resultset
        && (std::find(stlAll.begin(), stlAll.end(), n2) != stlAll.end())
        && (std::find(stlAll.begin(), stlAll.end(), n4) != stlAll.end())
        , "Checking direct derivations with condition");
    }

    /* Checking named node method */
    MITK_TEST_CONDITION(ds->GetNamedNode("Node 2 - Surface Node") == n2, "Checking named node method");
    MITK_TEST_CONDITION(ds->GetNamedNode(std::string("Node 2 - Surface Node")) == n2, "Checking named node(std::string) method");

    /* Checking named node method with wrong name */
    MITK_TEST_CONDITION(ds->GetNamedNode("This name does not exist") == NULL, "Checking named node method with wrong name");

    /* Checking named object method */
    MITK_TEST_CONDITION(ds->GetNamedObject<mitk::Image>("Node 1 - Image Node") == image, "Checking named object method");
    MITK_TEST_CONDITION(ds->GetNamedObject<mitk::Image>(std::string("Node 1 - Image Node")) == image, "Checking named object(std::string) method");

    /* Checking named object method with wrong DataType */
    MITK_TEST_CONDITION(ds->GetNamedObject<mitk::Surface>("Node 1 - Image Node") == NULL, "Checking named object method with wrong DataType");

    /* Checking named object method with wrong name */
    MITK_TEST_CONDITION(ds->GetNamedObject<mitk::Image>("This name does not exist") == NULL, "Checking named object method with wrong name");

    /* Checking GetNamedDerivedNode with valid name and direct derivation only */
    MITK_TEST_CONDITION(ds->GetNamedDerivedNode("Node 2 - Surface Node", n1, true) == n2, "Checking GetNamedDerivedNode with valid name & direct derivation only");

    /* Checking GetNamedDerivedNode with invalid Name and direct derivation only */
    MITK_TEST_CONDITION(ds->GetNamedDerivedNode("wrong name", n1, true) == NULL, "Checking GetNamedDerivedNode with invalid name & direct derivation only");

    /* Checking GetNamedDerivedNode with invalid Name and direct derivation only */
    MITK_TEST_CONDITION(ds->GetNamedDerivedNode("Node 3 - Empty Node", n1, false) == n3, "Checking GetNamedDerivedNode with invalid name & direct derivation only");


    /* Checking GetNamedDerivedNode with valid Name but direct derivation only */
    MITK_TEST_CONDITION(ds->GetNamedDerivedNode("Node 3 - Empty Node", n1, true) == NULL, "Checking GetNamedDerivedNode with valid Name but direct derivation only");

    /* Checking GetNode with valid predicate */
    {
      mitk::NodePredicateDataType::Pointer p(mitk::NodePredicateDataType::New("Image"));
      MITK_TEST_CONDITION(ds->GetNode(p) == n1, "Checking GetNode with valid predicate");
    }
    /* Checking GetNode with invalid predicate */
    {
      mitk::NodePredicateDataType::Pointer p(mitk::NodePredicateDataType::New("PointSet"));
      MITK_TEST_CONDITION(ds->GetNode(p) == NULL, "Checking GetNode with invalid predicate");
    }

  } // object retrieval methods
  catch(...)
  {
    MITK_TEST_FAILED_MSG( << "Exeption during object retrieval (GetXXX() Methods)");
  }

  try  /* object removal methods */
  {

    /* Checking removal of a node without relations */
    {
      mitk::DataNode::Pointer extra = mitk::DataNode::New();
      extra->SetProperty("name", mitk::StringProperty::New("extra"));
      mitk::ReferenceCountWatcher::Pointer watcher = new mitk::ReferenceCountWatcher(extra);
      int refCountbeforeDS = watcher->GetReferenceCount();
      ds->Add(extra);
      MITK_TEST_CONDITION(ds->GetNamedNode("extra") == extra, "Adding extra node");
      ds->Remove(extra);
      MITK_TEST_CONDITION(
        (ds->GetNamedNode("extra") == NULL)
        && (refCountbeforeDS == watcher->GetReferenceCount())
        , "Checking removal of a node without relations");
      extra = NULL;
    }

    /* Checking removal of a node with a parent */
    {
      mitk::DataNode::Pointer extra = mitk::DataNode::New();
      extra->SetProperty("name", mitk::StringProperty::New("extra"));

      mitk::ReferenceCountWatcher::Pointer watcher = new mitk::ReferenceCountWatcher(extra);
      int refCountbeforeDS = watcher->GetReferenceCount();
      ds->Add(extra, n1);   // n1 is parent of extra

      MITK_TEST_CONDITION(
        (ds->GetNamedNode("extra") == extra)
        && (ds->GetDerivations(n1)->Size() == 2)   // n2 and extra should be derived from n1
        , "Adding extra node");
      ds->Remove(extra);
      MITK_TEST_CONDITION(
        (ds->GetNamedNode("extra") == NULL)
        && (refCountbeforeDS == watcher->GetReferenceCount())
        && (ds->GetDerivations(n1)->Size() == 1)
        , "Checking removal of a node with a parent");
      extra = NULL;
    }

    /* Checking removal of a node with two parents */
    {
      mitk::DataNode::Pointer extra = mitk::DataNode::New();
      extra->SetProperty("name", mitk::StringProperty::New("extra"));

      mitk::ReferenceCountWatcher::Pointer watcher = new mitk::ReferenceCountWatcher(extra);
      int refCountbeforeDS = watcher->GetReferenceCount();
      mitk::DataStorage::SetOfObjects::Pointer p = mitk::DataStorage::SetOfObjects::New();
      p->push_back(n1);
      p->push_back(n2);
      ds->Add(extra, p);   // n1 and n2 are parents of extra


      MITK_TEST_CONDITION(
        (ds->GetNamedNode("extra") == extra)
        && (ds->GetDerivations(n1)->Size() == 2)    // n2 and extra should be derived from n1
        && (ds->GetDerivations(n2)->Size() == 3)
        , "add extra node");

      ds->Remove(extra);
      MITK_TEST_CONDITION(
        (ds->GetNamedNode("extra") == NULL)
        && (refCountbeforeDS == watcher->GetReferenceCount())
        && (ds->GetDerivations(n1)->Size() == 1)   // after remove, only n2 should be derived from n1
        && (ds->GetDerivations(n2)->Size() == 2)   // after remove, only n3 and n4 should be derived from n2
        , "Checking removal of a node with two parents");
      extra = NULL;
    }

    /* Checking removal of a node with two derived nodes */
    {
      mitk::DataNode::Pointer extra = mitk::DataNode::New();
      extra->SetProperty("name", mitk::StringProperty::New("extra"));
      mitk::ReferenceCountWatcher::Pointer watcher = new mitk::ReferenceCountWatcher(extra);
      int refCountbeforeDS = watcher->GetReferenceCount();
      ds->Add(extra);
      mitk::DataNode::Pointer d1 = mitk::DataNode::New();
      d1->SetProperty("name", mitk::StringProperty::New("d1"));
      ds->Add(d1, extra);
      mitk::DataNode::Pointer d2 = mitk::DataNode::New();
      d2->SetProperty("name", mitk::StringProperty::New("d2"));
      ds->Add(d2, extra);

      MITK_TEST_CONDITION(
        (ds->GetNamedNode("extra") == extra)
        && (ds->GetNamedNode("d1") == d1)
        && (ds->GetNamedNode("d2") == d2)
        && (ds->GetSources(d1)->Size() == 1)    // extra should be source of d1
        && (ds->GetSources(d2)->Size() == 1)    // extra should be source of d2
        && (ds->GetDerivations(extra)->Size() == 2)    // d1 and d2 should be derived from extra
        , "add extra node");

      ds->Remove(extra);
      MITK_TEST_CONDITION(
        (ds->GetNamedNode("extra") == NULL)
        && (ds->GetNamedNode("d1") == d1)
        && (ds->GetNamedNode("d2") == d2)
        && (refCountbeforeDS == watcher->GetReferenceCount())
        && (ds->GetSources(d1)->Size() == 0)   // after remove, d1 should not have a source anymore
        && (ds->GetSources(d2)->Size() == 0)   // after remove, d2 should not have a source anymore
        , "Checking removal of a node with two derived nodes");
      extra = NULL;
    }

    /* Checking removal of a node with two parents and two derived nodes */
    {
      mitk::DataNode::Pointer extra = mitk::DataNode::New();
      extra->SetProperty("name", mitk::StringProperty::New("extra"));
      mitk::ReferenceCountWatcher::Pointer watcher = new mitk::ReferenceCountWatcher(extra);
      mitk::ReferenceCountWatcher::Pointer n1watcher = new mitk::ReferenceCountWatcher(n1);
      int refCountbeforeDS = watcher->GetReferenceCount();

      mitk::DataStorage::SetOfObjects::Pointer p = mitk::DataStorage::SetOfObjects::New();
      p->push_back(n1);
      p->push_back(n2);
      ds->Add(extra, p);   // n1 and n2 are parents of extra

      mitk::DataNode::Pointer d1 = mitk::DataNode::New();
      d1->SetProperty("name", mitk::StringProperty::New("d1x"));
      ds->Add(d1, extra);
      mitk::DataNode::Pointer d2 = mitk::DataNode::New();
      d2->SetProperty("name", mitk::StringProperty::New("d2x"));
      ds->Add(d2, extra);

      MITK_TEST_CONDITION(
        (ds->GetNamedNode("extra") == extra)
        && (ds->GetNamedNode("d1x") == d1)
        && (ds->GetNamedNode("d2x") == d2)
        && (ds->GetSources(d1)->Size() == 1)    // extra should be source of d1
        && (ds->GetSources(d2)->Size() == 1)    // extra should be source of d2
        && (ds->GetDerivations(n1)->Size() == 2)    // n2 and extra should be derived from n1
        && (ds->GetDerivations(n2)->Size() == 3)   // n3, n4 and extra should be derived from n2
        && (ds->GetDerivations(extra)->Size() == 2)    // d1 and d2 should be derived from extra
        , "add extra node");

      ds->Remove(extra);
      MITK_TEST_CONDITION(
        (ds->GetNamedNode("extra") == NULL)
        && (ds->GetNamedNode("d1x") == d1)
        && (ds->GetNamedNode("d2x") == d2)
        && (refCountbeforeDS == watcher->GetReferenceCount())
        && (ds->GetDerivations(n1)->Size() == 1)    // after remove, only n2 should be derived from n1
        && (ds->GetDerivations(n2)->Size() == 2)    // after remove, only n3 and n4 should be derived from n2
        && (ds->GetSources(d1)->Size() == 0)        // after remove, d1 should not have a source anymore
        && (ds->GetSources(d2)->Size() == 0)       // after remove, d2 should not have a source anymore
        , "Checking removal of a node with two parents and two derived nodes");
      extra = NULL;
    }
  }
  catch(...)
  {
    MITK_TEST_FAILED_MSG( << "Exeption during object removal methods");
  }



  /* Checking for node is it's own parent exception */
  {
    MITK_TEST_FOR_EXCEPTION_BEGIN(...);
    mitk::DataNode::Pointer extra = mitk::DataNode::New();
    extra->SetProperty("name", mitk::StringProperty::New("extra"));
    mitk::DataStorage::SetOfObjects::Pointer p = mitk::DataStorage::SetOfObjects::New();
    p->push_back(n1);
    p->push_back(extra); // extra is parent of extra!!!
    ds->Add(extra, p);
    MITK_TEST_FOR_EXCEPTION_END(...);
  }


  /* Checking reference count of node after add and remove */
  {
    mitk::DataNode::Pointer extra = mitk::DataNode::New();
    mitk::ReferenceCountWatcher::Pointer watcher = new mitk::ReferenceCountWatcher(extra);
    extra->SetProperty("name", mitk::StringProperty::New("extra"));
    mitk::DataStorage::SetOfObjects::Pointer p = mitk::DataStorage::SetOfObjects::New();
    p->push_back(n1);
    p->push_back(n3);
    ds->Add(extra, p);
    extra = NULL;
    ds->Remove(ds->GetNamedNode("extra"));
    MITK_TEST_CONDITION(watcher->GetReferenceCount() == 0, "Checking reference count of node after add and remove");
  }

  /* Checking removal of a node with two derived nodes [ dataStorage->GetDerivations( rootNode )] see bug #3426 */
  {
    mitk::DataNode::Pointer extra = mitk::DataNode::New();
    extra->SetProperty("name", mitk::StringProperty::New("extra"));

    ds->Add(extra);
    mitk::DataNode::Pointer d1y = mitk::DataNode::New();
    d1y->SetProperty("name", mitk::StringProperty::New("d1y"));
    mitk::ReferenceCountWatcher::Pointer watcherD1y = new mitk::ReferenceCountWatcher(d1y);
    int refCountbeforeDS = watcherD1y->GetReferenceCount();
    ds->Add(d1y, extra);
    mitk::DataNode::Pointer d2y = mitk::DataNode::New();
    d2y->SetProperty("name", mitk::StringProperty::New("d2y"));
    ds->Add(d2y, extra);

    MITK_TEST_CONDITION(
      (ds->GetNamedNode("extra") == extra)
      && (ds->GetNamedNode("d1y") == d1y)
      && (ds->GetNamedNode("d2y") == d2y)
      && (ds->GetSources(d1y)->Size() == 1)    // extra should be source of d1y
      && (ds->GetSources(d2y)->Size() == 1)    // extra should be source of d2y
      && (ds->GetDerivations(extra)->Size() == 2)    // d1y and d2y should be derived from extra
      , "add extra node");

    ds->Remove(ds->GetDerivations( extra));       
    MITK_TEST_CONDITION(
      (ds->GetNamedNode("extra") == extra)
      && (ds->GetNamedNode("d1y") == NULL) // d1y should be NULL now
      && (ds->GetNamedNode("d2y") == NULL) // d2y should be NULL now
      && (refCountbeforeDS == watcherD1y->GetReferenceCount())      
      , "Checking removal of subset of two derived nodes from one parent node");

    ds->Remove(extra);
    MITK_TEST_CONDITION(
      (ds->GetNamedNode("extra") == NULL)
      , "Checking removal of a parent node");
    extra = NULL;
  }

  /* Checking GetGrouptags() */
  {
    const std::set<std::string> groupTags = ds->GetGroupTags();
    MITK_TEST_CONDITION(
      (groupTags.size() == 2)
      && (std::find(groupTags.begin(), groupTags.end(), "Resection Proposal 1") != groupTags.end())
      && (std::find(groupTags.begin(), groupTags.end(), "Resection Proposal 2") != groupTags.end())
      , "Checking GetGrouptags()");
  }


  /* Checking Event handling */
  DSEventReceiver listener;
  try
  {
    ds->AddNodeEvent += mitk::MessageDelegate1<DSEventReceiver, const mitk::DataNode*>(&listener, &DSEventReceiver::OnAdd);
    ds->RemoveNodeEvent += mitk::MessageDelegate1<DSEventReceiver, const mitk::DataNode*>(&listener, &DSEventReceiver::OnRemove);

    mitk::DataNode::Pointer extra = mitk::DataNode::New();
    mitk::ReferenceCountWatcher::Pointer watcher = new mitk::ReferenceCountWatcher(extra);
    ds->Add(extra);

    MITK_TEST_CONDITION(listener.m_NodeAdded == extra.GetPointer(), "Checking AddEvent");

    ds->Remove(extra);
    MITK_TEST_CONDITION(listener.m_NodeRemoved == extra.GetPointer(), "Checking RemoveEvent");

    /* RemoveListener */
    ds->AddNodeEvent -= mitk::MessageDelegate1<DSEventReceiver, const mitk::DataNode*>(&listener, &DSEventReceiver::OnAdd);
    ds->RemoveNodeEvent -= mitk::MessageDelegate1<DSEventReceiver, const mitk::DataNode*>(&listener, &DSEventReceiver::OnRemove);
    listener.m_NodeAdded = NULL;
    listener.m_NodeRemoved = NULL;
    ds->Add(extra);
    ds->Remove(extra);
    MITK_TEST_CONDITION((listener.m_NodeRemoved == NULL) && (listener.m_NodeAdded == NULL), "Checking RemoveListener");


    std::cout << "Pointer handling after event handling: " << std::flush;
    extra = NULL; // delete reference to the node. its memory should be freed now
    MITK_TEST_CONDITION(watcher->GetReferenceCount() == 0, "Pointer handling after event handling");
  }
  catch(...)
  {
    /* cleanup */
    ds->AddNodeEvent -= mitk::MessageDelegate1<DSEventReceiver, const mitk::DataNode*>(&listener, &DSEventReceiver::OnAdd);
    ds->RemoveNodeEvent -= mitk::MessageDelegate1<DSEventReceiver, const mitk::DataNode*>(&listener, &DSEventReceiver::OnRemove);
    MITK_TEST_FAILED_MSG( << "Exception during object removal methods");
  }

  //Checking ComputeBoundingGeometry3D method*/
  const mitk::DataStorage::SetOfObjects::ConstPointer all = ds->GetAll();
  mitk::TimeSlicedGeometry::Pointer geometry = ds->ComputeBoundingGeometry3D();
  MITK_TEST_CONDITION(geometry->GetTimeSteps()==10, "Test for number or time steps with ComputeBoundingGeometry()");
  mitk::TimeBounds timebounds = geometry->GetTimeBounds();
  MITK_TEST_CONDITION((timebounds[0]==0)&&(timebounds[1]==10),"Test for timebounds with ComputeBoundingGeometry()");
  for (unsigned int i=0; i<geometry->GetTimeSteps(); i++)
  {
    mitk::Geometry3D::Pointer subGeometry = geometry->GetGeometry3D(i);
    mitk::TimeBounds bounds = subGeometry->GetTimeBounds();
    MITK_TEST_CONDITION((bounds[0]==i)&&(bounds[1]==i+1),"Test for timebounds of geometry at different time steps with ComputeBoundingGeometry()");
  }
  geometry = ds->ComputeBoundingGeometry3D(all);
  MITK_TEST_CONDITION(geometry->GetTimeSteps()==10, "Test for number or time steps with ComputeBoundingGeometry(allNodes)");
  timebounds = geometry->GetTimeBounds();
  MITK_TEST_CONDITION((timebounds[0]==0)&&(timebounds[1]==10),"Test for timebounds with ComputeBoundingGeometry(allNodes)");
  for (unsigned int i=0; i<geometry->GetTimeSteps(); i++)
  {
    mitk::Geometry3D::Pointer subGeometry = geometry->GetGeometry3D(i);
    mitk::TimeBounds bounds = subGeometry->GetTimeBounds();
    MITK_TEST_CONDITION((bounds[0]==i)&&(bounds[1]==i+1),"Test for timebounds of geometry at different time steps with ComputeBoundingGeometry()");
  }


  /* Clear DataStorage */
  ds->Remove(ds->GetAll());
  MITK_TEST_CONDITION(ds->GetAll()->Size() == 0, "Checking Clear DataStorage");
}
