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
#ifndef __itkTreeNode_h
#define __itkTreeNode_h

#include <vector>
#include <algorithm>
#include <iostream>


#include <itkObject.h>

namespace itk
{

template <class TValueType>
class TreeNode : public Object
{

public:

  /** Standard typedefs */
  typedef Object Superclass;
  typedef TreeNode<TValueType> Self;
  typedef SmartPointer<Self>  Pointer;
  typedef SmartPointer<const Self>  ConstPointer;

  typedef std::vector<Pointer> ChildrenListType;

  /** Method for creation through the object factory. */
  itkNewMacro( Self );
 
  /** Run-time type information (and related methods). */ 
  itkTypeMacro( Self, Superclass );

  //TreeNode( TValueType data, TreeNode<TValueType>* parent = NULL, int defaultChildrenCount = 3 );

  //TreeNode( TreeNode<TValueType>* parent = NULL, int defaultChildrenCount = 3 );

  const TValueType& Get( ) const;

  TValueType Set( TValueType data);

  TreeNode<TValueType>* GetChild( int number ) const;

  TreeNode<TValueType>* GetParent( ) const;

  bool HasChildren( ) const;

  bool HasParent( ) const;

  void SetParent( TreeNode<TValueType>* n );

  int CountChildren( ) const;

  bool Remove( TreeNode<TValueType> *n );

  unsigned int GetNumberOfChildren( unsigned int depth=0, char * name=NULL ) const;

  bool ReplaceChild( TreeNode<TValueType> *oldChild, TreeNode<TValueType> *newChild );

  int ChildPosition( const TreeNode<TValueType> *node ) const;

  int ChildPosition( TValueType node ) const;

  void AddChild( TreeNode<TValueType> *node );

  virtual void AddChild( int number, TreeNode<TValueType> *node );

  virtual ChildrenListType* GetChildren( unsigned int depth=0, char * name=NULL) const;
  
  // To work directly on the internal list of children
  virtual ChildrenListType& GetChildrenList() {return m_Children;}

  virtual void SetData(TValueType data) {m_Data = data;}

protected:

  TreeNode();
  virtual ~TreeNode();
  TValueType m_Data;
  typename TreeNode<TValueType>::Pointer m_Parent;
  ChildrenListType m_Children;
};

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkTreeNode.txx"
#endif

#endif

