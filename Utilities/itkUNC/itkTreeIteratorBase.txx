/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __itkTreeContainerBase_txx
#define __itkTreeContainerBase_txx

#include <itkTreeIteratorBase.h>
#include "itkTreeChangeEvent.h"

namespace itk
{

/** Constructor */
template <class TTreeType>
TreeIteratorBase<TTreeType>::TreeIteratorBase( TTreeType* tree, const TreeNodeType* start)
{ 
  if(start)
    {
    m_Root = start;
    } 
  else
    {
    m_Root = dynamic_cast<const TreeNodeType*>(tree->GetRoot());
    }

  m_Position = const_cast<TreeNodeType*>(m_Root);
  m_Tree = tree;
  m_Begin = m_Position;
  m_End = NULL;
}

/** Constructor */
template <class TTreeType>
TreeIteratorBase<TTreeType>::TreeIteratorBase( const TTreeType* tree, const TreeNodeType* start)
{
  if(start)
    {
    m_Root = start;
    } 
  else
    {
    m_Root = const_cast<TreeNodeType*>(dynamic_cast<const TreeNodeType*>(tree->GetRoot()));
    }
  m_Position = const_cast<TreeNodeType*>(m_Root);
  m_Tree = const_cast<TTreeType*>(tree);
  m_Begin = m_Position;
  m_End = NULL;
}

/** */
template <class TTreeType>
const typename TreeIteratorBase<TTreeType>::ValueType& 
TreeIteratorBase<TTreeType>::Get() const 
{
  return m_Position->Get();
}


/** */
template <class TTreeType>
typename TreeIteratorBase<TTreeType>::ValueType& 
TreeIteratorBase<TTreeType>::Set(ValueType element) 
{
  ValueType oldValue = m_Position->Get();
  m_Position->Set(element);
  return oldValue;
}


/** */
template <class TTreeType>
bool 
TreeIteratorBase<TTreeType>::Add(ValueType element)
{
  if ( m_Position == NULL && m_Root == NULL ) 
    {
    bool returnValue;
    if ( m_Tree )
      {
      returnValue = const_cast<TTreeType*>(m_Tree)->SetRoot( element );
      }

      m_Root = dynamic_cast<const TreeNodeType*>(const_cast<TTreeType*>(m_Tree)->GetRoot());
      m_Position =  const_cast<TreeNodeType*>(m_Root);
      m_Tree->Modified();
      m_Tree->InvokeEvent( TreeAddEvent<TTreeType>(*this) );
      return returnValue;
    } 
  else if ( m_Position == NULL )
    {
    return false;
    }

  typename TreeNodeType::Pointer node = TreeNodeType::New();
  node->SetData(element);
  node->SetParent(m_Position);
  m_Position->AddChild( node) ; //m_Position) );
  m_Tree->Modified();
  m_Tree->InvokeEvent( TreeAddEvent<TTreeType>(*this) );
  return true;
}

/** */
template <class TTreeType>
bool 
TreeIteratorBase<TTreeType>::Add( int childPosition, ValueType element )
{
  if ( m_Position ) 
    {
    typename TreeNodeType::Pointer node = TreeNodeType::New();
    node->SetData(element);
    node->SetParent(m_Position);
    m_Position->AddChild(node) ; // new TreeNodeType( element, m_Position ) );
    m_Tree->Modified();
    m_Tree->InvokeEvent( TreeAddEvent<TTreeType>(*this) );
    return true;
    }
  return false;
}



/** Return true if the current pointed node is a leaf */
template <class TTreeType>
bool 
TreeIteratorBase<TTreeType>::IsLeaf() const 
{
  return !(m_Position->HasChildren());
} 


/** Return true if the current pointed node is a root */
template <class TTreeType>
bool 
TreeIteratorBase<TTreeType>::IsRoot( ) const 
{
  if ( m_Root == NULL )
    {
    return false;
    }
    
  if ( m_Position == m_Root )
    {
    return true;
    }
  return false;
}


/** TO FIX*/
template <class TTreeType>
bool 
TreeIteratorBase<TTreeType>::Add( TTreeType& subTree ) 
{
std::cout << "reeIteratorBase<ValueType>::Add( TTreeType& subTree ) " << " TO FIX" << std::endl;
/*
    if ( subTree.Count() == 0 )
        return false;

    if ( m_Root == NULL ) {
        ATIR i = subTree.ArbitraryTreeIterator( );
        ValueType data = i->Get();
        TreeNodeType* newNode = new LinkedNode<ValueType>( data, m_Position, 2 );

        m_Root = newNode;
        addTree( newNode, i );
    m_Tree->InformTreeChangeListener(*this);
        return true;
    } else {

        if ( m_Position == NULL )
            return false;

        ATIR i = subTree.arbitraryTreeIterator( );
        ValueType data = i->get();
        TreeNodeType* newNode = new LinkedNode<ValueType>( data, m_Position, 2 );
        m_Position->AddChild( newNode );
        addTree( newNode, i );
    }*/
    return false;
}


/**
 * Private Methode, fügt rekursiv einen Baum als Unterbaum an
 * @param node aktueller Knoten, an den alle Kinder von i hinzugefügt
 *              werden sollen
 * @param i aktuelle Iteratorposition des Subbaums
 */
/*template <class TTreeType>
void TreeIteratorBase<TTreeType>::addTree( TreeNodeType *node, ATIR& i ) {

    TIR children = i->children();
    TreeNodeType* newNode = NULL;

    while ( children->HasNext() ) {
        ValueType data = children->Next();
        newNode = new LinkedNode<ValueType>( data, node, 2 );
        node->AddChild( newNode );
        ATIR newPos = new WalkTreeIterator<ValueType>( children.get() );
        addTree( newNode, newPos );
    }
  m_Tree->InformTreeChangeListener(*this);
}*/

/** Return the subtree */
template <class TTreeType>
TTreeType*
TreeIteratorBase<TTreeType>::GetSubTree() const 
{
  typename TTreeType::Pointer tree = TTreeType::New();
  tree->SetSubtree(true);
  tree->SetRoot(m_Position);
  return tree;
  return NULL;
}


/** */
template <class TTreeType>
bool 
TreeIteratorBase<TTreeType>::HasChild( int number ) const
{
  if ( m_Position == NULL )
    {
    return false;
    }
 if ( m_Position->GetChild( number ) != NULL )
   {
   return true;
   }
  return false;
}


/** */
template <class TTreeType>
int 
TreeIteratorBase<TTreeType>::ChildPosition( ValueType element ) const 
{
  if ( !m_Position )
    {
    return -1;
    }
  return m_Position->ChildPosition( element );
}


/** */
template <class TTreeType>
bool 
TreeIteratorBase<TTreeType>::RemoveChild( int number ) 
{
  if( !HasChild( number ) )
    {
    return false;
    }
  TreeNodeType* child = dynamic_cast<TreeNodeType*>(m_Position->GetChild( number ));

  if( child != NULL ) 
    {
    TreeNodeType* help = m_Position;
    m_Position = dynamic_cast<TreeNodeType*>(child);
    m_Tree->InvokeEvent( TreeRemoveEvent<TTreeType>( *this ) );
    m_Position = dynamic_cast<TreeNodeType*>(help);
    const_cast<TreeNodeType*>(m_Position)->Remove( child );
    m_Tree->Modified();
    return true;
    }
  return false;
}


/** Count the number of children */
template <class TTreeType>
int 
TreeIteratorBase<TTreeType>::CountChildren() const 
{
  if ( m_Position == NULL )
    {
    return -1;
    }
  return m_Position->CountChildren( );
}


/** Return true of the pointed node has a parent */
template <class TTreeType>
bool 
TreeIteratorBase<TTreeType>::HasParent( ) const 
{
  if ( m_Position == NULL && m_Position->GetParent() == NULL )
    {
    return false;
    }
  return true;
}


/** */
template <class TTreeType>
bool 
TreeIteratorBase<TTreeType>::Disconnect( )
{
    if ( m_Position == NULL )
        return false;

    if( m_Position->HasParent() == false )
        return false;

    TreeNodeType* parent = dynamic_cast<TreeNodeType*>(m_Position->GetParent());
    m_Tree->InvokeEvent( TreeRemoveEvent<TTreeType>( *this ) );
    parent->Remove( const_cast<TreeNodeType*>(m_Position) );
    m_Tree->Modified();
    int size = m_Position->CountChildren();

    for( int i=0; i< size; i++ ) {
        TreeNodeType* child = dynamic_cast<TreeNodeType*>(m_Position->GetChild(i));
        parent->AddChild( child );
        child->SetParent( parent );
    }

    m_Position = NULL;
//  m_Tree->InformTreeChangeListener(*this);
    return true;
}

/** */
template <class TTreeType>
TreeIteratorBase<TTreeType>* 
TreeIteratorBase<TTreeType>::Children( ) 
{
//  TreeIteratorBase<TTreeType>& r = *this;
//  return new ChildTreeIterator<ValueType>(r);
  itkGenericOutputMacro("Not implemented yet");
  ::itk::ExceptionObject e_(__FILE__, __LINE__, "Not implemented yet");
  throw e_; /* Explicit naming to work around Intel compiler bug.  */ \
  return 0;
}


/** Return the first parent found */
template <class TTreeType>
const typename TreeIteratorBase<TTreeType>::TreeNodeType* 
TreeIteratorBase<TTreeType>::GetParent() const
{
  if(m_Position == NULL)
    {
    return NULL;
    }
 
  return m_Position->GetParent();
}

/**  */
template <class TTreeType>
TreeIteratorBase<TTreeType>* TreeIteratorBase<TTreeType>::Parents() 
{
  //TreeIteratorBase<TTreeType>& r = *this;
  //return new RootTreeIterator<ValueType>(r);
  return 0;
}


/** Go to a child */
template <class TTreeType>
bool TreeIteratorBase<TTreeType>::GoToChild( int number ) 
{
  if ( m_Position == NULL )
    {
    return false;
    }

  TreeNodeType* next = dynamic_cast<TreeNodeType*>(m_Position->GetChild( number ));

  if ( next == NULL )
    {
    return false;
    }
  m_Position = next;
  return true;
}


/** Go to a parent */
template <class TTreeType>
bool TreeIteratorBase<TTreeType>::GoToParent( ) 
{
  if ( m_Position == NULL )
    {
    return false;
    }

  if( !m_Position->HasParent() ) 
    {
    return false;
    }

  m_Position = dynamic_cast<TreeNodeType*>(m_Position->GetParent());
  return true;
}

/** */
template <class TTreeType>
TreeIteratorBase<TTreeType>* TreeIteratorBase<TTreeType>::GetChild( int number ) const 
{
  if ( !m_Position )
    {
    return NULL;
    }
    
  TreeNodeType* child = dynamic_cast<TreeNodeType*>(m_Position->GetChild( number ));

  if ( !child )
    {
    return NULL;
    }
//    return new WalkTreeIterator<ValueType,P>( child, m_Root, m_Tree, getType() );
    return NULL;
}


/** Count the number of nodes from the beginning */
template <class TTreeType>
int TreeIteratorBase<TTreeType>::Count()
{
  int size = 0;
  this->GoToBegin();
  if ( !m_Position->HasChildren() )
    {
    return 0;
    }
  while (this->Next() ) 
    {    
    size++;
    }
  return size;
}


/**  */
template <class TTreeType>
int TreeIteratorBase<TTreeType>::Count(TreeNodeType* node ) 
{
  int size = 0;

  if ( !node->hasChildren() )
    {
    return 0;
    }

  TreeIteratorBase<TTreeType> it(this,node);

  while ( !it.IsAtEnd() )
   {
   ++it; 
   size++;
   }
  return size;
}


/** Get the node pointed by the iterator */
template <class TTreeType>
typename TreeIteratorBase<TTreeType>::TreeNodeType* 
TreeIteratorBase<TTreeType>::GetNode() 
{
  return const_cast<TreeNodeType*>(m_Position);
}

/** Get the node pointed by the iterator */
template <class TTreeType>
const typename TreeIteratorBase<TTreeType>::TreeNodeType* 
TreeIteratorBase<TTreeType>::GetNode() const
{
  return m_Position;
}

/** Get the root */
template <class TTreeType>
typename TreeIteratorBase<TTreeType>::TreeNodeType* &
TreeIteratorBase<TTreeType>::GetRoot() 
{
  return const_cast<TreeNodeType*>(m_Root);
}

/** Get the root (const) */
template <class TTreeType>
const typename TreeIteratorBase<TTreeType>::TreeNodeType* &
TreeIteratorBase<TTreeType>::GetRoot() const
{
  return m_Root;
}

/** Remove a specific node */
template <class TTreeType>
bool 
TreeIteratorBase<TTreeType>::Remove() 
{
  if ( m_Position == NULL )
    {
    return false;
    }
  
  if ( m_Position->HasParent() )
    {
    m_Tree->InvokeEvent( TreeRemoveEvent<TTreeType>(*this) );
    TreeNodeType* parent = m_Position->GetParent();
    parent->Remove( m_Position );
    m_Tree->Modified();
    } 
  else if (m_Root == m_Position)
    {
    m_Tree->InvokeEvent( TreeRemoveEvent<TTreeType>(*this) );
    m_Root = NULL;
    m_Tree->Modified();
    }

  m_Position = NULL;  // Smart pointer
  return true;
}

/** Return the tree */
template <class TTreeType>
TTreeType* 
TreeIteratorBase<TTreeType>::GetTree()
{
  return m_Tree;
}


} // namespace itk



#endif

