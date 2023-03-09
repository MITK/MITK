/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkMovieGeneratorWin32_h
#define mitkMovieGeneratorWin32_h

#include "MitkAlgorithmsExtExports.h"
#include "mitkMovieGenerator.h"
#include <comdef.h>
#include <memory.h>
#include <string.h>
#include <tchar.h>
#include <vfw.h>

#pragma message("     _Adding library: vfw32.lib")
#pragma comment(lib, "vfw32.lib")

namespace mitk
{
  class MITKALGORITHMSEXT_EXPORT MovieGeneratorWin32 : public MovieGenerator
  {
  public:
    mitkClassMacro(MovieGeneratorWin32, MovieGenerator);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

      virtual void SetFileName(const char *fileName);

  protected:
    MovieGeneratorWin32();

    //! reads out size of current OpenGL context and stores it
    void InitBitmapHeader();

    //! called directly before the first frame is added
    virtual bool InitGenerator();

    //! used to add a frame
    virtual bool AddFrame(void *data);

    //! called after the last frame is added
    virtual bool TerminateGenerator();

    //! name of output file
    _bstr_t m_sFile;

    //! structure contains information for a single stream
    BITMAPINFOHEADER m_bih;

    //! last error string
    _bstr_t m_sError;

  private:
    //! frame counter
    long m_lFrame;

    //! file interface pointer
    PAVIFILE m_pAVIFile;

    //! Address of the stream interface
    PAVISTREAM m_pStream;

    //! Address of the compressed video stream
    PAVISTREAM m_pStreamCompressed;
  };

} // namespace mitk

#endif
