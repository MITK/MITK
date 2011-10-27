/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2010-03-31 16:40:27 +0200 (Mi, 31 Mrz 2010) $
Version:   $Revision: 21975 $ 
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef QmitkToFUtilView_h
#define QmitkToFUtilView_h

#include <QmitkFunctionality.h>
#include "QmitkToFImageBackground.h"
#include <QmitkVideoBackground.h>

#include <ui_QmitkToFUtilViewControls.h>

#include <QTimer>

#include <mitkRealTimeClock.h>
#include <mitkToFImageGrabber.h>
#include <mitkOpenCVVideoSource.h>
#include <mitkSurface.h>
#include <mitkToFDistanceImageToSurfaceFilter.h>
#include <mitkToFSurfaceVtkMapper3D.h>
#include <mitkToFImageRecorder.h>
#include <mitkToFCompositeFilter.h>

#include <vtkLookupTable.h>

typedef itk::Image<float, 2> ItkImageType2D;

/*!
  \brief QmitkToFUtilView 

  \warning  This application is still under active development. Complete documentation will be added when class is finished.

  \sa QmitkFunctionality
  \ingroup Functionalities
*/
class QmitkToFUtilView : public QmitkFunctionality
{  
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT
  
  public:  

    static const std::string VIEW_ID;

    QmitkToFUtilView();
    QmitkToFUtilView(const QmitkToFUtilView& other)
	{
	  Q_UNUSED(other)
	  throw std::runtime_error("Copy constructor not implemented");
	}
    ~QmitkToFUtilView();

    virtual void CreateQtPartControl(QWidget *parent);
    /// \brief Called when the functionality is activated
    virtual void Activated();
    virtual void Deactivated();
    virtual void StdMultiWidgetAvailable (QmitkStdMultiWidget &stdMultiWidget);
    virtual void StdMultiWidgetNotAvailable();

  protected slots:
  
    /// \brief Called when the user clicks the GUI button
    void OnUpdateCamera();

	/*!
    \This method is automatically called by the connection widgets when the configuration is completed 
    */
    void OnToFCameraConnected();

    void OnToFCameraDisconnected();

    void OnToFCameraStarted();

    void OnToFCameraStopped();

    void OnToFCameraStop();

    void OnToFCameraSelected(const QString selected);

    void OnTextureCheckBoxChecked(bool checked);

    void OnVideoTextureCheckBoxChecked(bool checked);

  protected:

    void InitImage(mitk::Image::Pointer image, int numOfChannel);

    void InitTexture(unsigned char* &image, int width, int height);

    void PrepareImageForBackground(vtkLookupTable* lut, float* floadData, unsigned char* image);

    void PrepareImageForBackground(vtkColorTransferFunction* colorTransferFunction, float* floatData, unsigned char* image);

    void RemoveBackground();

    void AddBackground();

    Ui::QmitkToFUtilViewControls* m_Controls;

    QmitkStdMultiWidget* m_MultiWidget;

    QTimer* m_Frametimer;

    mitk::ToFImageGrabber* m_ToFImageGrabber;
    
    mitk::Image::Pointer m_MitkDistanceImage; ///< object to hold the distance image

    mitk::Image::Pointer m_MitkAmplitudeImage; ///< object to hold the amplitude image

    mitk::Image::Pointer m_MitkIntensityImage; ///< object to hold the amplitude image

    mitk::Surface::Pointer m_Surface;

    mitk::DataNode::Pointer m_DistanceImageNode; ///< DataNode to get the distance image into the DataStorage

    mitk::DataNode::Pointer m_AmplitudeImageNode; ///< DataNode to get the amplitude image into the DataStorage

    mitk::DataNode::Pointer m_IntensityImageNode; ///< DataNode to get the intensity image into the DataStorage

    mitk::DataNode::Pointer m_SurfaceNode;

    mitk::ToFImageRecorder::Pointer m_ToFImageRecorder;

    int m_ImageSequence;

    mitk::RealTimeClock::Pointer m_RealTimeClock;

    int m_StepsForFramerate; 

    double m_2DTimeBefore, m_2DTimeAfter;

    QmitkToFImageBackground* m_QmitkToFImageBackground1;
    QmitkToFImageBackground* m_QmitkToFImageBackground2;
    QmitkToFImageBackground* m_QmitkToFImageBackground3;

    mitk::OpenCVVideoSource::Pointer m_VideoSource;
    unsigned char* m_VideoTexture;
    unsigned char* m_Widget1Texture;
    unsigned char* m_Widget2Texture;
    unsigned char* m_Widget3Texture;

    int m_ToFCaptureWidth;
    int m_ToFCaptureHeight;
    int m_VideoCaptureWidth;
    int m_VideoCaptureHeight;

    mitk::ToFDistanceImageToSurfaceFilter::Pointer m_ToFDistanceImageToSurfaceFilter;

    mitk::ToFCompositeFilter::Pointer m_ToFCompositeFilter;
    mitk::ToFVisualizationFilter::Pointer m_ToFVisualizationFilter;

  private:

    void RemoveNode(const char* nodename, mitk::DataNode::Pointer node);

    void CreateNode(const char* nodename, mitk::DataNode::Pointer& node);

    void* GetDataFromImage(std::string imageType);

    void RenderWidget(QmitkRenderWindow* mitkWidget, QmitkToFImageBackground* imageBackground, std::string& oldImageType, std::string newImageType, 
      vtkColorTransferFunction* colorTransferFunction, unsigned char* videoTexture, unsigned char* tofTexture );

    void ProcessVideoTransform();

    bool m_TransferFunctionInitialized;

    int m_SurfaceDisplayCount;
	  int m_2DDisplayCount;
    bool m_VideoEnabled;

    std::string m_Widget1ImageType;
    std::string m_Widget2ImageType;
    std::string m_Widget3ImageType;

    mitk::ToFSurfaceVtkMapper3D::Pointer m_ToFSurfaceVtkMapper3D;
};



#endif // _QMITKTOFUTILVIEW_H_INCLUDED

