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
#include <mitkImageToItkMultiplexer.h>
#include <mitkDataTree.h>

#include <qfiledialog.h>
#include "ipPic.h"
#include "ipFunc.h"

#include <qstring.h>
#include <qfiledialog.h>
#include <qregexp.h>

#include <itkRescaleIntensityImageFilter.h>
#include "itkImageSeriesReader.h"
#include "itkImageSeriesWriter.h"
#include "itkImageFileWriter.h"
#include "itkNumericSeriesFileNames.h"

#include <itksys/SystemTools.hxx>

#include <vtkSTLWriter.h>
#include <vtkPolyDataWriter.h>


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
    node->GetPropertyList()->SetProperty("levelwindow",levWinProp);
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


static mitk::DataTreeNode::Pointer FileOpen()
    {
#ifdef MBI_INTERNAL
      QString fileName = QFileDialog::getOpenFileName(NULL,"all (*.seq *.pic *.pic.gz *.seq.gz *.pvtk *.stl *.vtk *.ves *.uvg *.par *.dcm hpsonos.db HPSONOS.DB);;DKFZ Pic (*.seq *.pic *.pic.gz *.seq.gz);;surface files (*.stl *.vtk);;stl files (*.stl);;vtk surface files (*.vtk);;vtk image files (*.pvtk);;vessel files (*.ves *.uvg);;par/rec files (*.par);;DSR files (hpsonos.db HPSONOS.DB);;DICOM files (*.dcm)");
#else
      QString fileName = QFileDialog::getOpenFileName(NULL,"all (*.seq *.pic *.pic.gz *.seq.gz *.pvtk *.stl *.vtk *.par);;DKFZ Pic (*.seq *.pic *.pic.gz *.seq.gz);;surface files (*.stl *.vtk);;stl files (*.stl);;vtk surface files (*.vtk);;vtk image files (*.pvtk);;par/rec files (*.par);;DICOM files (*.dcm)");
#endif
      if ( !fileName.isNull() )
      {
        mitk::DataTreeNode::Pointer result = FileOpen(fileName.ascii());
        if ( result.IsNull() )
        {
          return FileOpenImageSequence(fileName);
        }
        else 
        {
          return result;
        }
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
        return FileOpenImageSequence(fileName);
      }
      else
      {
        return NULL;
      }
    }

    static mitk::DataTreeNode::Pointer FileOpenImageSequence(QString fileName)
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


    template < typename TImageType >
    static void SaveImage(mitk::Image* image)
    {
      QString fileName = QFileDialog::getSaveFileName(QString("NewImage.mhd"),"DKFZ Pic (*.tif *.tiff *.png *.jpeg *.pic)");
      if (fileName == NULL ) 
      {
        fileName = QString("NewImage.mhd");
      }

      try 
      {
        typename TImageType::Pointer itkImage = TImageType::New();
        mitk::CastToItkImage( image, itkImage );
        
        if ( fileName.contains(".pic") == 0 )
        {
          if (fileName.contains(".mhd") != 0)
          {
            itk::ImageFileWriter<TImageType>::Pointer writer = itk::ImageFileWriter<TImageType>::New();
            writer->SetInput( itkImage );
            writer->SetFileName( fileName.ascii() );
            writer->Update();
          }
          else if (fileName.contains(".png") != 0 || fileName.contains(".tif") != 0 || fileName.contains(".jpg") != 0)
          {
            typedef itk::Image<unsigned char,3> OutputImage3DType;
            typedef itk::Image<unsigned char,2> OutputImage2DType;
            itk::RescaleIntensityImageFilter<TImageType, OutputImage3DType>::Pointer rescaler = itk::RescaleIntensityImageFilter<TImageType, OutputImage3DType>::New();
            rescaler->SetInput(itkImage);
            rescaler->SetOutputMinimum(0);
            rescaler->SetOutputMaximum(255);
            itk::ImageSeriesWriter<OutputImage3DType, OutputImage2DType>::Pointer writer = itk::ImageSeriesWriter<OutputImage3DType, OutputImage2DType >::New();
            writer->SetInput( rescaler->GetOutput() );
            int pos = fileName.findRev(".",fileName.length()-1);
            fileName.insert(pos,".%d");
            writer->SetSeriesFormat( fileName.ascii() );
            writer->Update();
          }
          else 
          {
            typedef itk::Image<typename TImageType::PixelType,2> OutputImage2DType;
            itk::ImageSeriesWriter<TImageType, OutputImage2DType>::Pointer writer = itk::ImageSeriesWriter<TImageType, OutputImage2DType >::New();
            writer->SetInput( itkImage );
            int pos = fileName.findRev(".",fileName.length()-1);
            fileName.insert(pos,".%d");
            writer->SetSeriesFormat( fileName.ascii() );
            writer->Update();
          }

        }
        else
        {
          ipPicDescriptor * picImage = image->GetPic();
          picImage = ipFuncRefl(picImage,3);
          ipPicPut((char*)(fileName.ascii()), picImage);
        }

      }
      catch ( itk::ExceptionObject &err)
      {
        std::cout << "Exception object caught! in texture measure calculations" <<std::endl;
        std::cout << err << std::endl;
        return;
      }
    }

    static void SaveSurface(mitk::Surface* surface, std::string name = 0)
    {
      std::string selectedItemsName = itksys::SystemTools::GetFilenameWithoutExtension(name);
      selectedItemsName += ".stl";
      QString fileName = QFileDialog::getSaveFileName(QString(selectedItemsName),"Surface Data(*.stl *.vtk)");
        if (fileName != NULL ) 
        {
          if(fileName.endsWith(".stl")==true)
          {
            vtkSTLWriter *writer=vtkSTLWriter::New();
            writer->SetInput( surface->GetVtkPolyData() );
            writer->SetFileName(fileName.ascii());
            writer->SetFileTypeToBinary();
            writer->Write();
            writer->Delete();
          }
          else 
          {
            if (fileName.endsWith(".vtk")==false) fileName += ".vtk";
            vtkPolyDataWriter * polyWriter = vtkPolyDataWriter::New();
            polyWriter->SetInput( surface->GetVtkPolyData() );
            polyWriter->SetFileName(fileName.ascii());
            polyWriter->Write();
            polyWriter->Delete();
          }
        }
    }

    static mitk::Image* GetFirstImageInDataTree(mitk::DataTree::Pointer dataTree)
    {
      mitk::DataTreePreOrderIterator dataTreeIterator( dataTree );

      if ( dataTree.IsNull() )
      {
        std::cout << "iterator to data tree is NULL. I cannot work without datatree !!"  << std::endl;
        return NULL;
      }

      mitk::DataTreeIteratorClone it = dataTreeIterator;
      while ( !it->IsAtEnd() )
      {
        mitk::DataTreeNode::Pointer node = it->Get();
        if ( node->GetData() != NULL )
        {
          // access the original image
          mitk::Image::Pointer img = dynamic_cast<mitk::Image*>( node->GetData() );

          // enquiry whether img is NULL
          if ( img.IsNotNull() )
          {
            return img;
          }
        }
        ++it;
      }
      std::cout << "No node containing an mitk::Image found, returning NULL..." << std::endl;
      return NULL;
    }


};
#endif // _CommonFunctionality__h_
