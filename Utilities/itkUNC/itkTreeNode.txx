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
#ifndef _itkTreeNode_txx
#define _itkTreeNode_txx

#include "itkTreeNode.h"

namespace itk
{

/** Constructor */
/*template <class TValueType>
TreeNode<TValueType>::TreeNode( TValueType data, TreeNode<TValueType>* parent, int defaultChildrenCount )
{
  m_Parent = parent;
  m_Children.reserve(defaultChildrenCount);
  m_Data = data;
}*/


/** Constructor */
/*template <class TValueType>
TreeNode<TValueType>::TreeNode( TreeNode<TValueType>* parent, int defaultChildrenCount )
{
  m_Parent = parent;
  m_Children.reserve(defaultChildrenCount);
}*/

/** Constructor */
template <class TValueType>
TreeNode<TValueType>::TreeNode()
{
  m_Children.reserve(3);
  m_Parent = NULL;
}


/** Destructor */
template <class TValueType>
TreeNode<TValueType>::~TreeNode() 
{
  if ( m_Parent )
    {
    m_Parent->Remove(this);
    }
    
 for ( int i=m_Children.size() - 1; i >= 0; i-- )
   {
   delete m_Children[i];
   }
  m_Children.clear();
  m_Data = 0;
}


/** Return the parent node */
template <class TValueType>
TreeNode<TValueType>* 
TreeNode<TValueType>
::GetParent( ) const 
{
  return m_Parent;
}


/** Get a child */
template <class TValueType>
TreeNode<TValueType>* 
TreeNode<TValueType>
::GetChild( int number ) const 
{
  if ( (unsigned int)number < m_Children.size() )
    {
    return m_Children[number];
    }
  else
    {
    return NULL;
    }
}


/** Set the value of a node */
template <class TValueType>
TValueType TreeNode<TValueType>::Set(TValueType data)
{
  TValueType help = this->data;
  this->data = data;
  return help;
}


/** Get the data of node */
template <class TValueType>
const TValueType& TreeNode<TValueType>::Get() const 
{
  return m_Data;
}


/**
 * Überprüft, ob der aktuelle Knoten einen Vorgänger besitzt
 * @return true der aktuelle Knoten besitzt einen Vorgänger
 *         false der aktuelle Knoten besitzt keinen Vorgänger,
 *         d. h. der aktuelle Knoten ist der root
 */
template <class TValueType>
bool 
TreeNode<TValueType>::HasParent() const 
{
  return (m_Parent)?true:false;
}


/**
 * Setzt den Vaterknoten
 * @param n Vaterkonten
 */
template <class TValueType>
void 
TreeNode<TValueType>::SetParent( TreeNode<TValueType>* n) 
{
  m_Parent = n;
}

/**
 * Überprüft, ob der Knoten Nachfolger besitzt
 * @return true der aktuelle Knoten besitzt Kindknoten
 *               false der aktuelle Knoten besitzt keine Kindknoten,
 *         d. h. er ist ein Blattknoten (leaf)
 */
template <class TValueType>
bool TreeNode<TValueType>::HasChildren( ) const {
    return (m_Children.size()>0)?true:false;  // return m_Children.size() > 0; sollte auch reichen!!
}


/**
 * Gibt die Anzahl der direkten Kindknoten zurück
 * @return Anzahl der unmitelbaren Kinder
 */
template <class TValueType>
int 
TreeNode<TValueType>
::CountChildren( ) const 
{
    return m_Children.size();
}


/** Remove a child node from the current node */
template <class TValueType>
bool 
TreeNode<TValueType>
::Remove( TreeNode<TValueType> *n ) 
{
  typename std::vector<Pointer>::iterator pos;
  pos = std::find(m_Children.begin(), m_Children.end(), n );
  if ( pos != m_Children.end() ) 
    {
    m_Children.erase(pos);
    n->SetParent(NULL);
    return true;
    }
  return false;
}


/**
 * Ersetzt einen Kindknoten durch einen neuen
 * @param oldChild zu ersetzender Kindknoten
 * @param newChild neuer Kindknoten
 * @return Erfolg der Operation
 */
template <class TValueType>
bool TreeNode<TValueType>::ReplaceChild( TreeNode<TValueType> *oldChild, TreeNode<TValueType> *newChild ) {
    int size = m_Children.size();

    for ( int i=0; i<size; i++ ) {
        if ( m_Children[i] == oldChild ) {
            m_Children[i] = newChild;
            return true;
        }
    }
    return false;
}


/**
 * Liefert die Position des spezifizierten Kindknotens
 * @param node gesuchter Kindknoten
 * @return Position des Knotens, sollte der Node nicht enthalten sein
 * wird -1 zurück gegeben
 */
template <class TValueType>
int TreeNode<TValueType>::ChildPosition( const TreeNode<TValueType> *node ) const 
{
  for ( unsigned int i=0; i < m_Children.size(); i++ )
    {
    if ( m_Children[i] == node )
      {
      return i;
      }
    }
  return -1;
}

/**
 * Liefert die Position des Kindknotens, der element enthält
 * @param element Element vom Typ TValueType
 * @return Position des Elements, sollte das Element nicht enthalten sein
 * wird -1 zurück gegeben
 */
template <class TValueType>
int TreeNode<TValueType>::ChildPosition( TValueType element ) const {
    for ( unsigned int i=0; i < m_Children.size(); i++ ) {
        if ( m_Children[i]->Get() == element )
            return i;
    }

    return -1;
}


/**
 * Fügt einen Knoten als Kind hinzu
 * @param node neuer Kindknoten
 */
template <class TValueType>
void TreeNode<TValueType>::AddChild( TreeNode<TValueType> *node ) 
{
  node->SetParent(this);
  m_Children.push_back(node);
}

/**
 * Fügt einen Knoten als Kind an die vorgegebene Position hinzu
 * oder ersetzt den an dieser Position existierenden Knoten
 * Fehlende Zwischenpositionen werden mit NULL aufgefüllt.
 * @param number position des neuen Knotens
 * @param node neuer Knoten
 */
template <class TValueType>
void 
TreeNode<TValueType>
::AddChild( int number, TreeNode<TValueType> *node ) 
{  
 int size = m_Children.size();

    if ( number > size ) {
        for ( int i=size; i <= number; i++ )
            m_Children[i] = NULL;

        m_Children[number] = node;
        return;
    }

    TreeNode<TValueType>* old = m_Children[number];
    if ( old != NULL )
        delete old;

    m_Children[number] = node;
}

/** Get the number of children given a name and a depth */
template <class TValueType>
unsigned int
TreeNode<TValueType>
::GetNumberOfChildren( unsigned int depth, char * name ) const
{
  typename ChildrenListType::const_iterator it = m_Children.begin();
  typename ChildrenListType::const_iterator itEnd = m_Children.end();

  unsigned int cnt = 0;
  while(it != itEnd)
    {
    if(name == NULL || strstr(typeid(**it).name(), name))
      {
      cnt++;
      }
    it++;
    }

  it = m_Children.begin();
  itEnd = m_Children.end();
  if( depth > 0 )
    {
    while(it != itEnd)
      {
      cnt += (*it)->GetNumberOfChildren( depth-1, name );
      it++;
      }
    }

  return cnt;
}


/** Get children given a name and a depth */
template <class TValueType>
typename TreeNode<TValueType>::ChildrenListType* 
TreeNode<TValueType>
::GetChildren( unsigned int depth, char * name) const
{
  ChildrenListType * children = new ChildrenListType;

  typename ChildrenListType::const_iterator childrenListIt = 
    m_Children.begin();
  typename ChildrenListType::const_iterator childrenListEnd = 
    m_Children.end();

  while( childrenListIt != childrenListEnd )
    {
    if( name == NULL )
      {
      children->push_back(*childrenListIt);
      }
    if( depth > 0 )
      {
      ChildrenListType * nextchildren = (**childrenListIt).GetChildren(depth-1, name);  
      // Add the child to the current list
      typename ChildrenListType::const_iterator nextIt = nextchildren->begin();
      while(nextIt != nextchildren->end())
        {
        children->push_back(*nextIt);
        nextIt++;
        }
      delete nextchildren;
      }
    childrenListIt++;
    }

  return children;
}


} // namespace itk

#endif
