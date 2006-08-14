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
  m_ObserverTag = 0;
  m_PropertyModifiedTag = 0;
  m_DataTreeIteratorClone = NULL;
  m_AutoTopMost = TRUE;
}

mitk::LevelWindowManager::~LevelWindowManager()
{
  if(m_ObserverTag && m_DataTreeIteratorClone.IsNotNull() )
  {
    m_DataTreeIteratorClone->GetTree()->RemoveObserver(m_ObserverTag);
  }
  if( m_PropertyModifiedTag && m_LevWinProp.IsNotNull())
  {
    m_LevWinProp->RemoveObserver(m_PropertyModifiedTag);
  }
}

void mitk::LevelWindowManager::SetDataTreeIteratorClone(DataTreeIteratorClone& it)
{
  if(m_ObserverTag && m_DataTreeIteratorClone.IsNotNull() )
  {
    m_DataTreeIteratorClone->GetTree()->RemoveObserver(m_ObserverTag);
  }
  m_DataTreeIteratorClone = it;
  if(m_DataTreeIteratorClone.IsNotNull())
  {
    itk::ReceptorMemberCommand<LevelWindowManager>::Pointer command = itk::ReceptorMemberCommand<LevelWindowManager>::New();
    command->SetCallbackFunction(this, &LevelWindowManager::TreeChanged);
    m_ObserverTag = m_DataTreeIteratorClone->GetTree()->AddObserver(itk::TreeChangeEvent<mitk::DataTreeBase>(), command);
  }
}

void mitk::LevelWindowManager::OnPropertyModified(const itk::EventObject& e)
{
  Modified();
}

void mitk::LevelWindowManager::SetAutoTopMostImage()
{
  if( m_PropertyModifiedTag && m_LevWinProp.IsNotNull())
  {
    m_LevWinProp->RemoveObserver(m_PropertyModifiedTag);
  }
  m_DataTreeIteratorClone->GoToBegin();
  int maxLayer = itk::NumericTraits<int>::min();
  m_LevWinProp = NULL;
  while ( !m_DataTreeIteratorClone->IsAtEnd() )
  {
    if ( (m_DataTreeIteratorClone->Get().IsNotNull()) && (m_DataTreeIteratorClone->Get()->IsVisible(NULL)) )
    {
      int layer = 0;
      m_DataTreeIteratorClone->Get()->GetIntProperty("layer", layer);
      if ( layer >= maxLayer )
      {
        bool binary = false;
        m_DataTreeIteratorClone->Get()->GetBoolProperty("binary", binary);
        if( binary == false)
        {
          mitk::LevelWindowProperty::Pointer levWinProp = dynamic_cast<mitk::LevelWindowProperty*>(m_DataTreeIteratorClone->Get()->GetProperty("levelwindow").GetPointer());
          if (levWinProp.IsNotNull())
          {
            m_LevWinProp = levWinProp;
            maxLayer = layer;
          }
        }
      }
    }
    ++m_DataTreeIteratorClone;
  }
  if (m_LevWinProp)
  {
    itk::ReceptorMemberCommand<LevelWindowManager>::Pointer command = itk::ReceptorMemberCommand<LevelWindowManager>::New();
    command->SetCallbackFunction(this, &LevelWindowManager::OnPropertyModified);
    m_PropertyModifiedTag = m_LevWinProp->AddObserver( itk::ModifiedEvent(), command );
  }
  m_AutoTopMost = TRUE;
  Modified();
}

void mitk::LevelWindowManager::SetLevWinProp(LevelWindowProperty::Pointer levWinProp)
{
  if( m_PropertyModifiedTag && m_LevWinProp.IsNotNull())
  {
    m_LevWinProp->RemoveObserver(m_PropertyModifiedTag);
  }
  m_LevWinProp = levWinProp;
  itk::ReceptorMemberCommand<LevelWindowManager>::Pointer command = itk::ReceptorMemberCommand<LevelWindowManager>::New();
  command->SetCallbackFunction(this, &LevelWindowManager::OnPropertyModified);
  m_PropertyModifiedTag = m_LevWinProp->AddObserver( itk::ModifiedEvent(), command );
  m_AutoTopMost = FALSE;
  Modified();
}

mitk::LevelWindowProperty::Pointer mitk::LevelWindowManager::GetLevWinProp()
{
  return m_LevWinProp;
}

const mitk::LevelWindow& mitk::LevelWindowManager::GetLevelWindow()
{
  if (m_LevWinProp && m_LevWinProp.IsNotNull())
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

void mitk::LevelWindowManager::TreeChanged(const itk::EventObject& treeChangedEvent)
{
  bool stillExists = FALSE;
  if (m_LevWinProp && !m_AutoTopMost)
  {
      m_DataTreeIteratorClone->GoToBegin();
      while ( !m_DataTreeIteratorClone->IsAtEnd() )
      {
        if ( (m_DataTreeIteratorClone->Get().IsNotNull()) && (m_DataTreeIteratorClone->Get()->IsVisible(NULL)) )
        {
          bool binary = false;
          m_DataTreeIteratorClone->Get()->GetBoolProperty("binary", binary);
          if( binary == false)
          { 
            mitk::LevelWindowProperty::Pointer levWinProp = dynamic_cast<mitk::LevelWindowProperty*>(m_DataTreeIteratorClone->Get()->GetProperty("levelwindow").GetPointer());
            if (levWinProp.IsNotNull())
            {
              if (m_LevWinProp == levWinProp)
                stillExists = TRUE;
            }
          }
        }
        ++m_DataTreeIteratorClone;
      }
  }
  if (stillExists == FALSE)
  {
    SetAutoTopMostImage();
  }
  RenderingManager::GetInstance()->RequestUpdateAll();
}

mitk::DataTreeIteratorClone& mitk::LevelWindowManager::GetDataTreeIteratorClone()
{
  return m_DataTreeIteratorClone;
}

bool mitk::LevelWindowManager::isAutoTopMost()
{
  return m_AutoTopMost;
}
