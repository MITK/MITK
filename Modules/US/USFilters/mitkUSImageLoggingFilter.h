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
  /** An object of this class is a filter which saves/logs a clone of the current image whenever
   *  Update() is called. Additionally a timestamp of this image is saved. Optionally you can
   *  add messages. All data (images, timestamps and messages) is written to the harddisc when
   *  the method SaveImages(...) is called.
   *
   *  Caution: only supports logging of one input at the moment, multiple inputs are ignored!
   *
   *  \ingroup US
   */
  class MITKUS_EXPORT USImageLoggingFilter : public mitk::ImageToImageFilter
  {
  public:

    mitkClassMacro(USImageLoggingFilter, mitk::ImageToImageFilter);

    itkNewMacro(USImageLoggingFilter);

    /** This method is internally called by the Update() mechanism of the pipeline. Don't call it directly. */
    virtual void GenerateData() override;

    /** Adds a message to the current (last logged) image. This message is internally stored and written to the
     *  harddisc when SaveImages(...) is called.
     * @param message The string which contains the message which is logged to the current image
     */
    void AddMessageToCurrentImage(std::string message);

    /** Saves all logged data to the given path. Every image is written to a separate image file.
     *  Additionaly a csv file containing a list of all images together with timestamps and messages is saved.
     *  For one call of this method all files will start with a unique number to avoid overwrite of old files.
     *  @param[in]     path            Should contain a valid path were all logging data will be stored.
     *  @param[out]    imageFilenames  Returns a list of all images filenames which were stored to the harddisc.
     *  @param[out]    csvFileName     Returns the filename of the csv list with the timestamps and the messages.
     *  @throw         mitk::Exception Throws an exception if there is a problem during writing the images. E.g.,
     *                                 if the path is not valid / not writable.
     */
    void SaveImages(std::string path, std::vector<std::string>& imageFilenames, std::string& csvFileName);

     /** Saves all logged data to the given path. Every image is written to a separate image file.
     *  Additionaly a csv file containing a list of all images together with timestamps and messages is saved.
     *  For one call of this method all files will start with a unique number to avoid overwrite of old files.
     *  @param[in]     path            Should contain a valid path were all logging data will be stored.
     *  @throw         mitk::Exception Throws an exception if there is a problem during writing the images. E.g.,
     *                                 if the path is not valid / not writable.
     */
    void SaveImages(std::string path);

    /** Sets the extension of the output images which alse defines the file type. E.g., ".nrrd" or ".jpg".
     *  ".nrrd" is default.
     *  @return Returns true if the file extension was successfully set which means it is supported. False if not.
     */
    bool SetImageFilesExtension(std::string extension);


  protected:
    USImageLoggingFilter();
    virtual ~USImageLoggingFilter();
    typedef std::vector<mitk::Image::Pointer> ImageCollection;
    mitk::RealTimeClock::Pointer m_SystemTimeClock;  ///< system time clock for system time tag

    //members for logging
    ImageCollection m_LoggedImages; ///< An image collection for every input. The string identifies the input.
    std::map<int, std::string> m_LoggedMessages; ///< (Optional) messages for every logged image
    std::vector<double> m_LoggedMITKSystemTimes; ///< Logged system times for every logged image
    std::string m_ImageExtension; ///< stores the image extension, default is ".nrrd"

  };
} // namespace mitk
#endif /* MITKUSImageSource_H_HEADER_INCLUDED_ */
