/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2008-04-01 08:56:22 +0200 (Di, 01 Apr 2008) $
Version:   $Revision: 13931 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef MITKLOADFROMCHILI_H_HEADER_INCLUDED
#define MITKLOADFROMCHILI_H_HEADER_INCLUDED

//CHILI
#include <chili/isg.h>  //geometry
#include <ipPic/ipPic.h>  //ipPicDescriptor
#include <chili/cdbTypes.h>  //series_t*, study_t*, ...
//MITK
#include "mitkDataTree.h"
#include "mitkParentChild.h"
#include "mitkPACSPlugin.h"

#ifndef WIN32  //TODO this function integrated to plugin.h at CHILI-Version 3.14 (current 3.12); at this time it ist possible to use this function for suse and windows -> change code
extern "C" ipPicDescriptor *ipPicDecompressJPEG( ipPicDescriptor *pic, ipUInt4_t frame, ipUInt4_t total_frames, ipPicDescriptor *result, ipUInt4_t *offset_table = NULL );  // <= CHILI 3.12
#endif

class QcLightbox;
class QcPlugin;

namespace mitk {

class LoadFromCHILI: public itk::Object
{
  public:

    void SetReaderType( unsigned int readerType );

    std::vector<DataTreeNode::Pointer> LoadImagesFromLightbox( QcPlugin* instance, QcLightbox* lightbox, const std::string& tmpDirectory );

    std::vector<DataTreeNode::Pointer> LoadFromSeries( QcPlugin* instance, const std::string& seriesOID, const std::string& tmpDirectory );

    std::vector<DataTreeNode::Pointer> LoadImagesFromSeries( QcPlugin* instance, const std::string& seriesOID, const std::string& tmpDirectory );

    std::vector<DataTreeNode::Pointer> LoadTextsFromSeries( QcPlugin* instance, const std::string& seriesOID, const std::string& tmpDirectory );

    DataTreeNode::Pointer LoadSingleText( QcPlugin* instance, const std::string& textOID, const std::string& tmpDirectory );

    DataTreeNode::Pointer LoadSingleText( QcPlugin* instance, const std::string& seriesOID, const std::string& textOID, const std::string& textPath, const std::string& tmpDirectory );

    DataTreeNode::Pointer LoadParentChildElement( QcPlugin* instance, const std::string& seriesOID, const std::string& label, const std::string& tmpDirectory );

    PACSPlugin::ParentChildRelationInformationList GetSeriesRelationInformation( QcPlugin* instance, const std::string& seriesOID, const std::string& tmpDirectory );

    PACSPlugin::ParentChildRelationInformationList GetStudyRelationInformation( QcPlugin* instance, const std::string& studyOID, const std::string& tmpDirectory );

    mitkClassMacro( LoadFromCHILI, itk::Object );
    itkNewMacro( LoadFromCHILI );
    virtual ~LoadFromCHILI();

  private:

    struct TextFilePathAndOIDStruct
    {
      std::string TextFilePath;
      std::string TextFileOID;
    };
    std::list<TextFilePathAndOIDStruct> m_TextList;

    struct StreamImageStruct
    {
      std::list<ipPicDescriptor*> imageList;
      interSliceGeometry_t* geometry;
      std::string seriesDescription;
    };

    std::list<ipPicDescriptor*> m_ImageList;
    std::list<StreamImageStruct> m_StreamImageList;
    std::list<std::string> m_UnknownImageFormatPath;

    std::list<std::string> m_VolumeImageInstanceUIDs;

    std::string m_tmpDirectory;
    unsigned int m_UsedReader;
    ParentChild::Pointer m_ParentChild;

    StreamImageStruct LoadStreamImage( ipPicDescriptor* pic);

    std::vector<DataTreeNode::Pointer> CreateNodesFromLists( QcPlugin* instance, const std::string& seriesOID, bool deletePicDescriptor, const std::string& tmpDirectory );

    static ipBool_t GlobalIterateLoadImages( int rows, int row, image_t* image, void* user_data );
    static ipBool_t GlobalIterateLoadSinglePics( int rows, int row, image_t* image, void* user_data );
    static ipBool_t GlobalIterateLoadTexts( int rows, int row, text_t *text, void *user_data );

  protected:

    LoadFromCHILI();
};

} // namespace mitk

#endif
