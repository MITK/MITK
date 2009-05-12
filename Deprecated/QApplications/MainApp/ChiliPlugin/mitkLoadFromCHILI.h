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

//MITK
#include "mitkPACSPlugin.h"

#include "mitkIpPicUnmangle.h"
//CHILI
#include <ipPic/ipPic.h>
#include <chili/isg.h>  //geometry
#include <chili/cdbTypes.h>  //series_t, study_t, ...

#ifndef _WIN32
extern "C" mitkIpPicDescriptor *ipPicDecompressJPEG( mitkIpPicDescriptor *pic, ipUInt4_t frame, ipUInt4_t total_frames, mitkIpPicDescriptor *result, ipUInt4_t *offset_table = NULL );
#endif

class QcLightbox;
class QcPlugin;

namespace mitk {

class LoadFromCHILI: public itk::Object
{
  public:

    /*
     0 for ImageNumberFilter
     1 for SpacingSetFilter
     2 for SingleSpacingFilter
     */
    void SetReaderType( unsigned int readerType );

    /*!
    \brief Should acquisition number be used to group images?

    If this flag is on, images are only grouped if they have an equal acquisition number.
    This is useful to separate images from the same series/lightbox that would
    otherwise be sorted into the same image.

    Default if false, because time series are assumed.

    \warning Only useful with ImageNumberFilter.
    */
    void SetSeparateByAcquisitionNumber(bool on);

    std::vector<DataTreeNode::Pointer> LoadImagesFromLightbox( QcPlugin* instance, 
                                                               QcLightbox* lightbox, 
                                                               const std::string& tmpDirectory );

    std::vector<DataTreeNode::Pointer> LoadImagesFromSeries( QcPlugin* instance, 
                                                             const std::string& seriesInstanceUID, 
                                                             const std::string& tmpDirectory );

    std::vector<DataTreeNode::Pointer> LoadImagesFromSeries( QcPlugin* instance, 
                                                             std::vector<std::string> seriesInstanceUIDs,
                                                             const std::string& tmpDirectory );

    std::vector<DataTreeNode::Pointer> LoadTextsFromSeries( QcPlugin* instance, 
                                                            const std::string& seriesInstanceUID, 
                                                            const std::string& tmpDirectory );

    DataTreeNode::Pointer LoadSingleText( QcPlugin* instance, 
                                          const std::string& seriesInstanceUID, 
                                          unsigned int instanceNumber, 
                                          const std::string& tmpDirectory );

    void DownloadSingleFile( QcPlugin* plugin,
                             const std::string& seriesInstanceUID, 
                             unsigned int instanceNumber, 
                             const std::string& filename );


    mitkClassMacro( LoadFromCHILI, itk::Object );
    itkNewMacro( LoadFromCHILI );

  private:

    struct TextFilePathAndOIDStruct
    {
      unsigned int instanceNumber;
      std::string TextFilePath;
      std::string TextFileOID;
    };

    std::list<TextFilePathAndOIDStruct> m_TextList;
    unsigned int m_TextListCounter;
    std::string m_SeriesInstanceUID;

    struct StreamImageStruct
    {
      std::list<mitkIpPicDescriptor*> imageList;
      interSliceGeometry_t* geometry;
      std::string seriesDescription;
    };

    std::list<mitkIpPicDescriptor*> m_ImageList;
    std::list<StreamImageStruct> m_StreamImageList;
    std::list<std::string> m_UnknownImageFormatPath;

    std::string m_tmpDirectory;
    unsigned int m_UsedReader;

    DataTreeNode::Pointer LoadSingleText( QcPlugin* instance, 
                                          const std::string& seriesInstanceUID, 
                                          unsigned int instanceNumber, 
                                          const std::string& textPath, 
                                          const std::string& tmpDirectory );

    StreamImageStruct LoadStreamImage( mitkIpPicDescriptor* pic);

    std::vector<DataTreeNode::Pointer> CreateNodesFromLists( QcPlugin* instance, const std::string& seriesInstanceUID, const std::string& tmpDirectory );

    static ipBool_t GlobalIterateLoadImages( int rows, int row, image_t* image, void* user_data );
    static ipBool_t GlobalIterateLoadTexts( int rows, int row, text_t *text, void *user_data );

  protected:

    LoadFromCHILI();
    virtual ~LoadFromCHILI();
    
    bool m_GroupByAcquisitionNumber;
};

} // namespace mitk

#endif

