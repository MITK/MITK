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
#include "mitkUSTelemedSDKHeader.h"

#include "mitkImage.h"

/**
*     The telemed sdk needs a plugin. This is the plugin.
      It will be registerd by the usgfw2 class and
      SetScanConverterPlugin is to call. After that the
      sdk of usgfw knows the function which it have to call.
      So every X timeunits some functions here will be called
      with a Pointer to the acutal image memory.

      In this functions here we get the memory an copy it to
      our m_cLiveView memory*/
class USTelemedScanConverterPlugin : public IUsgfwScanConverterPluginCB

{
public:
  USTelemedScanConverterPlugin( );
  ~USTelemedScanConverterPlugin( );

  virtual HRESULT __stdcall QueryInterface(const IID& iid,void** ppv);
  virtual ULONG __stdcall AddRef();
  virtual ULONG __stdcall Release();

  void SetOutputImage(mitk::Image::Pointer outputImage);

  // begin: plug-in must implement IUsgfwScanConverterPluginCB interface that is defined at usgfw.h

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
    );

// end: plug-in must implement IUsgfwScanConverterPluginCB interface that is defined at usgfw.h

  STDMETHOD(SetScanConverterPlugin)(IDispatch* plugin);
  //STDMETHOD(getSource)(LONG* plugin);

protected:
  void ReleasePlugin( );

  IUsgfwScanConverterPlugin*  m_Plugin;
  mitk::Image::Pointer        m_OutputImage;

private:
  long m_cRef ;
};


#endif // MITKUSTelemedScanConverterPlugin_H_HEADER_INCLUDED_