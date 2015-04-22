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
#include "mitkSpectroCamController.h"
#include "mitkLogMacros.h"

#include <fstream>
#include <string>
#include <iostream>
#include <functional>

//OpenCv includes
#include <opencv\cv.h>
#include <opencv\cxcore.h>
#include <opencv\highgui.h>

// itk includes
#include <itkImage.h>
#include <itkOpenCVImageBridge.h>
#include <itkComposeImageFilter.h>

//Spectrocam includes
#include <ISpectroCam.h>
#include <Jai_Factory.h>



using namespace std;
using namespace cv;


namespace mitk {

    /**
    Here basically all of the implementation for the Spectrocam Controller is located.
    It is necessary because of the additional JAI and SpectroCam libraries, which should be totally hidden
    to external modules/plugins.
    */
    class SpectroCamController_pimpl
    {
    public:
        SpectroCamController_pimpl();
        ~SpectroCamController_pimpl();

        bool Ini();
        int OpenCameraConnection();
        int CloseCameraConnection();
        bool isCameraRunning();

        J_tIMAGE_INFO m_CnvImageInfo;                    //Image Info

        std::string mode;
        std::string model;

    private:

        bool m_IsCameraRunning;

        unsigned m_NumRecordedImages;

        ISpectroCam*  spectroCam;                        //SpectroCam var

        STREAM_HANDLE   m_hDS;                            // Handle to the data stream
        HANDLE          m_hStreamThread;                  // Handle to the image acquisition thread
        HANDLE          m_hStreamEvent;                    // Thread used to signal when image thread stops
        VIEW_HANDLE     m_hView;                          // View window handles

        uint32_t        m_iValidBuffers;                // Number of buffers allocated to image acquisition
        BUF_HANDLE      m_pAquBufferID;                  // Handles for all the image buffers
        HANDLE          m_hEventKill;                    // Event used for speeding up the termination of image capture
        bool            m_bEnableThread;                // Flag indicating if the image thread should run


        //Vars for Ini from file
        FastModeSettings fastSettings;
        SequenceModeSettings seqSettings;
        IndexModeSettings indexSettings;

        void SaveCameraStreamToDisk();
    };

}


// Implementation


static mitk::SpectroCamController_pimpl* my_SpectroCamController;

mitk::SpectroCamController_pimpl::SpectroCamController_pimpl()
    :m_hDS(NULL),
    m_hStreamThread(NULL),
    m_hStreamEvent(NULL),
    m_hView(NULL),
    m_NumRecordedImages(1),
    m_IsCameraRunning(false)
{
    my_SpectroCamController = this;
}



mitk::SpectroCamController_pimpl::~SpectroCamController_pimpl()
{
}





void mitk::SpectroCamController_pimpl::SaveCameraStreamToDisk()
{
    /*
    //=================================Save Images to HDD=================================
    imagesRecoreded=0;

    //If Rec is pressed -> Save Images to Harddrive
    if (rec == true)      //On pressing the Rec-Button we wan to save x Stacks with 8 images each
    {
    //Save Image
    saveImage(in);   //std::cout<< "save no."<< imagesRecoreded<< std::endl;
    //Icrement counter
    imagesRecoreded++;

    if (imagesRecoreded >= (NumberOfStacksToBeRecorded*8) )  //If number of images is bigger or equal to the the Image Stack we want to tecord, untoggle the rec button and reset the counter!
    {
    imagesRecoreded=0;
    rec=false;
    ReCButtonControl_Pointer->EnableWindow(TRUE);
    }
    }
    */
}



//Initialize Camera Controller
bool mitk::SpectroCamController_pimpl::Ini()
{
    //===============Get Ini from File===============
    //Get model from file    std::string CChildWindowSampleDlg::getModelNameFromIni()
    std::ifstream fin("C:\\ModeSettings.txt");   //Set File to read
    std::ofstream fout("C:\\ModeSettingsCheck.txt");   //Set output
    const int bufferSize = 1000;
    char buffer[bufferSize];

    if (fin.fail())
    {
        MITK_INFO << "Failed opening file ModeSettings.txt!" << endl ;
    }

    fin.getline(buffer, bufferSize);
    fin.getline(buffer, bufferSize);
    model = buffer;


    //Get mode from file
    //Skipping model, getting mode
    for (int i = 0; i < 3; ++i)
    {
        fin.getline(buffer, bufferSize);
        fout << buffer << endl;
    }

    mode = buffer;

    // [FastModeExposure]
    for (int i = 0; i < 2; ++i)
    {
        fin.getline(buffer, bufferSize);
        fout << buffer << endl;
    }

    fin >> fastSettings.exposure;
    fout << fastSettings.exposure << endl;;

    // [FastModeGain]
    for (int i = 0; i < 3; ++i)
    {
        fin.getline(buffer, bufferSize);
        fout << buffer << endl;
    }

    fin >> fastSettings.gain;
    fout << fastSettings.gain << endl;

    // [SequenceModeExposures]
    for (int i = 0; i < 3; ++i)
    {
        fin.getline(buffer, bufferSize);
        fout << buffer << endl;
    }

    for (int i = 0; i < NUMBER_FILTERS; ++i)
    {
        fin >> seqSettings.exposures[i];
        fout << seqSettings.exposures[i] << endl;
    }

    // [SequenceModeGains]
    for (int i = 0; i < 3; ++i)
    {
        fin.getline(buffer, bufferSize);
        fout << buffer << endl;
    }

    for (int i = 0; i < NUMBER_FILTERS; ++i)
    {
        fin >> seqSettings.gains[i];
        fout << seqSettings.gains[i] << endl;
    }

    // [IndexModeExposures]
    for (int i = 0; i < 3; ++i)
    {
        fin.getline(buffer, bufferSize);
        fout << buffer << endl;
    }

    for (int i = 0; i < NUMBER_FILTERS; ++i)
    {
        fin >> indexSettings.exposures[i];
        fout << indexSettings.exposures[i] << endl;
    }

    // [IndexModeGains]
    for (int i = 0; i < 3; ++i)
    {
        fin.getline(buffer, bufferSize);
        fout << buffer << endl;
    }
    for (int i = 0; i < NUMBER_FILTERS; ++i)
    {
        fin >> indexSettings.gains[i];
        fout << indexSettings.gains[i] << endl;
    }

    // [IndexModeNumberFilterIterations]
    for (int i = 0; i < 3; ++i)
    {
        fin.getline(buffer, bufferSize);
        fout << buffer << endl;
    }

    for (int i = 0; i < NUMBER_FILTERS; ++i)
    {
        fin >> indexSettings.numFilterIterations[i];
        fout << indexSettings.numFilterIterations[i] << endl;
    }

    //After reading files -> close stream
    fin.close();
    fout.close();

    return 0;

}


static void DisplayCameraStream(SpectroCamImage image)
{
    my_SpectroCamController->m_CnvImageInfo = *image.m_pAcqImage;

    MITK_INFO << "image callback call";
    try
    {
        if (image.m_FilterNum < 0 || image.m_FilterNum >= NUMBER_FILTERS)
        {
            std::cout << "Filter number out of range.\n"<<  std::endl;
        }
        else
        {
            // Allocate the buffer to hold converted the image. (We only want to do this once for performance reasons)
            if (image.m_pAcqImage->pImageBuffer == NULL)
            {
                if (J_Image_Malloc(image.m_pAcqImage, image.m_pAcqImage) != J_ST_SUCCESS)
                {
                    return;
                }
            }

            //============= Get data from spectrocam to opencv

            cv::Mat data = cv::Mat( image.m_pAcqImage->iSizeY, image.m_pAcqImage->iSizeX, CV_16U);
            memcpy( data.datastart , image.m_pAcqImage->pImageBuffer , image.m_pAcqImage->iImageSize); //Copy Image from JAI-Format to OCV´s IplImage


            ////============= From opencv to mitk::Image (VectorImage)
            //
            //typedef itk::Image<unsigned short, 2> CameraImageType;
            //// 1. convert to itk::Image using itkOpenCVImageBridge//
            //CameraImageType::Pointer cameraImage = itk::OpenCVImageBridge::CVMatToITKImage<CameraImageType> >(data);

            //// 2. convert to itk::VectorImage using itk::CompositImageFilter


            //// 3. convert to mitk::Image using ...


            //============= Display image as opencv window ==

            cv::Mat display;
            cv::resize(data, display, cvSize(614, 514)  );   //do some resizeing for faster display
            display *= 16; // image is only 12 bit large, but datatype is 16 bit. Expand to full range for displaying by multiplying by 2^4.

            //Display Image
            cv::imshow("Display window", display);            //Display image
            cv::waitKey(1);



            //============= TODO: live oxygenation estimation

        }

    }//try

    catch (std::exception &e) {}
}



int mitk::SpectroCamController_pimpl::OpenCameraConnection()
{
    //=====================OpenFactoryAndCamera=====================
    //Create Factory and cam based on   //BOOL OpenFactoryAndCamera();        // Open factory and search for cameras. Open first camera
    spectroCam = CreateSpectroCam(&DisplayCameraStream, nullptr, 0);
    MITK_INFO << "Camera " <<  model << " is running in: " << mode << "-mode" << endl;



    //=====================Open Streams=====================
    J_STATUS_TYPE status = spectroCam->initialize(model.c_str(), (std::string("C:\\") + model + "\\").c_str());

    if (status != J_ST_SUCCESS)
    {
        MITK_INFO << "Could not initialize camera!" << endl;
    }


    //=====================Open Streams=====================
    if (mode == "Fast")
    {
        status = status | spectroCam->start(fastSettings);
    }

    else if (mode == "Sequence")
    {
        status = status |  spectroCam->start(seqSettings);
    }

    else if (mode == "IndexFast")
    {
        indexSettings.filterModeSpeed = IndexModeSettings::INDEX_FAST;
        status = status |  spectroCam->start(indexSettings);
    }

    else if (mode == "IndexSlow")
    {
        indexSettings.filterModeSpeed = IndexModeSettings::INDEX_SLOW;
        status = status | spectroCam->start(indexSettings);
    }

    else if (mode == "IndexTriggered")
    {
        indexSettings.filterModeSpeed = IndexModeSettings::INDEX_TRIGGERED;
        status = status |  spectroCam->start(indexSettings);
    }

    else
    {
        status = status |  spectroCam->start(fastSettings);
    }

    MITK_INFO << "status flag: " << status;

    if (status == J_ST_SUCCESS)
    {
        m_IsCameraRunning = true;
    }

    return status;
}

bool mitk::SpectroCamController_pimpl::isCameraRunning()
{
    return m_IsCameraRunning;
}



//Method to close down connections
int mitk::SpectroCamController_pimpl::CloseCameraConnection()
{

    // On click -> Stop acquisition
    J_STATUS_TYPE retval = 0;
    CAM_HANDLE hCam = spectroCam->GetCameraHandle();

    // Stop Acquision
    if (hCam)
    {
        retval = retval | J_Camera_ExecuteCommand(hCam, (int8_t*) NODE_NAME_ACQSTOP);
    }


    // Close stream (this frees all allocated buffers)
    {
        // Reset the thread execution flag
        m_bEnableThread = false;

        // Signal the image thread to stop faster
        SetEvent(m_hEventKill);

        // Stop the image acquisition engine
        J_DataStream_StopAcquisition(m_hDS, ACQ_STOP_FLAG_KILL);



        // Wait for the thread to end
        {
            WaitForSingleObject(m_hStreamEvent, INFINITE);

            //Close the thread handle and stream event handle
            //void CChildWindowSampleDlg::CloseThreadHandle(void)
            {
                if(m_hStreamThread)
                {
                    CloseHandle(m_hStreamThread);
                    m_hStreamThread = NULL;
                }

                if(m_hStreamEvent)
                {
                    CloseHandle(m_hStreamEvent);
                    m_hStreamEvent = NULL;
                }
            }
        }
    }


    {
        WaitForSingleObject(m_hStreamEvent, INFINITE);

        // Close the thread handle and stream event handle
        {
            if(m_hStreamThread)
            {
                CloseHandle(m_hStreamThread);
                m_hStreamThread = NULL;
            }

            if(m_hStreamEvent)
            {
                CloseHandle(m_hStreamEvent);
                m_hStreamEvent = NULL;
            }
        }
    }



    // UnPrepare Buffers (this removed the buffers from the acquisition engine and frees buffers)
    {
        void      *pPrivate;
        void      *pBuffer;

        // Flush Queues
        J_DataStream_FlushQueue(m_hDS, ACQ_QUEUE_INPUT_TO_OUTPUT);
        J_DataStream_FlushQueue(m_hDS, ACQ_QUEUE_OUTPUT_DISCARD);

        // Remove the frame buffer from the Acquisition engine.
        J_DataStream_RevokeBuffer(m_hDS, m_pAquBufferID, &pBuffer , &pPrivate);

        m_pAquBufferID = 0;

        m_iValidBuffers = 0;
    }



    // Close Stream
    if(m_hDS)
    {
        J_DataStream_Close(m_hDS);
        m_hDS = NULL;
    }



    // View window opened?
    if (m_hView)
    {
        // Close view window
        J_Image_CloseViewWindow(m_hView);
        m_hView = NULL;
        cout<< "Closed view window\n" << endl;
    }


    //===================Close Factory and destroy Cam=====================
    //void CloseFactoryAndCamera();       // Close camera and factory to clean up
    retval = retval | spectroCam->stop();

    // if (m_CnvImageInfo.pImageBuffer)    J_Image_Free(&m_CnvImageInfo);

    //BOOL TerminateStreamThread(void);   // Terminate the image acquisition thread
    DestroySpectroCam(spectroCam);          //Destroy SpectroCam-Objekt

    if (J_ST_SUCCESS == retval)
    {
        m_IsCameraRunning = false;
    }

    return retval;
}


mitk::SpectroCamController::SpectroCamController()
{
    m_SpectroCamController_pimpl = new SpectroCamController_pimpl();
}

mitk::SpectroCamController::~SpectroCamController()
{
    delete m_SpectroCamController_pimpl;
}

int mitk::SpectroCamController::OpenCameraConnection()
{
    return m_SpectroCamController_pimpl->OpenCameraConnection();
}

int mitk::SpectroCamController::CloseCameraConnection()
{
    return m_SpectroCamController_pimpl->CloseCameraConnection();
}

bool mitk::SpectroCamController::Ini()
{
    return m_SpectroCamController_pimpl->Ini();
}

bool mitk::SpectroCamController::isCameraRunning()
{
    return m_SpectroCamController_pimpl->isCameraRunning();
}