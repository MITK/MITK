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
#include "mitkSpectroCamRecorder.h"
#include <opencv/cv.h>
#include <opencv/highgui.h>

mitk::SpectroCamRecorder& mitk::SpectroCamRecorder::GetInstance()
{
  static SpectroCamRecorder instance;
  return instance;
}

mitk::SpectroCamRecorder::SpectroCamRecorder()
  : m_NumRecordedImages(0),
    m_Image(cvCreateImage(cvSize(2456, 2058), IPL_DEPTH_16U, 1)),
    m_8BitImage(cvCreateImage(cvSize(2456, 2058), IPL_DEPTH_8U, 1)),
    m_Controller(NULL)
{
}

mitk::SpectroCamRecorder::~SpectroCamRecorder()
{
}

IplImage* mitk::SpectroCamRecorder::GetImage() const
{
  return m_Image;
}


IplImage* mitk::SpectroCamRecorder::Get8BitImage() const
{
  return m_8BitImage;
}

void mitk::SpectroCamRecorder::SetController(SpectroCamController* controller)
{
  m_Controller = controller;
}


void mitk::SpectroCamRecorder::DisplayCameraStream(SpectroCamImage image)
{
            try
            {
                    m_Controller->GetSpectroCam()->ShowVerboseMsg("Filter number: %d", image.m_FilterNum);
                    if (image.m_FilterNum < 0 || image.m_FilterNum >= NUMBER_FILTERS)
                    {
                        std::cout << "Filter number out of range.\n"<<  std::endl;
                    }

                    else
                    {
                        // Allocate the buffer to hold converted the image. (We only want to do this once for performance reasons)
                        if (m_Controller->GetImageInfo().pImageBuffer == NULL)
                        {
                            if (J_Image_Malloc(image.m_pAcqImage, &m_Controller->GetImageInfo()) != J_ST_SUCCESS)
                            {
                                return;
                            }
                        }


                            ////IF "Show all Images" is selected, show all files
                            //if ( Selection == 0)
                                {
                                //=============Converting Images to OpenCV to enable brightness changes==
                                    memcpy( m_Image->imageData , image.m_pAcqImage->pImageBuffer , m_Image->imageSize); //Copy Image from JAI-Format to OCV´s IplImage
                                    cvConvertScale(m_Image, m_8BitImage, 1./40); //Convert image from 16 to 8 bit to properly display it

                                  //Change brightness via addition + resizing the image
                                    cv::Mat temp_8bitMat = cv::Mat(m_8BitImage, false);       //Convert to proper cv::matrix
                                    cv::Mat display;
                                    cv::resize(temp_8bitMat, display, cvSize(1228, 1029)  );   //do some resizeing for faster display
                                //=======================================================================

                                        //Display Image
                                            cv::imshow("Display window", display);            //Display image
                                            cv::waitKey(1);

                                    }


                        ////Show only images aproriate to the selection
                        //else if ()
                        //    {
                        //    }
                    }

            }//try

            catch (std::exception &e) {}
}



void mitk::SpectroCamRecorder::SaveCameraStreamToDisk()
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
