/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/


#ifndef _QmitkIOUtil__h_
#define _QmitkIOUtil__h_

#include "QmitkExtExports.h"
#include <mitkCommon.h>
#include <mitkIOUtil.h>

// std
#include <string>
#include <vector>

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

namespace mitk
{

class QmitkExt_EXPORT QmitkIOUtil
{
    /**
     *
     *
     *
    */
public:

  QmitkExt_EXPORT mitk::DataNode::Pointer FileOpen( const QString& fileName); //nur intern verwendet
  QmitkExt_EXPORT mitk::DataNode::Pointer FileOpen( const char *fileName ); //nur intern
  QmitkExt_EXPORT mitk::DataNode::Pointer FileOpenSpecific( const QString& fileExtensions); //nur intern
  QmitkExt_EXPORT mitk::DataNode::Pointer FileOpenSpecific( const char *fileExtensions ); //nur intern
  QmitkExt_EXPORT mitk::DataNode::Pointer FileOpenImageSequence(const QString& fileName); //nur intern
  QmitkExt_EXPORT mitk::DataNode::Pointer FileOpenImageSequence(const char* fileName); //nur intern
  QmitkExt_EXPORT mitk::DataNode::Pointer FileOpenImageSequence(); //nur intern
  QmitkExt_EXPORT mitk::DataNode::Pointer FileOpen(); //nur intern
  QmitkExt_EXPORT mitk::DataNode::Pointer OpenVolumeOrSliceStack(); //nur intern

  QmitkExt_EXPORT std::string SaveImageWithQDialog(mitk::Image* image, const char* aFileName = NULL, bool askForDifferentFilename = false); //nur intern

  QmitkExt_EXPORT std::string SaveSurface(mitk::Surface* surface, const char* fileName = NULL); //nur intern
  QmitkExt_EXPORT void SaveToFileWriter( mitk::FileWriterWithInformation::Pointer fileWriter, mitk::BaseData::Pointer data, const char* aFileName, const char* propFileName);//nur intern

  // internal vessel graph save code removed
  QmitkExt_EXPORT void SaveBaseData( mitk::BaseData* data, const char* name = NULL ); //nur in datamanagerview
  /** @brief Opens a dialog to define a save filename which starts in the same directory like last time. */
  QString GetSaveFileNameStartingInLastDirectory(QString caption, QString defaultFilename, QString filter, QString* selectedFilter = 0); //nur intern

  //##### neue methoden
  SaveSurface
  SaveImage
  SavePointSet
  SaveDataNode
  LoadPointSet
  LoadImage
  LoadSurface
  LoadDataNode
  LoadBaseData
};
} //end namespace mitk
#endif // _QmitkIOUtil__h_
