/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
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

/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you want to add, delete, or rename functions or slots, use
** Qt Designer to update this file, preserving your code.
**
** You should not define a constructor or destructor in this file.
** Instead, write your code in functions called init() and destroy().
** These will automatically be called by the form's constructor and
** destructor.
*****************************************************************************/

#include <vtkRenderWindow.h>
#include <mitkBaseRenderer.h>

#include <mitkGlobalInteraction.h>
#include <mitkFocusManager.h>

#include <qapplication.h>
#include <qclipboard.h>

void QmitkRenderWindowSelector::init()
{
  m_UpdateRendererListCommand = itk::SimpleMemberCommand<QmitkRenderWindowSelector>::New();
  m_UpdateRendererListCommand->SetCallbackFunction(
    this, &QmitkRenderWindowSelector::UpdateRendererList
  );

  mitk::FocusManager* fm =
    mitk::GlobalInteraction::GetInstance()->GetFocusManager();
  mitk::FocusEvent fe;
  fm->AddObserver(fe, m_UpdateRendererListCommand);

  this->UpdateRendererList();
}

void QmitkRenderWindowSelector::UpdateRendererList()
{
  vtkRenderWindow* focusedRenderWindow = NULL;

  mitk::FocusManager* fm =
    mitk::GlobalInteraction::GetInstance()->GetFocusManager();

  mitk::BaseRenderer::ConstPointer br = fm->GetFocused();
  if (br.IsNotNull())
  {
    focusedRenderWindow = br->GetRenderWindow();
  }

  int selectedItem = -1;
  int itemNumber = 0;
  m_lbRenderWindows->clear();


  for(mitk::BaseRendererMapType::iterator mapit = mitk::baseRendererMap.begin(); 
      mapit != mitk::baseRendererMap.end(); mapit++, itemNumber++)
  {
    if( (*mapit).second->GetName())
    {
      m_lbRenderWindows->insertItem(QString((*mapit).second->GetName()));
      if(focusedRenderWindow==(*mapit).first)
        selectedItem = itemNumber;
    }
  }

  if (selectedItem>=0)
  {
    m_lbRenderWindows->setCurrentItem(selectedItem);
  }
  else
  {
    m_lbRenderWindows->clearSelection();
  }
}


vtkRenderWindow* QmitkRenderWindowSelector::GetSelectedRenderWindow()
{  
  int selectedItem = m_lbRenderWindows->currentItem();
  int itemNumber = -1;

  mitk::BaseRendererMapType::iterator mapit;
  for(mapit = mitk::baseRendererMap.begin(); 
      mapit != mitk::baseRendererMap.end(); mapit++, itemNumber++)
  {
    ++itemNumber;
    if(itemNumber==selectedItem)
      break;
  }
  if(itemNumber==selectedItem)
  {
    return (*mapit).first;
  }
  return NULL;
}
