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
#ifndef __itkTreeContainer_h
#define __itkTreeContainer_h

#include "itkMacro.h"
#include <itkTreeContainerBase.h>
#include <itkPreOrderTreeIterator.h>

namespace itk
{

//template <class TValueType> class PreOrderTreeIterator;

/** \class TreeContainer
 *  \brief Array class with size defined at construction time.
 * 
 * This class derives from the vnl_vector<> class. 
 * Its size is assigned at construction time (run time) and can 
 * not be changed afterwards except by using assignment to another
 * Array.
 *
 * The class is templated over the type of the elements.
 *
 * Template parameters for class Array:
 *
 * - TValueType = Element type stored at each location in the array.
 *
 * \ingroup DataRepresentation 
 */
template <class TValueType> //, class TTreeObjectSuperclass>
class TreeContainer : public TreeContainerBase<TValueType>
{

public:

  /** Standard typedefs */
  typedef TreeContainerBase<TValueType> Superclass;
  typedef TreeContainer<TValueType> Self;
  typedef SmartPointer<Self>  Pointer;
  typedef SmartPointer<const Self>  ConstPointer;
  typedef TValueType ValueType;
  typedef TreeNode<ValueType> TreeNodeType;

  /** Iterators typedef */
//  typedef typename Superclass::IteratorType IteratorType;
  
   typedef TreeIteratorBase<Self> IteratorType;
  //typedef PreOrderTreeIterator<Self> IteratorType;
  typedef PreOrderTreeIterator<Self> PreOrderIteratorType;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(TreeContainer, TreeContainerBase);
 
//  typedef typename TreeContainerBase<TValueType>::TreeChangeListener TreeChangeListener;

  TreeContainer( int defaultChildrenCount );

  TreeContainer( TreeContainer<TValueType>& tree );

  virtual bool SetRoot( const TValueType element);

  bool SetRoot( IteratorType& pos );

  virtual bool SetRoot( TreeNode<TValueType>* node);

  bool Contains( const TValueType element ) ;

  int Count() const;

  bool IsLeaf( const TValueType element );

  bool IsRoot( const TValueType element );

  bool Clear( );

  bool operator==( TreeContainer<TValueType>& tree );


  bool Swap( IteratorType& v, IteratorType& w );
/*
  virtual bool AddTreeChangeListener(TreeChangeListener* treeChangeListener);

  virtual bool RemoveTreeChangeListener(TreeChangeListener* treeChangeListener);

  virtual void InformTreeChangeListener(IteratorType& changedTreePosition);
*/
  //TreeNode<TValueType>* GetRoot() {return m_Root;}
  const TreeNodeType* GetRoot() const {return m_Root;}

  /** Add a child to a given parent using values*/
  bool Add(const TValueType child, const TValueType parent);

  /** Get node given a value */
  //TreeNode<TValueType>* GetNode(TValueType val);
  const TreeNodeType* GetNode(TValueType val) const;

protected:
  
  TreeContainer(); 
  virtual ~TreeContainer();

  typename TreeNodeType::Pointer    m_Root;
//  std::vector<TreeChangeListener*>               m_TreeChangeListenerList;
  int   m_DefaultChildrenCount;

  void PrintSelf(std::ostream& os, Indent indent) const;

};

} // namespace itk


#ifndef ITK_MANUAL_INSTANTIATION
#include "itkTreeContainer.txx"
#endif


#endif
