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


#ifndef LIGHTBOXIMAGEREADER_H_HEADER_INCLUDED_C1F48A22
#define LIGHTBOXIMAGEREADER_H_HEADER_INCLUDED_C1F48A22

#include "mitkImageSource.h"
#include "mitkCommon.h"
#include <chili/qclightbox.h>


namespace mitk {

class LightBoxImageReader : public ImageSource 
{
public:
    /** Standard class typedefs. */
    mitkClassMacro(LightBoxImageReader, ImageSource);

    /** Method for creation through the object factory. */
    itkNewMacro(Self);

    void SetLightBox(QcLightbox* lightbox);
    QcLightbox* GetLightBox() const;

protected:
    typedef struct _localImageInfo
    {
      int pos;
      int imageNumber;
      mitk::Vector3D origin;
      mitk::Vector3D* direction;
    } LocalImageInfo;
    typedef std::vector<LocalImageInfo> LocalImageInfoArray;

    static bool ImageOriginLesser ( const LocalImageInfo& elem1, const LocalImageInfo& elem2 );
    static bool ImageNumberLesser ( const LocalImageInfo& elem1, const LocalImageInfo& elem2 );

    virtual void GenerateData();

    virtual void GenerateOutputInformation();

    LightBoxImageReader();

    ~LightBoxImageReader();

    /** Time when Header was last read. */
    itk::TimeStamp m_ReadHeaderTime;
    //double ConvertTime(ipPicDescriptor*  pic);
    int GetRealPosition(int position, LocalImageInfoArray& liste);
    void SortImage(LocalImageInfoArray& imageNumbers);
    mitk::Vector3D GetSpacingFromLB(LocalImageInfoArray& imageNumbers);
    QcLightbox* m_LightBox;

    LocalImageInfoArray m_ImageNumbers;

};

} // namespace mitk
typedef struct imageInfo
{
  int timePoints;
  int nbOfTimePoints;
  //float imagetime;
  mitk::Point3D startPosition;
}imageInfo;


#endif /* LIGHTBOXIMAGEREADER_H_HEADER_INCLUDED_C1F48A22 */
