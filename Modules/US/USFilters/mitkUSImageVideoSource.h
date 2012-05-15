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


#ifndef MITKUSImageVideoSource_H_HEADER_INCLUDED_
#define MITKUSImageVideoSource_H_HEADER_INCLUDED_

#include <itkProcessObject.h>
#include "mitkUSImage.h"
#include "mitkOpenCVVideoSource.h"
#include "mitkOpenCVToMitkImageFilter.h"

namespace mitk {

  /**Documentation
  * \brief This class can be pointed to a video file or a videodevice and delivers USImages with default metadata Sets
  *
  * \ingroup US
  */
  class MitkUS_EXPORT USImageVideoSource : public itk::Object
  {
  public:
    mitkClassMacro(USImageVideoSource, itk::ProcessObject);
    itkNewMacro(Self);

    /**
    *\brief Opens a video file for streaming. If nothing goes wrong, the 
    * VideoSource is ready to deliver images after calling this function.
    */
    void SetVideoFileInput(std::string path);

    /**
    *\brief Opens a video device for streaming. Takes the Device id. Try -1 for "grab the first you can get"
    * which works quite well if only one device is available. If nothing goes wrong, the 
    * VideoSource is ready to deliver images after calling this function.
    */
    void SetCameraInput(int deviceID);

    /**
    *\brief Retrieves the next frame. This will typically be the next frame in a file
    * or the last cahced file in a devcie.
    */
    mitk::USImage::Pointer GetNextImage();
    
    // Getter & Setter
    itkGetMacro(OpenCVVideoSource, mitk::OpenCVVideoSource::Pointer);
    itkSetMacro(OpenCVVideoSource, mitk::OpenCVVideoSource::Pointer);
    itkGetMacro(IsVideoReady, bool);
    itkGetMacro(IsMetadataReady, bool);
    itkGetMacro(IsGeometryReady, bool);


  protected:
    USImageVideoSource();
    virtual ~USImageVideoSource();
    

    /**
    * \brief The source of the video
    */
    mitk::OpenCVVideoSource::Pointer m_OpenCVVideoSource;

    /**
    * \brief The Following flags are used internally, to assure that all necessary steps are taken before capturing
    */
   bool m_IsVideoReady;
   bool m_IsMetadataReady;
   bool m_IsGeometryReady;
   mitk::OpenCVToMitkImageFilter::Pointer m_OpenCVToMitkFilter;
    

  };
} // namespace mitk
#endif /* MITKUSImageVideoSource_H_HEADER_INCLUDED_ */