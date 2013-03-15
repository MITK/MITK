/*=============================================================================

  Library: CppMicroServices

  Copyright (c) German Cancer Research Center,
    Division of Medical and Biological Informatics

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

=============================================================================*/

#define MINIZ_NO_TIME
#define MINIZ_NO_ARCHIVE_APIS
#include "miniz.c"

#include <limits.h>
#include <stdio.h>

typedef unsigned char uint8;
typedef unsigned int uint;

#define COMPRESS_MSG_BUFFER_SIZE 1024
static char us_compress_error[COMPRESS_MSG_BUFFER_SIZE];

#define my_max(a,b) (((a) > (b)) ? (a) : (b))
#define my_min(a,b) (((a) < (b)) ? (a) : (b))

#define BUF_SIZE (1024 * 1024)
static uint8 s_inbuf[BUF_SIZE];
static uint8 s_outbuf[BUF_SIZE];


const char* us_resource_compressor_error()
{
  return us_compress_error;
}

FILE* us_resource_compressor(FILE* pInfile, long file_loc, int level, long* out_size)
{
  const uint infile_size = (uint)file_loc;
  FILE* pOutfile = NULL;
  z_stream stream;
  uint infile_remaining = infile_size;
  unsigned char header[4];
  long bytes_written = 0;

  memset(us_compress_error, 0, COMPRESS_MSG_BUFFER_SIZE);

  if (file_loc < 0 || file_loc > INT_MAX)
  {
    sprintf(us_compress_error, "Resource too large to be processed.");
    return NULL;
  }

  // Open output file.
  pOutfile = tmpfile();
  if (!pOutfile)
  {
    sprintf(us_compress_error, "Failed opening temporary file.");
    return NULL;
  }

  // Init the z_stream
  memset(&stream, 0, sizeof(stream));
  stream.next_in = s_inbuf;
  stream.avail_in = 0;
  stream.next_out = s_outbuf;
  stream.avail_out = BUF_SIZE;

  // Compression.
  if (deflateInit2(&stream, level, MZ_DEFLATED, -MZ_DEFAULT_WINDOW_BITS, 9, MZ_DEFAULT_STRATEGY) != Z_OK)
  {
    sprintf(us_compress_error, "deflateInit() failed.");
    return NULL;
  }

  // Write the uncompressed file size in the first four bytes
  header[0] = (unsigned char)((file_loc & 0xff000000) >> 24);
  header[1] = (unsigned char)((file_loc & 0x00ff0000) >> 16);
  header[2] = (unsigned char)((file_loc & 0x0000ff00) >> 8);
  header[3] = (unsigned char)(file_loc & 0x000000ff);
  fwrite(header, 1, sizeof(header), pOutfile);

  bytes_written = sizeof(header);
  for ( ; ; )
  {
    int status;
    if (!stream.avail_in)
    {
      // Input buffer is empty, so read more bytes from input file.
      uint n = my_min(BUF_SIZE, infile_remaining);

      if (fread(s_inbuf, 1, n, pInfile) != n)
      {
        sprintf(us_compress_error, "Failed reading from input file.");
        return NULL;
      }

      stream.next_in = s_inbuf;
      stream.avail_in = n;

      infile_remaining -= n;
    }

    status = deflate(&stream, infile_remaining ? Z_NO_FLUSH : Z_FINISH);

    if ((status == Z_STREAM_END) || (!stream.avail_out))
    {
      // Output buffer is full, or compression is done, so write buffer to output file.
      uint n = BUF_SIZE - stream.avail_out;
      if (fwrite(s_outbuf, 1, n, pOutfile) != n)
      {
        sprintf(us_compress_error, "Failed writing to output file.");
        return NULL;
      }
      stream.next_out = s_outbuf;
      stream.avail_out = BUF_SIZE;
      bytes_written += n;
    }

    if (status == Z_STREAM_END)
      break;
    else if (status != Z_OK)
    {
      sprintf(us_compress_error, "deflate() failed with status %i.", status);
      return NULL;
    }
  }

  if (deflateEnd(&stream) != Z_OK)
  {
    sprintf(us_compress_error, "deflateEnd() failed.");
    return NULL;
  }

  if (out_size != NULL)
  {
    *out_size = bytes_written;
  }
  fseek(pOutfile, 0, SEEK_SET);
  return pOutfile;
}
