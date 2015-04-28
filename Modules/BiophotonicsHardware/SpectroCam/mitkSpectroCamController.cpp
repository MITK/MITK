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
#include <mitkOpenCVToMitkImageFilter.h>
#include <mitkImageCast.h>


#include <fstream>
#include <string>
#include <iostream>
#include <functional>

//OpenCv includes
#include <opencv\cv.h>
#include <opencv\cxcore.h>
#include <opencv\highgui.h>
#include <opencv2/contrib/contrib.hpp>

// itk includes
#include <itkImage.h>
#include <itkComposeImageFilter.h>

//Spectrocam includes
#include <ISpectroCam.h>
#include <Jai_Factory.h>



using namespace std;
using namespace cv;


namespace mitk {


  typedef itk::VectorImage<unsigned short, 2> CompositeCameraImageType;

  /**
  Here basically all of the implementation for the Spectrocam Controller is located.
  This pimpl implementation is necessary because of the additional JAI and SpectroCam libraries, which should be totally hidden
  to external modules/plugins.
  + due to the c - nature of the Spectrocam callback mechanism we need some global variable sharing which is not nice and should
  be hidden from users.
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


    void SetCurrentImageAsWhiteBalance();

    mitk::Image::Pointer GetCurrentImage();

    CompositeCameraImageType::Pointer m_CompositeItkImage_1;
    CompositeCameraImageType::Pointer m_CompositeItkImage_2;
    mitk::Image::Pointer m_CompositeMitkImage;

    cv::Mat m_CurrentStackSmall;
    cv::Mat m_CurrentTransformedStack;
    cv::Mat m_FlatfieldSmall;
    cv::Mat m_LLSQSolutionSmall;

    const int m_FullWidth;
    const int m_FullHeight;

    const int m_SmallWidth;
    const int m_SmallHeight;

    bool m_ShowOxygenation;

    // for image double buffer swap
    bool m_Image1Selected;

    std::string mode;
    std::string model;

  private:

    void InitializeItkImage(mitk::CompositeCameraImageType::Pointer& compositeImage);

    bool m_IsCameraRunning;

    unsigned m_NumRecordedImages;

    ISpectroCam*  spectroCam;                        //SpectroCam var

    STREAM_HANDLE   m_hDS;                            // Handle to the data stream

    uint32_t        m_iValidBuffers;                // Number of buffers allocated to image acquisition
    BUF_HANDLE      m_pAquBufferID;                  // Handles for all the image buffers
    HANDLE          m_hEventKill;                    // Event used for speeding up the termination of image capture


    //Vars for Ini from file
    FastModeSettings fastSettings;
    SequenceModeSettings seqSettings;
    IndexModeSettings indexSettings;

    void SaveCameraStreamToDisk();
  };

}


// Implementation

static mitk::SpectroCamController_pimpl* my_SpectroCamController;

mitk::Image::Pointer mitk::SpectroCamController_pimpl::GetCurrentImage()
{
  mitk::CompositeCameraImageType::Pointer selectedImage;

  // TODO SW: semaphore here so it cannot interfere with callback readout of m_Image1Selected
  if (this->m_Image1Selected)
  {
    this->m_Image1Selected = !this->m_Image1Selected;
    selectedImage = this->m_CompositeItkImage_1;
  }
  else
  {
    this->m_Image1Selected = !this->m_Image1Selected;
    selectedImage = this->m_CompositeItkImage_2;
  }

  this->m_CompositeMitkImage = mitk::Image::New();
  MITK_INFO << "Image created";
  this->m_CompositeMitkImage->InitializeByItk<mitk::CompositeCameraImageType>(selectedImage);
  MITK_INFO << "Initialized image by ITK";
  this->m_CompositeMitkImage->SetVolume(selectedImage->GetBufferPointer());
  MITK_INFO << "Copied data";


  return m_CompositeMitkImage;
}

static cv::Mat createLLSQSolutionFromHMatrix()
{
  //Create the H-Matrix
  //            Ox            DOx                  //Lookup values at http://omlc.org/spectra/hemoglobin/summary.html
  float H[8][4]= {{50104    ,   37020.   ,    405,  1.}, //Filter0 = 580nm  +- 10
  {33209.2  ,   16156.4 ,    785., 1.}, //Filter1 = 470nm  +- 10
  {319.6    ,   3226.56 ,    280.,  1.}, //Filter2 = 660nm  +- 10
  {32613.2  ,   53788   ,    495.,  1.}, //Filter3 = 560nm  +- 10
  {26629.2  ,   14550   ,    760.,  1.}, //Filter4 = 480nm +- 12,5
  {20035.2  ,   25773.6 ,    665.,  1.}, //Filter5 = 511nm +- 10         ->took 510nm
  {3200     ,   14677.2 ,    380.,  1.}, //Filter6 = 600nm  +- 10
  {290      ,   1794.28 ,    220.,  1.}}; //Filter7 = 700nm

  //Create the hMatrix
  cv::Mat hMatrix = cv::Mat(8, 4, CV_32F, &H ); //cv::Mat(rows, cols, type, fill with)

  cv::Mat transH;
  transpose(hMatrix,transH);
  cv::Mat mulImage = transH * hMatrix;
  cv::Mat invImage = mulImage.inv();
  cv::Mat HCompononentsForLLSQ =  invImage  * transH;

  return HCompononentsForLLSQ;
}

mitk::SpectroCamController_pimpl::SpectroCamController_pimpl()
  :m_hDS(NULL),
  m_NumRecordedImages(1),
  m_IsCameraRunning(false),
  m_SmallWidth(614),
  m_SmallHeight(514),
  m_FullWidth(2456),
  m_FullHeight(2058),
  m_Image1Selected(true),
  m_ShowOxygenation(false)
{
  my_SpectroCamController = this;
  m_CurrentStackSmall = cv::Mat(8, m_SmallWidth * m_SmallHeight, CV_32F, cv::Scalar(0));
  m_FlatfieldSmall    = cv::Mat(8, m_SmallWidth * m_SmallHeight, CV_32F, cv::Scalar(1));
  m_CurrentTransformedStack = cv::Mat(8, m_SmallWidth * m_SmallHeight, CV_32F, cv::Scalar(1));


  m_LLSQSolutionSmall = createLLSQSolutionFromHMatrix();


}

void mitk::SpectroCamController_pimpl::SetCurrentImageAsWhiteBalance()
{
  // deep copy of current image stack
  m_FlatfieldSmall = m_CurrentStackSmall.clone();

  cv::namedWindow("Oxygenation Estimate", WINDOW_AUTOSIZE);
  m_ShowOxygenation = true;
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

void mitk::SpectroCamController_pimpl::InitializeItkImage(mitk::CompositeCameraImageType::Pointer& compositeImage)
{
  if (compositeImage.IsNull())
  {

    MITK_INFO << "initializing itk::Composite Image";
    mitk::CompositeCameraImageType::RegionType region;
    mitk::CompositeCameraImageType::RegionType::SizeType size;
    mitk::CompositeCameraImageType::RegionType::IndexType index;
    mitk::CompositeCameraImageType::SpacingType spacing;
    size.Fill( 1 );
    size[0] = this->m_FullWidth;
    size[1] = this->m_FullHeight;
    index.Fill(0);
    spacing.Fill(1);
    region.SetSize(size);
    region.SetIndex(index);

    compositeImage = mitk::CompositeCameraImageType::New();
    compositeImage->SetRegions(region);
    compositeImage->SetSpacing(spacing);
    compositeImage->SetNumberOfComponentsPerPixel(NUMBER_FILTERS);
    compositeImage->Allocate();
  }
}

/**
* this c callback function is where the magic happens.
* it is called every time a new image has arrived from the SpectroCam.
*/
static void DisplayCameraStream(SpectroCamImage image)
{

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

      // TODO SW: probably we can get around this memcopy by simply setting data pointer? Not sure.
      cv::Mat data = cv::Mat( image.m_pAcqImage->iSizeY, image.m_pAcqImage->iSizeX, CV_16U);
      memcpy( data.datastart , image.m_pAcqImage->pImageBuffer , image.m_pAcqImage->iImageSize); //Copy Image from JAI-Format to OCV´s IplImage


      //============= From opencv to mitk::Image (VectorImage)

      mitk::CompositeCameraImageType::Pointer selectedImage;

      if (my_SpectroCamController->m_Image1Selected)
      {
        selectedImage = my_SpectroCamController->m_CompositeItkImage_1;
      }
      else
      {
        selectedImage = my_SpectroCamController->m_CompositeItkImage_2;
      }

      itk::ImageRegionIterator<mitk::CompositeCameraImageType> imageIterator(selectedImage, selectedImage->GetLargestPossibleRegion());

      MatConstIterator_<unsigned short> it, end;
      it  = data.begin<unsigned short>();
      end = data.end<unsigned short>();


      while(!imageIterator.IsAtEnd())
      {
        mitk::CompositeCameraImageType::PixelType compositePixel = imageIterator.Get();

        compositePixel[image.m_FilterNum] = *it;

        ++it;
        ++imageIterator;
      }

       //both matrix and itk image shall reach end at the same time.
      assert(it == end);


      //============= Display image as opencv window ==

      cv::Mat display;
      cv::resize(data, display, cvSize(my_SpectroCamController->m_SmallWidth, my_SpectroCamController->m_SmallHeight)  );   //do some resizeing for faster display

      display *= 16; // image is only 12 bit large, but datatype is 16 bit. Expand to full range for displaying by multiplying by 2^4.



      //Display Image
      cv::imshow("Display window", display);            //Display image
      //MITK_INFO << "pixel 100,100" << display.at<unsigned short>(0,100);


      //============= TODO: live oxygenation estimation

      if (my_SpectroCamController->m_ShowOxygenation)
      {
        cv::Range slice[2];
        slice[0] = cv::Range( image.m_FilterNum, image.m_FilterNum+1 );
        slice[1] = cv::Range::all();

        cv::Mat currentSlice = my_SpectroCamController->m_CurrentStackSmall(slice);

        cv::Mat currentImageF32;
        display.convertTo(currentImageF32, CV_32F);

        currentImageF32 = currentImageF32.reshape(0, 1);
        currentImageF32.copyTo(currentSlice);

        cv::Mat currentWorkingSlice = currentSlice.clone();

        cv::Mat currentFlatfieldSlice = my_SpectroCamController->m_FlatfieldSmall(slice);
        MITK_INFO << "flat current: " << currentFlatfieldSlice.at<float>(0,100);


        MITK_INFO << "raw measured pixel value: " << currentWorkingSlice.at<float>(0,100);

        cv::divide(currentWorkingSlice, currentFlatfieldSlice, currentWorkingSlice);
        MITK_INFO << "corrected by flatfield pixel: " << currentWorkingSlice.at<float>(0,100);

        cv::log(currentWorkingSlice, currentWorkingSlice);
        currentWorkingSlice = -0.43429 * currentWorkingSlice;
        MITK_INFO << "to absorption: " << currentWorkingSlice.at<float>(0,100);

        currentWorkingSlice.copyTo(my_SpectroCamController->m_CurrentTransformedStack(slice) );

        //MITK_INFO << "slice 0: " << my_SpectroCamController->m_CurrentTransformedStack.at<float>(0,100);;

        cv::Mat currentEstimate = my_SpectroCamController->m_LLSQSolutionSmall * my_SpectroCamController->m_CurrentTransformedStack;
        cv::Range oxyHemo[2];
        oxyHemo[0] = cv::Range(0,1);
        oxyHemo[1] = cv::Range::all();

        cv::Range deOxyHemo[2];
        deOxyHemo[0] = cv::Range(1,2);
        deOxyHemo[1] = cv::Range::all();

        cv::Mat saO2 = currentEstimate(oxyHemo) / (currentEstimate(oxyHemo) + currentEstimate(deOxyHemo));

        cv::Mat saO2Image = saO2.reshape(0, my_SpectroCamController->m_SmallHeight);
        MITK_INFO << "saO2, 200 200: " << saO2Image.at<float>(200,200);

        cv::threshold(saO2Image, saO2Image, 1., 1., cv::THRESH_TRUNC);
        cv::threshold(saO2Image, saO2Image, 0., 0., cv::THRESH_TOZERO);

        saO2Image = saO2Image * 637.;// 255.;

        cv::Mat SaO2IntImage;
        saO2Image.convertTo(SaO2IntImage, CV_8U);
        // MITK_INFO << saO2Image.at<float>(0,100);

        cv::Mat colorImage;
        cv::applyColorMap(SaO2IntImage, colorImage, COLORMAP_JET);
        cv::imshow("Oxygenation Estimate", colorImage);            //Display image
      }


      cv::waitKey(1);
    }

  }//try

  catch (std::exception &e) {
    MITK_INFO << e.what();
  }
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

  // initialize VectorImage
  this->InitializeItkImage(this->m_CompositeItkImage_1);
  this->InitializeItkImage(this->m_CompositeItkImage_2);



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


  cv::namedWindow( "Display window", WINDOW_AUTOSIZE );// Create a window for display.

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


  MITK_INFO << "execute acqstop command";


  // Close stream (this frees all allocated buffers)
  // Stop the image acquisition engine
  J_DataStream_StopAcquisition(m_hDS, ACQ_STOP_FLAG_KILL);


  MITK_INFO << "execute stop aqui";

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

  MITK_INFO << "unprepared buffers";


  // Close Stream
  if(m_hDS)
  {
    J_DataStream_Close(m_hDS);
    m_hDS = NULL;
  }


  MITK_INFO << "closed stream";

  //===================Close Factory and destroy Cam=====================
  //void CloseFactoryAndCamera();       // Close camera and factory to clean up
  retval = retval | spectroCam->stop();


  MITK_INFO << "stopped camera";

  //BOOL TerminateStreamThread(void);   // Terminate the image acquisition thread
  if (spectroCam)
  {
    //DestroySpectroCam(spectroCam);          //Destroy SpectroCam-Objekt
  }

  MITK_INFO << "destroyed spectrocam";

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


mitk::Image::Pointer mitk::SpectroCamController::GetCurrentImage()
{
  return m_SpectroCamController_pimpl->GetCurrentImage();
}

void mitk::SpectroCamController::SetCurrentImageAsWhiteBalance()
{
  m_SpectroCamController_pimpl->SetCurrentImageAsWhiteBalance();
}