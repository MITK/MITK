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
#ifndef __itkPreOrderTreeIterator_h
#define __itkPreOrderTreeIterator_h

#include <itkTreeIteratorBase.h>


namespace itk{

//template <class TTreeType> class TreeContainerBase;

template <class TTreeType>
class PreOrderTreeIterator : public TreeIteratorBase<TTreeType> 
{
public:

  /** Constructor */
  typedef typename TTreeType::ValueType  ValueType;
  typedef TreeIteratorBase<TTreeType> Superclass;
  typedef typename Superclass::TreeNodeType TreeNodeType;
 
  PreOrderTreeIterator( const TTreeType* tree, TreeNodeType* start = NULL );

  /** Get the type of the iterator */
  int GetType() const;
  TreeIteratorBase<TTreeType>* Clone();

protected:
  const ValueType& Next();
  bool HasNext() const;
  
private:  

  const TreeNodeType* FindNextNode() const;

};


/** Constructor */
template <class TTreeType>
PreOrderTreeIterator<TTreeType>::PreOrderTreeIterator( const TTreeType* tree, TreeNodeType* start )
  :TreeIteratorBase<TTreeType>(tree,start) 
{

}

/** Return the type of the iterator */
template <class TTreeType>
int 
PreOrderTreeIterator<TTreeType>::GetType() const 
{
  return TreeIteratorBase<TTreeType>::PREORDER; 
}


/** Has next? */
template <class TTreeType>
bool 
PreOrderTreeIterator<TTreeType>::HasNext() const
{
  if ( const_cast<TreeNodeType* >(FindNextNode()) != NULL )
    {
    return true;
    }
  return false;
}


/** Return the next node */
template <class TTreeType>
const typename PreOrderTreeIterator<TTreeType>::ValueType&
PreOrderTreeIterator<TTreeType>::Next() 
{
  m_Position = const_cast<TreeNodeType* >(FindNextNode());
  return m_Position->Get();
}

/** Return the next node const */
/*
template <class TTreeType>
const typename PreOrderTreeIterator<TTreeType>::TreeNodeType* 
PreOrderTreeIterator<TTreeType>::Next() const
{
  m_Position = const_cast<TreeNodeType* >(FindNextNode());
  return m_Position;
}
*/

/** Find the next node */
template <class TTreeType>
const typename PreOrderTreeIterator<TTreeType>::TreeNodeType* 
PreOrderTreeIterator<TTreeType>::FindNextNode() const
{ 
  if ( m_Position == NULL )
    {
    return NULL;
    }
  if ( m_Position->HasChildren() )
    {
    return dynamic_cast<const TreeNodeType*>(m_Position->GetChild(0));
    }

  if ( !m_Position->HasParent() )
    {
    return NULL;
    }

  TreeNodeType* child = m_Position;
  TreeNodeType* parent = dynamic_cast<TreeNodeType*>(m_Position->GetParent());

  int childPosition = parent->ChildPosition( child ); 
  int lastChildPosition = parent->CountChildren() - 1;

  while ( childPosition < lastChildPosition ) 
    {
    TreeNodeType* help = dynamic_cast<TreeNodeType*>(parent->GetChild( childPosition + 1 ));

    if ( help != NULL )
      {
      return help;
      }
    childPosition++;
    }
  
  while ( parent->HasParent() ) 
    {
    child = parent;
    parent = dynamic_cast<TreeNodeType*>(parent->GetParent());

    // Subtree
    if( parent->ChildPosition( m_Root ) >= 0 )
      {
      return NULL;
      }

    childPosition = parent->ChildPosition(child);
    lastChildPosition = parent->CountChildren() - 1;

    while ( childPosition < lastChildPosition ) 
      {
      TreeNodeType* help = dynamic_cast<TreeNodeType*>(parent->GetChild( childPosition + 1 ));

      if ( help != NULL )
        {
        return help;
        }
      }
    }
  return NULL;
}


/** */
template <class TTreeType>
TreeIteratorBase<TTreeType>* PreOrderTreeIterator<TTreeType>::Clone() 
{
  PreOrderTreeIterator<TTreeType>* clone = new PreOrderTreeIterator<TTreeType>( m_Tree, m_Position );
  *clone = *this;
  return clone;
}

} // end namespace itk

#endif

