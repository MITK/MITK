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
#ifndef __itkTreeIteratorBase_h
#define __itkTreeIteratorBase_h

#include <itkTreeNode.h>

namespace itk {

//template <class ValueType> class TreeNode;

template <class TTreeType>
class TreeIteratorBase
{
public: 
  
  typedef TreeIteratorBase<TTreeType> Self;
  typedef typename TTreeType::ValueType ValueType;
  typedef typename TTreeType::TreeNodeType TreeNodeType;

  // virtual const ValueType& Next() const = 0; // ????

  virtual bool Add(ValueType element);

  virtual bool Add(int position, ValueType element);

  virtual bool Add(TTreeType& subTree);

  virtual const ValueType& Get() const ;

  virtual TTreeType* GetSubTree() const ;

  virtual bool IsLeaf() const;

  virtual bool IsRoot() const;

  virtual int GetType() const = 0;

  virtual bool GoToChild( int number = 0 );
  
  virtual bool GoToParent( );

  ValueType& Set( ValueType element);

  virtual bool HasChild(int number = 0) const;

  virtual int ChildPosition(ValueType element) const;

  virtual bool RemoveChild(int number);

  virtual int CountChildren() const;

  virtual bool HasParent() const;

  virtual bool Disconnect();

  virtual TreeIteratorBase<TTreeType>* Children();

  virtual TreeIteratorBase<TTreeType>* Parents();

  virtual TreeIteratorBase<TTreeType>* GetChild(int number) const;

  virtual int Count();

  bool Remove();

  virtual TreeNodeType* GetNode();
  
  virtual const TreeNodeType* GetNode() const;

  TreeNodeType* &GetRoot();
  const TreeNodeType* &GetRoot() const;
  TTreeType* GetTree();

  /** return the first parent found */
  const TreeNodeType* GetParent() const;

  /** Move an iterator to the beginning of the tree */
  void GoToBegin()
    {
    m_Position = m_Begin;
    };

  /** Move an iterator to the end of the tree. */
  void GoToEnd()
    {
    m_Position = m_End;
    };

  /** Is the iterator at the beginning of the tree? */
  bool IsAtBegin(void) const
    {
    return (m_Position == m_Begin);
    }

  /** Is the iterator at the end of the tree?. The iterator is at the end if it points to NULL*/
  bool IsAtEnd(void) const
    {
    return (m_Position == m_End);
    }

  /** Clone the interator */

  virtual TreeIteratorBase<TTreeType>* Clone() = NULL;


  /** Enumerations */
  enum{
    UNDEFIND   = 0,
    PREORDER   = 1,
    INORDER    = 2,
    POSTORDER  = 3,
    LEVELORDER = 4,
    CHILD   = 5,
    ROOT     = 6,
    LEAF     = 7
  };

  /** operator++ */
  Self &
  operator++()
  {
    this->Next();
    return *this;
  }

  virtual Self& operator=(Self& iterator) {

    m_Position = iterator.m_Position; 
    m_Begin  = iterator.m_Begin;
    m_End = iterator.m_End;
    m_Root = iterator.m_Root;
    m_Tree = iterator.m_Tree;
    return *this;
    }

protected:

  //typedef TreeNode<ValueType> TreeNodeType;
  TreeIteratorBase( TTreeType* tree, const TreeNodeType* start);
  TreeIteratorBase( const TTreeType* tree, const TreeNodeType* start);

//    void addTree( TreeNode<ValueType> *node, ATIR& i);
  mutable TreeNodeType* m_Position; // Current position of the iterator
  mutable TreeNodeType* m_Begin;
  mutable TreeNodeType* m_End;
  const TreeNodeType* m_Root;
  TTreeType* m_Tree;
  int Count(TreeNodeType* node);

  virtual bool HasNext() const = 0;
  virtual const ValueType& Next() = 0;
};

} //end namespace itk


#ifndef ITK_MANUAL_INSTANTIATION
#include "itkTreeIteratorBase.txx"
#endif


#endif
