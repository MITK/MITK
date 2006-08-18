/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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


#include "mitkLevelWindowManager.h"
#include <itkCommand.h>
#include <mitkRenderingManager.h>

mitk::LevelWindowManager::LevelWindowManager()
{
  m_LevWinProp = NULL;
  m_IsObserverTagSet = false;
  m_IsPropertyModifiedTagSet = false;
  m_DataTree = NULL;
  m_AutoTopMost = true;
}

mitk::LevelWindowManager::~LevelWindowManager()
{
  if (m_IsObserverTagSet && m_DataTree.IsNotNull())
  {
    m_DataTree->RemoveObserver(m_ObserverTag);
    m_IsObserverTagSet = false;
  }
  if (m_IsPropertyModifiedTagSet && m_LevWinProp.IsNotNull())
  {
    m_LevWinProp->RemoveObserver(m_PropertyModifiedTag);
    m_IsPropertyModifiedTagSet = false;
  }
}

void mitk::LevelWindowManager::SetDataTree(DataTree* tree)
{
  if (tree)
  {
    if (m_IsObserverTagSet)
    {
      m_DataTree->RemoveObserver(m_ObserverTag);
      m_IsObserverTagSet = false;
    }
    m_DataTree = tree;
    itk::ReceptorMemberCommand<LevelWindowManager>::Pointer command = itk::ReceptorMemberCommand<LevelWindowManager>::New();
    command->SetCallbackFunction(this, &LevelWindowManager::TreeChanged);
    m_ObserverTag = m_DataTree->AddObserver(itk::TreeChangeEvent<mitk::DataTreeBase>(), command);
    m_IsObserverTagSet = true;
  }
}

void mitk::LevelWindowManager::OnPropertyModified(const itk::EventObject& )
{
  Modified();
}

void mitk::LevelWindowManager::SetAutoTopMostImage()
{
  m_AutoTopMost = true;
  if (m_IsPropertyModifiedTagSet)
  {
    m_LevWinProp->RemoveObserver(m_PropertyModifiedTag);
    m_IsPropertyModifiedTagSet = false;
  }

  if (m_DataTree.IsNotNull())
  {
    DataTreeIteratorClone iter = m_DataTree->GetIteratorToNode( m_DataTree, NULL );
    iter->GoToBegin();

    int maxLayer = itk::NumericTraits<int>::min();
    m_LevWinProp = NULL;
    while ( !iter->IsAtEnd() )
    {
      if ( (iter->Get().IsNotNull()) && (iter->Get()->IsVisible(NULL)) )
      {
        int layer = 0;
        iter->Get()->GetIntProperty("layer", layer);
        if ( layer >= maxLayer )
        {
          bool binary = false;
          iter->Get()->GetBoolProperty("binary", binary);
          if( binary == false)
          {
            mitk::LevelWindowProperty::Pointer levWinProp = dynamic_cast<mitk::LevelWindowProperty*>(iter->Get()->GetProperty("levelwindow").GetPointer());
            if (levWinProp.IsNotNull())
            { 
              m_LevWinProp = levWinProp;
              maxLayer = layer;
            }
          }
        }
      }
      ++iter;
    }
    if (m_LevWinProp.IsNotNull())
    {
      itk::ReceptorMemberCommand<LevelWindowManager>::Pointer command = itk::ReceptorMemberCommand<LevelWindowManager>::New();
      command->SetCallbackFunction(this, &LevelWindowManager::OnPropertyModified);
      m_PropertyModifiedTag = m_LevWinProp->AddObserver( itk::ModifiedEvent(), command );
      m_IsPropertyModifiedTagSet = true;
    }
    m_AutoTopMost = true;
    Modified();
  }
}

void mitk::LevelWindowManager::SetLevWinProp(LevelWindowProperty::Pointer levWinProp)
{
  if (levWinProp.IsNotNull())
  {
    if (m_IsPropertyModifiedTagSet)
    {
      m_LevWinProp->RemoveObserver(m_PropertyModifiedTag);
      m_IsPropertyModifiedTagSet = false;
    }
    m_LevWinProp = levWinProp;
    itk::ReceptorMemberCommand<LevelWindowManager>::Pointer command = itk::ReceptorMemberCommand<LevelWindowManager>::New();
    command->SetCallbackFunction(this, &LevelWindowManager::OnPropertyModified);
    m_PropertyModifiedTag = m_LevWinProp->AddObserver( itk::ModifiedEvent(), command );
    m_IsPropertyModifiedTagSet = true;
    m_AutoTopMost = false;
    Modified();
  }
}

mitk::LevelWindowProperty::Pointer mitk::LevelWindowManager::GetLevWinProp()
{
  return m_LevWinProp;
}

const mitk::LevelWindow& mitk::LevelWindowManager::GetLevelWindow()
{
  if (m_LevWinProp.IsNotNull())
    return m_LevWinProp->GetLevelWindow();
  else
    throw "No LevelWindow available!";
}

void mitk::LevelWindowManager::SetLevelWindow(const mitk::LevelWindow& levWin)
{
  if (m_LevWinProp.IsNotNull())
  {
    m_LevWinProp->SetLevelWindow(levWin);
    RenderingManager::GetInstance()->RequestUpdateAll();
  }
}

void mitk::LevelWindowManager::TreeChanged(const itk::EventObject&)
{
  bool stillExists = false;

  if (m_LevWinProp.IsNotNull() && !m_AutoTopMost && m_DataTree.IsNotNull())
  {
    DataTreeIteratorClone iter = m_DataTree->GetIteratorToNode( m_DataTree, NULL );
    iter->GoToBegin();

    while ( !iter->IsAtEnd() )
    {
      if ( (iter->Get().IsNotNull()) && (iter->Get()->IsVisible(NULL)) )
      {
        bool binary = false;
        iter->Get()->GetBoolProperty("binary", binary);
        if( binary == false)
        { 
          mitk::LevelWindowProperty* levWinProp = dynamic_cast<mitk::LevelWindowProperty*>(iter->Get()->GetProperty("levelwindow").GetPointer());
          if (levWinProp)
          {
            if (m_LevWinProp.GetPointer() == levWinProp)
              stillExists = true;
          }
        }
      }
      ++iter;
    }
  }
  if (stillExists == false && m_DataTree.IsNotNull())
  {
    SetAutoTopMostImage();
  }
  RenderingManager::GetInstance()->RequestUpdateAll();
}

mitk::DataTree* mitk::LevelWindowManager::GetDataTree()
{
  return m_DataTree.GetPointer();
}

bool mitk::LevelWindowManager::isAutoTopMost()
{
  return m_AutoTopMost;
}
