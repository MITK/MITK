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
#define NO_pVersion
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

    mitk::Vector3D GetSpacingFromLB();

    void SetLightBox(QcLightbox* lightbox);
    QcLightbox* GetLightBox();

protected:
    virtual void GenerateData();

    virtual void GenerateOutputInformation();

    LightBoxImageReader();

    ~LightBoxImageReader();

    /** Time when Header was last read. */
    itk::TimeStamp m_ReadHeaderTime;
    //double ConvertTime(ipPicDescriptor*  pic);
    int GetRealPosition(int position, std::list<int> liste);
    std::list<int> SortImage();
    QcLightbox* m_LightBox;
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
