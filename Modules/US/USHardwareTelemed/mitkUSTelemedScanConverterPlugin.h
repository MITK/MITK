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

#ifndef MITKUSTelemedScanConverterPlugin_H_HEADER_INCLUDED_
#define MITKUSTelemedScanConverterPlugin_H_HEADER_INCLUDED_

#include <initguid.h>
#include <strmif.h>
#include <usgfw2.h>
#include <usgfw.h>
#include <usgscanb.h>

#include "mitkUSTelemedSDKHeader.h"
#include "mitkImage.h"

#include "itkFastMutexLock.h"

/**
  * \brief Telemed API plugin for getting images from scan lines.
  * Implements a COM interface whereat only the function InterimOutBufferCB
  * is used for copying given image buffer into a mitk::Image.
  *
  * A pointer to this mitk::Image must be set by calling
  * mitk::USTelemedScanConverterPlugin::SetOutputImage() first.
  * The image content is then updated every time the Telemed API calls
  * the implemented callback function of this class.
  *
  * For more infomration about the implemented COM interface refer to the
  * Telemed API documentation.
  */
class USTelemedScanConverterPlugin : public IUsgfwScanConverterPluginCB

{
public:
  USTelemedScanConverterPlugin( );
  ~USTelemedScanConverterPlugin( );

  // internal functions for Telemed API
  virtual HRESULT __stdcall QueryInterface(const IID& iid,void** ppv);
  virtual ULONG __stdcall AddRef();
  virtual ULONG __stdcall Release();

  /**
    * Setter for a pointer to a mitk::Image in which the current
    * image buffer from the Telemed API will be stored at every
    * API callback. This function must be called before image data
    * can be got from this class.
    * A pointer to a mutex can be set in addition. This mutex will
    * be locked on every writing to the given image.
    */
  void SetOutputImage(mitk::Image::Pointer outputImage, itk::FastMutexLock::Pointer outputImageMutex = 0);

  // receives pointers to input and output media samples
  STDMETHOD(SampleCB) (
    IMediaSample *pSampleIn,
    IMediaSample *pSampleOut,
    int nOutX1,
    int nOutY1,
    int nOutX2,
    int nOutY2
    ) {return S_OK;}

  // receives pointers to input and output sample buffers
  STDMETHOD(BufferCB) (
    PBYTE pBufferIn,
    int nInBufferLen,
    PBYTE pBufferOut,
    int nOutBufferLen,
    int nOutX1,
    int nOutY1,
    int nOutX2,
    int nOutY2
    ) {return S_OK;}

  // receives pointers to input and intermediate sample buffers
  STDMETHOD(InInterimBufferCB) (
    PBYTE pBufferIn,
    int nInBufferLen,
    PBYTE pBufferInterim,
    int nInterimBufferLen,
    int nOutX1,
    int nOutY1,
    int nOutX2,
    int nOutY2
    )  {return S_OK;}

  // receves pointers to input media sample and intermediatesample buffer
  STDMETHOD(InInterimSampleCB) (
    IMediaSample *pSampleIn,
    PBYTE pBufferInterim,
    int nInterimBufferLen,
    int nOutX1,
    int nOutY1,
    int nOutX2,
    int nOutY2
    )  {return S_OK;}

  // receives pointers to output and intermediate sample buffers
  STDMETHOD(InterimOutBufferCB) (
    PBYTE pBufferInterim,
    int nInterimBufferLen,
    PBYTE pBufferOut,
    int nOutBufferLen,
    int nOutX1,
    int nOutY1,
    int nOutX2,
    int nOutY2
    );

  // receives pointers to output media sample and intermediate sample buffer
  STDMETHOD(InterimOutSampleCB) (
    PBYTE pBufferInterim,
    int nInterimBufferLen,
    IMediaSample *pSampleIn,
    int nOutX1,
    int nOutY1,
    int nOutX2,
    int nOutY2
    ) {return S_OK;}

  // receives conversion parameter change pin index
  // if parameter is negative parameter was changed by some filter interface
  STDMETHOD(ParameterCB) (
    int nPin
    ) { return S_OK; }

  STDMETHOD(SetScanConverterPlugin)(IDispatch* plugin);
  //STDMETHOD(getSource)(LONG* plugin);

protected:
  /**
    * Remove Telemed API callback and release and delete m_Plugin attribute.
    */
  void ReleasePlugin( );

  /**
    * Telemed API object for handling callbacks on new image data.
    */
  IUsgfwScanConverterPlugin*  m_Plugin;

  /**
    * Pointer to mitk::Image in which the current image buffer
    * from the Telemed API will be stored at every API callback.
    */
  mitk::Image::Pointer        m_OutputImage;

  /**
    * Mutex for the output image. Has to be set together with the
    * output image via SetOutputImage().
    */
  itk::FastMutexLock::Pointer m_OutputImageMutex;

private:
  long m_cRef ;
};

#endif // MITKUSTelemedScanConverterPlugin_H_HEADER_INCLUDED_