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
#ifndef __itkChildTreeIterator_h
#define __itkChildTreeIterator_h

#include <itkTreeIteratorBase.h>


namespace itk{

template <class TTreeType>
class ChildTreeIterator : public TreeIteratorBase<TTreeType> 
{
public:
  
  typedef TreeIteratorBase<TTreeType>  Superclass;
  typedef TTreeType TreeType;
  typedef typename Superclass::Self Self;
  typedef typename TTreeType::ValueType ValueType;
  typedef typename Superclass::TreeNodeType TreeNodeType;

  /** Constructor */
  ChildTreeIterator( TreeType* tree,const  TreeNodeType* start=NULL );   
  int GetType( ) const;

  virtual bool GoToChild( int number = 0 );  
  virtual bool GoToParent( );
  TreeIteratorBase<TTreeType>* Clone();

  Self& operator=(Superclass& iterator) {
  
    Superclass::operator=(iterator);
    ChildTreeIterator<TTreeType>& it = static_cast<ChildTreeIterator<TTreeType>&>(iterator);
    m_ListPosition = it.m_ListPosition;
    m_ParentNode = it.m_ParentNode;
    return *this;
  }

protected:
  const ValueType& Next();
  bool HasNext() const;

private:

  mutable int m_ListPosition;
  TreeNode<ValueType>* m_ParentNode;

};

/** */
template <class TTreeType>
ChildTreeIterator<TTreeType>::ChildTreeIterator( TTreeType* tree, const TreeNodeType* start)  
  :TreeIteratorBase<TTreeType>(tree, start)
{
  m_ListPosition = 0;
  m_ParentNode = m_Position;
  m_Position = m_ParentNode->GetChild( m_ListPosition );
}

/** */
template <class TTreeType>
bool 
ChildTreeIterator<TTreeType>::GoToChild( int number )
{
  if ( m_ParentNode->GetChild( number ) == NULL )
    return false;

  m_ListPosition = 0;
  m_ParentNode = m_ParentNode->GetChild( number );
  m_Position = m_ParentNode->GetChild( m_ListPosition );
  m_Begin = m_Position;
  // m_End = m_ParentNode->GetChild( m_ParentNode->CountChildren() - 1 );
  return true;
}

/** */
template <class TTreeType>
bool 
ChildTreeIterator<TTreeType>::GoToParent( )
{
  TreeNode<ValueType>* parent =  m_ParentNode->GetParent();
    
  if ( parent == NULL )
    return false;

  m_ListPosition = 0;
  m_ParentNode = parent;
  m_Position = m_ParentNode->GetChild( m_ListPosition );
  m_Begin = m_Position;
  // m_End = m_ParentNode->GetChild( m_ParentNode->CountChildren() - 1 );
  return true;
}

/** */
template <class TTreeType>
int 
ChildTreeIterator<TTreeType>::GetType() const
{
  return TreeIteratorBase<TTreeType>::CHILD;
}


/** */
template <class TTreeType>
bool 
ChildTreeIterator<TTreeType>::HasNext() const
{
  if( m_ListPosition < m_ParentNode->CountChildren() - 1 )
    return true;
  else
    return false;
}


/** */
template <class TTreeType>
const typename ChildTreeIterator<TTreeType>::ValueType&
ChildTreeIterator<TTreeType>::Next() 
{
  m_ListPosition++;
  m_Position = m_ParentNode->GetChild( m_ListPosition );
  return m_Position->Get();
}

/** */
template <class TTreeType>
TreeIteratorBase<TTreeType>* ChildTreeIterator<TTreeType>::Clone() 
{
  ChildTreeIterator<TTreeType>* clone = new ChildTreeIterator<TTreeType>( const_cast<TTreeType*>(m_Tree), m_Position );   
  *clone = *this;
  return clone;
}

} // end namespace itk

#endif

