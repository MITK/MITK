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

#include <mitkImageToItkMultiplexer.h>
#include <mitkStringProperty.h>
#include <mitkLevelWindowProperty.h>

template < typename TImageType >
mitk::DataTreeNode::Pointer 
mitk::DataTreeHelper::AddItkImageToDataTree(TImageType* itkImage, mitk::DataTreeIteratorBase* iterator, const char* name, bool replaceByName)
{
  itkImage->Update();

  mitk::Image::Pointer mitkImage = mitk::Image::New();
  mitk::CastToMitkImage(itkImage, mitkImage);

  mitk::DataTreeNode::Pointer node = NULL;
  bool done=false;
  if((replaceByName) && (name != NULL))
  {
    mitk::DataTreeIteratorClone it=iterator;
    mitk::DataTreeIteratorClone itByName = ((mitk::DataTree *) it->GetTree())->GetNext("name", new mitk::StringProperty( name ));

    if( (itByName->IsAtEnd() == false) && (itByName->Get().IsNotNull()) )
    {
      node = itByName->Get();
      node->SetData(mitkImage);
      done = true;
    }
  }
  if(done==false)
  {
    node=mitk::DataTreeNode::New();
    node->SetData(mitkImage);
    if(name!=NULL)
      node->SetProperty("name", new mitk::StringProperty(name));
    mitk::DataTreeNodeFactory::SetDefaultImageProperties(node);
    iterator->Add(node);
  }

  mitk::LevelWindowProperty::Pointer levWinProp = new mitk::LevelWindowProperty();
  mitk::LevelWindow levelwindow;
  levelwindow.SetAuto( mitkImage->GetPic() );
  levWinProp->SetLevelWindow( levelwindow );
  node->SetProperty( "levelwindow", levWinProp );

  return node;
}
