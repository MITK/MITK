/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date: 2007-05-30 13:17:52 +0200 (Mi, 30 Mai 2007) $
Version:   $Revision: 10537 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef IMAGETOPICDESCRIPTOR_H_HEADER_INCLUDED
#define IMAGETOPICDESCRIPTOR_H_HEADER_INCLUDED

#include <ipPic/ipPic.h>
#include <iostream>
#include <list>

#include <mitkLevelWindow.h>
#include <mitkImage.h>
#include "mitkCommon.h"

namespace mitk {

  /**
  This class separate a mitk::Image into a list of ipPicDescriptor.

  WARNING:
  This class arranged as helper-class. Dont use this class, use mitk::ChiliPlugin.
  */

class ImageToPicDescriptor : public itk::Object
{
  public:

    /** Struct to transport the PicTags (with Description and Content ). */
    struct TagInformationStruct
    {
      std::string PicTagDescription;
      std::string PicTagContent;
    };

    /** We need more than one PicTag. */
    typedef std::list< TagInformationStruct > TagInformationList;

   mitkClassMacro( ImageToPicDescriptor, itk::Object );
   itkNewMacro( ImageToPicDescriptor );

    /*!
    \brief This function set a mitk::Image as input.
    @param sourceImage   The mitk::Image.
    This function have to be use, otherwise update dont work.
    */
    void SetImage( Image* sourceImage);

    /*!
    \brief This function set a TagInformaitionList and a bool variable.
    @param inputTags   This list provides information about the study, patient and series. This tags are the minimun which needed to save.
    @param useSavedPicTags   This bool attribute decided if the picDescriptor create new slices (false), or override existing slices (true). To override existing slices all pic-tags have to be the same. So the tags dont get changed. If you want to create new slices, the new slices needed the current Date, Time, ImageInstanceUID, ... . With false all this tags created and added to the ipPicDescriptors. But therefore the Patient-, Study- and Series-Information needed. This provide the inputTags. The mitkChiliPlugin have a function to create the needed one.
    This function have to be use, otherwise update dont work.
    */
    void SetTagList( TagInformationList inputTags, bool useSavedPicTags );

    /*!
    \brief This function set a mitk::Levelwindow.
    @param levelWindow   The mitk::Levelwindow.
    This function can be use. If the levelwindow dont get set, it will be created with SetAuto().
    */
    void SetLevelWindow( LevelWindow levelWindow );

    /*!
    \brief This function set the imageNumber.
    @param imageNumber   This is the image number for the tags. If the picdescriptors added to an existing series, the image number sould not be twice. Then the slice-stack-reader can splitt the results better. Therefor you can set the start-number.
    This function can be use. If the imageNumber dont set, the number set to one.
    IMPORTANT: Its recommend to set the ImageNumber!
    */
    void SetImageNumber( int imageNumber );

    /*!
    \brief This function separate a mitk::image into a list of ipPicDescriptor.
    If no input set before, the function create an empty output.
    */
    void Update();

    /*!
    \brief Return the generated Output.
    @returns The list of ipPicDescriptors.
    */
    std::list< ipPicDescriptor* > GetOutput();

    /*!
    \brief Return the saved ImageInstanceUIDs.
    @returns A list of strings.
    The imageInstanceUIDs get used to identify the single slices. The single slices addicted a volume. Volumes have parent-child-relations. Therefore they needed.
    */
    std::list< std::string > GetSaveImageInstanceUIDs();

  protected:

    /** Constuctor */
    ImageToPicDescriptor();
    /** Destructor */
    ~ImageToPicDescriptor();

    /** The list for the output. */
    std::list< ipPicDescriptor* > m_Output;
    /** We need the created imageInstanceUIDs to save the parent-child-realtionship. */
    std::list< std::string > m_imageInstanceUIDs;
    /** The image to seperate. */
    Image::Pointer m_SourceImage;
    /** The levelwindow of the image. */
    LevelWindow m_LevelWindow;
    bool m_LevelWindowInitialized;
    /** The taginformationlist. */
    TagInformationList m_TagList;
    bool m_UseSavedPicTags;
    bool m_TagListInitialized;
    /** The imageNumber. */
    int m_ImageNumber;
    bool m_ImageNumberInitialized;

    /** The Dicom-Header have to deleted, because the function QcPlugin::addDicomHeader() dont change the existing, it create a new one.
    If we dont want to loose to much information, we copy them from the Dicom- to Pic-Header. */
    void CopyDicomHeaderInformationToPicHeader( ipPicDescriptor* );
    /** Create a String-Pic-Tag. */
    ipPicTSV_t* CreateASCIITag( std::string Description = "", std::string Content = "" );
    /** Create a Int-Pic-Tag. */
    ipPicTSV_t* CreateIntTag( std::string Description = "", int Content = 0 );
    /** Create a Unsinged-Int-Pic-Tag. */
    ipPicTSV_t* CreateUIntTag( std::string Description = "", int Content = 0 );
    /** Function to delete a pic-tag. Only existing tags can be deleted. */
    void DeleteTag( ipPicDescriptor* cur, std::string description );
};

} // namespace mitk

#endif
