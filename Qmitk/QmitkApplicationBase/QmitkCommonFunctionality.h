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


#ifndef _CommonFunctionality__h_
#define _CommonFunctionality__h_

// std
#include <string>

#include <ipPic.h>

// itk includes
#include <itkMinimumMaximumImageCalculator.h>

// mitk includes
#include <mitkProperties.h>
#include <mitkLevelWindowProperty.h>
#include <mitkStringProperty.h>
#include <mitkDataTreeNode.h>
#include <mitkSurface.h>
#include <mitkDataTreeNodeFactory.h>

#include <qfiledialog.h>
#include "ipPic.h"
#include "ipFunc.h"

#include <qstring.h>
#include <qfiledialog.h>
#include <qregexp.h>

class CommonFunctionality
{

public:
  /** \brief compute min and max 
  */
  template < typename TImageType >
    static void MinMax(typename TImageType::Pointer image, float &min, float &max)
  {
    typedef itk::MinimumMaximumImageCalculator<TImageType> MinMaxCalcType;
    typename MinMaxCalcType::Pointer minmax = MinMaxCalcType::New();
    minmax->SetImage( image );
    minmax->ComputeMinimum();
    minmax->ComputeMaximum();

    min = (float) minmax->GetMinimum();
    max = (float) minmax->GetMaximum();
  }


  /** 
  * \brief converts the itk image to mitk image, creates a datatreenode and adds the node to 
  * the referenced datatree
  */
  template < typename TImageType >
    static mitk::DataTreeNode::Pointer 
    AddItkImageToDataTree(typename TImageType::Pointer itkImage, mitk::DataTreeIteratorBase* iterator, std::string str)
  {
    mitk::DataTreeIteratorClone it=iterator;

    mitk::Image::Pointer image = mitk::Image::New();
    image->InitializeByItk(itkImage.GetPointer());
    image->SetVolume(itkImage->GetBufferPointer());

    mitk::DataTreeNode::Pointer node = NULL;
    mitk::DataTreeIteratorClone subTree = ((mitk::DataTree *) it->GetTree())->GetNext("name", new mitk::StringProperty( str.c_str() ));

    if (subTree->IsAtEnd() || subTree->Get().IsNull() )
    {
      node=mitk::DataTreeNode::New();
      mitk::StringProperty::Pointer nameProp = new mitk::StringProperty(str.c_str());
      node->SetProperty("name",nameProp);
      it->Add(node);
    }
    else
    {
      node = subTree->Get();
    }
    node->SetData(image);

    mitk::LevelWindowProperty::Pointer levWinProp = new mitk::LevelWindowProperty();
    mitk::LevelWindow levelWindow;
    //levelWindow.SetAuto( image->GetPic() );
    //levWinProp->SetLevelWindow(levelWindow);
    //node->GetPropertyList()->SetProperty("levelwindow",levWinProp);
    node->SetProperty("volumerendering",new mitk::BoolProperty(false));

    float extrema[2];
    extrema[0] = 0;
    extrema[1] = 4096;

    CommonFunctionality::MinMax<TImageType>(itkImage,extrema[0],extrema[1]);
    if (extrema[0] == 0 && extrema[1] ==1)
    {
      mitk::BoolProperty::Pointer binProp = new mitk::BoolProperty(true);
      node->GetPropertyList()->SetProperty("binary",binProp);
    }
    return node;
  }


  /** 
  * \brief converts the itk image to mitk image, creates a datatreenode and adds the node to 
  * the referenced datatree
  */
  template < typename TMeshType >
    static void AddItkMeshToDataTree(typename TMeshType::Pointer itkMesh, mitk::DataTreeIteratorBase* iterator, std::string str)
  {
    mitk::DataTreeIteratorClone it=iterator;

    mitk::DataTreeNode::Pointer node = NULL;
    mitk::DataTreeIteratorClone subTree = ((mitk::DataTree *) it->GetTree())->GetNext("name", new mitk::StringProperty( str.c_str() ));

    if (subTree->IsAtEnd() || subTree->Get() == NULL )
    {
      node=mitk::DataTreeNode::New();
      mitk::StringProperty::Pointer nameProp = new mitk::StringProperty(str.c_str());
      node->SetProperty("name",nameProp);
      it->Add(node);
    }
    else
    {
      node = subTree->Get();
    }

    mitk::Surface::Pointer surface = mitk::Surface::New();
    //    vtkPolyData* polys = MeshUtil<TMeshType>::meshToPolyData( itkMesh );
    /**
    * @todo include Algorithms/itkMeshDeformation into Framework module so the upper line can be used
    * and the conversion works correctly
    */
    vtkPolyData* polys = vtkPolyData::New();
    surface->SetVtkPolyData(polys);
    node->SetData( surface );
    node->SetProperty("layer", new mitk::IntProperty(1));
    node->SetVisibility(true,NULL);

    float meshColor[3] = {.5f,.5f,.5f};
    node->SetColor(meshColor,  NULL );
    node->SetVisibility(true, NULL );
  }
  /** 
  * \brief converts the itk image to mitk image, creates a datatreenode and adds the node to 
  * the referenced datatree
  */
  static mitk::DataTreeNode::Pointer AddVtkMeshToDataTree(vtkPolyData* polys, mitk::DataTreeIteratorBase* iterator, std::string str)
  {
    mitk::DataTreeIteratorClone it=iterator;

    mitk::DataTreeNode::Pointer node = NULL;
    mitk::DataTreeIteratorClone subTree = ((mitk::DataTree *) it->GetTree())->GetNext("name", new mitk::StringProperty( str.c_str() ));

    if (subTree->IsAtEnd() || subTree->Get().IsNull() )
    {
      node=mitk::DataTreeNode::New();
      mitk::StringProperty::Pointer nameProp = new mitk::StringProperty(str.c_str());
      node->SetProperty("name",nameProp);
      it->Add(node);
    }
    else
    {
      node = subTree->Get();
    }

    mitk::Surface::Pointer surface = mitk::Surface::New();
    surface->SetVtkPolyData(polys);
    node->SetData( surface );
    node->SetProperty("layer", new mitk::IntProperty(1));
    node->SetVisibility(true,NULL);

    float meshColor[3] = {1.0,0.5,0.5};
    node->SetColor(meshColor,  NULL );
    node->SetVisibility(true, NULL );

    return node;
  }

  /** 
  * \brief creates a datatreenode for th PIC image and adds the node to 
  * the referenced datatree
  */
  static mitk::DataTreeNode::Pointer AddPicImageToDataTree(ipPicDescriptor * pic, mitk::DataTreeIteratorBase* iterator, std::string str)
  {
    mitk::DataTreeIteratorClone it=iterator;

    mitk::Image::Pointer image = mitk::Image::New();
    image->Initialize(pic);
    image->SetPicVolume(pic);

    mitk::DataTreeNode::Pointer node = NULL;
    mitk::DataTreeIteratorClone subTree = ((mitk::DataTree *) it->GetTree())->GetNext("name", new mitk::StringProperty( str.c_str() ));

    if (subTree->IsAtEnd() || subTree->Get().IsNull() )
    {
      node=mitk::DataTreeNode::New();
      mitk::StringProperty::Pointer nameProp = new mitk::StringProperty(str.c_str());
      node->SetProperty("name",nameProp);
      it->Add(node);
    }
    else
    {
      node = subTree->Get();
    }
    node->SetData(image);

    mitk::LevelWindowProperty::Pointer levWinProp = new mitk::LevelWindowProperty();
    mitk::LevelWindow levelWindow;
    levelWindow.SetAuto( image->GetPic() );
    levWinProp->SetLevelWindow(levelWindow);
    node->GetPropertyList()->SetProperty("levelwindow",levWinProp);
    return node;
  }

  static void SetWidgetPlanesEnabled(mitk::DataTree* dataTree, bool enable)
  {
    // widget plane are made visible again
    mitk::DataTreeIteratorClone it = dataTree->GetNext("name", new mitk::StringProperty("widget1Plane"));
    mitk::DataTreeNode::Pointer node = it->Get();
    if ( node.IsNotNull() )  {
      node->SetVisibility(enable, NULL);
    }

    it = dataTree->GetNext("name", new mitk::StringProperty("widget2Plane"));
    node = it->Get();
    if ( node.IsNotNull() )  
    {
      node->SetVisibility(enable, NULL);
    }
    it = dataTree->GetNext("name", new mitk::StringProperty("widget3Plane"));
    node = it->Get();
    if ( node.IsNotNull() )  
    {
      node->SetVisibility(enable, NULL);
    }
  }


/*  static void SaveImage(std::string imageName, mitk::DataTreeIteratorBase* iterator)
{
  std::cout << "1" << std::endl;
  mitk::DataTreeIteratorBase* it=iterator->clone();
  mitk::DataTreeNode::Pointer node = NULL;
  mitk::DataTreeIterator* subTree = ((mitk::DataTree *) it->getTree())->GetNext("name", new mitk::StringProperty( imageName ));

  std::cout << "2" << std::endl;

  if (subTree != NULL && subTree->get() != NULL)
  {
    node = subTree->get();
    mitk::BaseData::Pointer data=node->GetData();

    if (data.IsNotNull())
    {
      std::cout << "3" << std::endl;

      mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(data.GetPointer());
      ipPicDescriptor * picImage = image->GetPic();

      std::cout << "4" << std::endl;
      picImage = ipFuncRefl(picImage,3);

      char* fileName = (char*) QFileDialog::getSaveFileName(QString(imageName),"DKFZ Pic (*.seq *.pic *.pic.gz *.seq.gz *.dcm)").ascii();
      if (fileName == NULL ) 
      {
        fileName = (char*)imageName.c_str();
      }
      ipPicPut(fileName, picImage);
    }

  }
}
*/

    static mitk::DataTreeNode::Pointer FileOpen()
    {
#ifdef MBI_INTERNAL
      QString fileName = QFileDialog::getOpenFileName(NULL,"all (*.seq *.pic *.pic.gz *.seq.gz *.pvtk *.stl *.vtk *.ves *.uvg *.par *.dcm hpsonos.db HPSONOS.DB);;DKFZ Pic (*.seq *.pic *.pic.gz *.seq.gz);;surface files (*.stl *.vtk);;stl files (*.stl);;vtk surface files (*.vtk);;vtk image files (*.pvtk);;vessel files (*.ves *.uvg);;par/rec files (*.par);;DSR files (hpsonos.db HPSONOS.DB);;DICOM files (*.dcm)");
#else
      QString fileName = QFileDialog::getOpenFileName(NULL,"all (*.seq *.pic *.pic.gz *.seq.gz *.pvtk *.stl *.vtk *.par);;DKFZ Pic (*.seq *.pic *.pic.gz *.seq.gz);;surface files (*.stl *.vtk);;stl files (*.stl);;vtk surface files (*.vtk);;vtk image files (*.pvtk);;par/rec files (*.par);;DICOM files (*.dcm)");
#endif
      if ( !fileName.isNull() )
      {
        return FileOpen(fileName.ascii());
      }
      else
      {
        return NULL;
      }
    }

    static mitk::DataTreeNode::Pointer FileOpen( const char * fileName )
    {
      mitk::DataTreeNodeFactory::Pointer factory = mitk::DataTreeNodeFactory::New();

      try
      {
        factory->SetFileName( fileName );
        factory->Update();
        return factory->GetOutput( 0 );
      }
      catch ( itk::ExceptionObject & ex )
      {
        itkGenericOutputMacro( << "Exception during file open: " << ex );
        return NULL;
      }
    }

    static mitk::DataTreeNode::Pointer FileOpenImageSequence()
    {
      QString fileName = QFileDialog::getOpenFileName(NULL,"DKFZ Pic (*.seq *.pic *.pic.gz *.seq.gz);;Sets of 2D slices (*.pic *.pic.gz *.png *.dcm);;stl files (*.stl);;DICOM files(*.dcm *.DCM)");

      if ( !fileName.isNull() )
      {
        int fnstart = fileName.findRev( QRegExp("[/\\\\]"), fileName.length() );
        if(fnstart<0) fnstart=0;
        int start = fileName.find( QRegExp("[0-9]"), fnstart );
        if(start<0)
        {
          return FileOpen(fileName.ascii());;
        }

        char prefix[1024], pattern[1024];

        strncpy(prefix, fileName.ascii(), start);
        prefix[start]=0;

        int stop=fileName.find( QRegExp("[^0-9]"), start );
        sprintf(pattern, "%%s%%0%uu%s",stop-start,fileName.ascii()+stop);


        mitk::DataTreeNodeFactory::Pointer factory = mitk::DataTreeNodeFactory::New();

        factory->SetFilePattern( pattern );
        factory->SetFilePrefix( prefix );
        factory->Update();
        return factory->GetOutput( 0 );
      }
      else
      {
        return NULL;
      }
    }

};
#endif // _CommonFunctionality__h_
