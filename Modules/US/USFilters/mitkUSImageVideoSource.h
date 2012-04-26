/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
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


#ifndef MITKUSImageVideoSource_H_HEADER_INCLUDED_
#define MITKUSImageVideoSource_H_HEADER_INCLUDED_

#include <itkProcessObject.h>
#include "mitkUSImage.h"
#include "mitkOpenCVVideoSource.h"

namespace mitk {

  /**Documentation
  * \brief TODO
  *
  * \ingroup US
  */
  class MitkUS_EXPORT USImageVideoSource : public itk::ProcessObject
  {
  public:
    mitkClassMacro(USImageVideoSource, itk::ProcessObject);
    itkNewMacro(Self);

    /**
    *\brief Opens a video file for streaming. If nothing goes wrong, the 
    * VideoSource is ready to deliver images after calling this function.
    */
    void OpenVideoFile(std::string path);


    /**
    *\brief return the output (output with id 0) of the filter
    */
    USImage* GetOutput(void);

    /**
    *\brief return the output with id idx of the filter
    */
    USImage* GetOutput(unsigned int idx);

      
    /**
    *\brief Graft the specified DataObject onto this ProcessObject's output.
    *
    * See itk::ImageSource::GraftNthOutput for details
    */
    virtual void GraftNthOutput(unsigned int idx, itk::DataObject *graft);

    /**
    * \brief Graft the specified DataObject onto this ProcessObject's output.
    *
    * See itk::ImageSource::Graft Output for details
    */
    virtual void GraftOutput(itk::DataObject *graft);

    /**
    * \brief Make a DataObject of the correct type to used as the specified output.
    *
    * This method is automatically called when DataObject::DisconnectPipeline()
    * is called.  DataObject::DisconnectPipeline, disconnects a data object
    * from being an output of its current source.  When the data object
    * is disconnected, the ProcessObject needs to construct a replacement
    * output data object so that the ProcessObject is in a valid state.
    * Subclasses of USImageVideoSource that have outputs of different
    * data types must overwrite this method so that proper output objects
    * are created.
    */
    virtual DataObjectPointer MakeOutput(unsigned int idx);

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

    

  };
} // namespace mitk
#endif /* MITKUSImageVideoSource_H_HEADER_INCLUDED_ */