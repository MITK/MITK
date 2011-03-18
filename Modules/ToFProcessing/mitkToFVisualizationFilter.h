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
#ifndef __mitkToFVisualizationFilter_h
#define __mitkToFVisualizationFilter_h


#include <mitkImage.h>
#include "mitkImageToImageFilter.h"
#include <mitkToFProcessingExports.h>
//#include <mitkRealTimeClock.h>
#include <cv.h>
#include <vtkFloatArray.h>
#include <vtkImageData.h>
#include <vtkLookupTable.h>
#include <vtkColorTransferFunction.h>

namespace mitk
{

  class mitkToFProcessing_EXPORT ToFVisualizationFilter : public ImageToImageFilter
  {
  public:

    mitkClassMacro( ToFVisualizationFilter , ImageToImageFilter );
    itkNewMacro( Self );

     /*!
    \brief sets the input of this filter
    \param distanceImage input is the distance image of e.g. a ToF camera
    */
    virtual void SetInput( Image* distanceImage);

    /*!
    \brief sets the input of this filter at idx
    \param idx number of the current input
    \param distanceImage input is the distance image of e.g. a ToF camera
    \param CameraModel This is the camera model which holds parameters like focal length, pixel size, etc. which are needed for the reconstruction of the surface.
    */
    virtual void SetInput(unsigned int idx,  Image* distanceImage);

    /*!
    \brief returns the input of this filter
    */
    Image* GetInput();

    /*!
    \brief returns the input with id idx of this filter
    */
    Image* GetInput(unsigned int idx);

    vtkColorTransferFunction* GetWidget1ColorTransferFunction();

    void SetWidget1ColorTransferFunction(vtkColorTransferFunction* colorTransferFunction);

    vtkColorTransferFunction* GetWidget2ColorTransferFunction();

    void SetWidget2ColorTransferFunction(vtkColorTransferFunction* colorTransferFunction);

    vtkColorTransferFunction* GetWidget3ColorTransferFunction();

    void SetWidget3ColorTransferFunction(vtkColorTransferFunction* colorTransferFunction);

    //vtkColorTransferFunction* GetWidget4ColorTransferFunction();

    //void SetWidget4ColorTransferFunction(vtkColorTransferFunction* colorTransferFunction);

    vtkColorTransferFunction* GetColorTransferFunctionByImageType(std::string imageType);

    std::string GetWidget1ImageType();

    void SetWidget1ImageType(std::string imageType);

    std::string GetWidget2ImageType();

    void SetWidget2ImageType(std::string imageType);

    std::string GetWidget3ImageType();

    void SetWidget3ImageType(std::string imageType);

    int GetWidget1TransferFunctionType();

    void SetWidget1TransferFunctionType(int transferFunctionType);

    int GetWidget2TransferFunctionType();

    void SetWidget2TransferFunctionType(int transferFunctionType);

    int GetWidget3TransferFunctionType();

    void SetWidget3TransferFunctionType(int transferFunctionType);

  protected:
    /*!
    \brief standard constructor
    */
    ToFVisualizationFilter();
    /*!
    \brief standard destructor
    */
    ~ToFVisualizationFilter();
    virtual void GenerateOutputInformation();
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

    void SetImageType(std::string& str, int index);

    void CheckTransferFunction(int idx, mitk::Image* image);

    void InitializeTransferFunction(int idx, mitk::Image* image);

    void ResetTransferFunction(vtkColorTransferFunction* colorTransferFunction, int type, double min, double max);

    void InitImage(mitk::Image::Pointer image);

    float* GetDataFromImageByImageType(std::string imageType);

    void ConvertFloatImageToRGBImage(vtkColorTransferFunction* colorTransferFunction, float* floatData, unsigned char* image, bool flip=false);

    int m_ImageWidth;
    int m_ImageHeight;
    int m_ImageSize;

    float* m_DistanceFloatData;
    float* m_AmplitudeFloatData;
    float* m_IntensityFloatData;

    mitk::Image::Pointer m_MitkDistanceImage;
    mitk::Image::Pointer m_MitkAmplitudeImage;
    mitk::Image::Pointer m_MitkIntensityImage;

    //double m_TimeBefore, m_TimeAfter;
    //mitk::RealTimeClock::Pointer m_RealTimeClock;

    vtkColorTransferFunction* m_Widget1ColorTransferFunction;
    vtkColorTransferFunction* m_Widget2ColorTransferFunction;
    vtkColorTransferFunction* m_Widget3ColorTransferFunction;
    //vtkColorTransferFunction* m_Widget4ColorTransferFunction;

    std::string m_Widget1ImageType;
    std::string m_Widget2ImageType;
    std::string m_Widget3ImageType;
    //std::string m_Widget4ImageType;

    int m_Widget1TransferFunctionType;
    int m_Widget2TransferFunctionType;
    int m_Widget3TransferFunctionType;

  };
} //END mitk namespace
#endif
