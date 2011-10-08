/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date $
Version:   $Revision $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __mitkToFImageRecorderFilter_h
#define __mitkToFImageRecorderFilter_h

#include <mitkImageToImageFilter.h>
#include <mitkToFHardwareExports.h>

#include <mitkToFImageWriter.h>

namespace mitk
{
  /**
  * @brief Filter that allows recording of processed ToF images
  * Internally the ToFImageWriter is used for writing.
  *
  * @ingroup ToFProcessing
  */
  class MITK_TOFHARDWARE_EXPORT ToFImageRecorderFilter : public ImageToImageFilter
  {
  public:
    mitkClassMacro( ToFImageRecorderFilter , ImageToImageFilter );
    itkNewMacro( Self );

    /*!
    \brief Set file name for writing image files
    This filename will be appended by "_DistanceImage", "_AmplitudeImage", or "_IntensityImage" for the single images
    \param fileName base file name to save image filtes
    */
    void SetFileName(std::string fileName);
    /*!
    \brief Set image type for recording
    \param tofImageType either 3D Volume (ToFImageType3D) or temporal image stack (ToFImageType2DPlusT)
    */
    void SetImageType(ToFImageWriter::ToFImageType tofImageType);
    /*!
    \brief Returns a pointer to the ToFImageWriter internally used
    \return ToFImageWriter
    */
    ToFImageWriter::Pointer GetToFImageWriter();
    /*!
    \brief Sets a pointer to the ToFImageWriter internally used
    \param tofImageWriter ToFImageWriter
    */
    void SetToFImageWriter(ToFImageWriter::Pointer tofImageWriter);
    /*!
    \brief start recording of data
    */
    void StartRecording();
    /*!
    \brief stop recording of data
    */
    void StopRecording();


    /*!
    \brief sets the input of this filter
    \param distanceImage input is the distance image of e.g. a ToF camera
    */
    virtual void SetInput( Image* input);
    /*!
    \brief sets the input of this filter at idx
    \param idx number of the current input
    \param distanceImage input is the distance image of e.g. a ToF camera
    */
    virtual void SetInput(unsigned int idx,  Image* input);
    /*!
    \brief returns the input of this filter
    */
     Image* GetInput();
    /*!
    \brief returns the input with id idx of this filter
    */
     Image* GetInput(unsigned int idx);

  protected:
    /*!
    \brief standard constructor
    */
    ToFImageRecorderFilter();
    /*!
    \brief standard destructor
    */
    ~ToFImageRecorderFilter();
    /*!
    \brief method generating the output of this filter. Called in the updated process of the pipeline.
    This method generates the output of the ToFSurfaceSource: The generated surface of the 3d points
    */
    virtual void GenerateData();
    /**
    * \brief Create an output for each input
    *
    * This Method sets the number of outputs to the number of inputs
    * and creates missing outputs objects.
    * \warning any additional outputs that exist before the method is called are deleted
    */
    void CreateOutputsForAllInputs();

    ToFImageWriter::Pointer m_ToFImageWriter; ///< image writer used for streaming input data to file
    bool m_RecordingStarted; ///< flag indicating if filter is currently recording
    std::string m_FileExtension;
  };
} //END mitk namespace
#endif
