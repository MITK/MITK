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


#ifndef DATATREEHELPER_H_HEADER_INCLUDED_C1C7797C
#define DATATREEHELPER_H_HEADER_INCLUDED_C1C7797C

#include "mitkCommon.h"
#include "mitkDataTree.h"

#include "mitkImageCast.h"
#include "mitkStringProperty.h"
#include "mitkLevelWindowProperty.h"
#include "mitkDataTreeNodeFactory.h"

namespace mitk
{

  //##Documentation
  //## @brief Convenience methods for common tasks concerning the data tree
  //## @ingroup DataTree
  namespace DataTreeHelper
  {
    //##Documentation
    //## @brief Search a data tree for (the first) iterator to a given node
    MITK_CORE_EXPORT DataTreeIteratorClone FindIteratorToNode(mitk::DataTreeBase* tree, const DataTreeNode* node);

    //##Documentation
    //## @brief Search a data tree for (the first) iterator to a given node
    MITK_CORE_EXPORT DataTreeIteratorClone FindIteratorToNode(const DataTreeIteratorBase* startPosition, const DataTreeNode* node);

    //##Documentation
    //## @brief Add itk-image to data tree
    //##
    //## The itk-image is converted into an mitk::Image.
    //## @param itkImage the itk-image to be added
    //## @param iterator the iterator used for adding the node, containing the image
    //## @param name string with the name of the image, stored in the property "name"
    //## @param replaceByName if @a true, the tree is searched starting at
    //##   @a iterator for a node called @a name, which data is replaced with
    //##   @a itkImage
    template < typename TImageType >
    mitk::DataTreeNode::Pointer
    AddItkImageToDataTree(TImageType* itkImage, mitk::DataTreeIteratorBase* iterator, const char* name=NULL, bool replaceByName=false)
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
      levelwindow.SetAuto( mitkImage );
      levWinProp->SetLevelWindow( levelwindow );
      node->SetProperty( "levelwindow", levWinProp );

      return node;
    }
    /** Get child node which has a specified name
     * @returns child node of iterator which has the given name
    */
    MITK_CORE_EXPORT DataTreeIteratorClone GetNamedChild(mitk::DataTreeIteratorBase* iterator, std::string name);
  };

} // namespace mitk

#endif /* DATATREEHELPER_H_HEADER_INCLUDED_C1C7797C */


