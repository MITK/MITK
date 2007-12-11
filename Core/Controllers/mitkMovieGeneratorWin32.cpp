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

#include "mitkMovieGeneratorWin32.h"
#include <GL/gl.h>


mitk::MovieGeneratorWin32::MovieGeneratorWin32()
{
}


void mitk::MovieGeneratorWin32::SetFileName( const char *fileName )
{
  m_sFile = _T( fileName );
  if( _tcsstr( (char*)m_sFile, _T("avi") ) == NULL ) m_sFile += _T( ".avi" );
}


void mitk::MovieGeneratorWin32::InitBitmapHeader()
{
  if (m_renderer) m_renderer->MakeCurrent();  // take the correct viewport!
  GLint viewport[4]; // Where The Viewport Values Will Be Stored
  glGetIntegerv( GL_VIEWPORT, viewport ); // Retrieves The Viewport Values (X, Y, Width, Height)
  m_width = viewport[2];  m_width -= m_width % 4;
  m_height = viewport[3];  m_height -= m_height % 4;

  BITMAPINFOHEADER bih;
  bih.biSize = sizeof( BITMAPINFOHEADER );
  bih.biWidth = m_width;
  bih.biHeight = m_height;
  bih.biPlanes = 1;
  int imgSize = 3 /* BRG*/ * bih.biWidth* bih.biHeight;
  bih.biBitCount = 24;
  bih.biCompression = BI_RGB;
  bih.biSizeImage = imgSize;
  bih.biClrUsed = 0;
  bih.biClrImportant = 0;

  //ASSERT(bih.biWidth%4==0);
  //ASSERT(bih.biHeight%4==0);

  // copying bitmap info structure.
  memcpy( &m_bih, &bih, sizeof( BITMAPINFOHEADER ) );
}


bool mitk::MovieGeneratorWin32::InitGenerator()
{
  InitBitmapHeader();

  AVISTREAMINFO strHdr; // information for a single stream 
  AVICOMPRESSOPTIONS opts;
  AVICOMPRESSOPTIONS FAR * aopts[1] = {&opts};

  TCHAR szBuffer[1024];
  HRESULT hr;

  m_sError=_T("Ok");

  // Step 0 : Let's make sure we are running on 1.1 
  DWORD wVer = HIWORD(VideoForWindowsVersion());
  if (wVer < 0x010a) {
    // oops, we are too old, blow out of here 
    m_sError=_T("Version of Video for Windows too old. Come on, join the 21th century!");
    return false;
  }

  // Step 1 : initialize AVI engine
  AVIFileInit();

  // Step 2 : Open the movie file for writing....
  hr = AVIFileOpen(&m_pAVIFile,      // Address to contain the new file interface pointer
    (LPCTSTR)m_sFile,        // Null-terminated string containing the name of the file to open
    OF_WRITE | OF_CREATE,      // Access mode to use when opening the file. 
    NULL);            // use handler determined from file extension.
  // Name your file .avi -> very important

  if (hr != AVIERR_OK) {
    _tprintf(szBuffer,_T("AVI Engine failed to initialize. Check filename %s."),m_sFile);
    m_sError=szBuffer;
    // Check it succeded.
    switch(hr) {
    case AVIERR_BADFORMAT: 
      m_sError+=_T("The file couldn't be read, indicating a corrupt file or an unrecognized format.");
      break;
    case AVIERR_MEMORY:    
      m_sError+=_T("The file could not be opened because of insufficient memory."); 
      break;
    case AVIERR_FILEREAD:
      m_sError+=_T("A disk error occurred while reading the file."); 
      break;
    case AVIERR_FILEOPEN:    
      m_sError+=_T("A disk error occurred while opening the file.");
      break;
    case REGDB_E_CLASSNOTREG:    
      m_sError+=_T("According to the registry, the type of file specified in AVIFileOpen does not have a handler to process it");
      break;
    }

    return false;
  }

  // Fill in the header for the video stream....
  memset(&strHdr, 0, sizeof(strHdr));
  strHdr.fccType                = streamtypeVIDEO;  // video stream type
  strHdr.fccHandler             = 0;
  strHdr.dwScale                = 1;          // should be one for video
  strHdr.dwRate                 = static_cast<DWORD>(m_FrameRate);        // fps
  strHdr.dwSuggestedBufferSize  = m_bih.biSizeImage;  // Recommended buffer size, in bytes, for the stream.
  SetRect(&strHdr.rcFrame, 0, 0,        // rectangle for stream
    (int) m_bih.biWidth,
    (int) m_bih.biHeight);

  // Step 3 : Create the stream;
  hr = AVIFileCreateStream(m_pAVIFile,        // file pointer
    &m_pStream,        // returned stream pointer
    &strHdr);      // stream header

  // Check it succeded.
  if (hr != AVIERR_OK) {
    m_sError=_T("AVI Stream creation failed. Check Bitmap info.");
    if (hr==AVIERR_READONLY) {
      m_sError+=_T(" Read only file.");
    }
    return false;
  }

  // Step 4: Get codec and infos about codec
  memset(&opts, 0, sizeof(opts));
  
  // predefine MS-CRAM as standard codec
  opts.fccType = streamtypeVIDEO;
  //creates a video with minor quality! Use different codec (must be installed on local machine) to generate movies with higher quality
  opts.fccHandler = mmioFOURCC('M','S','V','C'); 
  opts.dwQuality = 90000; // means 90% quality; dwQuality goes from [0...10000]
  
  
  
  // Poping codec dialog
  // GUI Codec selection does not work in a vs 2005 compiled mitk, since we do not pass a hwnd as first parameter
  // of AVISaveOptions
  #if ! (_MSC_VER >= 1400)

  if (!AVISaveOptions(NULL, 0, 1, &m_pStream, (LPAVICOMPRESSOPTIONS FAR *) &aopts))  {
    AVISaveOptionsFree(1,(LPAVICOMPRESSOPTIONS FAR *) &aopts);
    //return false;
  }

  #endif

  // Step 5:  Create a compressed stream using codec options.
  hr = AVIMakeCompressedStream(&m_pStreamCompressed, 
    m_pStream, 
    &opts, 
    NULL);

  if (hr != AVIERR_OK)
  {
    m_sError=_T("AVI Compressed Stream creation failed.");

    switch(hr)
    {
    case AVIERR_NOCOMPRESSOR:
      m_sError+=_T(" A suitable compressor cannot be found.");
      break;
    case AVIERR_MEMORY:
      m_sError+=_T(" There is not enough memory to complete the operation.");
      break; 
    case AVIERR_UNSUPPORTED:
      m_sError+=_T("Compression is not supported for this type of data. This error might be returned if you try to compress data that is not audio or video.");
      break;
    }

    return false;
  }

  // releasing memory allocated by AVISaveOptionFree
  hr=AVISaveOptionsFree(1,(LPAVICOMPRESSOPTIONS FAR *) &aopts);
  if (hr!=AVIERR_OK) {
    m_sError=_T("Error releasing memory");
    return false;
  }

  // Step 6 : sets the format of a stream at the specified position
  hr = AVIStreamSetFormat(m_pStreamCompressed, 
    0,      // position
    &m_bih,      // stream format
    m_bih.biSize +   // format size
    m_bih.biClrUsed * sizeof(RGBQUAD));

  if (hr != AVIERR_OK) {
    m_sError=_T("AVI Compressed Stream format setting failed.");
    return false;
  }

  // Step 6 : Initialize step counter
  m_lFrame=0;

  return true;
}


bool mitk::MovieGeneratorWin32::AddFrame( void *data )
{
  HRESULT hr = AVIStreamWrite(m_pStreamCompressed,  // stream pointer
    m_lFrame,              // time of this frame
    1,                    // number to write
    (BYTE*)data,          // image buffer
    m_bih.biSizeImage,    // size of this frame
    AVIIF_KEYFRAME,        // flags....
    NULL,
    NULL                );
  // updating frame counter
  m_lFrame++;

  if (hr==AVIERR_OK) return true;
  else return false;
}


bool mitk::MovieGeneratorWin32::TerminateGenerator()
{
  if (m_pStream) {
    AVIStreamRelease(m_pStream);
    m_pStream=NULL;
  }
  if (m_pStreamCompressed) {
    AVIStreamRelease(m_pStreamCompressed);
    m_pStreamCompressed=NULL;
  }
  if (m_pAVIFile)  {
    AVIFileRelease(m_pAVIFile);
    m_pAVIFile=NULL;
  }
  // Close engine
  AVIFileExit();
  return true;
}

