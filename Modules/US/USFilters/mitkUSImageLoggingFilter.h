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

#ifndef MITKUSImageLoggingFiler_H_HEADER_INCLUDED_
#define MITKUSImageLoggingFiler_H_HEADER_INCLUDED_

// MITK
#include <MitkUSExports.h>
#include <mitkImageToImageFilter.h>
#include <mitkRealTimeClock.h>


namespace mitk {
  /**
  * \brief TODO
  *
  * \ingroup US
  */
  class MitkUS_EXPORT USImageLoggingFilter : public mitk::ImageToImageFilter
  {
  public:

    mitkClassMacro(USImageLoggingFilter, mitk::ImageToImageFilter);

    itkNewMacro(USImageLoggingFilter);

    virtual void GenerateData();

    void AddMessageToCurrentImage(std::string message);

    void SaveImages(std::string path, std::vector<std::string>& imageFilenames = std::vector<std::string>(), std::string& csvFileName = std::string());

  protected:
    USImageLoggingFilter();
    virtual ~USImageLoggingFilter();
    typedef std::vector<mitk::Image::Pointer> ImageCollection;
    mitk::RealTimeClock::Pointer m_SystemTimeClock;  ///< system time clock for system time tag

    //members for logging
    ImageCollection m_LoggedImages; ///< An image collection for every input. The string identifies the input.
    std::map<int, std::string> m_LoggedMessages; ///< (Optional) messages for every logged image
    std::vector<double> m_LoggedMITKSystemTimes; ///< Logged system times for every logged image

  };
} // namespace mitk
#endif /* MITKUSImageSource_H_HEADER_INCLUDED_ */
