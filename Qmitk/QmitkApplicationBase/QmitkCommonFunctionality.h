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

#include <mitkImageAccessByItk.h>

// std
#include <string>
#include <vector>

#include <ipPic/ipPic.h>

// itk includes
#include <itkMinimumMaximumImageCalculator.h>

// mitk includes
#include <mitkProperties.h>
#include <mitkLevelWindowProperty.h>
#include <mitkStringProperty.h>
#include <mitkDataTreeNode.h>
#include <mitkSurface.h>
#include <mitkDataTreeNodeFactory.h>
#include <mitkImageCast.h>
#include <mitkDataTree.h>
#include <mitkPicFileReader.h>

#include <qfiledialog.h>
#include "ipPic/ipPic.h"
#include "ipFunc/ipFunc.h"

#include <qstring.h>
#include <qfiledialog.h>
#include <qregexp.h>

#include <itkRescaleIntensityImageFilter.h>
#include "itkImageSeriesReader.h"
#include "itkImageSeriesWriter.h"
#include "itkImageFileWriter.h"
#include "itkNumericSeriesFileNames.h"

#include <itksys/SystemTools.hxx>

#include "mitkSurfaceVtkWriter.h"
#include <vtkSTLWriter.h>
#include <vtkPolyDataWriter.h>

#include "mitkLevelWindow.h"

#define EXTERNAL_FILE_EXTENSIONS "All known formats(*.dcm *.DCM *.pic *.pic.gz *.png *.jog *.tiff);;DICOM files(*.dcm *.DCM);;DKFZ Pic (*.seq *.pic *.pic.gz *.seq.gz);;Sets of 2D slices (*.pic *.pic.gz *.png *.dcm);;stl files (*.stl)"
#define INTERNAL_FILE_EXTENSIONS "all (*.seq *.pic *.pic.gz *.seq.gz *.pvtk *.stl *.vtk *.ves *.uvg *.dvg *.par *.dcm *.mhd hpsonos.db HPSONOS.DB *.png *.tiff *.jpg);;DKFZ Pic (*.seq *.pic *.pic.gz *.seq.gz);;surface files (*.stl *.vtk);;stl files (*.stl);;vtk surface files (*.vtk);;vtk image files (*.pvtk);;vessel files (*.ves *.uvg *.dvg);;par/rec files (*.par);;DSR files (hpsonos.db HPSONOS.DB);;DICOM files (*.dcm)"
#define SAVE_FILE_EXTENSIONS "all (*.pic *.mhd *.png *.tiff *.jpg)"


/**
 * This class provides some data handling methods, like loading data or adding different 
 * types to the data tree...
 */
namespace CommonFunctionality
{

  static const char* GetInternalFileExtensions() { return INTERNAL_FILE_EXTENSIONS; };
  static const char* GetExternalFileExtensions() { return EXTERNAL_FILE_EXTENSIONS; };
  static const char* GetSaveFileExtensions() { return SAVE_FILE_EXTENSIONS; };

  typedef std::vector<mitk::DataTreeNode*> DataTreeNodeVector;
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

  template < typename TImageType >
  static void AutoLevelWindow( mitk::DataTreeNode::Pointer node )
  {
    mitk::Image::Pointer image = dynamic_cast<mitk::Image*> (node->GetData() );
    if ( image.IsNotNull() )
    {
      typename TImageType::Pointer itkImage= TImageType::New();
      mitk::CastToItkImage( image, itkImage);
      float extrema[2];
      extrema[0] = 0;
      extrema[1] = 4096;
      MinMax<TImageType>(itkImage,extrema[0],extrema[1]);

      mitk::LevelWindowProperty::Pointer levWinProp = dynamic_cast<mitk::LevelWindowProperty*>(node->GetPropertyList()->GetProperty("levelwindow").GetPointer());
      if( levWinProp.IsNull() )
      {
        levWinProp = new mitk::LevelWindowProperty();
        node->GetPropertyList()->SetProperty("levelwindow", levWinProp);
      }
  
      double window = (extrema[1] - extrema[0])/10.0;
      double level  = window/2;

      mitk::LevelWindow levWin = levWinProp->GetLevelWindow();
      levWin.SetRangeMin(extrema[0]);
      levWin.SetRangeMax(extrema[1]);
      levWin.SetLevel( level );
      levWin.SetWindow( window );
      levWinProp->SetLevelWindow(levWin);

    }
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
      node->SetData(image);
      it->Add(node);
  }
    else
    {
      node = subTree->Get();
      node->SetData(image);
    }

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
      node->SetData(image);
      it->Add(node);
    }
    else
    {
      node = subTree->Get();
      node->SetData(image);
    }

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
    if (!it->IsAtEnd())
    {
      mitk::DataTreeNode::Pointer node = it->Get();
      if ( node.IsNotNull() )  {
        node->SetVisibility(enable, NULL);
      }
    }
    it = dataTree->GetNext("name", new mitk::StringProperty("widget2Plane"));
    if (!it->IsAtEnd())
    {
      mitk::DataTreeNode::Pointer node = it->Get();
      if ( node.IsNotNull() )  
      {
        node->SetVisibility(enable, NULL);
      }
    }
    it = dataTree->GetNext("name", new mitk::StringProperty("widget3Plane"));
    if (!it->IsAtEnd())
    {
      mitk::DataTreeNode::Pointer node = it->Get();
      if ( node.IsNotNull() )  
      {
        node->SetVisibility(enable, NULL);
      }
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


static mitk::DataTreeNode::Pointer FileOpenImageSequence(QString fileName)
{
  mitk::DataTreeNodeFactory::Pointer factory = mitk::DataTreeNodeFactory::New();

  if (!fileName.contains("dcm") && !fileName.contains("DCM"))
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


    factory->SetFilePattern( pattern );
    factory->SetFilePrefix( prefix );
  }
  else
  {
//    factory->SetFileName( fileName );
    factory->SetFilePattern( fileName );
    factory->SetFilePrefix( fileName );
  }
  factory->Update();
  return factory->GetOutput( 0 );

}

static mitk::DataTreeNode::Pointer FileOpenImageSequence()
{
  QString fileName = QFileDialog::getOpenFileName(NULL,GetExternalFileExtensions());

  if ( !fileName.isNull() )
  {
    return FileOpenImageSequence(fileName);
  }
  else
  {
    return NULL;
  }
}

static mitk::DataTreeNode::Pointer FileOpen()
{
#ifdef MBI_INTERNAL
  QString fileName = QFileDialog::getOpenFileName(NULL,GetInternalFileExtensions() );
#else
  QString fileName = QFileDialog::getOpenFileName(NULL,GetExternalFileExtensions() );
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

static mitk::DataTreeNode::Pointer OpenVolumeOrSliceStack()
{
  mitk::DataTreeNode::Pointer newNode = NULL;

  QString fileName = QFileDialog::getOpenFileName(NULL,GetExternalFileExtensions() );
  if ( !fileName.isNull() ) 
  {
    newNode = CommonFunctionality::FileOpen(fileName);
    if (newNode.IsNotNull())
    {
      mitk::Image::Pointer imageData = dynamic_cast<mitk::Image*> (newNode->GetData()) ;
      if (imageData->GetDimension(2) == 1)
      {
//        std::string dir = itksys::SystemTools::GetFilenamePath( std::string(fileName.ascii()) )
        newNode = CommonFunctionality::FileOpenImageSequence(fileName);
        imageData = dynamic_cast<mitk::Image*> (newNode->GetData());
      }
      return newNode;
    }
  }
  {
    return NULL;
  }
}

template < typename TImageType >
static QString SaveImage(mitk::Image* image)
{
  QString fileName = QFileDialog::getSaveFileName(QString("NewImage.pic"),GetSaveFileExtensions());
  if (fileName == NULL ) 
    return NULL;

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

        int numberOfSlices = itkImage->GetLargestPossibleRegion().GetSize()[2];
        writer->SetStartIndex(numberOfSlices);
        writer->SetIncrementIndex(-1);

        int pos = fileName.findRev(".",fileName.length()-1);
        fileName.insert(pos,".%d");
        writer->SetSeriesFormat( fileName.ascii() );
        writer->Update();
      }
      else 
      {
        typedef typename TImageType::PixelType PixelType;
        typedef itk::Image<PixelType,2> OutputImage2DType;
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
      picImage = ipPicClone(picImage);
      mitk::PicFileReader::ConvertHandedness(picImage);
      //set tag "REAL PIXEL SIZE"
      mitk::SlicedGeometry3D* slicedGeometry = image->GetSlicedGeometry();
      if (slicedGeometry != NULL)
      {
        const mitk::Vector3D & spacing = slicedGeometry->GetSpacing();
        ipPicTSV_t *pixelSizeTag;
        pixelSizeTag = ipPicQueryTag( picImage, "REAL PIXEL SIZE" );
        if (!pixelSizeTag)
        {
          pixelSizeTag = (ipPicTSV_t *) malloc( sizeof(ipPicTSV_t) );
          pixelSizeTag->type = ipPicFloat;
          pixelSizeTag->bpe = 32;
          strcpy(pixelSizeTag->tag, "REAL PIXEL SIZE");
          pixelSizeTag->dim = 1;
          pixelSizeTag->n[0] = 3;            
          pixelSizeTag->value = malloc( sizeof(float) * 3 );                
          ipPicAddTag (picImage, pixelSizeTag);            
        }
        ((float*)pixelSizeTag->value)[0] = spacing[0];
        ((float*)pixelSizeTag->value)[1] = spacing[1];
        ((float*)pixelSizeTag->value)[2] = spacing[2];       
      }
      ipPicPut((char*)(fileName.ascii()), picImage);
      ipPicFree(picImage);
    }
  }
  catch ( itk::ExceptionObject &err)
  {
    std::cout << "Exception object caught!" <<std::endl;
    std::cout << err << std::endl;
    return NULL;
  }
  return fileName;
}

    static void SaveSurface(mitk::Surface* surface, std::string name = 0)
    {
      std::string selectedItemsName = itksys::SystemTools::GetFilenameWithoutExtension(name);
      selectedItemsName += ".stl";
      QString fileName = QFileDialog::getSaveFileName(QString(selectedItemsName.c_str()),"Surface Data(*.stl *.vtk)");
        if (fileName != NULL ) 
        {
          if(fileName.endsWith(".stl")==true)
          {
            mitk::SurfaceVtkWriter<vtkSTLWriter>::Pointer writer=mitk::SurfaceVtkWriter<vtkSTLWriter>::New();
            writer->SetInput( surface );
            writer->SetFileName(fileName.latin1());
            writer->GetVtkWriter()->SetFileTypeToBinary();
            writer->Write();
          }
          else 
          {
            if (fileName.endsWith(".vtk")==false) 
                fileName += ".vtk";
            mitk::SurfaceVtkWriter<vtkPolyDataWriter>::Pointer writer=mitk::SurfaceVtkWriter<vtkPolyDataWriter>::New();
            writer->SetInput( surface );
            writer->SetFileName(fileName.latin1());
            writer->Write();
          }
        }
    }

	static mitk::DataTreeIteratorBase* GetIteratorToFirstImage(mitk::DataTreeIteratorBase* dataTreeIterator)
	{
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
            return it->Clone();
          }
        }
        ++it;
      }
      std::cout << "No node containing an mitk::Image found, returning NULL..." << std::endl;
      return NULL;
	}

    static mitk::DataTreeIteratorBase* GetIteratorToFirstImageInDataTree(mitk::DataTree::Pointer dataTree)
    {
      mitk::DataTreePreOrderIterator dataTreeIterator( dataTree );

      if ( dataTree.IsNull() )
      {
        std::cout << "iterator to data tree is NULL. I cannot work without datatree !!"  << std::endl;
        return NULL;
      }

	  return GetIteratorToFirstImage(&dataTreeIterator);
    }

    static mitk::Image* GetFirstImageInDataTree(mitk::DataTree::Pointer dataTree)
    {
      mitk::DataTreeIteratorClone it = GetIteratorToFirstImageInDataTree(dataTree);
      if(it.IsNull())
        return NULL;
      return static_cast<mitk::Image*>(it->Get()->GetData());
    }

    /**
     * Searches for the first node in the data tree, which holds a given type. 
     * @param it an iterator pointing to the position in the data tree, where
     *          the search should start
     * @returns the first node in the data tree which is of the type given by
     *          the template parameter T, or NULL otherwise.
     */
    template <typename T>
    static mitk::DataTreeNode* GetFirstNodeByType( mitk::DataTreeIteratorClone it )
    {
        if ( it.GetPointer() == NULL )
        {
            return NULL;
        }

        mitk::DataTreeIteratorClone iteratorClone = it;
        while ( !iteratorClone->IsAtEnd() )
        {
            mitk::DataTreeNode::Pointer node = iteratorClone->Get();
            if ( node->GetData() != NULL )
            {
                // access the original data
                T* data = dynamic_cast<T*>( node->GetData() );

                // enquiry whether img is NULL
                if ( data != NULL )
                {
                    return node.GetPointer();
                }
            }
            ++iteratorClone;
        }
        return NULL;
    }

    /**
     * Searches for the data object in the data tree, which matches a given type. 
     * @param it an iterator pointing to the position in the data tree, where
     *          the search should start
     * @returns the first data oobject in the data tree which is of the type given by
     *          the template parameter T, or NULL otherwise.
     */
    template <typename T>
    static T* GetFirstDataByType( mitk::DataTreeIteratorClone it )
    {
        mitk::DataTreeNode* node = GetFirstNodeByType<T>(it);
        if ( node == NULL )
        {
            return NULL;
        }
        else
        {
            return dynamic_cast<T*>( node->GetData() );
        }
    }

    /**
     * Searches for the first node in the data tree, which holds a given type 
     * and matches a given property key and value. This may be used to search
     * e.g. for a node holding an image with a given name in the data tree.
     * @param it an iterator pointing to the position in the data tree, where
     *          the search should start
     * @param propertyKey the name of the property we want to compare with
     * @param property the value of the property we want to search for in the data tree
     * @returns the first node in the data tree which is of the type given by
     *          the template parameter T, and matches propertyKey and property, or NULL otherwise.
     */
    template <typename T>
    static mitk::DataTreeNode* GetFirstNodeByTypeAndProperty( mitk::DataTreeIteratorClone it, std::string propertyKey, mitk::BaseProperty* property )
    {
        if ( it.GetPointer() == NULL )
        {
            return NULL;
        }

        mitk::DataTreeIteratorClone iteratorClone = it;
        while ( !iteratorClone->IsAtEnd() )
        {
            mitk::DataTreeNode::Pointer node = iteratorClone->Get();
            if ( node->GetData() != NULL )
            {
                // access the original data
                T* data = dynamic_cast<T*>( node->GetData() );

                // enquiry whether img is NULL
                if ( data != NULL )
                {
                    // check, if the data has the given property...
                    mitk::BaseProperty::Pointer tmp = node->GetPropertyList()->GetProperty( propertyKey.c_str() );
                    if ( (*property) == *(tmp) )
                        return node.GetPointer();
                }
            }
            ++iteratorClone;
        }
        return NULL;
    }

    /**
     * Searches for the first data object in the data tree, which is of a given type 
     * and whose node matches a given property key and value. This may be used to search
     * e.g. for an image with a given name in the data tree.
     * @param it an iterator pointing to the position in the data tree, where
     *          the search should start
     * @param propertyKey the name of the property we want to compare with
     * @param property the value of the property we want to search for in the data tree
     * @returns the first data object in the data tree which is of the type given by
     *          the template parameter T, and matches propertyKey and property, or NULL otherwise.
     */
    template <typename T>
    static T* GetFirstDataByTypeAndProperty( mitk::DataTreeIteratorClone it, std::string propertyKey, mitk::BaseProperty* property )
    {
        mitk::DataTreeNode* node = GetFirstNodeByTypeAndProperty<T>( it, propertyKey, property );
        if ( node == NULL )
        {
            return NULL;
        }
        else
        {
            return dynamic_cast<T*>( node->GetData() );
        }
    }

    /**
     * Searches for the first node in the data tree, which matches a given 
     * property key and value. This may be used to search e.g. for a node holding
     * a data object with a given name in the data tree.
     * @param it an iterator pointing to the position in the data tree, where
     *          the search should start
     * @param propertyKey the name of the property we want to compare with
     * @param property the value of the property we want to search for in the data tree
     * @returns the first node in the data tree which matches propertyKey and property, or NULL otherwise.
     */
    static mitk::DataTreeNode* GetFirstNodeByProperty( mitk::DataTreeIteratorClone it, std::string propertyKey, mitk::BaseProperty* property )
    {
        mitk::DataTreeIteratorClone pos = dynamic_cast<mitk::DataTree*>( it->GetTree() )->GetNext( propertyKey.c_str(), property, it.GetPointer() );
        if ( ! pos->IsAtEnd() )
        {
            return pos->Get();
        }
        else
        {
            return NULL;
        }
        
    }

    /**
     * Searches for the first data object in the data tree, whose node matches a given 
     * property key and value. This may be used to search e.g. for a node holding
     * a data object with a given name in the data tree.
     * @param it an iterator pointing to the position in the data tree, where
     *          the search should start
     * @param propertyKey the name of the property we want to compare with
     * @param property the value of the property we want to search for in the data tree
     * @returns the first data object in the data tree whose node matches propertyKey and property, or NULL otherwise.
     */
    static mitk::BaseData* GetFirstDataByProperty( mitk::DataTreeIteratorClone it, std::string propertyKey, mitk::BaseProperty* property )
    {
        mitk::DataTreeNode* node = GetFirstNodeByProperty( it, propertyKey, property );
        if ( node == NULL )
        {
            return NULL;
        }
        else
        {
            return node->GetData();
        }
    }

    /**
     * Searches for the node in the data tree which holds a given mitk::BaseData 
     * @param it an iterator pointing to the position in the data tree, where
     *          the search should start
     * @param data the data object, for which the node in the tree should be searched.
     * @returns the node holding data, or NULL otherwise.
     */
    static mitk::DataTreeNode* GetNodeForData( mitk::DataTreeIteratorClone it, mitk::BaseData* data )
    {
        if ( it.GetPointer() == NULL )
        {
            return NULL;
        }

        mitk::DataTreeIteratorClone iteratorClone = it;
        while ( !iteratorClone->IsAtEnd() )
        {
            mitk::DataTreeNode::Pointer node = iteratorClone->Get();
            if ( node.IsNotNull() )
            {
                if ( node->GetData() == data )
                    return node.GetPointer();
            }
            ++iteratorClone;
        }
        return NULL;
    }
    
    template <typename BaseDataType>
    static DataTreeNodeVector GetNodesForDataType(mitk::DataTreeIteratorClone it) {
      DataTreeNodeVector result;

        if ( it.GetPointer() != NULL )
        {
       
        mitk::DataTreeIteratorClone iteratorClone = it->Clone();
        while ( !iteratorClone->IsAtEnd() )
        {
            mitk::DataTreeNode::Pointer node = iteratorClone->Get();
            if ( dynamic_cast<BaseDataType*>( node->GetData() ) )
            {
              result.push_back(node);
	    }
	    ++iteratorClone;
        }
        }
	return result;
         
    }
    
    static DataTreeNodeVector FilterNodes(mitk::DataTreeIteratorClone it, bool (* FilterFunction)(mitk::DataTreeNode*)) {
      
      DataTreeNodeVector result;

        if ( it.GetPointer() != NULL )
        {
       
        mitk::DataTreeIteratorClone iteratorClone = it;
        while ( !iteratorClone->IsAtEnd() )
        {
            mitk::DataTreeNode::Pointer node = iteratorClone->Get();
            if ( FilterFunction( node ) )
            {
	      result.push_back(node);
	    }
	    ++iteratorClone;
        }
        }
	return result;
         
    }

};
#endif // _CommonFunctionality__h_
