#include <iostream>
#include <sstream>
#include "mitkDataTreeFilter.h"
#include "mitkDataTreeFilterEvents.h"

#include <string>
#include <itkVectorContainer.h>

#include<mitkPointData.h> 
#include<mitkVector.h> 

#include <mitkImage.h>
#include <mitkDataTree.h>
#include <mitkRenderWindow.h>
#include <mitkImageMapper2D.h>
#include <mitkProperties.h>

// TODO how to get _all_ properties? (visible = []?) Ist das erwünscht? Soll der data manager ersetzt werden? vielleicht eher nicht
// TODO DeleteSelected sinnvoll?
// TODO Views schreiben:
//       combobox: prop displayed_property (default "name")
//       listview: prop underdrückte_properties
//       die propertyviews aus dem datamanager wiedervewrwenden?
// TODO  ExtendedSelect (mehrfache auswahl möglich, aber nur items am stück, ohne lücke)

int verbose;



class Test
{
  public:

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
    
    static bool EinsZwoFilter( mitk::DataTreeNode* node )
    {
      std::string name;
      node->GetName( name );
      if ( name == "Eins" || name == "Zwo" )
        return true;
      else 
        return false;
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
          tree_filter->SetFilter( EinsZwoFilter );
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
          tree_filter->SetFilter( EinsZwoFilter );
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
          assert ( ost.str() == soll.str() || verbose > 1 );
      
          const mitk::DataTreeFilter::ItemSet* selectedItems = tree_filter->GetSelectedItems();
          assert (selectedItems->empty());
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
          assert ( ost.str() == soll.str() || verbose > 1 );
          
          const mitk::DataTreeFilter::ItemSet* selectedItems = tree_filter->GetSelectedItems();
          assert (selectedItems->empty());
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
          assert ( ost.str() == soll.str() || verbose > 1 );
          
          const mitk::DataTreeFilter::ItemSet* selectedItems = tree_filter->GetSelectedItems();
          assert (selectedItems->empty());
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
          assert ( ost.str() == soll.str() || verbose > 1 );
          
          const mitk::DataTreeFilter::ItemSet* selectedItems = tree_filter->GetSelectedItems();
          assert (selectedItems->empty());
          break;
        }
        case 4: {
soll <<
"+- Ruut" << std::endl <<
"    +- Eins" << std::endl <<
"    +- Zwo*" << std::endl;
          assert ( ost.str() == soll.str() || verbose > 1 );
          
          const mitk::DataTreeFilter::ItemSet* selectedItems = tree_filter->GetSelectedItems();
          int goodSelectedItems(0);
          for (mitk::DataTreeFilter::ItemSet::const_iterator iter = selectedItems->begin(); iter != selectedItems->end(); ++iter)
          {
            if (static_cast<const std::string>((*iter)->GetProperty("name")) == std::string("Zwo")) ++goodSelectedItems;
          }
          assert( goodSelectedItems == 1 );
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
          assert ( ost.str() == soll.str() || verbose > 1 );
          
          const mitk::DataTreeFilter::ItemSet* selectedItems = tree_filter->GetSelectedItems();
          int goodSelectedItems(0);
          for (mitk::DataTreeFilter::ItemSet::const_iterator iter = selectedItems->begin(); iter != selectedItems->end(); ++iter)
          {
            if (static_cast<const std::string>((*iter)->GetProperty("name")) == std::string("Ruut")) ++goodSelectedItems;
            if (static_cast<const std::string>((*iter)->GetProperty("name")) == std::string("Zwo")) ++goodSelectedItems;
          }
          assert( goodSelectedItems == 2 );
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
          assert ( ost.str() == soll.str() || verbose > 1 );
          
          const mitk::DataTreeFilter::ItemSet* selectedItems = tree_filter->GetSelectedItems();
          int goodSelectedItems(0);
          for (mitk::DataTreeFilter::ItemSet::const_iterator iter = selectedItems->begin(); iter != selectedItems->end(); ++iter)
          {
            if (static_cast<const std::string>((*iter)->GetProperty("name")) == std::string("Ruut")) ++goodSelectedItems;
            if (static_cast<const std::string>((*iter)->GetProperty("name")) == std::string("Zwo")) ++goodSelectedItems;
          }
          assert( goodSelectedItems == 2 );
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
          assert ( ost.str() == soll.str() || verbose > 1 );
          
          const mitk::DataTreeFilter::ItemSet* selectedItems = tree_filter->GetSelectedItems();
          int goodSelectedItems(0);
          for (mitk::DataTreeFilter::ItemSet::const_iterator iter = selectedItems->begin(); iter != selectedItems->end(); ++iter)
          {
            if (static_cast<const std::string>((*iter)->GetProperty("name")) == std::string("Zwo")) ++goodSelectedItems;
          }
          assert( goodSelectedItems == 1 );
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
          assert ( ost.str() == soll.str() || verbose > 1 );
          
          const mitk::DataTreeFilter::ItemSet* selectedItems = tree_filter->GetSelectedItems();
          int goodSelectedItems(0);
          for (mitk::DataTreeFilter::ItemSet::const_iterator iter = selectedItems->begin(); iter != selectedItems->end(); ++iter)
          {
            if (static_cast<const std::string>((*iter)->GetProperty("name")) == std::string("Eins")) ++goodSelectedItems;
            if (static_cast<const std::string>((*iter)->GetProperty("name")) == std::string("Zwo")) ++goodSelectedItems;
          }
          assert( goodSelectedItems == 2 );
          break;
        }
        case 9: {
soll <<
"+- Eins" << std::endl <<
"|    name: Eins" << std::endl <<
"+- Zwo" << std::endl <<
"|    name: Zwo" << std::endl;
          assert ( ost.str() == soll.str() || verbose > 1 );

          break;
        }
        case 10: {
          const mitk::DataTreeFilter::ItemSet* selectedItems = tree_filter->GetSelectedItems();
          assert( selectedItems->empty() );
          assert( tree_filter->GetItems()->empty() );
          break;
        }
        case 11: {
soll <<
"+- Eins" << std::endl <<
"|    name: Eins" << std::endl <<
"+- Zwo" << std::endl <<
"|    name: Zwo" << std::endl;
          assert ( ost.str() == soll.str() || verbose > 1 );

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
  assert ( nix.IsNull() );
  }

  // test simple construction/destruction
  ContainerPointer vec = Container::New(); 
  assert ( vec.IsNotNull() );
  
  vec = NULL;
  assert ( vec.IsNull() );
 
  // test sizing
  vec = Container::New(); 
  assert ( vec.IsNotNull() );
 
#define NUM_ELEMENTS 500
  vec->Reserve(NUM_ELEMENTS);
  assert ( vec->Size() == NUM_ELEMENTS );
  vec = NULL;
  assert ( vec.IsNull() );
  vec = Container::New(); 
  assert ( vec.IsNotNull() );
  vec->Reserve(2);

  {
  // test iterator to empty elements (copy constructor)
  Container::Iterator iter(vec->Begin());
  assert ( iter.Index() == 0 );
  assert ( iter != vec->End() );
  
  // test iterator to empty elements assignment
  iter = vec->Begin(); assert ( iter.Index() == 0 );
  assert ( iter != vec->End() );
 
  // test element access via iterator
  assert ( iter.Value() == *iter );
  assert ( *iter == iter.Value() );
  assert ( *iter == vec->ElementAt(0) );
  assert ( *iter == (void*)0 );

  // same tests for const iterator
  Container::ConstIterator citer(vec->Begin());
  assert ( citer.Index() == 0 );
  assert ( citer != vec->End() );
  
  // test citerator to empty elements assignment
  citer = vec->Begin();
  assert ( citer.Index() == 0 );
  assert ( citer != vec->End() );
 
  // test element access via citerator
  assert ( citer.Value() == *citer );
  assert ( *citer == citer.Value() );
  assert ( *citer == vec->GetElement(0) );
  assert ( *citer == vec->ElementAt(0) );
  assert ( *citer == (void*)0 );

  // compare between const and non-const iterators
  assert ( citer.Value() == *iter );
  assert ( *citer == iter.Value() );
  assert ( iter.Value() == *citer );
  assert ( *iter == citer.Value() );

  assert ( citer.operator->() == iter.operator->() );

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

  assert( vec->Size() == 5 );
 
  // test operator++ and operator--
  Container::ConstIterator lastiter(vec->Begin());
  for ( Container::ConstIterator citer(vec->Begin()); citer != vec->End(); )
  {
    ++citer;
    assert( lastiter != citer );
    ++lastiter;
  }
  
  lastiter = vec->Begin();
  for ( Container::ConstIterator citer(vec->Begin()); citer != vec->End(); )
  {
    citer++;
    assert( lastiter != citer );
    lastiter++;
  }
  
  lastiter = vec->End();
  for ( Container::ConstIterator citer(vec->End()); citer != vec->Begin(); )
  {
    citer--;
    assert( lastiter != citer );
    lastiter--;
  }
  
  lastiter = vec->End();
  for ( Container::ConstIterator citer(vec->End()); citer != vec->Begin(); )
  {
    --citer;
    assert( lastiter != citer );
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
  
  assert ( iter->GetPoint3D() == dp );
  iter++ = *iter; // copy from next element
 
  assert ( iter->GetPoint3D() != dp ); // should have changed
  
  // end testing
  vec = NULL;
  assert ( vec.IsNull() );

}

int mitkDataTreeFilterTest(int argc, char ** const argv)
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

  return EXIT_SUCCESS;
}

