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

#ifndef MovieGeneratorWin32WIN32_H_HEADER_INCLUDED
#define MovieGeneratorWin32WIN32_H_HEADER_INCLUDED

#include "mitkMovieGenerator.h"
#include "MitkAlgorithmsExtExports.h"
#include <comdef.h>
#include <memory.h>
#include <tchar.h>
#include <string.h>
#include <vfw.h>

#pragma message("     _Adding library: vfw32.lib" )
#pragma comment ( lib, "vfw32.lib")


namespace mitk {


class MITKALGORITHMSEXT_EXPORT MovieGeneratorWin32 : public MovieGenerator
{

public:

  mitkClassMacro(MovieGeneratorWin32, MovieGenerator);
  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)

  virtual void SetFileName( const char *fileName );


protected:

  MovieGeneratorWin32();

  //! reads out size of current OpenGL context and stores it
  void InitBitmapHeader();

  //! called directly before the first frame is added
  virtual bool InitGenerator();

  //! used to add a frame
  virtual bool AddFrame( void *data );

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

#endif /* MovieGeneratorWin32WIN32_H_HEADER_INCLUDED */
