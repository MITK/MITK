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

#include <iostream>
#include <sstream>
#include "mitkDataTreeFilter.h"
#include "mitkDataTreeFilterEvents.h"
#include "mitkDataTreeFilterFunctions.h"
#include "mitkNodePredicateProperty.h"

#include <string>
#include <itkVectorContainer.h>

#include<mitkPointData.h> 
#include<mitkVector.h> 

#include <mitkImage.h>
#include <mitkDataTree.h>
#include <mitkRenderWindow.h>
#include <mitkImageMapper2D.h>
#include <mitkProperties.h>

// TODO how to get _all_ properties? (visible = []?) Ist das erwuenscht? Soll der data manager ersetzt werden? vielleicht eher nicht
// TODO DeleteSelected sinnvoll?
// TODO Views schreiben:
//       combobox: prop displayed_property (default "name")
//       listview: prop underdrueckte_properties
//       die propertyviews aus dem datamanager wiedervewrwenden?
// TODO  ExtendedSelect (mehrfache auswahl moeglich, aber nur items am stueck, ohne luecke)

#ifndef DOXYGEN_SKIP

#define TEST_ASSERT(x) \
if (!(x)) \
{\
  std::cerr << "Test condition '" #x "' failed." << std::endl; exit(EXIT_FAILURE);\
}

int verbose;

class EinsZwoFilterDing : public mitk::DataTreeFilterFunction
{
  public:
  
  bool NodeMatches( mitk::DataTreeNode* node ) const
    {
      std::string name;
      node->GetName( name );
      if ( name == "Eins" || name == "Zwo" )
        return true;
      else 
        return false;
    }

    DataTreeFilterFunction* Clone() const
    {
      return new EinsZwoFilterDing();
    }
}; 


class Test
{
  public:

    EinsZwoFilterDing m_EinsZwoFilter;

    Test()
    {
      std::cout << "Test constructor ... ";
      std::cout.flush();
    }

    ~Test()
    {
      std::cout << "Test destructor ... ";
      std::cout.flush();

      tree_filter = NULL; // release filter
      data_tree = NULL;  // release data tree
    }
    
    void prepare(int type)
    {
      switch (type)
      { // clear tree, clear filter
        case -1: {
          tree_filter = NULL;
          data_tree = NULL;
          data_tree = mitk::DataTree::New();
          tree_filter = mitk::DataTreeFilter::New(data_tree);
          // change from default to "pass nearly all" filter
          tree_filter->SetFilter( mitk::IsDataTreeNode() );
          // change from visible props default "name" to nothing
          mitk::DataTreeFilter::PropertyList visible_props;
          tree_filter->SetVisibleProperties(visible_props);
          break;
        }
        case -2: {
          // create some test data
          mitk::Image::Pointer image;
          mitk::PixelType pt(typeid(int));
          unsigned int dim[]={10,10,20}; // image dimensions

          image = mitk::Image::New();
          image->Initialize(mitk::PixelType(typeid(int)), 3, dim);
          int *p = (int*)image->GetData(); // pointer to pixel data
          int size = dim[0]*dim[1]*dim[2];
          for(int i=0; i<size; ++i, ++p) *p=i; // fill image
        
          {
            mitk::DataTreeNode::Pointer node = mitk::DataTreeNode::New();
            node->SetData(image);
          }
    
          mitk::DataTreePreOrderIterator it(data_tree);
  
          {
            mitk::DataTreeNode::Pointer node = mitk::DataTreeNode::New(); node->SetData(image);
            node->SetProperty("name", new mitk::StringProperty("Ruut"));
            it.Set(node);
          }
          {
            mitk::DataTreeNode::Pointer node = mitk::DataTreeNode::New(); node->SetData(image);
            node->SetProperty("name", new mitk::StringProperty("Eins"));
            node->SetProperty("visible", new mitk::BoolProperty(false));
            it.Add(node);
          }
          {
            mitk::DataTreeNode::Pointer node = mitk::DataTreeNode::New(); node->SetData(image);
            node->SetProperty("name", new mitk::StringProperty("Zwo"));
            it.Add(node);
          }
          break;
        } 
        case 0: { // tree ready, filter not initialized
          prepare(-1);
          prepare(-2);
          break;
        }
        case 1: { // tree ready, "visible" and "name" accessible
          prepare(0);

          mitk::DataTreeFilter::PropertyList visible_props;
          visible_props.push_back("visible");
          visible_props.push_back("name");
          tree_filter->SetVisibleProperties(visible_props);
          break;
        }
        case 2: { // tree ready, "visible" and "name" accessible
          prepare(1);

          mitk::DataTreeFilter::PropertyList editable_props;
          tree_filter->SetEditableProperties(editable_props);
          break;
        }
        case 3: { // no hierarchy
          prepare(-1);
          
          mitk::DataTreeFilter::PropertyList visible_props;
          visible_props.push_back("visible");
          visible_props.push_back("name");
          tree_filter->SetVisibleProperties(visible_props);
          
          tree_filter->SetHierarchyHandling( mitk::DataTreeFilter::FLATTEN_HIERARCHY );
          
          prepare(-2);

          break;
        }
        case 4: { // selection test (no props visible)
          prepare(-1);
          prepare(-2);

          // following line should not compile due to const correctness
          // tree_filter->GetItems()->ElementAt(0)->GetChildren()->ElementAt(1)->SetSelected(true);

          tree_filter->SelectItem( tree_filter->GetItems()->ElementAt(0)->GetChildren()->ElementAt(1) );

          break;
        }
        case 5: { // selection test (name visible)
          prepare(-1);
          
          mitk::DataTreeFilter::PropertyList visible_props;
          visible_props.push_back("name");
          tree_filter->SetVisibleProperties(visible_props);
          
          prepare(-2);

          tree_filter->SelectItem( tree_filter->GetItems()->ElementAt(0)->GetChildren()->ElementAt(1) );
          tree_filter->SelectItem( tree_filter->GetItems()->ElementAt(0) );

          break;
        }
        case 6: { // selection test (name visible), flat hierarchy

          prepare(-1);
          
          mitk::DataTreeFilter::PropertyList visible_props;
          visible_props.push_back("name");
          tree_filter->SetVisibleProperties(visible_props);
          
          tree_filter->SetHierarchyHandling( mitk::DataTreeFilter::FLATTEN_HIERARCHY );
          
          prepare(-2);

          // following line should not compile due to const correctness
          // tree_filter->GetItems()->ElementAt(0)->GetChildren()->ElementAt(1)->SetSelected(true);

          tree_filter->SelectItem( tree_filter->GetItems()->ElementAt(0) );
          tree_filter->SelectItem( tree_filter->GetItems()->ElementAt(2) );

          break;
        }
        case 7: {
          tree_filter->SelectItem( tree_filter->GetItems()->ElementAt(0), false );
          break;
        }
        case 8: {
          tree_filter->SelectItem( tree_filter->GetItems()->ElementAt(1), true );
          break;
        }
        case 9: { // install filter that lets only names "Eins" and "Zwo" pass
          tree_filter->SetFilter( m_EinsZwoFilter );
          break;
        }
        case 10: { // delete tree, then selections and all items should be removed
          data_tree->Clear();
          break;
        }
        case 11: {  // should give the same as 9 (hierarchy is different here)
          prepare(0);
          mitk::DataTreeFilter::PropertyList visible_props;
          visible_props.push_back("name");
          tree_filter->SetVisibleProperties(visible_props);
          tree_filter->SetFilter( m_EinsZwoFilter );
          break;
        }
        default: {
            // bla
          break;
        }
      }  // switch
    }

    void print_item_list(ostream& out, const mitk::DataTreeFilter::ItemList* list, int level)
    {
      mitk::DataTreeFilter::ConstItemIterator iter(list->Begin()); // copy constructor
      iter = list->Begin(); // assignment
      
      while ( iter != list->End() )
      {
        //const mitk::BaseProperty* prop = iter->GetProperty("name");
        const std::string name = iter->GetProperty("name");
        
        for (int i = 0; i < level; ++i) out << "    ";
        out << "+- " << name;

        if ( iter->IsSelected() ) out << "*";

        if (verbose > 1)
        {
          out << ": ";
          out << "IsRoot(" << iter->IsRoot() << ") ";
          out << "Index(" << (iter->GetIndex()!=0) << ") ";
          out << "HasChildren(" << (iter->HasChildren()!=0) << ") ";
          out << "Selected(" << iter->IsSelected() << ") ";
          out << "Address(" << (*iter).GetPointer() << ") ";
          out << "Parent(" << iter->GetParent() << ") ";
        }
        
        out << std::endl;
       
        // all available properties
        for( mitk::DataTreeFilter::PropertyList::const_iterator propiter = tree_filter->GetVisibleProperties().begin();
             propiter != tree_filter->GetVisibleProperties().end();
             ++propiter )
        {
          for (int i = 0; i < level; ++i) out << "    "; out << "|    ";
          out << *propiter << ": "
                    << (const std::string) iter->GetProperty(*propiter)  
                    << std::endl;
        }

        if ( iter->HasChildren() )
        {
          print_item_list( out, iter->GetChildren(), level + 1 );
        }
        
        ++iter;
      }
    }
    
    void check_filter(int type)
    {
      // following  line should not compile! Can this be tested using Dart?
      // mitk::DataTreeFilter::ItemList* items = tree_filter->GetItems();
      const mitk::DataTreeFilter::ItemList* items = tree_filter->GetItems();
      std::ostringstream ost;
      print_item_list( ost, items, 0 );

      if (verbose) std::cout << ost.str();
      
      std::ostringstream soll; // expected output
      switch (type)
      {
        case 0: {
soll <<
"+- Ruut" << std::endl <<
"    +- Eins" << std::endl <<
"    +- Zwo" << std::endl;
          TEST_ASSERT ( ost.str() == soll.str() || verbose > 1 );
      
          const mitk::DataTreeFilter::ItemSet* selectedItems = tree_filter->GetSelectedItems();
          TEST_ASSERT (selectedItems->empty());
          break;
        }
        case 1: {
soll <<
"+- Ruut" << std::endl <<
"|    visible: 1" << std::endl <<
"|    name: Ruut" << std::endl <<
"    +- Eins" << std::endl <<
"    |    visible: 0" << std::endl <<
"    |    name: Eins" << std::endl <<
"    +- Zwo" << std::endl <<
"    |    visible: 1" << std::endl <<
"    |    name: Zwo" << std::endl;
          TEST_ASSERT ( ost.str() == soll.str() || verbose > 1 );
          
          const mitk::DataTreeFilter::ItemSet* selectedItems = tree_filter->GetSelectedItems();
          TEST_ASSERT (selectedItems->empty());
          break;
        }
        case 2: {
soll <<
"+- Ruut" << std::endl <<
"|    visible: 1" << std::endl <<
"|    name: Ruut" << std::endl <<
"    +- Eins" << std::endl <<
"    |    visible: 0" << std::endl <<
"    |    name: Eins" << std::endl <<
"    +- Zwo" << std::endl <<
"    |    visible: 1" << std::endl <<
"    |    name: Zwo" << std::endl;
          TEST_ASSERT ( ost.str() == soll.str() || verbose > 1 );
          
          const mitk::DataTreeFilter::ItemSet* selectedItems = tree_filter->GetSelectedItems();
          TEST_ASSERT (selectedItems->empty());
          break;
        }
        case 3: {
soll <<
"+- Ruut" << std::endl <<
"|    visible: 1" << std::endl <<
"|    name: Ruut" << std::endl <<
"+- Eins" << std::endl <<
"|    visible: 0" << std::endl <<
"|    name: Eins" << std::endl <<
"+- Zwo" << std::endl <<
"|    visible: 1" << std::endl <<
"|    name: Zwo" << std::endl;
          TEST_ASSERT ( ost.str() == soll.str() || verbose > 1 );
          
          const mitk::DataTreeFilter::ItemSet* selectedItems = tree_filter->GetSelectedItems();
          TEST_ASSERT (selectedItems->empty());
          break;
        }
        case 4: {
soll <<
"+- Ruut" << std::endl <<
"    +- Eins" << std::endl <<
"    +- Zwo*" << std::endl;
          TEST_ASSERT ( ost.str() == soll.str() || verbose > 1 );
          
          const mitk::DataTreeFilter::ItemSet* selectedItems = tree_filter->GetSelectedItems();
          int goodSelectedItems(0);
          for (mitk::DataTreeFilter::ItemSet::const_iterator iter = selectedItems->begin(); iter != selectedItems->end(); ++iter)
          {
            if (static_cast<const std::string>((*iter)->GetProperty("name")) == std::string("Zwo")) ++goodSelectedItems;
          }
          TEST_ASSERT( goodSelectedItems == 1 );
          break;
        }
        case 5: {
soll <<
"+- Ruut*" << std::endl <<
"|    name: Ruut" << std::endl <<
"    +- Eins" << std::endl <<
"    |    name: Eins" << std::endl <<
"    +- Zwo*" << std::endl <<
"    |    name: Zwo" << std::endl;
          TEST_ASSERT ( ost.str() == soll.str() || verbose > 1 );
          
          const mitk::DataTreeFilter::ItemSet* selectedItems = tree_filter->GetSelectedItems();
          int goodSelectedItems(0);
          for (mitk::DataTreeFilter::ItemSet::const_iterator iter = selectedItems->begin(); iter != selectedItems->end(); ++iter)
          {
            if (static_cast<const std::string>((*iter)->GetProperty("name")) == std::string("Ruut")) ++goodSelectedItems;
            if (static_cast<const std::string>((*iter)->GetProperty("name")) == std::string("Zwo")) ++goodSelectedItems;
          }
          TEST_ASSERT( goodSelectedItems == 2 );
          break;
        }
        case 6: {
soll <<
"+- Ruut*" << std::endl <<
"|    name: Ruut" << std::endl <<
"+- Eins" << std::endl <<
"|    name: Eins" << std::endl <<
"+- Zwo*" << std::endl <<
"|    name: Zwo" << std::endl;
          TEST_ASSERT ( ost.str() == soll.str() || verbose > 1 );
          
          const mitk::DataTreeFilter::ItemSet* selectedItems = tree_filter->GetSelectedItems();
          int goodSelectedItems(0);
          for (mitk::DataTreeFilter::ItemSet::const_iterator iter = selectedItems->begin(); iter != selectedItems->end(); ++iter)
          {
            if (static_cast<const std::string>((*iter)->GetProperty("name")) == std::string("Ruut")) ++goodSelectedItems;
            if (static_cast<const std::string>((*iter)->GetProperty("name")) == std::string("Zwo")) ++goodSelectedItems;
          }
          TEST_ASSERT( goodSelectedItems == 2 );
          break;
        }
        case 7: {
soll <<
"+- Ruut" << std::endl <<
"|    name: Ruut" << std::endl <<
"+- Eins" << std::endl <<
"|    name: Eins" << std::endl <<
"+- Zwo*" << std::endl <<
"|    name: Zwo" << std::endl;
          TEST_ASSERT ( ost.str() == soll.str() || verbose > 1 );
          
          const mitk::DataTreeFilter::ItemSet* selectedItems = tree_filter->GetSelectedItems();
          int goodSelectedItems(0);
          for (mitk::DataTreeFilter::ItemSet::const_iterator iter = selectedItems->begin(); iter != selectedItems->end(); ++iter)
          {
            if (static_cast<const std::string>((*iter)->GetProperty("name")) == std::string("Zwo")) ++goodSelectedItems;
          }
          TEST_ASSERT( goodSelectedItems == 1 );
          break;
        }
        case 8: {
soll <<
"+- Ruut" << std::endl <<
"|    name: Ruut" << std::endl <<
"+- Eins*" << std::endl <<
"|    name: Eins" << std::endl <<
"+- Zwo*" << std::endl <<
"|    name: Zwo" << std::endl;
          TEST_ASSERT ( ost.str() == soll.str() || verbose > 1 );
          
          const mitk::DataTreeFilter::ItemSet* selectedItems = tree_filter->GetSelectedItems();
          int goodSelectedItems(0);
          for (mitk::DataTreeFilter::ItemSet::const_iterator iter = selectedItems->begin(); iter != selectedItems->end(); ++iter)
          {
            if (static_cast<const std::string>((*iter)->GetProperty("name")) == std::string("Eins")) ++goodSelectedItems;
            if (static_cast<const std::string>((*iter)->GetProperty("name")) == std::string("Zwo")) ++goodSelectedItems;
          }
          TEST_ASSERT( goodSelectedItems == 2 );
          break;
        }
        case 9: {
soll <<
"+- Eins*" << std::endl <<
"|    name: Eins" << std::endl <<
"+- Zwo*" << std::endl <<
"|    name: Zwo" << std::endl;
          TEST_ASSERT ( ost.str() == soll.str() || verbose > 1 );

          break;
        }
        case 10: {
          const mitk::DataTreeFilter::ItemSet* selectedItems = tree_filter->GetSelectedItems();
          TEST_ASSERT( selectedItems->empty() );
          TEST_ASSERT( tree_filter->GetItems()->empty() );
          break;
        }
        case 11: {
soll <<
"+- Eins" << std::endl <<
"|    name: Eins" << std::endl <<
"+- Zwo" << std::endl <<
"|    name: Zwo" << std::endl;
          TEST_ASSERT ( ost.str() == soll.str() || verbose > 1 );

          break;
        }
        default: {
            // bla
          break;
        }
      }

    }

  private:
    mitk::DataTree::Pointer data_tree;
    mitk::DataTreeFilter::Pointer tree_filter;
};
 
void smart_pointer_vector_container_test()
{

  typedef itk::SmartPointerVectorContainer<unsigned int,mitk::PointData>          Container;
  typedef itk::SmartPointerVectorContainer<unsigned int,mitk::PointData>::Pointer ContainerPointer;

  {
  ContainerPointer nix;
  TEST_ASSERT ( nix.IsNull() );
  }

  // test simple construction/destruction
  ContainerPointer vec = Container::New(); 
  TEST_ASSERT ( vec.IsNotNull() );
  
  vec = NULL;
  TEST_ASSERT ( vec.IsNull() );
 
  // test sizing
  vec = Container::New(); 
  TEST_ASSERT ( vec.IsNotNull() );
 
#define NUM_ELEMENTS 500
  vec->Reserve(NUM_ELEMENTS);
  TEST_ASSERT ( vec->Size() == NUM_ELEMENTS );
  vec = NULL;
  TEST_ASSERT ( vec.IsNull() );
  vec = Container::New(); 
  TEST_ASSERT ( vec.IsNotNull() );
  vec->Reserve(2);

  {
  // test iterator to empty elements (copy constructor)
  Container::Iterator iter(vec->Begin());
  TEST_ASSERT ( iter.Index() == 0 );
  TEST_ASSERT ( iter != vec->End() );
  
  // test iterator to empty elements assignment
  iter = vec->Begin(); TEST_ASSERT ( iter.Index() == 0 );
  TEST_ASSERT ( iter != vec->End() );
 
  // test element access via iterator
  TEST_ASSERT ( iter.Value() == *iter );
  TEST_ASSERT ( *iter == iter.Value() );
  TEST_ASSERT ( *iter == vec->ElementAt(0) );
  TEST_ASSERT ( *iter == (void*)0 );

  // same tests for const iterator
  Container::ConstIterator citer(vec->Begin());
  TEST_ASSERT ( citer.Index() == 0 );
  TEST_ASSERT ( citer != vec->End() );
  
  // test citerator to empty elements assignment
  citer = vec->Begin();
  TEST_ASSERT ( citer.Index() == 0 );
  TEST_ASSERT ( citer != vec->End() );
 
  // test element access via citerator
  TEST_ASSERT ( citer.Value() == *citer );
  TEST_ASSERT ( *citer == citer.Value() );
  TEST_ASSERT ( *citer == vec->GetElement(0) );
  TEST_ASSERT ( *citer == vec->ElementAt(0) );
  TEST_ASSERT ( *citer == (void*)0 );

  // compare between const and non-const iterators
  TEST_ASSERT ( citer.Value() == *iter );
  TEST_ASSERT ( *citer == iter.Value() );
  TEST_ASSERT ( iter.Value() == *citer );
  TEST_ASSERT ( *iter == citer.Value() );

  TEST_ASSERT ( citer.operator->() == iter.operator->() );

  } // end scope iterators
  
  // fill the vector with some elements
  // Keep in mind: ElementAt uses vector::operator[], which does no boundary checking! This can easily result in segfaults
  vec->CreateElementAt(0) = mitk::PointData::New();
  mitk::Point3D dp;
  mitk::FillVector3D( dp, 1, 40, 20 );
  vec->ElementAt(0)->SetPoint3D( dp );
  
  vec->CreateElementAt(1) = mitk::PointData::New();
  mitk::FillVector3D( dp, 2, 42, 90 );
  vec->ElementAt(1)->SetPoint3D( dp );
  
  vec->CreateElementAt(2) = mitk::PointData::New();
  mitk::FillVector3D( dp, 3, 33, 02 );
  vec->ElementAt(2)->SetPoint3D( dp );
  
  vec->CreateElementAt(3) = mitk::PointData::New();
  mitk::FillVector3D( dp, 4, 65, 32 );
  vec->ElementAt(3)->SetPoint3D( dp );
  
  vec->CreateElementAt(4) = mitk::PointData::New();
  mitk::FillVector3D( dp, 5, 81, 12 );
  vec->ElementAt(4)->SetPoint3D( dp );

  TEST_ASSERT( vec->Size() == 5 );
 
  // test operator++ and operator--
  Container::ConstIterator lastiter(vec->Begin());
  for ( Container::ConstIterator citer(vec->Begin()); citer != vec->End(); )
  {
    ++citer;
    TEST_ASSERT( lastiter != citer );
    ++lastiter;
  }
  
  lastiter = vec->Begin();
  for ( Container::ConstIterator citer(vec->Begin()); citer != vec->End(); )
  {
    citer++;
    TEST_ASSERT( lastiter != citer );
    lastiter++;
  }
  
  lastiter = vec->End();
  for ( Container::ConstIterator citer(vec->End()); citer != vec->Begin(); )
  {
    citer--;
    TEST_ASSERT( lastiter != citer );
    lastiter--;
  }
  
  lastiter = vec->End();
  for ( Container::ConstIterator citer(vec->End()); citer != vec->Begin(); )
  {
    --citer;
    TEST_ASSERT( lastiter != citer );
    --lastiter;
  }
  
  // try assignments -- commented block should not compile because of constness
  /*
  Container::ConstIterator citer = vec->Begin();
  ++citer;
  ++citer;
  *citer = mitk::PointData::New();
  mitk::FillVector3D( **citer, 1, 40, 20 );
  */

  Container::Iterator iter = vec->Begin();
  ++iter;
  ++iter;
  *iter = mitk::PointData::New();
  mitk::FillVector3D( dp, 77, 40, 20 );
  iter->SetPoint3D(dp);
  
  TEST_ASSERT ( iter->GetPoint3D() == dp );
  iter++ = *iter; // copy from next element
 
  TEST_ASSERT ( iter->GetPoint3D() != dp ); // should have changed
  
  // end testing
  vec = NULL;
  TEST_ASSERT ( vec.IsNull() );

}

/// This test checks, if the DataTreeFilter works in conjunction with the mitk::DataStorage.
/// It creates a DataStorage with some elements in it and a assoiated DataTree with some more elements
/// and checks which elements the DataTreeFilter returns


int testDataStorageCompliance()
{
  int returnValue = EXIT_SUCCESS;

  mitk::DataTree::Pointer tree = mitk::DataTree::New();
  mitk::DataStorage::Pointer ds = mitk::DataStorage::CreateInstance(tree);
  ds->SetManageCompleteTree(false);
  mitk::Color color;  color.Set(1.0f, 0.0f, 0.0f);

  /* create some objects and add them to the datastorage. */
  mitk::DataTreeNode::Pointer n1 = mitk::DataTreeNode::New();
  ds->Add(n1);
  mitk::DataTreeNode::Pointer n2 = mitk::DataTreeNode::New();
  n2->SetColor(color);
  ds->Add(n2);
  mitk::DataTreeNode::Pointer n3 = mitk::DataTreeNode::New();
  n3->SetColor(color);
  ds->Add(n3);

  /* now create some more nodes and add them to the tree directly */
  mitk::DataTreePreOrderIterator it(tree);
  mitk::DataTreeNode::Pointer t1 = mitk::DataTreeNode::New();
  it.Add(t1);
  mitk::DataTreeNode::Pointer t2 = mitk::DataTreeNode::New();
  t2->SetColor(color);
  it.Add(t2);
  mitk::DataTreeNode::Pointer t3 = mitk::DataTreeNode::New();
  t3->SetColor(color);
  it.Add(t3);

  /* create a DataTreeFilter and assign the resultset to it */
  mitk::DataTreeFilter::Pointer dTF = mitk::DataTreeFilter::New(tree);

  /* get a resultset of n1-n3 */
  std::cout << "Testing all of DataStorage: ";
  mitk::DataStorage::SetOfObjects::ConstPointer rs = ds->GetAll();
  dTF->SetDataStorageResultset(rs);

  const mitk::DataTreeFilter::ItemList* items = dTF->GetItems();
  if (   (items->Size() == 3))
  {
    bool n1found = false;
    bool n2found = false;
    bool n3found = false;
    for (mitk::DataTreeFilter::ConstItemIterator it = items->Begin(); it != items->End(); it++)  // search n1
      if (it->GetNode() == n1)
      {
        n1found = true;
        break;
      };
    for (mitk::DataTreeFilter::ConstItemIterator it = items->Begin(); it != items->End(); it++)  // search n2
      if (it->GetNode() == n2)
      {
        n2found = true;
        break;
      };
    for (mitk::DataTreeFilter::ConstItemIterator it = items->Begin(); it != items->End(); it++)  // search n3
      if (it->GetNode() == n3)
      {
        n3found = true;
        break;
      };
    if (n1found && n2found && n3found)
      std::cout << "[PASSED]" << std::endl;
    else
    {
      std::cout << "[FAILED]" << std::endl;
      returnValue = EXIT_FAILURE;
    }
  }
  else
  {
    std::cout << "[FAILED]" << std::endl;
    returnValue = EXIT_FAILURE;
  }
  /* next, get a resultset of n2-n3 */
  std::cout << "Testing subset of DataStorage: ";
  mitk::NodePredicateProperty p("color", new mitk::ColorProperty(color));
  mitk::DataStorage::SetOfObjects::ConstPointer rs2 = ds->GetSubset(p);
  dTF->SetDataStorageResultset(rs2);

  const mitk::DataTreeFilter::ItemList* items2 = dTF->GetItems();
  if (   (items2->Size() == 2))
  {
    bool n2found = false;
    bool n3found = false;
    for (mitk::DataTreeFilter::ConstItemIterator it = items2->Begin(); it != items2->End(); it++)  // search n2
      if (it->GetNode() == n2)
      {
        n2found = true;
        break;
      };
    for (mitk::DataTreeFilter::ConstItemIterator it = items2->Begin(); it != items2->End(); it++)  // search n3
      if (it->GetNode() == n3)
      {
        n3found = true;
        break;
      };
    if (n2found && n3found)
      std::cout << "[PASSED]" << std::endl;
    else
    {
      std::cout << "[FAILED]" << std::endl;
      returnValue = EXIT_FAILURE;
    }

  }
  else
  {
    std::cout << "[FAILED]" << std::endl;
    returnValue = EXIT_FAILURE;
  }

  return returnValue;
}


int mitkDataTreeFilterTest(int argc, char ** const /*argv*/)
{
  verbose = argc - 1;

  smart_pointer_vector_container_test();

  std::cout << "TreeFilterTest started (verboseness: " << verbose << ")" << std::endl;

  {
  Test test;
  std::cout << "[PASSED]" << std::endl;

  for (int testcase = 0; testcase < 12; ++testcase)
  {
    
    std::cout << "Testcase " << testcase << " ... " << std::endl;
    
    test.prepare(testcase);
    test.check_filter(testcase);

  }
  }
  std::cout << "[PASSED]" << std::endl;

  std::cout << "[TEST DONE]" << std::endl;

  /* now test the compliance to mitk::DataStorage */
  int result = testDataStorageCompliance();
  return result;

  //return EXIT_SUCCESS;
}

#endif 

