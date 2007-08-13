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

#include "mitkChiliPlugin.h"

namespace mitk {

class Image;
class LevelWindow;

  /**
  This class separate a mitk::Image into a list of ipPicDescriptor.

  WARNING:
  This class arranged as helper-class and dont check the input.
  Probably you dont should use this class. Use mitk::ChiliPlugin::SaveImageToLightBox(...) or mitk::ChiliPlugin::SaveToChili(...). If you do yet, use it carefully.
  */

class ImageToPicDescriptor
{
  public:

    /** constuctor and desctructor */
    ImageToPicDescriptor();
    ~ImageToPicDescriptor();

   /*!
   \brief set the mitk::Image, mitk::LevelWindow as Input
   @param sourceImage   the image which get seperated
   @param levelWindow   the levelwindow from the image;
   @param studyPatientAndSeriesTags   if this list are empty, no picTags get override, so we have the original and can override them
   if its not empty, all current picTags get delete and the new one used, this happens if you want to save a new series
   */
    void SetInput( Image* sourceImage, ChiliPlugin::TagInformationList studyPatientAndSeriesTags, LevelWindow levelWindow );

    //void SetInput( Image* sourceImage, const mitk::PropertyList::Pointer propertyList, LevelWindow levelWindow );

   /*!
   \brief this function separate a mitk::image into a list of ipPicDescriptor
   */
    void Write();

   /*!
   \brief return the generated Output
   @returns the list of ipPicDescriptors
   */
    std::list< ipPicDescriptor* > GetOutput();

  protected:

    /** the list for the output */
    std::list< ipPicDescriptor* > m_Output;
    /** the image to seperate */
    Image* m_SourceImage;
    /** the levelwindow of the image */
    LevelWindow m_levelWindow;
    /** this tags contained the study-, patient- and series-information from the new series */
    ChiliPlugin::TagInformationList m_StudyPatientAndSeriesTags;

    //mitk::PropertyList::Pointer m_PropertyList;

    /** create a String-Pic-Tag */
    ipPicTSV_t* CreateASCIITag( std::string Description = "", std::string Content = "" );
    /** create a Int-Pic-Tag */
    ipPicTSV_t* CreateIntTag( std::string Description = "", int Content = 0 );
    /** create a Unsinged-Int-Pic-Tag */
    ipPicTSV_t* CreateUIntTag( std::string Description = "", int Content = 0 );
    /** delete a Pic-Tag if exist */
    void DeleteTag( ipPicDescriptor* cur = NULL, std::string Description = "" );
};

} // namespace mitk

#endif
