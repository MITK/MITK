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


#ifndef LIGHTBOXIMAGEREADERIMPL_H
#define LIGHTBOXIMAGEREADERIMPL_H

#include "mitkImageSource.h"
#include "mitkCommon.h"
#include "mitkLightBoxImageReader.h"

class QcLightbox;

namespace mitk {

//##Documentation
//## @brief Read images from Chili LightBox
//## @ingroup Process
//## @ingroup Chili
class LightBoxImageReaderImpl : public LightBoxImageReader 
{
public:
    /** Standard class typedefs. */
    mitkClassMacro(LightBoxImageReaderImpl, LightBoxImageReader);

    /** Method for creation through the object factory. */
    itkNewMacro(Self);

    //##Description 
    //## @brief Set the lightbox to read from
    void SetLightBox(QcLightbox* lightbox);

    //##Description 
    //## @brief Set the lightbox to read from to the current lightbox
    //##
    //## The current lightbox at the time of the method
    //## call is set as lightbox to read from.
  //## \sa SetLightBox
    void SetLightBoxToCurrentLightBox();

    virtual const std::string GetSeriesDescription();

    //##Description 
    //## @brief Get the lightbox to read from
    QcLightbox* GetLightBox() const;

protected:
    //##Description 
    //## @brief Struct used to sort the image slices
    typedef struct _localImageInfo
    {
      int pos;
      int imageNumber;
      mitk::Vector3D origin;
      mitk::Vector3D* direction;
    } LocalImageInfo;
    //##Description 
    //## @brief Vector of structs used to sort the image slices
    typedef std::vector<LocalImageInfo> LocalImageInfoArray;

    //##Description 
    //## @brief Struct to store time information of the slices
    typedef struct _sliceInfo
    {
      int numberOfTimePoints;
      //float imagetime;
      mitk::Point3D startPosition;
    } sliceInfo;
    //##Description 
    //## @brief Vector of structs to store time information of the slices
    typedef std::list<sliceInfo> SliceInfoArray;

    //##Description 
    //## @brief Functors for sorting the image slices
    static bool ImageOriginLesser ( const LocalImageInfo& elem1, const LocalImageInfo& elem2 );
    static bool ImageNumberLesser ( const LocalImageInfo& elem1, const LocalImageInfo& elem2 );

    virtual void GenerateData();

    virtual void GenerateOutputInformation();

    LightBoxImageReaderImpl();

    ~LightBoxImageReaderImpl();

    //##Description 
    //## @brief Time when Header was last read
    itk::TimeStamp m_ReadHeaderTime;

    //##Description 
    //## @brief Convert the position of the sorted image slices into the lightbox position
    //##
    //## @param position The position in the virtually sorted lightbox
    //## @param list The list with the image number of the virtually sorted lightbox, created by 
    //## SortImage()
    int GetRealPosition(int position, LocalImageInfoArray& list);

    //##Description 
    //## @brief Virtually sort the lightbox
    void SortImage(LocalImageInfoArray& imageNumbers);

    mitk::Vector3D GetSpacingFromLB(LocalImageInfoArray& imageNumbers);

    QcLightbox* m_LightBox;

    LocalImageInfoArray m_ImageNumbers;

    std::string m_SeriesDescription;
};

} // namespace mitk


#endif /* LIGHTBOXIMAGEREADER_H_HEADER_INCLUDED_C1F48A22 */
