/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2008-07-30 13:49:05 +0200 (Mi, 30 Jul 2008) $
Version:   $Revision: 14840 $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/


#ifndef _CommonFunctionality__h_
#define _CommonFunctionality__h_

#include "mitkConfig.h"
#include "QmitkExtExports.h"
#include <mitkCommon.h>

// std
#include <string>
#include <vector>

#include <vtkRenderWindow.h>

// itk includes
#include <itkMinimumMaximumImageCalculator.h>

// mitk includes
#include <mitkProperties.h>
#include <mitkLevelWindowProperty.h>
#include <mitkStringProperty.h>
#include <mitkDataNode.h>
#include <mitkDataNodeFactory.h>
#include <mitkImageCast.h>
#include <mitkSurface.h>
#include "itkImage.h"
#include <mitkFileWriterWithInformation.h>

#include <qstring.h>
#include <qfiledialog.h>

#include <itksys/SystemTools.hxx>

#include "mitkLevelWindow.h"
#include "vtkPolyData.h"

struct mitkIpPicDescriptor;

namespace mitk
{
  class Surface;
  class Image;
  class VesselTreeData;
}

/**
 * This class provides some data handling methods, like loading data or adding different 
 * types to the data tree...
 */
namespace CommonFunctionality
{

  typedef std::vector<mitk::DataNode*> DataNodeVector;
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
  static void AutoLevelWindow( mitk::DataNode::Pointer node )
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

      mitk::LevelWindowProperty::Pointer levWinProp = dynamic_cast<mitk::LevelWindowProperty*>(node->GetPropertyList()->GetProperty("levelwindow"));
      if( levWinProp.IsNull() )
      {
        levWinProp = mitk::LevelWindowProperty::New();
        node->GetPropertyList()->SetProperty("levelwindow", levWinProp);
      }

      double window = (extrema[1] - extrema[0])/10.0;
      double level  = window/2;

      mitk::LevelWindow levWin = levWinProp->GetLevelWindow();
      levWin.SetRangeMinMax(extrema[0], extrema[1]);
      levWin.SetLevelWindow( level, window );
      levWinProp->SetLevelWindow(levWin);

    }
  }

  QmitkExt_EXPORT mitk::DataNode::Pointer FileOpen( const QString& fileName);
  QmitkExt_EXPORT mitk::DataNode::Pointer FileOpen( const char *fileName );
  QmitkExt_EXPORT mitk::DataNode::Pointer FileOpenSpecific( const QString& fileExtensions);
  QmitkExt_EXPORT mitk::DataNode::Pointer FileOpenSpecific( const char *fileExtensions );
  QmitkExt_EXPORT mitk::DataNode::Pointer FileOpenImageSequence(const QString& fileName);
  QmitkExt_EXPORT mitk::DataNode::Pointer FileOpenImageSequence(const char* fileName);
  QmitkExt_EXPORT mitk::DataNode::Pointer FileOpenImageSequence();
  QmitkExt_EXPORT mitk::DataNode::Pointer FileOpen();
  QmitkExt_EXPORT mitk::DataNode::Pointer OpenVolumeOrSliceStack();

  QmitkExt_EXPORT std::string SaveImage(mitk::Image* image, const char* aFileName = NULL, bool askForDifferentFilename = false);

  QmitkExt_EXPORT std::string SaveSurface(mitk::Surface* surface, const char* fileName = NULL);
  
  /**
   * Saves a screenshot of the given renderer in a file.
   * @param renderWindow the render window to generate a screenshot from. If renderWindow
   *                     is NULL, the function returns an empty string.
   * @param filename     the filename, under which the file should be saved. If 
   *                     filename is NULL, a file selector pops up and asks for a
   *                     file name. The file type is determined via the extension
   *                     of the screen shot
   * @returns the filename of the saved file
   */
  QmitkExt_EXPORT std::string SaveScreenshot( vtkRenderWindow* renderWindow , const char* filename = NULL );

  QmitkExt_EXPORT void SaveToFileWriter( mitk::FileWriterWithInformation::Pointer fileWriter, mitk::BaseData::Pointer data, const char* aFileName, const char* propFileName);

  // internal vessel graph save code removed ...
  
  QmitkExt_EXPORT void SaveBaseData( mitk::BaseData* data, const char* name = NULL );

  /**
  * Checks if the filename is valid for writing. This includes checking whether the file already exists. 
  * If so, the user is asked whether he wants to overwrite the existing file. In addition, the writing permission is checked.
  * @param filename the filename, under which the file should be saved. 
  * @returns true if the filename is valid for writing  
  */
  QmitkExt_EXPORT bool IsFilenameValidForWriting(const char* aFileName);
};
#endif // _CommonFunctionality__h_
