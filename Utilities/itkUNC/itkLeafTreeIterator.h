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
#ifndef __itkLeafTreeIterator_h
#define __itkLeafTreeIterator_h

#include <itkTreeIteratorBase.h>
#include <itkPreOrderTreeIterator.h>

namespace itk{

template <class TTreeType>
class LeafTreeIterator : public TreeIteratorBase<TTreeType> 
{
public:

  /** Typedefs*/
  typedef TreeIteratorBase<TTreeType> Superclass;
  typedef TTreeType TreeType;
  typedef typename TreeType::ValueType ValueType;
  typedef TreeNode<ValueType>  TreeNodeType;

  LeafTreeIterator( const TreeType* tree );
  LeafTreeIterator( TreeType* tree );

  ~LeafTreeIterator();

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
LeafTreeIterator<TTreeType>::LeafTreeIterator(  const TTreeType* tree ) 
  :TreeIteratorBase<TTreeType>(tree,NULL)
{
  m_Begin =  const_cast<TreeNodeType* >(this->FindNextNode()); // Position the iterator to the first leaf;
}

/** Constructor */
template <class TTreeType>
LeafTreeIterator<TTreeType>::LeafTreeIterator( TTreeType* tree ) 
  :TreeIteratorBase<TTreeType>(tree,NULL)
{
  m_Begin =  const_cast<TreeNodeType* >(this->FindNextNode()); // Position the iterator to the first leaf;
}


/** Destructor */
template <class TTreeType>
LeafTreeIterator<TTreeType>::~LeafTreeIterator() 
{
}


/**
 * Gibt den Iteratortyp zurück
 * @return Typ des Iterators
 */
template <class TTreeType>
int LeafTreeIterator<TTreeType>::GetType() const 
{
  return TreeIteratorBase<TTreeType>::LEAF;
}


/**
 * Überprüft, ob es ein weiteres Element gibt
 * @return true es gibt einen weiteren Knoten 
 *         false es gibt keinen weiteren Knoten
 */
template <class TTreeType>
bool LeafTreeIterator<TTreeType>::HasNext() const
{
  if(m_Position == NULL)
    {
    return false;
    }
  if ( const_cast<TreeNodeType* >(FindNextNode()) != NULL )
    {
    return true;
    }
  return false;
}


/**
 * Setzt den Iterator um eins weiter und gibt den Wert zurück
 * @return Wert des nächsten Knotens
 */
template <class TTreeType>
const typename LeafTreeIterator<TTreeType>::ValueType&
LeafTreeIterator<TTreeType>::Next() 
{
  m_Position = const_cast<TreeNodeType* >(FindNextNode());
  return m_Position->Get();
}

/** Const next */
/*
template <class TTreeType>
const typename LeafTreeIterator<TTreeType>::TreeNodeType* 
LeafTreeIterator<TTreeType>::Next() const
{
  m_Position = const_cast<TreeNodeType* >(FindNextNode());
  return m_Position;
}
*/

/** Find the next node given the position */
template <class TTreeType>
const typename LeafTreeIterator<TTreeType>::TreeNodeType* 
LeafTreeIterator<TTreeType>::FindNextNode() const 
{
  PreOrderTreeIterator<TTreeType> it(m_Tree,m_Position); 
  ++it; // go next
  if(it.IsAtEnd())
    {
    return NULL;
    }

  if(!it.HasChild())
    {
    return it.GetNode();
    }

  while( !it.IsAtEnd() )
    {
    if(!it.HasChild())
      {
      return it.GetNode();
      }
    ++it;
    }
  
  return NULL;
}

/** */
template <class TTreeType>
TreeIteratorBase<TTreeType>* LeafTreeIterator<TTreeType>::Clone() 
{
  LeafTreeIterator<TTreeType>* clone = new LeafTreeIterator<TTreeType>( m_Tree );
  *clone = *this;
  return clone;
}

} // end namespace itk

#endif

