/* miniz.c v1.15 - public domain deflate/inflate, zlib-subset, ZIP reading/writing/appending, PNG writing
   See "unlicense" statement at the end of this file.
   Rich Geldreich <richgel99@gmail.com>, last updated Oct. 13, 2013
   Implements RFC 1950: https://www.ietf.org/rfc/rfc1950.txt and RFC 1951: https://www.ietf.org/rfc/rfc1951.txt

   Most API's defined in miniz.c are optional. For example, to disable the archive related functions just define
   US_MZ_NO_ARCHIVE_APIS, or to get rid of all stdio usage define US_MZ_NO_STDIO (see the list below for more macros).

   * Change History
     10/13/13 v1.15 r4 - Interim bugfix release while I work on the next major release with Zip64 support (almost there!):
       - Critical fix for the US_MZ_ZIP_FLAG_DO_NOT_SORT_CENTRAL_DIRECTORY bug (thanks kahmyong.moon@hp.com) which could cause locate files to not find files. This bug
        would only have occurred in earlier versions if you explicitly used this flag, OR if you used us_mz_zip_extract_archive_file_to_heap() or us_mz_zip_add_mem_to_archive_file_in_place()
        (which used this flag). If you can't switch to v1.15 but want to fix this bug, just remove the uses of this flag from both helper funcs (and of course don't use the flag).
       - Bugfix in us_mz_zip_reader_extract_to_mem_no_alloc() from kymoon when pUser_read_buf is not NULL and compressed size is > uncompressed size
       - Fixing us_mz_zip_reader_extract_*() funcs so they don't try to extract compressed data from directory entries, to account for weird zipfiles which contain zero-size compressed data on dir entries.
         Hopefully this fix won't cause any issues on weird zip archives, because it assumes the low 16-bits of zip external attributes are DOS attributes (which I believe they always are in practice).
       - Fixing us_mz_zip_reader_is_file_a_directory() so it doesn't check the internal attributes, just the filename and external attributes
       - us_mz_zip_reader_init_file() - missing US_MZ_FCLOSE() call if the seek failed
       - Added cmake support for Linux builds which builds all the examples, tested with clang v3.3 and gcc v4.6.
       - Clang fix for us_tdefl_write_image_to_png_file_in_memory() from toffaletti
       - Merged US_MZ_FORCEINLINE fix from hdeanclark
       - Fix <time.h> include before config #ifdef, thanks emil.brink
       - Added us_tdefl_write_image_to_png_file_in_memory_ex(): supports Y flipping (super useful for OpenGL apps), and explicit control over the compression level (so you can
        set it to 1 for real-time compression).
       - Merged in some compiler fixes from paulharris's github repro.
       - Retested this build under Windows (VS 2010, including static analysis), tcc  0.9.26, gcc v4.6 and clang v3.3.
       - Added example6.c, which dumps an image of the mandelbrot set to a PNG file.
       - Modified example2 to help test the US_MZ_ZIP_FLAG_DO_NOT_SORT_CENTRAL_DIRECTORY flag more.
       - In r3: Bugfix to us_mz_zip_writer_add_file() found during merge: Fix possible src file fclose() leak if alignment bytes+local header file write faiiled
       - In r4: Minor bugfix to us_mz_zip_writer_add_from_zip_reader(): Was pushing the wrong central dir header offset, appears harmless in this release, but it became a problem in the zip64 branch
     5/20/12 v1.14 - MinGW32/64 GCC 4.6.1 compiler fixes: added US_MZ_FORCEINLINE, #include <time.h> (thanks fermtect).
     5/19/12 v1.13 - From jason@cornsyrup.org and kelwert@mtu.edu - Fix us_mz_crc32() so it doesn't compute the wrong CRC-32's when us_mz_ulong is 64-bit.
       - Temporarily/locally slammed in "typedef unsigned long us_mz_ulong" and re-ran a randomized regression test on ~500k files.
       - Eliminated a bunch of warnings when compiling with GCC 32-bit/64.
       - Ran all examples, miniz.c, and tinfl.c through MSVC 2008's /analyze (static analysis) option and fixed all warnings (except for the silly
        "Use of the comma-operator in a tested expression.." analysis warning, which I purposely use to work around a MSVC compiler warning).
       - Created 32-bit and 64-bit Codeblocks projects/workspace. Built and tested Linux executables. The codeblocks workspace is compatible with Linux+Win32/x64.
       - Added miniz_tester solution/project, which is a useful little app derived from LZHAM's tester app that I use as part of the regression test.
       - Ran miniz.c and tinfl.c through another series of regression testing on ~500,000 files and archives.
       - Modified example5.c so it purposely disables a bunch of high-level functionality (US_MZ_NO_STDIO, etc.). (Thanks to corysama for the US_MZ_NO_STDIO bug report.)
       - Fix ftell() usage in examples so they exit with an error on files which are too large (a limitation of the examples, not miniz itself).
     4/12/12 v1.12 - More comments, added low-level example5.c, fixed a couple minor level_and_flags issues in the archive API's.
      level_and_flags can now be set to US_MZ_DEFAULT_COMPRESSION. Thanks to Bruce Dawson <bruced@valvesoftware.com> for the feedback/bug report.
     5/28/11 v1.11 - Added statement from unlicense.org
     5/27/11 v1.10 - Substantial compressor optimizations:
      - Level 1 is now ~4x faster than before. The L1 compressor's throughput now varies between 70-110MB/sec. on a
      - Core i7 (actual throughput varies depending on the type of data, and x64 vs. x86).
      - Improved baseline L2-L9 compression perf. Also, greatly improved compression perf. issues on some file types.
      - Refactored the compression code for better readability and maintainability.
      - Added level 10 compression level (L10 has slightly better ratio than level 9, but could have a potentially large
       drop in throughput on some files).
     5/15/11 v1.09 - Initial stable release.

   * Low-level Deflate/Inflate implementation notes:

     Compression: Use the "tdefl" API's. The compressor supports raw, static, and dynamic blocks, lazy or
     greedy parsing, match length filtering, RLE-only, and Huffman-only streams. It performs and compresses
     approximately as well as zlib.

     Decompression: Use the "tinfl" API's. The entire decompressor is implemented as a single function
     coroutine: see us_tinfl_decompress(). It supports decompression into a 32KB (or larger power of 2) wrapping buffer, or into a memory
     block large enough to hold the entire file.

     The low-level tdefl/tinfl API's do not make any use of dynamic memory allocation.

   * zlib-style API notes:

     miniz.c implements a fairly large subset of zlib. There's enough functionality present for it to be a drop-in
     zlib replacement in many apps:
        The z_stream struct, optional memory allocation callbacks
        deflateInit/deflateInit2/deflate/deflateReset/deflateEnd/deflateBound
        inflateInit/inflateInit2/inflate/inflateEnd
        compress, compress2, compressBound, uncompress
        CRC-32, Adler-32 - Using modern, minimal code size, CPU cache friendly routines.
        Supports raw deflate streams or standard zlib streams with adler-32 checking.

     Limitations:
      The callback API's are not implemented yet. No support for gzip headers or zlib static dictionaries.
      I've tried to closely emulate zlib's various flavors of stream flushing and return status codes, but
      there are no guarantees that miniz.c pulls this off perfectly.

   * PNG writing: See the us_tdefl_write_image_to_png_file_in_memory() function, originally written by
     Alex Evans. Supports 1-4 bytes/pixel images.

   * ZIP archive API notes:

     The ZIP archive API's where designed with simplicity and efficiency in mind, with just enough abstraction to
     get the job done with minimal fuss. There are simple API's to retrieve file information, read files from
     existing archives, create new archives, append new files to existing archives, or clone archive data from
     one archive to another. It supports archives located in memory or the heap, on disk (using stdio.h),
     or you can specify custom file read/write callbacks.

     - Archive reading: Just call this function to read a single file from a disk archive:

      void *us_mz_zip_extract_archive_file_to_heap(const char *pZip_filename, const char *pArchive_name,
        size_t *pSize, us_mz_uint zip_flags);

     For more complex cases, use the "us_mz_zip_reader" functions. Upon opening an archive, the entire central
     directory is located and read as-is into memory, and subsequent file access only occurs when reading individual files.

     - Archives file scanning: The simple way is to use this function to scan a loaded archive for a specific file:

     int us_mz_zip_reader_locate_file(us_mz_zip_archive *pZip, const char *pName, const char *pComment, us_mz_uint flags);

     The locate operation can optionally check file comments too, which (as one example) can be used to identify
     multiple versions of the same file in an archive. This function uses a simple linear search through the central
     directory, so it's not very fast.

     Alternately, you can iterate through all the files in an archive (using us_mz_zip_reader_get_num_files()) and
     retrieve detailed info on each file by calling us_mz_zip_reader_file_stat().

     - Archive creation: Use the "us_mz_zip_writer" functions. The ZIP writer immediately writes compressed file data
     to disk and builds an exact image of the central directory in memory. The central directory image is written
     all at once at the end of the archive file when the archive is finalized.

     The archive writer can optionally align each file's local header and file data to any power of 2 alignment,
     which can be useful when the archive will be read from optical media. Also, the writer supports placing
     arbitrary data blobs at the very beginning of ZIP archives. Archives written using either feature are still
     readable by any ZIP tool.

     - Archive appending: The simple way to add a single file to an archive is to call this function:

      us_mz_bool us_mz_zip_add_mem_to_archive_file_in_place(const char *pZip_filename, const char *pArchive_name,
        const void *pBuf, size_t buf_size, const void *pComment, us_mz_uint16 comment_size, us_mz_uint level_and_flags);

     The archive will be created if it doesn't already exist, otherwise it'll be appended to.
     Note the appending is done in-place and is not an atomic operation, so if something goes wrong
     during the operation it's possible the archive could be left without a central directory (although the local
     file headers and file data will be fine, so the archive will be recoverable).

     For more complex archive modification scenarios:
     1. The safest way is to use a us_mz_zip_reader to read the existing archive, cloning only those bits you want to
     preserve into a new archive using using the us_mz_zip_writer_add_from_zip_reader() function (which compiles the
     compressed file data as-is). When you're done, delete the old archive and rename the newly written archive, and
     you're done. This is safe but requires a bunch of temporary disk space or heap memory.

     2. Or, you can convert an us_mz_zip_reader in-place to an us_mz_zip_writer using us_mz_zip_writer_init_from_reader(),
     append new files as needed, then finalize the archive which will write an updated central directory to the
     original archive. (This is basically what us_mz_zip_add_mem_to_archive_file_in_place() does.) There's a
     possibility that the archive's central directory could be lost with this method if anything goes wrong, though.

     - ZIP archive support limitations:
     No zip64 or spanning support. Extraction functions can only handle unencrypted, stored or deflated files.
     Requires streams capable of seeking.

   * This is a header file library, like stb_image.c. To get only a header file, either cut and paste the
     below header, or create miniz.h, #define US_MZ_HEADER_FILE_ONLY, and then include miniz.c from it.

   * Important: For best perf. be sure to customize the below macros for your target platform:
     #define US_MZ_USE_UNALIGNED_LOADS_AND_STORES 1
     #define US_MZ_LITTLE_ENDIAN 1
     #define US_MZ_HAS_64BIT_REGISTERS 1

   * On platforms using glibc, Be sure to "#define _LARGEFILE64_SOURCE 1" before including miniz.c to ensure miniz
     uses the 64-bit variants: fopen64(), stat64(), etc. Otherwise you won't be able to process large files
     (i.e. 32-bit stat() fails for me on files > 0x7FFFFFFF bytes).
*/

#ifndef US_MZ_HEADER_INCLUDED
#define US_MZ_HEADER_INCLUDED

#include <stdlib.h>

// Defines to completely disable specific portions of miniz.c:
// If all macros here are defined the only functionality remaining will be CRC-32, adler-32, tinfl, and tdefl.

// Define US_MZ_NO_STDIO to disable all usage and any functions which rely on stdio for file I/O.
//#define US_MZ_NO_STDIO

// If US_MZ_NO_TIME is specified then the ZIP archive functions will not be able to get the current time, or
// get/set file times, and the C run-time funcs that get/set times won't be called.
// The current downside is the times written to your archives will be from 1979.
//#define US_MZ_NO_TIME

// Define US_MZ_NO_ARCHIVE_APIS to disable all ZIP archive API's.
//#define US_MZ_NO_ARCHIVE_APIS

// Define US_MZ_NO_ARCHIVE_APIS to disable all writing related ZIP archive API's.
//#define US_MZ_NO_ARCHIVE_WRITING_APIS

// Define US_MZ_NO_ZLIB_APIS to remove all ZLIB-style compression/decompression API's.
//#define US_MZ_NO_ZLIB_APIS

// Define US_MZ_NO_ZLIB_COMPATIBLE_NAME to disable zlib names, to prevent conflicts against stock zlib.
//#define US_MZ_NO_ZLIB_COMPATIBLE_NAMES

// Define US_MZ_NO_MALLOC to disable all calls to malloc, free, and realloc.
// Note if US_MZ_NO_MALLOC is defined then the user must always provide custom user alloc/free/realloc
// callbacks to the zlib and archive API's, and a few stand-alone helper API's which don't provide custom user
// functions (such as us_tdefl_compress_mem_to_heap() and us_tinfl_decompress_mem_to_heap()) won't work.
//#define US_MZ_NO_MALLOC

#if defined(__TINYC__) && (defined(__linux) || defined(__linux__))
  // TODO: Work around "error: include file 'sys\utime.h' when compiling with tcc on Linux
  #define US_MZ_NO_TIME
#endif

#if !defined(US_MZ_NO_TIME) && !defined(US_MZ_NO_ARCHIVE_APIS)
  #include <time.h>
#endif

#if defined(_M_IX86) || defined(_M_X64) || defined(__i386__) || defined(__i386) || defined(__i486__) || defined(__i486) || defined(i386) || defined(__ia64__) || defined(__x86_64__)
// US_MZ_X86_OR_X64_CPU is only used to help set the below macros.
#define US_MZ_X86_OR_X64_CPU 1
#endif

#if (__BYTE_ORDER__==__ORDER_LITTLE_ENDIAN__) || US_MZ_X86_OR_X64_CPU
// Set US_MZ_LITTLE_ENDIAN to 1 if the processor is little endian.
#define US_MZ_LITTLE_ENDIAN 1
#endif

#if US_MZ_X86_OR_X64_CPU
// Set US_MZ_USE_UNALIGNED_LOADS_AND_STORES to 1 on CPU's that permit efficient integer loads and stores from unaligned addresses.
#define US_MZ_USE_UNALIGNED_LOADS_AND_STORES 1
#endif

#if defined(_M_X64) || defined(_WIN64) || defined(__MINGW64__) || defined(_LP64) || defined(__LP64__) || defined(__ia64__) || defined(__x86_64__)
// Set US_MZ_HAS_64BIT_REGISTERS to 1 if operations on 64-bit integers are reasonably fast (and don't involve compiler generated calls to helper functions).
#define US_MZ_HAS_64BIT_REGISTERS 1
#endif

#ifdef __cplusplus
extern "C" {
#endif

// ------------------- zlib-style API Definitions.

// For more compatibility with zlib, miniz.c uses unsigned long for some parameters/struct members. Beware: us_mz_ulong can be either 32 or 64-bits!
typedef unsigned long us_mz_ulong;

// us_mz_free() internally uses the US_MZ_FREE() macro (which by default calls free() unless you've modified the US_MZ_MALLOC macro) to release a block allocated from the heap.
void us_mz_free(void *p);

#define US_MZ_ADLER32_INIT (1)
// us_mz_adler32() returns the initial adler-32 value to use when called with ptr==NULL.
us_mz_ulong us_mz_adler32(us_mz_ulong adler, const unsigned char *ptr, size_t buf_len);

#define US_MZ_CRC32_INIT (0)
// us_mz_crc32() returns the initial CRC-32 value to use when called with ptr==NULL.
us_mz_ulong us_mz_crc32(us_mz_ulong crc, const unsigned char *ptr, size_t buf_len);

// Compression strategies.
enum { US_MZ_DEFAULT_STRATEGY = 0, US_MZ_FILTERED = 1, US_MZ_HUFFMAN_ONLY = 2, US_MZ_RLE = 3, US_MZ_FIXED = 4 };

// Method
#define US_MZ_DEFLATED 8

#ifndef US_MZ_NO_ZLIB_APIS

// Heap allocation callbacks.
// Note that us_mz_alloc_func parameter types purpsosely differ from zlib's: items/size is size_t, not unsigned long.
typedef void *(*us_mz_alloc_func)(void *opaque, size_t items, size_t size);
typedef void (*us_mz_free_func)(void *opaque, void *address);
typedef void *(*us_mz_realloc_func)(void *opaque, void *address, size_t items, size_t size);

#define US_MZ_VERSION          "9.1.15"
#define US_MZ_VERNUM           0x91F0
#define US_MZ_VER_MAJOR        9
#define US_MZ_VER_MINOR        1
#define US_MZ_VER_REVISION     15
#define US_MZ_VER_SUBREVISION  0

// Flush values. For typical usage you only need US_MZ_NO_FLUSH and US_MZ_FINISH. The other values are for advanced use (refer to the zlib docs).
enum { US_MZ_NO_FLUSH = 0, US_MZ_PARTIAL_FLUSH = 1, US_MZ_SYNC_FLUSH = 2, US_MZ_FULL_FLUSH = 3, US_MZ_FINISH = 4, US_MZ_BLOCK = 5 };

// Return status codes. US_MZ_PARAM_ERROR is non-standard.
enum { US_MZ_OK = 0, US_MZ_STREAM_END = 1, US_MZ_NEED_DICT = 2, US_MZ_ERRNO = -1, US_MZ_STREAM_ERROR = -2, US_MZ_DATA_ERROR = -3, US_MZ_MEM_ERROR = -4, US_MZ_BUF_ERROR = -5, US_MZ_VERSION_ERROR = -6, US_MZ_PARAM_ERROR = -10000 };

// Compression levels: 0-9 are the standard zlib-style levels, 10 is best possible compression (not zlib compatible, and may be very slow), US_MZ_DEFAULT_COMPRESSION=US_MZ_DEFAULT_LEVEL.
enum { US_MZ_NO_COMPRESSION = 0, US_MZ_BEST_SPEED = 1, US_MZ_BEST_COMPRESSION = 9, US_MZ_UBER_COMPRESSION = 10, US_MZ_DEFAULT_LEVEL = 6, US_MZ_DEFAULT_COMPRESSION = -1 };

// Window bits
#define US_MZ_DEFAULT_WINDOW_BITS 15

struct us_mz_internal_state;

// Compression/decompression stream struct.
typedef struct us_mz_stream_s
{
  const unsigned char *next_in;     // pointer to next byte to read
  unsigned int avail_in;            // number of bytes available at next_in
  us_mz_ulong total_in;                // total number of bytes consumed so far

  unsigned char *next_out;          // pointer to next byte to write
  unsigned int avail_out;           // number of bytes that can be written to next_out
  us_mz_ulong total_out;               // total number of bytes produced so far

  char *msg;                        // error msg (unused)
  struct us_mz_internal_state *state;  // internal state, allocated by zalloc/zfree

  us_mz_alloc_func zalloc;             // optional heap allocation function (defaults to malloc)
  us_mz_free_func zfree;               // optional heap free function (defaults to free)
  void *opaque;                     // heap alloc function user pointer

  int data_type;                    // data_type (unused)
  us_mz_ulong adler;                   // adler32 of the source or uncompressed data
  us_mz_ulong reserved;                // not used
} us_mz_stream;

typedef us_mz_stream *us_mz_streamp;

// Returns the version string of miniz.c.
const char *us_mz_version(void);

// us_mz_deflateInit() initializes a compressor with default options:
// Parameters:
//  pStream must point to an initialized us_mz_stream struct.
//  level must be between [US_MZ_NO_COMPRESSION, US_MZ_BEST_COMPRESSION].
//  level 1 enables a specially optimized compression function that's been optimized purely for performance, not ratio.
//  (This special func. is currently only enabled when US_MZ_USE_UNALIGNED_LOADS_AND_STORES and US_MZ_LITTLE_ENDIAN are defined.)
// Return values:
//  US_MZ_OK on success.
//  US_MZ_STREAM_ERROR if the stream is bogus.
//  US_MZ_PARAM_ERROR if the input parameters are bogus.
//  US_MZ_MEM_ERROR on out of memory.
int us_mz_deflateInit(us_mz_streamp pStream, int level);

// us_mz_deflateInit2() is like us_mz_deflate(), except with more control:
// Additional parameters:
//   method must be US_MZ_DEFLATED
//   window_bits must be US_MZ_DEFAULT_WINDOW_BITS (to wrap the deflate stream with zlib header/adler-32 footer) or -US_MZ_DEFAULT_WINDOW_BITS (raw deflate/no header or footer)
//   mem_level must be between [1, 9] (it's checked but ignored by miniz.c)
int us_mz_deflateInit2(us_mz_streamp pStream, int level, int method, int window_bits, int mem_level, int strategy);

// Quickly resets a compressor without having to reallocate anything. Same as calling us_mz_deflateEnd() followed by us_mz_deflateInit()/us_mz_deflateInit2().
int us_mz_deflateReset(us_mz_streamp pStream);

// us_mz_deflate() compresses the input to output, consuming as much of the input and producing as much output as possible.
// Parameters:
//   pStream is the stream to read from and write to. You must initialize/update the next_in, avail_in, next_out, and avail_out members.
//   flush may be US_MZ_NO_FLUSH, US_MZ_PARTIAL_FLUSH/US_MZ_SYNC_FLUSH, US_MZ_FULL_FLUSH, or US_MZ_FINISH.
// Return values:
//   US_MZ_OK on success (when flushing, or if more input is needed but not available, and/or there's more output to be written but the output buffer is full).
//   US_MZ_STREAM_END if all input has been consumed and all output bytes have been written. Don't call us_mz_deflate() on the stream anymore.
//   US_MZ_STREAM_ERROR if the stream is bogus.
//   US_MZ_PARAM_ERROR if one of the parameters is invalid.
//   US_MZ_BUF_ERROR if no forward progress is possible because the input and/or output buffers are empty. (Fill up the input buffer or free up some output space and try again.)
int us_mz_deflate(us_mz_streamp pStream, int flush);

// us_mz_deflateEnd() deinitializes a compressor:
// Return values:
//  US_MZ_OK on success.
//  US_MZ_STREAM_ERROR if the stream is bogus.
int us_mz_deflateEnd(us_mz_streamp pStream);

// us_mz_deflateBound() returns a (very) conservative upper bound on the amount of data that could be generated by deflate(), assuming flush is set to only US_MZ_NO_FLUSH or US_MZ_FINISH.
us_mz_ulong us_mz_deflateBound(us_mz_streamp pStream, us_mz_ulong source_len);

// Single-call compression functions us_mz_compress() and us_mz_compress2():
// Returns US_MZ_OK on success, or one of the error codes from us_mz_deflate() on failure.
int us_mz_compress(unsigned char *pDest, us_mz_ulong *pDest_len, const unsigned char *pSource, us_mz_ulong source_len);
int us_mz_compress2(unsigned char *pDest, us_mz_ulong *pDest_len, const unsigned char *pSource, us_mz_ulong source_len, int level);

// us_mz_compressBound() returns a (very) conservative upper bound on the amount of data that could be generated by calling us_mz_compress().
us_mz_ulong us_mz_compressBound(us_mz_ulong source_len);

// Initializes a decompressor.
int us_mz_inflateInit(us_mz_streamp pStream);

// us_mz_inflateInit2() is like us_mz_inflateInit() with an additional option that controls the window size and whether or not the stream has been wrapped with a zlib header/footer:
// window_bits must be US_MZ_DEFAULT_WINDOW_BITS (to parse zlib header/footer) or -US_MZ_DEFAULT_WINDOW_BITS (raw deflate).
int us_mz_inflateInit2(us_mz_streamp pStream, int window_bits);

// Decompresses the input stream to the output, consuming only as much of the input as needed, and writing as much to the output as possible.
// Parameters:
//   pStream is the stream to read from and write to. You must initialize/update the next_in, avail_in, next_out, and avail_out members.
//   flush may be US_MZ_NO_FLUSH, US_MZ_SYNC_FLUSH, or US_MZ_FINISH.
//   On the first call, if flush is US_MZ_FINISH it's assumed the input and output buffers are both sized large enough to decompress the entire stream in a single call (this is slightly faster).
//   US_MZ_FINISH implies that there are no more source bytes available beside what's already in the input buffer, and that the output buffer is large enough to hold the rest of the decompressed data.
// Return values:
//   US_MZ_OK on success. Either more input is needed but not available, and/or there's more output to be written but the output buffer is full.
//   US_MZ_STREAM_END if all needed input has been consumed and all output bytes have been written. For zlib streams, the adler-32 of the decompressed data has also been verified.
//   US_MZ_STREAM_ERROR if the stream is bogus.
//   US_MZ_DATA_ERROR if the deflate stream is invalid.
//   US_MZ_PARAM_ERROR if one of the parameters is invalid.
//   US_MZ_BUF_ERROR if no forward progress is possible because the input buffer is empty but the inflater needs more input to continue, or if the output buffer is not large enough. Call us_mz_inflate() again
//   with more input data, or with more room in the output buffer (except when using single call decompression, described above).
int us_mz_inflate(us_mz_streamp pStream, int flush);

// Deinitializes a decompressor.
int us_mz_inflateEnd(us_mz_streamp pStream);

// Single-call decompression.
// Returns US_MZ_OK on success, or one of the error codes from us_mz_inflate() on failure.
int us_mz_uncompress(unsigned char *pDest, us_mz_ulong *pDest_len, const unsigned char *pSource, us_mz_ulong source_len);

// Returns a string description of the specified error code, or NULL if the error code is invalid.
const char *us_mz_error(int err);

// Redefine zlib-compatible names to miniz equivalents, so miniz.c can be used as a drop-in replacement for the subset of zlib that miniz.c supports.
// Define US_MZ_NO_ZLIB_COMPATIBLE_NAMES to disable zlib-compatibility if you use zlib in the same project.
#ifndef US_MZ_NO_ZLIB_COMPATIBLE_NAMES
  typedef unsigned char Byte;
  typedef unsigned int uInt;
  typedef us_mz_ulong uLong;
  typedef Byte Bytef;
  typedef uInt uIntf;
  typedef char charf;
  typedef int intf;
  typedef void *voidpf;
  typedef uLong uLongf;
  typedef void *voidp;
  typedef void *const voidpc;
  #define Z_NULL                0
  #define Z_NO_FLUSH            US_MZ_NO_FLUSH
  #define Z_PARTIAL_FLUSH       US_MZ_PARTIAL_FLUSH
  #define Z_SYNC_FLUSH          US_MZ_SYNC_FLUSH
  #define Z_FULL_FLUSH          US_MZ_FULL_FLUSH
  #define Z_FINISH              US_MZ_FINISH
  #define Z_BLOCK               US_MZ_BLOCK
  #define Z_OK                  US_MZ_OK
  #define Z_STREAM_END          US_MZ_STREAM_END
  #define Z_NEED_DICT           US_MZ_NEED_DICT
  #define Z_ERRNO               US_MZ_ERRNO
  #define Z_STREAM_ERROR        US_MZ_STREAM_ERROR
  #define Z_DATA_ERROR          US_MZ_DATA_ERROR
  #define Z_MEM_ERROR           US_MZ_MEM_ERROR
  #define Z_BUF_ERROR           US_MZ_BUF_ERROR
  #define Z_VERSION_ERROR       US_MZ_VERSION_ERROR
  #define Z_PARAM_ERROR         US_MZ_PARAM_ERROR
  #define Z_NO_COMPRESSION      US_MZ_NO_COMPRESSION
  #define Z_BEST_SPEED          US_MZ_BEST_SPEED
  #define Z_BEST_COMPRESSION    US_MZ_BEST_COMPRESSION
  #define Z_DEFAULT_COMPRESSION US_MZ_DEFAULT_COMPRESSION
  #define Z_DEFAULT_STRATEGY    US_MZ_DEFAULT_STRATEGY
  #define Z_FILTERED            US_MZ_FILTERED
  #define Z_HUFFMAN_ONLY        US_MZ_HUFFMAN_ONLY
  #define Z_RLE                 US_MZ_RLE
  #define Z_FIXED               US_MZ_FIXED
  #define Z_DEFLATED            US_MZ_DEFLATED
  #define Z_DEFAULT_WINDOW_BITS US_MZ_DEFAULT_WINDOW_BITS
  #define alloc_func            us_mz_alloc_func
  #define free_func             us_mz_free_func
  #define internal_state        us_mz_internal_state
  #define z_stream              us_mz_stream
  #define deflateInit           us_mz_deflateInit
  #define deflateInit2          us_mz_deflateInit2
  #define deflateReset          us_mz_deflateReset
  #define deflate               us_mz_deflate
  #define deflateEnd            us_mz_deflateEnd
  #define deflateBound          us_mz_deflateBound
  #define compress              us_mz_compress
  #define compress2             us_mz_compress2
  #define compressBound         us_mz_compressBound
  #define inflateInit           us_mz_inflateInit
  #define inflateInit2          us_mz_inflateInit2
  #define inflate               us_mz_inflate
  #define inflateEnd            us_mz_inflateEnd
  #define uncompress            us_mz_uncompress
  #define crc32                 us_mz_crc32
  #define adler32               us_mz_adler32
  #define MAX_WBITS             15
  #define MAX_MEM_LEVEL         9
  #define zError                us_mz_error
  #define ZLIB_VERSION          US_MZ_VERSION
  #define ZLIB_VERNUM           US_MZ_VERNUM
  #define ZLIB_VER_MAJOR        US_MZ_VER_MAJOR
  #define ZLIB_VER_MINOR        US_MZ_VER_MINOR
  #define ZLIB_VER_REVISION     US_MZ_VER_REVISION
  #define ZLIB_VER_SUBREVISION  US_MZ_VER_SUBREVISION
  #define zlibVersion           us_mz_version
  #define zlib_version          us_mz_version()
#endif // #ifndef US_MZ_NO_ZLIB_COMPATIBLE_NAMES

#endif // US_MZ_NO_ZLIB_APIS

// ------------------- Types and macros

typedef unsigned char us_mz_uint8;
typedef signed short us_mz_int16;
typedef unsigned short us_mz_uint16;
typedef unsigned int us_mz_uint32;
typedef unsigned int us_mz_uint;
typedef long long us_mz_int64;
typedef unsigned long long us_mz_uint64;
typedef int us_mz_bool;

#define US_MZ_FALSE (0)
#define US_MZ_TRUE (1)

// An attempt to work around MSVC's spammy "warning C4127: conditional expression is constant" message.
#ifdef _MSC_VER
   #define US_MZ_MACRO_END while (0, 0)
#else
   #define US_MZ_MACRO_END while (0)
#endif

// ------------------- ZIP archive reading/writing

#ifndef US_MZ_NO_ARCHIVE_APIS

enum
{
  US_MZ_ZIP_MAX_IO_BUF_SIZE = 64*1024,
  US_MZ_ZIP_MAX_ARCHIVE_FILENAME_SIZE = 260,
  US_MZ_ZIP_MAX_ARCHIVE_FILE_COMMENT_SIZE = 256
};

typedef struct
{
  us_mz_uint32 m_file_index;
  us_mz_uint32 m_central_dir_ofs;
  us_mz_uint16 m_version_made_by;
  us_mz_uint16 m_version_needed;
  us_mz_uint16 m_bit_flag;
  us_mz_uint16 m_method;
#ifndef US_MZ_NO_TIME
  time_t m_time;
#endif
  us_mz_uint32 m_crc32;
  us_mz_uint64 m_comp_size;
  us_mz_uint64 m_uncomp_size;
  us_mz_uint16 m_internal_attr;
  us_mz_uint32 m_external_attr;
  us_mz_uint64 m_local_header_ofs;
  us_mz_uint32 m_comment_size;
  char m_filename[US_MZ_ZIP_MAX_ARCHIVE_FILENAME_SIZE];
  char m_comment[US_MZ_ZIP_MAX_ARCHIVE_FILE_COMMENT_SIZE];
} us_mz_zip_archive_file_stat;

typedef size_t (*us_mz_file_read_func)(void *pOpaque, us_mz_uint64 file_ofs, void *pBuf, size_t n);
typedef size_t (*us_mz_file_write_func)(void *pOpaque, us_mz_uint64 file_ofs, const void *pBuf, size_t n);

struct us_mz_zip_internal_state_tag;
typedef struct us_mz_zip_internal_state_tag us_mz_zip_internal_state;

typedef enum
{
  US_MZ_ZIP_MODE_INVALID = 0,
  US_MZ_ZIP_MODE_READING = 1,
  US_MZ_ZIP_MODE_WRITING = 2,
  US_MZ_ZIP_MODE_WRITING_HAS_BEEN_FINALIZED = 3
} us_mz_zip_mode;

typedef struct us_mz_zip_archive_tag
{
  us_mz_uint64 m_archive_size;
  us_mz_uint64 m_archive_file_ofs;
  us_mz_uint64 m_central_directory_file_ofs;
  us_mz_uint m_total_files;
  us_mz_zip_mode m_zip_mode;

  us_mz_uint m_file_offset_alignment;

  us_mz_alloc_func m_pAlloc;
  us_mz_free_func m_pFree;
  us_mz_realloc_func m_pRealloc;
  void *m_pAlloc_opaque;

  us_mz_file_read_func m_pRead;
  us_mz_file_write_func m_pWrite;
  void *m_pIO_opaque;

  us_mz_zip_internal_state *m_pState;

} us_mz_zip_archive;

typedef enum
{
  US_MZ_ZIP_FLAG_CASE_SENSITIVE                = 0x0100,
  US_MZ_ZIP_FLAG_IGNORE_PATH                   = 0x0200,
  US_MZ_ZIP_FLAG_COMPRESSED_DATA               = 0x0400,
  US_MZ_ZIP_FLAG_DO_NOT_SORT_CENTRAL_DIRECTORY = 0x0800
} us_mz_zip_flags;

// ZIP archive reading

// Inits a ZIP archive reader.
// These functions read and validate the archive's central directory.
us_mz_bool us_mz_zip_reader_init(us_mz_zip_archive *pZip, us_mz_uint64 size, us_mz_uint32 flags);
us_mz_bool us_mz_zip_reader_init_mem(us_mz_zip_archive *pZip, const void *pMem, size_t size, us_mz_uint32 flags);

#ifndef US_MZ_NO_STDIO
us_mz_bool us_mz_zip_reader_init_file(us_mz_zip_archive *pZip, const char *pFilename, us_mz_uint32 flags);
#endif

// Returns the total number of files in the archive.
us_mz_uint us_mz_zip_reader_get_num_files(us_mz_zip_archive *pZip);

// Returns detailed information about an archive file entry.
us_mz_bool us_mz_zip_reader_file_stat(us_mz_zip_archive *pZip, us_mz_uint file_index, us_mz_zip_archive_file_stat *pStat);

// Determines if an archive file entry is a directory entry.
us_mz_bool us_mz_zip_reader_is_file_a_directory(us_mz_zip_archive *pZip, us_mz_uint file_index);
us_mz_bool us_mz_zip_reader_is_file_encrypted(us_mz_zip_archive *pZip, us_mz_uint file_index);

// Retrieves the filename of an archive file entry.
// Returns the number of bytes written to pFilename, or if filename_buf_size is 0 this function returns the number of bytes needed to fully store the filename.
us_mz_uint us_mz_zip_reader_get_filename(us_mz_zip_archive *pZip, us_mz_uint file_index, char *pFilename, us_mz_uint filename_buf_size);

// Attempts to locates a file in the archive's central directory.
// Valid flags: US_MZ_ZIP_FLAG_CASE_SENSITIVE, US_MZ_ZIP_FLAG_IGNORE_PATH
// Returns -1 if the file cannot be found.
int us_mz_zip_reader_locate_file(us_mz_zip_archive *pZip, const char *pName, const char *pComment, us_mz_uint flags);

// Extracts a archive file to a memory buffer using no memory allocation.
us_mz_bool us_mz_zip_reader_extract_to_mem_no_alloc(us_mz_zip_archive *pZip, us_mz_uint file_index, void *pBuf, size_t buf_size, us_mz_uint flags, void *pUser_read_buf, size_t user_read_buf_size);
us_mz_bool us_mz_zip_reader_extract_file_to_mem_no_alloc(us_mz_zip_archive *pZip, const char *pFilename, void *pBuf, size_t buf_size, us_mz_uint flags, void *pUser_read_buf, size_t user_read_buf_size);

// Extracts a archive file to a memory buffer.
us_mz_bool us_mz_zip_reader_extract_to_mem(us_mz_zip_archive *pZip, us_mz_uint file_index, void *pBuf, size_t buf_size, us_mz_uint flags);
us_mz_bool us_mz_zip_reader_extract_file_to_mem(us_mz_zip_archive *pZip, const char *pFilename, void *pBuf, size_t buf_size, us_mz_uint flags);

// Extracts a archive file to a dynamically allocated heap buffer.
void *us_mz_zip_reader_extract_to_heap(us_mz_zip_archive *pZip, us_mz_uint file_index, size_t *pSize, us_mz_uint flags);
void *us_mz_zip_reader_extract_file_to_heap(us_mz_zip_archive *pZip, const char *pFilename, size_t *pSize, us_mz_uint flags);

// Extracts a archive file using a callback function to output the file's data.
us_mz_bool us_mz_zip_reader_extract_to_callback(us_mz_zip_archive *pZip, us_mz_uint file_index, us_mz_file_write_func pCallback, void *pOpaque, us_mz_uint flags);
us_mz_bool us_mz_zip_reader_extract_file_to_callback(us_mz_zip_archive *pZip, const char *pFilename, us_mz_file_write_func pCallback, void *pOpaque, us_mz_uint flags);

#ifndef US_MZ_NO_STDIO
// Extracts a archive file to a disk file and sets its last accessed and modified times.
// This function only extracts files, not archive directory records.
us_mz_bool us_mz_zip_reader_extract_to_file(us_mz_zip_archive *pZip, us_mz_uint file_index, const char *pDst_filename, us_mz_uint flags);
us_mz_bool us_mz_zip_reader_extract_file_to_file(us_mz_zip_archive *pZip, const char *pArchive_filename, const char *pDst_filename, us_mz_uint flags);
#endif

// Ends archive reading, freeing all allocations, and closing the input archive file if us_mz_zip_reader_init_file() was used.
us_mz_bool us_mz_zip_reader_end(us_mz_zip_archive *pZip);

// ZIP archive writing

#ifndef US_MZ_NO_ARCHIVE_WRITING_APIS

// Inits a ZIP archive writer.
us_mz_bool us_mz_zip_writer_init(us_mz_zip_archive *pZip, us_mz_uint64 existing_size);
us_mz_bool us_mz_zip_writer_init_heap(us_mz_zip_archive *pZip, size_t size_to_reserve_at_beginning, size_t initial_allocation_size);

#ifndef US_MZ_NO_STDIO
us_mz_bool us_mz_zip_writer_init_file(us_mz_zip_archive *pZip, const char *pFilename, us_mz_uint64 size_to_reserve_at_beginning);
#endif

// Converts a ZIP archive reader object into a writer object, to allow efficient in-place file appends to occur on an existing archive.
// For archives opened using us_mz_zip_reader_init_file, pFilename must be the archive's filename so it can be reopened for writing. If the file can't be reopened, us_mz_zip_reader_end() will be called.
// For archives opened using us_mz_zip_reader_init_mem, the memory block must be growable using the realloc callback (which defaults to realloc unless you've overridden it).
// Finally, for archives opened using us_mz_zip_reader_init, the us_mz_zip_archive's user provided m_pWrite function cannot be NULL.
// Note: In-place archive modification is not recommended unless you know what you're doing, because if execution stops or something goes wrong before
// the archive is finalized the file's central directory will be hosed.
us_mz_bool us_mz_zip_writer_init_from_reader(us_mz_zip_archive *pZip, const char *pFilename);

// Adds the contents of a memory buffer to an archive. These functions record the current local time into the archive.
// To add a directory entry, call this method with an archive name ending in a forwardslash with empty buffer.
// level_and_flags - compression level (0-10, see US_MZ_BEST_SPEED, US_MZ_BEST_COMPRESSION, etc.) logically OR'd with zero or more us_mz_zip_flags, or just set to US_MZ_DEFAULT_COMPRESSION.
us_mz_bool us_mz_zip_writer_add_mem(us_mz_zip_archive *pZip, const char *pArchive_name, const void *pBuf, size_t buf_size, us_mz_uint level_and_flags);
us_mz_bool us_mz_zip_writer_add_mem_ex(us_mz_zip_archive *pZip, const char *pArchive_name, const void *pBuf, size_t buf_size, const void *pComment, us_mz_uint16 comment_size, us_mz_uint level_and_flags, us_mz_uint64 uncomp_size, us_mz_uint32 uncomp_crc32);

#ifndef US_MZ_NO_STDIO
// Adds the contents of a disk file to an archive. This function also records the disk file's modified time into the archive.
// level_and_flags - compression level (0-10, see US_MZ_BEST_SPEED, US_MZ_BEST_COMPRESSION, etc.) logically OR'd with zero or more us_mz_zip_flags, or just set to US_MZ_DEFAULT_COMPRESSION.
us_mz_bool us_mz_zip_writer_add_file(us_mz_zip_archive *pZip, const char *pArchive_name, const char *pSrc_filename, const void *pComment, us_mz_uint16 comment_size, us_mz_uint level_and_flags);
#endif

// Adds a file to an archive by fully cloning the data from another archive.
// This function fully clones the source file's compressed data (no recompression), along with its full filename, extra data, and comment fields.
us_mz_bool us_mz_zip_writer_add_from_zip_reader(us_mz_zip_archive *pZip, us_mz_zip_archive *pSource_zip, us_mz_uint file_index);

// Finalizes the archive by writing the central directory records followed by the end of central directory record.
// After an archive is finalized, the only valid call on the us_mz_zip_archive struct is us_mz_zip_writer_end().
// An archive must be manually finalized by calling this function for it to be valid.
us_mz_bool us_mz_zip_writer_finalize_archive(us_mz_zip_archive *pZip);
us_mz_bool us_mz_zip_writer_finalize_heap_archive(us_mz_zip_archive *pZip, void **pBuf, size_t *pSize);

// Ends archive writing, freeing all allocations, and closing the output file if us_mz_zip_writer_init_file() was used.
// Note for the archive to be valid, it must have been finalized before ending.
us_mz_bool us_mz_zip_writer_end(us_mz_zip_archive *pZip);

// Misc. high-level helper functions:

// us_mz_zip_add_mem_to_archive_file_in_place() efficiently (but not atomically) appends a memory blob to a ZIP archive.
// level_and_flags - compression level (0-10, see US_MZ_BEST_SPEED, US_MZ_BEST_COMPRESSION, etc.) logically OR'd with zero or more us_mz_zip_flags, or just set to US_MZ_DEFAULT_COMPRESSION.
us_mz_bool us_mz_zip_add_mem_to_archive_file_in_place(const char *pZip_filename, const char *pArchive_name, const void *pBuf, size_t buf_size, const void *pComment, us_mz_uint16 comment_size, us_mz_uint level_and_flags);

// Reads a single file from an archive into a heap block.
// Returns NULL on failure.
void *us_mz_zip_extract_archive_file_to_heap(const char *pZip_filename, const char *pArchive_name, size_t *pSize, us_mz_uint zip_flags);

#endif // #ifndef US_MZ_NO_ARCHIVE_WRITING_APIS

#endif // #ifndef US_MZ_NO_ARCHIVE_APIS

// ------------------- Low-level Decompression API Definitions

// Decompression flags used by us_tinfl_decompress().
// US_TINFL_FLAG_PARSE_ZLIB_HEADER: If set, the input has a valid zlib header and ends with an adler32 checksum (it's a valid zlib stream). Otherwise, the input is a raw deflate stream.
// US_TINFL_FLAG_HAS_MORE_INPUT: If set, there are more input bytes available beyond the end of the supplied input buffer. If clear, the input buffer contains all remaining input.
// US_TINFL_FLAG_USING_NON_WRAPPING_OUTPUT_BUF: If set, the output buffer is large enough to hold the entire decompressed stream. If clear, the output buffer is at least the size of the dictionary (typically 32KB).
// US_TINFL_FLAG_COMPUTE_ADLER32: Force adler-32 checksum computation of the decompressed bytes.
enum
{
  US_TINFL_FLAG_PARSE_ZLIB_HEADER = 1,
  US_TINFL_FLAG_HAS_MORE_INPUT = 2,
  US_TINFL_FLAG_USING_NON_WRAPPING_OUTPUT_BUF = 4,
  US_TINFL_FLAG_COMPUTE_ADLER32 = 8
};

// High level decompression functions:
// us_tinfl_decompress_mem_to_heap() decompresses a block in memory to a heap block allocated via malloc().
// On entry:
//  pSrc_buf, src_buf_len: Pointer and size of the Deflate or zlib source data to decompress.
// On return:
//  Function returns a pointer to the decompressed data, or NULL on failure.
//  *pOut_len will be set to the decompressed data's size, which could be larger than src_buf_len on uncompressible data.
//  The caller must call us_mz_free() on the returned block when it's no longer needed.
void *us_tinfl_decompress_mem_to_heap(const void *pSrc_buf, size_t src_buf_len, size_t *pOut_len, int flags);

// us_tinfl_decompress_mem_to_mem() decompresses a block in memory to another block in memory.
// Returns US_TINFL_DECOMPRESS_MEM_TO_MEM_FAILED on failure, or the number of bytes written on success.
#define US_TINFL_DECOMPRESS_MEM_TO_MEM_FAILED ((size_t)(-1))
size_t us_tinfl_decompress_mem_to_mem(void *pOut_buf, size_t out_buf_len, const void *pSrc_buf, size_t src_buf_len, int flags);

// us_tinfl_decompress_mem_to_callback() decompresses a block in memory to an internal 32KB buffer, and a user provided callback function will be called to flush the buffer.
// Returns 1 on success or 0 on failure.
typedef int (*us_tinfl_put_buf_func_ptr)(const void* pBuf, int len, void *pUser);
int us_tinfl_decompress_mem_to_callback(const void *pIn_buf, size_t *pIn_buf_size, us_tinfl_put_buf_func_ptr pPut_buf_func, void *pPut_buf_user, int flags);

struct us_tinfl_decompressor_tag; typedef struct us_tinfl_decompressor_tag us_tinfl_decompressor;

// Max size of LZ dictionary.
#define US_TINFL_LZ_DICT_SIZE 32768

// Return status.
typedef enum
{
  US_TINFL_STATUS_BAD_PARAM = -3,
  US_TINFL_STATUS_ADLER32_MISMATCH = -2,
  US_TINFL_STATUS_FAILED = -1,
  US_TINFL_STATUS_DONE = 0,
  US_TINFL_STATUS_NEEDS_MORE_INPUT = 1,
  US_TINFL_STATUS_HAS_MORE_OUTPUT = 2
} us_tinfl_status;

// Initializes the decompressor to its initial state.
#define us_tinfl_init(r) do { (r)->m_state = 0; } US_MZ_MACRO_END
#define us_tinfl_get_adler32(r) (r)->m_check_adler32

// Main low-level decompressor coroutine function. This is the only function actually needed for decompression. All the other functions are just high-level helpers for improved usability.
// This is a universal API, i.e. it can be used as a building block to build any desired higher level decompression API. In the limit case, it can be called once per every byte input or output.
us_tinfl_status us_tinfl_decompress(us_tinfl_decompressor *r, const us_mz_uint8 *pIn_buf_next, size_t *pIn_buf_size, us_mz_uint8 *pOut_buf_start, us_mz_uint8 *pOut_buf_next, size_t *pOut_buf_size, const us_mz_uint32 decomp_flags);

// Internal/private bits follow.
enum
{
  US_TINFL_MAX_HUFF_TABLES = 3, US_TINFL_MAX_HUFF_SYMBOLS_0 = 288, US_TINFL_MAX_HUFF_SYMBOLS_1 = 32, US_TINFL_MAX_HUFF_SYMBOLS_2 = 19,
  US_TINFL_FAST_LOOKUP_BITS = 10, US_TINFL_FAST_LOOKUP_SIZE = 1 << US_TINFL_FAST_LOOKUP_BITS
};

typedef struct
{
  us_mz_uint8 m_code_size[US_TINFL_MAX_HUFF_SYMBOLS_0];
  us_mz_int16 m_look_up[US_TINFL_FAST_LOOKUP_SIZE], m_tree[US_TINFL_MAX_HUFF_SYMBOLS_0 * 2];
} us_tinfl_huff_table;

#if US_MZ_HAS_64BIT_REGISTERS
  #define US_TINFL_USE_64BIT_BITBUF 1
#endif

#if US_TINFL_USE_64BIT_BITBUF
  typedef us_mz_uint64 us_tinfl_bit_buf_t;
  #define US_TINFL_BITBUF_SIZE (64)
#else
  typedef us_mz_uint32 us_tinfl_bit_buf_t;
  #define US_TINFL_BITBUF_SIZE (32)
#endif

struct us_tinfl_decompressor_tag
{
  us_mz_uint32 m_state, m_num_bits, m_zhdr0, m_zhdr1, m_z_adler32, m_final, m_type, m_check_adler32, m_dist, m_counter, m_num_extra, m_table_sizes[US_TINFL_MAX_HUFF_TABLES];
  us_tinfl_bit_buf_t m_bit_buf;
  size_t m_dist_from_out_buf_start;
  us_tinfl_huff_table m_tables[US_TINFL_MAX_HUFF_TABLES];
  us_mz_uint8 m_raw_header[4], m_len_codes[US_TINFL_MAX_HUFF_SYMBOLS_0 + US_TINFL_MAX_HUFF_SYMBOLS_1 + 137];
};

// ------------------- Low-level Compression API Definitions

// Set US_TDEFL_LESS_MEMORY to 1 to use less memory (compression will be slightly slower, and raw/dynamic blocks will be output more frequently).
#define US_TDEFL_LESS_MEMORY 0

// us_tdefl_init() compression flags logically OR'd together (low 12 bits contain the max. number of probes per dictionary search):
// US_TDEFL_DEFAULT_MAX_PROBES: The compressor defaults to 128 dictionary probes per dictionary search. 0=Huffman only, 1=Huffman+LZ (fastest/crap compression), 4095=Huffman+LZ (slowest/best compression).
enum
{
  US_TDEFL_HUFFMAN_ONLY = 0, US_TDEFL_DEFAULT_MAX_PROBES = 128, US_TDEFL_MAX_PROBES_MASK = 0xFFF
};

// US_TDEFL_WRITE_ZLIB_HEADER: If set, the compressor outputs a zlib header before the deflate data, and the Adler-32 of the source data at the end. Otherwise, you'll get raw deflate data.
// US_TDEFL_COMPUTE_ADLER32: Always compute the adler-32 of the input data (even when not writing zlib headers).
// US_TDEFL_GREEDY_PARSING_FLAG: Set to use faster greedy parsing, instead of more efficient lazy parsing.
// US_TDEFL_NONDETERMINISTIC_PARSING_FLAG: Enable to decrease the compressor's initialization time to the minimum, but the output may vary from run to run given the same input (depending on the contents of memory).
// US_TDEFL_RLE_MATCHES: Only look for RLE matches (matches with a distance of 1)
// US_TDEFL_FILTER_MATCHES: Discards matches <= 5 chars if enabled.
// US_TDEFL_FORCE_ALL_STATIC_BLOCKS: Disable usage of optimized Huffman tables.
// US_TDEFL_FORCE_ALL_RAW_BLOCKS: Only use raw (uncompressed) deflate blocks.
// The low 12 bits are reserved to control the max # of hash probes per dictionary lookup (see US_TDEFL_MAX_PROBES_MASK).
enum
{
  US_TDEFL_WRITE_ZLIB_HEADER             = 0x01000,
  US_TDEFL_COMPUTE_ADLER32               = 0x02000,
  US_TDEFL_GREEDY_PARSING_FLAG           = 0x04000,
  US_TDEFL_NONDETERMINISTIC_PARSING_FLAG = 0x08000,
  US_TDEFL_RLE_MATCHES                   = 0x10000,
  US_TDEFL_FILTER_MATCHES                = 0x20000,
  US_TDEFL_FORCE_ALL_STATIC_BLOCKS       = 0x40000,
  US_TDEFL_FORCE_ALL_RAW_BLOCKS          = 0x80000
};

// High level compression functions:
// us_tdefl_compress_mem_to_heap() compresses a block in memory to a heap block allocated via malloc().
// On entry:
//  pSrc_buf, src_buf_len: Pointer and size of source block to compress.
//  flags: The max match finder probes (default is 128) logically OR'd against the above flags. Higher probes are slower but improve compression.
// On return:
//  Function returns a pointer to the compressed data, or NULL on failure.
//  *pOut_len will be set to the compressed data's size, which could be larger than src_buf_len on uncompressible data.
//  The caller must free() the returned block when it's no longer needed.
void *us_tdefl_compress_mem_to_heap(const void *pSrc_buf, size_t src_buf_len, size_t *pOut_len, int flags);

// us_tdefl_compress_mem_to_mem() compresses a block in memory to another block in memory.
// Returns 0 on failure.
size_t us_tdefl_compress_mem_to_mem(void *pOut_buf, size_t out_buf_len, const void *pSrc_buf, size_t src_buf_len, int flags);

// Compresses an image to a compressed PNG file in memory.
// On entry:
//  pImage, w, h, and num_chans describe the image to compress. num_chans may be 1, 2, 3, or 4.
//  The image pitch in bytes per scanline will be w*num_chans. The leftmost pixel on the top scanline is stored first in memory.
//  level may range from [0,10], use US_MZ_NO_COMPRESSION, US_MZ_BEST_SPEED, US_MZ_BEST_COMPRESSION, etc. or a decent default is US_MZ_DEFAULT_LEVEL
//  If flip is true, the image will be flipped on the Y axis (useful for OpenGL apps).
// On return:
//  Function returns a pointer to the compressed data, or NULL on failure.
//  *pLen_out will be set to the size of the PNG image file.
//  The caller must us_mz_free() the returned heap block (which will typically be larger than *pLen_out) when it's no longer needed.
void *us_tdefl_write_image_to_png_file_in_memory_ex(const void *pImage, int w, int h, int num_chans, size_t *pLen_out, us_mz_uint level, us_mz_bool flip);
void *us_tdefl_write_image_to_png_file_in_memory(const void *pImage, int w, int h, int num_chans, size_t *pLen_out);

// Output stream interface. The compressor uses this interface to write compressed data. It'll typically be called US_TDEFL_OUT_BUF_SIZE at a time.
typedef us_mz_bool (*us_tdefl_put_buf_func_ptr)(const void* pBuf, int len, void *pUser);

// us_tdefl_compress_mem_to_output() compresses a block to an output stream. The above helpers use this function internally.
us_mz_bool us_tdefl_compress_mem_to_output(const void *pBuf, size_t buf_len, us_tdefl_put_buf_func_ptr pPut_buf_func, void *pPut_buf_user, int flags);

enum { US_TDEFL_MAX_HUFF_TABLES = 3, US_TDEFL_MAX_HUFF_SYMBOLS_0 = 288, US_TDEFL_MAX_HUFF_SYMBOLS_1 = 32, US_TDEFL_MAX_HUFF_SYMBOLS_2 = 19, US_TDEFL_LZ_DICT_SIZE = 32768, US_TDEFL_LZ_DICT_SIZE_MASK = US_TDEFL_LZ_DICT_SIZE - 1, US_TDEFL_MIN_MATCH_LEN = 3, US_TDEFL_MAX_MATCH_LEN = 258 };

// US_TDEFL_OUT_BUF_SIZE MUST be large enough to hold a single entire compressed output block (using static/fixed Huffman codes).
#if US_TDEFL_LESS_MEMORY
enum { US_TDEFL_LZ_CODE_BUF_SIZE = 24 * 1024, US_TDEFL_OUT_BUF_SIZE = (US_TDEFL_LZ_CODE_BUF_SIZE * 13 ) / 10, US_TDEFL_MAX_HUFF_SYMBOLS = 288, US_TDEFL_LZ_HASH_BITS = 12, US_TDEFL_LEVEL1_HASH_SIZE_MASK = 4095, US_TDEFL_LZ_HASH_SHIFT = (US_TDEFL_LZ_HASH_BITS + 2) / 3, US_TDEFL_LZ_HASH_SIZE = 1 << US_TDEFL_LZ_HASH_BITS };
#else
enum { US_TDEFL_LZ_CODE_BUF_SIZE = 64 * 1024, US_TDEFL_OUT_BUF_SIZE = (US_TDEFL_LZ_CODE_BUF_SIZE * 13 ) / 10, US_TDEFL_MAX_HUFF_SYMBOLS = 288, US_TDEFL_LZ_HASH_BITS = 15, US_TDEFL_LEVEL1_HASH_SIZE_MASK = 4095, US_TDEFL_LZ_HASH_SHIFT = (US_TDEFL_LZ_HASH_BITS + 2) / 3, US_TDEFL_LZ_HASH_SIZE = 1 << US_TDEFL_LZ_HASH_BITS };
#endif

// The low-level tdefl functions below may be used directly if the above helper functions aren't flexible enough. The low-level functions don't make any heap allocations, unlike the above helper functions.
typedef enum
{
  US_TDEFL_STATUS_BAD_PARAM = -2,
  US_TDEFL_STATUS_PUT_BUF_FAILED = -1,
  US_TDEFL_STATUS_OKAY = 0,
  US_TDEFL_STATUS_DONE = 1,
} us_tdefl_status;

// Must map to US_MZ_NO_FLUSH, US_MZ_SYNC_FLUSH, etc. enums
typedef enum
{
  US_TDEFL_NO_FLUSH = 0,
  US_TDEFL_SYNC_FLUSH = 2,
  US_TDEFL_FULL_FLUSH = 3,
  US_TDEFL_FINISH = 4
} us_tdefl_flush;

// tdefl's compression state structure.
typedef struct
{
  us_tdefl_put_buf_func_ptr m_pPut_buf_func;
  void *m_pPut_buf_user;
  us_mz_uint m_flags, m_max_probes[2];
  int m_greedy_parsing;
  us_mz_uint m_adler32, m_lookahead_pos, m_lookahead_size, m_dict_size;
  us_mz_uint8 *m_pLZ_code_buf, *m_pLZ_flags, *m_pOutput_buf, *m_pOutput_buf_end;
  us_mz_uint m_num_flags_left, m_total_lz_bytes, m_lz_code_buf_dict_pos, m_bits_in, m_bit_buffer;
  us_mz_uint m_saved_match_dist, m_saved_match_len, m_saved_lit, m_output_flush_ofs, m_output_flush_remaining, m_finished, m_block_index, m_wants_to_finish;
  us_tdefl_status m_prev_return_status;
  const void *m_pIn_buf;
  void *m_pOut_buf;
  size_t *m_pIn_buf_size, *m_pOut_buf_size;
  us_tdefl_flush m_flush;
  const us_mz_uint8 *m_pSrc;
  size_t m_src_buf_left, m_out_buf_ofs;
  us_mz_uint8 m_dict[US_TDEFL_LZ_DICT_SIZE + US_TDEFL_MAX_MATCH_LEN - 1];
  us_mz_uint16 m_huff_count[US_TDEFL_MAX_HUFF_TABLES][US_TDEFL_MAX_HUFF_SYMBOLS];
  us_mz_uint16 m_huff_codes[US_TDEFL_MAX_HUFF_TABLES][US_TDEFL_MAX_HUFF_SYMBOLS];
  us_mz_uint8 m_huff_code_sizes[US_TDEFL_MAX_HUFF_TABLES][US_TDEFL_MAX_HUFF_SYMBOLS];
  us_mz_uint8 m_lz_code_buf[US_TDEFL_LZ_CODE_BUF_SIZE];
  us_mz_uint16 m_next[US_TDEFL_LZ_DICT_SIZE];
  us_mz_uint16 m_hash[US_TDEFL_LZ_HASH_SIZE];
  us_mz_uint8 m_output_buf[US_TDEFL_OUT_BUF_SIZE];
} us_tdefl_compressor;

// Initializes the compressor.
// There is no corresponding deinit() function because the tdefl API's do not dynamically allocate memory.
// pBut_buf_func: If NULL, output data will be supplied to the specified callback. In this case, the user should call the us_tdefl_compress_buffer() API for compression.
// If pBut_buf_func is NULL the user should always call the us_tdefl_compress() API.
// flags: See the above enums (US_TDEFL_HUFFMAN_ONLY, US_TDEFL_WRITE_ZLIB_HEADER, etc.)
us_tdefl_status us_tdefl_init(us_tdefl_compressor *d, us_tdefl_put_buf_func_ptr pPut_buf_func, void *pPut_buf_user, int flags);

// Compresses a block of data, consuming as much of the specified input buffer as possible, and writing as much compressed data to the specified output buffer as possible.
us_tdefl_status us_tdefl_compress(us_tdefl_compressor *d, const void *pIn_buf, size_t *pIn_buf_size, void *pOut_buf, size_t *pOut_buf_size, us_tdefl_flush flush);

// us_tdefl_compress_buffer() is only usable when the us_tdefl_init() is called with a non-NULL us_tdefl_put_buf_func_ptr.
// us_tdefl_compress_buffer() always consumes the entire input buffer.
us_tdefl_status us_tdefl_compress_buffer(us_tdefl_compressor *d, const void *pIn_buf, size_t in_buf_size, us_tdefl_flush flush);

us_tdefl_status us_tdefl_get_prev_return_status(us_tdefl_compressor *d);
us_mz_uint32 us_tdefl_get_adler32(us_tdefl_compressor *d);

// Can't use us_tdefl_create_comp_flags_from_zip_params if US_MZ_NO_ZLIB_APIS isn't defined, because it uses some of its macros.
#ifndef US_MZ_NO_ZLIB_APIS
// Create us_tdefl_compress() flags given zlib-style compression parameters.
// level may range from [0,10] (where 10 is absolute max compression, but may be much slower on some files)
// window_bits may be -15 (raw deflate) or 15 (zlib)
// strategy may be either US_MZ_DEFAULT_STRATEGY, US_MZ_FILTERED, US_MZ_HUFFMAN_ONLY, US_MZ_RLE, or US_MZ_FIXED
us_mz_uint us_tdefl_create_comp_flags_from_zip_params(int level, int window_bits, int strategy);
#endif // #ifndef US_MZ_NO_ZLIB_APIS

#ifdef __cplusplus
}
#endif

#endif // US_MZ_HEADER_INCLUDED

// ------------------- End of Header: Implementation follows. (If you only want the header, define US_MZ_HEADER_FILE_ONLY.)

#ifndef US_MZ_HEADER_FILE_ONLY

typedef unsigned char us_mz_validate_uint16[sizeof(us_mz_uint16)==2 ? 1 : -1];
typedef unsigned char us_mz_validate_uint32[sizeof(us_mz_uint32)==4 ? 1 : -1];
typedef unsigned char us_mz_validate_uint64[sizeof(us_mz_uint64)==8 ? 1 : -1];

#include <string.h>
#include <assert.h>

#define US_MZ_ASSERT(x) assert(x)

#ifdef US_MZ_NO_MALLOC
  #define US_MZ_MALLOC(x) NULL
  #define US_MZ_FREE(x) (void)x, ((void)0)
  #define US_MZ_REALLOC(p, x) NULL
#else
  #define US_MZ_MALLOC(x) malloc(x)
  #define US_MZ_FREE(x) free(x)
  #define US_MZ_REALLOC(p, x) realloc(p, x)
#endif

#define US_MZ_MAX(a,b) (((a)>(b))?(a):(b))
#define US_MZ_MIN(a,b) (((a)<(b))?(a):(b))
#define US_MZ_CLEAR_OBJ(obj) memset(&(obj), 0, sizeof(obj))

#if US_MZ_USE_UNALIGNED_LOADS_AND_STORES && US_MZ_LITTLE_ENDIAN
  #define US_MZ_READ_LE16(p) *((const us_mz_uint16 *)(p))
  #define US_MZ_READ_LE32(p) *((const us_mz_uint32 *)(p))
#else
  #define US_MZ_READ_LE16(p) ((us_mz_uint32)(((const us_mz_uint8 *)(p))[0]) | ((us_mz_uint32)(((const us_mz_uint8 *)(p))[1]) << 8U))
  #define US_MZ_READ_LE32(p) ((us_mz_uint32)(((const us_mz_uint8 *)(p))[0]) | ((us_mz_uint32)(((const us_mz_uint8 *)(p))[1]) << 8U) | ((us_mz_uint32)(((const us_mz_uint8 *)(p))[2]) << 16U) | ((us_mz_uint32)(((const us_mz_uint8 *)(p))[3]) << 24U))
#endif

#ifdef _MSC_VER
  #define US_MZ_FORCEINLINE __forceinline
#elif defined(__GNUC__)
  #define US_MZ_FORCEINLINE inline __attribute__((__always_inline__))
#else
  #define US_MZ_FORCEINLINE inline
#endif

#ifdef __cplusplus
  extern "C" {
#endif

// ------------------- zlib-style API's

us_mz_ulong us_mz_adler32(us_mz_ulong adler, const unsigned char *ptr, size_t buf_len)
{
  us_mz_uint32 i, s1 = (us_mz_uint32)(adler & 0xffff), s2 = (us_mz_uint32)(adler >> 16); size_t block_len = buf_len % 5552;
  if (!ptr) return US_MZ_ADLER32_INIT;
  while (buf_len) {
    for (i = 0; i + 7 < block_len; i += 8, ptr += 8) {
      s1 += ptr[0], s2 += s1; s1 += ptr[1], s2 += s1; s1 += ptr[2], s2 += s1; s1 += ptr[3], s2 += s1;
      s1 += ptr[4], s2 += s1; s1 += ptr[5], s2 += s1; s1 += ptr[6], s2 += s1; s1 += ptr[7], s2 += s1;
    }
    for ( ; i < block_len; ++i) s1 += *ptr++, s2 += s1;
    s1 %= 65521U, s2 %= 65521U; buf_len -= block_len; block_len = 5552;
  }
  return (s2 << 16) + s1;
}

// Karl Malbrain's compact CRC-32. See "A compact CCITT crc16 and crc32 C implementation that balances processor cache usage against speed"
us_mz_ulong us_mz_crc32(us_mz_ulong crc, const us_mz_uint8 *ptr, size_t buf_len)
{
  static const us_mz_uint32 s_crc32[16] = { 0, 0x1db71064, 0x3b6e20c8, 0x26d930ac, 0x76dc4190, 0x6b6b51f4, 0x4db26158, 0x5005713c,
    0xedb88320, 0xf00f9344, 0xd6d6a3e8, 0xcb61b38c, 0x9b64c2b0, 0x86d3d2d4, 0xa00ae278, 0xbdbdf21c };
  us_mz_uint32 crcu32 = (us_mz_uint32)crc;
  if (!ptr) return US_MZ_CRC32_INIT;
  crcu32 = ~crcu32; while (buf_len--) { us_mz_uint8 b = *ptr++; crcu32 = (crcu32 >> 4) ^ s_crc32[(crcu32 & 0xF) ^ (b & 0xF)]; crcu32 = (crcu32 >> 4) ^ s_crc32[(crcu32 & 0xF) ^ (b >> 4)]; }
  return ~crcu32;
}

void us_mz_free(void *p)
{
  US_MZ_FREE(p);
}

#ifndef US_MZ_NO_ZLIB_APIS

static void *def_alloc_func(void *opaque, size_t items, size_t size) { (void)opaque, (void)items, (void)size; return US_MZ_MALLOC(items * size); }
static void def_free_func(void *opaque, void *address) { (void)opaque, (void)address; US_MZ_FREE(address); }
static void *def_realloc_func(void *opaque, void *address, size_t items, size_t size) { (void)opaque, (void)address, (void)items, (void)size; return US_MZ_REALLOC(address, items * size); }

const char *us_mz_version(void)
{
  return US_MZ_VERSION;
}

int us_mz_deflateInit(us_mz_streamp pStream, int level)
{
  return us_mz_deflateInit2(pStream, level, US_MZ_DEFLATED, US_MZ_DEFAULT_WINDOW_BITS, 9, US_MZ_DEFAULT_STRATEGY);
}

int us_mz_deflateInit2(us_mz_streamp pStream, int level, int method, int window_bits, int mem_level, int strategy)
{
  us_tdefl_compressor *pComp;
  us_mz_uint comp_flags = US_TDEFL_COMPUTE_ADLER32 | us_tdefl_create_comp_flags_from_zip_params(level, window_bits, strategy);

  if (!pStream) return US_MZ_STREAM_ERROR;
  if ((method != US_MZ_DEFLATED) || ((mem_level < 1) || (mem_level > 9)) || ((window_bits != US_MZ_DEFAULT_WINDOW_BITS) && (-window_bits != US_MZ_DEFAULT_WINDOW_BITS))) return US_MZ_PARAM_ERROR;

  pStream->data_type = 0;
  pStream->adler = US_MZ_ADLER32_INIT;
  pStream->msg = NULL;
  pStream->reserved = 0;
  pStream->total_in = 0;
  pStream->total_out = 0;
  if (!pStream->zalloc) pStream->zalloc = def_alloc_func;
  if (!pStream->zfree) pStream->zfree = def_free_func;

  pComp = (us_tdefl_compressor *)pStream->zalloc(pStream->opaque, 1, sizeof(us_tdefl_compressor));
  if (!pComp)
    return US_MZ_MEM_ERROR;

  pStream->state = (struct us_mz_internal_state *)pComp;

  if (us_tdefl_init(pComp, NULL, NULL, comp_flags) != US_TDEFL_STATUS_OKAY)
  {
    us_mz_deflateEnd(pStream);
    return US_MZ_PARAM_ERROR;
  }

  return US_MZ_OK;
}

int us_mz_deflateReset(us_mz_streamp pStream)
{
  if ((!pStream) || (!pStream->state) || (!pStream->zalloc) || (!pStream->zfree)) return US_MZ_STREAM_ERROR;
  pStream->total_in = pStream->total_out = 0;
  us_tdefl_init((us_tdefl_compressor*)pStream->state, NULL, NULL, ((us_tdefl_compressor*)pStream->state)->m_flags);
  return US_MZ_OK;
}

int us_mz_deflate(us_mz_streamp pStream, int flush)
{
  size_t in_bytes, out_bytes;
  us_mz_ulong orig_total_in, orig_total_out;
  int us_mz_status = US_MZ_OK;

  if ((!pStream) || (!pStream->state) || (flush < 0) || (flush > US_MZ_FINISH) || (!pStream->next_out)) return US_MZ_STREAM_ERROR;
  if (!pStream->avail_out) return US_MZ_BUF_ERROR;

  if (flush == US_MZ_PARTIAL_FLUSH) flush = US_MZ_SYNC_FLUSH;

  if (((us_tdefl_compressor*)pStream->state)->m_prev_return_status == US_TDEFL_STATUS_DONE)
    return (flush == US_MZ_FINISH) ? US_MZ_STREAM_END : US_MZ_BUF_ERROR;

  orig_total_in = pStream->total_in; orig_total_out = pStream->total_out;
  for ( ; ; )
  {
    us_tdefl_status defl_status;
    in_bytes = pStream->avail_in; out_bytes = pStream->avail_out;

    defl_status = us_tdefl_compress((us_tdefl_compressor*)pStream->state, pStream->next_in, &in_bytes, pStream->next_out, &out_bytes, (us_tdefl_flush)flush);
    pStream->next_in += (us_mz_uint)in_bytes; pStream->avail_in -= (us_mz_uint)in_bytes;
    pStream->total_in += (us_mz_uint)in_bytes; pStream->adler = us_tdefl_get_adler32((us_tdefl_compressor*)pStream->state);

    pStream->next_out += (us_mz_uint)out_bytes; pStream->avail_out -= (us_mz_uint)out_bytes;
    pStream->total_out += (us_mz_uint)out_bytes;

    if (defl_status < 0)
    {
      us_mz_status = US_MZ_STREAM_ERROR;
      break;
    }
    else if (defl_status == US_TDEFL_STATUS_DONE)
    {
      us_mz_status = US_MZ_STREAM_END;
      break;
    }
    else if (!pStream->avail_out)
      break;
    else if ((!pStream->avail_in) && (flush != US_MZ_FINISH))
    {
      if ((flush) || (pStream->total_in != orig_total_in) || (pStream->total_out != orig_total_out))
        break;
      return US_MZ_BUF_ERROR; // Can't make forward progress without some input.
    }
  }
  return us_mz_status;
}

int us_mz_deflateEnd(us_mz_streamp pStream)
{
  if (!pStream) return US_MZ_STREAM_ERROR;
  if (pStream->state)
  {
    pStream->zfree(pStream->opaque, pStream->state);
    pStream->state = NULL;
  }
  return US_MZ_OK;
}

us_mz_ulong us_mz_deflateBound(us_mz_streamp pStream, us_mz_ulong source_len)
{
  (void)pStream;
  // This is really over conservative. (And lame, but it's actually pretty tricky to compute a true upper bound given the way tdefl's blocking works.)
  return US_MZ_MAX(128 + (source_len * 110) / 100, 128 + source_len + ((source_len / (31 * 1024)) + 1) * 5);
}

int us_mz_compress2(unsigned char *pDest, us_mz_ulong *pDest_len, const unsigned char *pSource, us_mz_ulong source_len, int level)
{
  int status;
  us_mz_stream stream;
  memset(&stream, 0, sizeof(stream));

  // In case us_mz_ulong is 64-bits (argh I hate longs).
  if ((source_len | *pDest_len) > 0xFFFFFFFFU) return US_MZ_PARAM_ERROR;

  stream.next_in = pSource;
  stream.avail_in = (us_mz_uint32)source_len;
  stream.next_out = pDest;
  stream.avail_out = (us_mz_uint32)*pDest_len;

  status = us_mz_deflateInit(&stream, level);
  if (status != US_MZ_OK) return status;

  status = us_mz_deflate(&stream, US_MZ_FINISH);
  if (status != US_MZ_STREAM_END)
  {
    us_mz_deflateEnd(&stream);
    return (status == US_MZ_OK) ? US_MZ_BUF_ERROR : status;
  }

  *pDest_len = stream.total_out;
  return us_mz_deflateEnd(&stream);
}

int us_mz_compress(unsigned char *pDest, us_mz_ulong *pDest_len, const unsigned char *pSource, us_mz_ulong source_len)
{
  return us_mz_compress2(pDest, pDest_len, pSource, source_len, US_MZ_DEFAULT_COMPRESSION);
}

us_mz_ulong us_mz_compressBound(us_mz_ulong source_len)
{
  return us_mz_deflateBound(NULL, source_len);
}

typedef struct
{
  us_tinfl_decompressor m_decomp;
  us_mz_uint m_dict_ofs, m_dict_avail, m_first_call, m_has_flushed; int m_window_bits;
  us_mz_uint8 m_dict[US_TINFL_LZ_DICT_SIZE];
  us_tinfl_status m_last_status;
} inflate_state;

int us_mz_inflateInit2(us_mz_streamp pStream, int window_bits)
{
  inflate_state *pDecomp;
  if (!pStream) return US_MZ_STREAM_ERROR;
  if ((window_bits != US_MZ_DEFAULT_WINDOW_BITS) && (-window_bits != US_MZ_DEFAULT_WINDOW_BITS)) return US_MZ_PARAM_ERROR;

  pStream->data_type = 0;
  pStream->adler = 0;
  pStream->msg = NULL;
  pStream->total_in = 0;
  pStream->total_out = 0;
  pStream->reserved = 0;
  if (!pStream->zalloc) pStream->zalloc = def_alloc_func;
  if (!pStream->zfree) pStream->zfree = def_free_func;

  pDecomp = (inflate_state*)pStream->zalloc(pStream->opaque, 1, sizeof(inflate_state));
  if (!pDecomp) return US_MZ_MEM_ERROR;

  pStream->state = (struct us_mz_internal_state *)pDecomp;

  us_tinfl_init(&pDecomp->m_decomp);
  pDecomp->m_dict_ofs = 0;
  pDecomp->m_dict_avail = 0;
  pDecomp->m_last_status = US_TINFL_STATUS_NEEDS_MORE_INPUT;
  pDecomp->m_first_call = 1;
  pDecomp->m_has_flushed = 0;
  pDecomp->m_window_bits = window_bits;

  return US_MZ_OK;
}

int us_mz_inflateInit(us_mz_streamp pStream)
{
   return us_mz_inflateInit2(pStream, US_MZ_DEFAULT_WINDOW_BITS);
}

int us_mz_inflate(us_mz_streamp pStream, int flush)
{
  inflate_state* pState;
  us_mz_uint n, first_call, decomp_flags = US_TINFL_FLAG_COMPUTE_ADLER32;
  size_t in_bytes, out_bytes, orig_avail_in;
  us_tinfl_status status;

  if ((!pStream) || (!pStream->state)) return US_MZ_STREAM_ERROR;
  if (flush == US_MZ_PARTIAL_FLUSH) flush = US_MZ_SYNC_FLUSH;
  if ((flush) && (flush != US_MZ_SYNC_FLUSH) && (flush != US_MZ_FINISH)) return US_MZ_STREAM_ERROR;

  pState = (inflate_state*)pStream->state;
  if (pState->m_window_bits > 0) decomp_flags |= US_TINFL_FLAG_PARSE_ZLIB_HEADER;
  orig_avail_in = pStream->avail_in;

  first_call = pState->m_first_call; pState->m_first_call = 0;
  if (pState->m_last_status < 0) return US_MZ_DATA_ERROR;

  if (pState->m_has_flushed && (flush != US_MZ_FINISH)) return US_MZ_STREAM_ERROR;
  pState->m_has_flushed |= (flush == US_MZ_FINISH);

  if ((flush == US_MZ_FINISH) && (first_call))
  {
    // US_MZ_FINISH on the first call implies that the input and output buffers are large enough to hold the entire compressed/decompressed file.
    decomp_flags |= US_TINFL_FLAG_USING_NON_WRAPPING_OUTPUT_BUF;
    in_bytes = pStream->avail_in; out_bytes = pStream->avail_out;
    status = us_tinfl_decompress(&pState->m_decomp, pStream->next_in, &in_bytes, pStream->next_out, pStream->next_out, &out_bytes, decomp_flags);
    pState->m_last_status = status;
    pStream->next_in += (us_mz_uint)in_bytes; pStream->avail_in -= (us_mz_uint)in_bytes; pStream->total_in += (us_mz_uint)in_bytes;
    pStream->adler = us_tinfl_get_adler32(&pState->m_decomp);
    pStream->next_out += (us_mz_uint)out_bytes; pStream->avail_out -= (us_mz_uint)out_bytes; pStream->total_out += (us_mz_uint)out_bytes;

    if (status < 0)
      return US_MZ_DATA_ERROR;
    else if (status != US_TINFL_STATUS_DONE)
    {
      pState->m_last_status = US_TINFL_STATUS_FAILED;
      return US_MZ_BUF_ERROR;
    }
    return US_MZ_STREAM_END;
  }
  // flush != US_MZ_FINISH then we must assume there's more input.
  if (flush != US_MZ_FINISH) decomp_flags |= US_TINFL_FLAG_HAS_MORE_INPUT;

  if (pState->m_dict_avail)
  {
    n = US_MZ_MIN(pState->m_dict_avail, pStream->avail_out);
    memcpy(pStream->next_out, pState->m_dict + pState->m_dict_ofs, n);
    pStream->next_out += n; pStream->avail_out -= n; pStream->total_out += n;
    pState->m_dict_avail -= n; pState->m_dict_ofs = (pState->m_dict_ofs + n) & (US_TINFL_LZ_DICT_SIZE - 1);
    return ((pState->m_last_status == US_TINFL_STATUS_DONE) && (!pState->m_dict_avail)) ? US_MZ_STREAM_END : US_MZ_OK;
  }

  for ( ; ; )
  {
    in_bytes = pStream->avail_in;
    out_bytes = US_TINFL_LZ_DICT_SIZE - pState->m_dict_ofs;

    status = us_tinfl_decompress(&pState->m_decomp, pStream->next_in, &in_bytes, pState->m_dict, pState->m_dict + pState->m_dict_ofs, &out_bytes, decomp_flags);
    pState->m_last_status = status;

    pStream->next_in += (us_mz_uint)in_bytes; pStream->avail_in -= (us_mz_uint)in_bytes;
    pStream->total_in += (us_mz_uint)in_bytes; pStream->adler = us_tinfl_get_adler32(&pState->m_decomp);

    pState->m_dict_avail = (us_mz_uint)out_bytes;

    n = US_MZ_MIN(pState->m_dict_avail, pStream->avail_out);
    memcpy(pStream->next_out, pState->m_dict + pState->m_dict_ofs, n);
    pStream->next_out += n; pStream->avail_out -= n; pStream->total_out += n;
    pState->m_dict_avail -= n; pState->m_dict_ofs = (pState->m_dict_ofs + n) & (US_TINFL_LZ_DICT_SIZE - 1);

    if (status < 0)
       return US_MZ_DATA_ERROR; // Stream is corrupted (there could be some uncompressed data left in the output dictionary - oh well).
    else if ((status == US_TINFL_STATUS_NEEDS_MORE_INPUT) && (!orig_avail_in))
      return US_MZ_BUF_ERROR; // Signal caller that we can't make forward progress without supplying more input or by setting flush to US_MZ_FINISH.
    else if (flush == US_MZ_FINISH)
    {
       // The output buffer MUST be large to hold the remaining uncompressed data when flush==US_MZ_FINISH.
       if (status == US_TINFL_STATUS_DONE)
          return pState->m_dict_avail ? US_MZ_BUF_ERROR : US_MZ_STREAM_END;
       // status here must be US_TINFL_STATUS_HAS_MORE_OUTPUT, which means there's at least 1 more byte on the way. If there's no more room left in the output buffer then something is wrong.
       else if (!pStream->avail_out)
          return US_MZ_BUF_ERROR;
    }
    else if ((status == US_TINFL_STATUS_DONE) || (!pStream->avail_in) || (!pStream->avail_out) || (pState->m_dict_avail))
      break;
  }

  return ((status == US_TINFL_STATUS_DONE) && (!pState->m_dict_avail)) ? US_MZ_STREAM_END : US_MZ_OK;
}

int us_mz_inflateEnd(us_mz_streamp pStream)
{
  if (!pStream)
    return US_MZ_STREAM_ERROR;
  if (pStream->state)
  {
    pStream->zfree(pStream->opaque, pStream->state);
    pStream->state = NULL;
  }
  return US_MZ_OK;
}

int us_mz_uncompress(unsigned char *pDest, us_mz_ulong *pDest_len, const unsigned char *pSource, us_mz_ulong source_len)
{
  us_mz_stream stream;
  int status;
  memset(&stream, 0, sizeof(stream));

  // In case us_mz_ulong is 64-bits (argh I hate longs).
  if ((source_len | *pDest_len) > 0xFFFFFFFFU) return US_MZ_PARAM_ERROR;

  stream.next_in = pSource;
  stream.avail_in = (us_mz_uint32)source_len;
  stream.next_out = pDest;
  stream.avail_out = (us_mz_uint32)*pDest_len;

  status = us_mz_inflateInit(&stream);
  if (status != US_MZ_OK)
    return status;

  status = us_mz_inflate(&stream, US_MZ_FINISH);
  if (status != US_MZ_STREAM_END)
  {
    us_mz_inflateEnd(&stream);
    return ((status == US_MZ_BUF_ERROR) && (!stream.avail_in)) ? US_MZ_DATA_ERROR : status;
  }
  *pDest_len = stream.total_out;

  return us_mz_inflateEnd(&stream);
}

const char *us_mz_error(int err)
{
  static struct { int m_err; const char *m_pDesc; } s_error_descs[] =
  {
    { US_MZ_OK, "" }, { US_MZ_STREAM_END, "stream end" }, { US_MZ_NEED_DICT, "need dictionary" }, { US_MZ_ERRNO, "file error" }, { US_MZ_STREAM_ERROR, "stream error" },
    { US_MZ_DATA_ERROR, "data error" }, { US_MZ_MEM_ERROR, "out of memory" }, { US_MZ_BUF_ERROR, "buf error" }, { US_MZ_VERSION_ERROR, "version error" }, { US_MZ_PARAM_ERROR, "parameter error" }
  };
  us_mz_uint i; for (i = 0; i < sizeof(s_error_descs) / sizeof(s_error_descs[0]); ++i) if (s_error_descs[i].m_err == err) return s_error_descs[i].m_pDesc;
  return NULL;
}

#endif //US_MZ_NO_ZLIB_APIS

// ------------------- Low-level Decompression (completely independent from all compression API's)

#define US_TINFL_MEMCPY(d, s, l) memcpy(d, s, l)
#define US_TINFL_MEMSET(p, c, l) memset(p, c, l)

#define US_TINFL_CR_BEGIN switch(r->m_state) { case 0:
#define US_TINFL_CR_RETURN(state_index, result) do { status = result; r->m_state = state_index; goto common_exit; case state_index:; } US_MZ_MACRO_END
#define US_TINFL_CR_RETURN_FOREVER(state_index, result) do { for ( ; ; ) { US_TINFL_CR_RETURN(state_index, result); } } US_MZ_MACRO_END
#define US_TINFL_CR_FINISH }

// TODO: If the caller has indicated that there's no more input, and we attempt to read beyond the input buf, then something is wrong with the input because the inflator never
// reads ahead more than it needs to. Currently US_TINFL_GET_BYTE() pads the end of the stream with 0's in this scenario.
#define US_TINFL_GET_BYTE(state_index, c) do { \
  if (pIn_buf_cur >= pIn_buf_end) { \
    for ( ; ; ) { \
      if (decomp_flags & US_TINFL_FLAG_HAS_MORE_INPUT) { \
        US_TINFL_CR_RETURN(state_index, US_TINFL_STATUS_NEEDS_MORE_INPUT); \
        if (pIn_buf_cur < pIn_buf_end) { \
          c = *pIn_buf_cur++; \
          break; \
        } \
      } else { \
        c = 0; \
        break; \
      } \
    } \
  } else c = *pIn_buf_cur++; } US_MZ_MACRO_END

#define US_TINFL_NEED_BITS(state_index, n) do { us_mz_uint c; US_TINFL_GET_BYTE(state_index, c); bit_buf |= (((us_tinfl_bit_buf_t)c) << num_bits); num_bits += 8; } while (num_bits < (us_mz_uint)(n))
#define US_TINFL_SKIP_BITS(state_index, n) do { if (num_bits < (us_mz_uint)(n)) { US_TINFL_NEED_BITS(state_index, n); } bit_buf >>= (n); num_bits -= (n); } US_MZ_MACRO_END
#define US_TINFL_GET_BITS(state_index, b, n) do { if (num_bits < (us_mz_uint)(n)) { US_TINFL_NEED_BITS(state_index, n); } b = bit_buf & ((1 << (n)) - 1); bit_buf >>= (n); num_bits -= (n); } US_MZ_MACRO_END

// US_TINFL_HUFF_BITBUF_FILL() is only used rarely, when the number of bytes remaining in the input buffer falls below 2.
// It reads just enough bytes from the input stream that are needed to decode the next Huffman code (and absolutely no more). It works by trying to fully decode a
// Huffman code by using whatever bits are currently present in the bit buffer. If this fails, it reads another byte, and tries again until it succeeds or until the
// bit buffer contains >=15 bits (deflate's max. Huffman code size).
#define US_TINFL_HUFF_BITBUF_FILL(state_index, pHuff) \
  do { \
    temp = (pHuff)->m_look_up[bit_buf & (US_TINFL_FAST_LOOKUP_SIZE - 1)]; \
    if (temp >= 0) { \
      code_len = temp >> 9; \
      if ((code_len) && (num_bits >= code_len)) \
      break; \
    } else if (num_bits > US_TINFL_FAST_LOOKUP_BITS) { \
       code_len = US_TINFL_FAST_LOOKUP_BITS; \
       do { \
          temp = (pHuff)->m_tree[~temp + ((bit_buf >> code_len++) & 1)]; \
       } while ((temp < 0) && (num_bits >= (code_len + 1))); if (temp >= 0) break; \
    } US_TINFL_GET_BYTE(state_index, c); bit_buf |= (((us_tinfl_bit_buf_t)c) << num_bits); num_bits += 8; \
  } while (num_bits < 15);

// US_TINFL_HUFF_DECODE() decodes the next Huffman coded symbol. It's more complex than you would initially expect because the zlib API expects the decompressor to never read
// beyond the final byte of the deflate stream. (In other words, when this macro wants to read another byte from the input, it REALLY needs another byte in order to fully
// decode the next Huffman code.) Handling this properly is particularly important on raw deflate (non-zlib) streams, which aren't followed by a byte aligned adler-32.
// The slow path is only executed at the very end of the input buffer.
#define US_TINFL_HUFF_DECODE(state_index, sym, pHuff) do { \
  int temp; us_mz_uint code_len, c; \
  if (num_bits < 15) { \
    if ((pIn_buf_end - pIn_buf_cur) < 2) { \
       US_TINFL_HUFF_BITBUF_FILL(state_index, pHuff); \
    } else { \
       bit_buf |= (((us_tinfl_bit_buf_t)pIn_buf_cur[0]) << num_bits) | (((us_tinfl_bit_buf_t)pIn_buf_cur[1]) << (num_bits + 8)); pIn_buf_cur += 2; num_bits += 16; \
    } \
  } \
  if ((temp = (pHuff)->m_look_up[bit_buf & (US_TINFL_FAST_LOOKUP_SIZE - 1)]) >= 0) \
    code_len = temp >> 9, temp &= 511; \
  else { \
    code_len = US_TINFL_FAST_LOOKUP_BITS; do { temp = (pHuff)->m_tree[~temp + ((bit_buf >> code_len++) & 1)]; } while (temp < 0); \
  } sym = temp; bit_buf >>= code_len; num_bits -= code_len; } US_MZ_MACRO_END

us_tinfl_status us_tinfl_decompress(us_tinfl_decompressor *r, const us_mz_uint8 *pIn_buf_next, size_t *pIn_buf_size, us_mz_uint8 *pOut_buf_start, us_mz_uint8 *pOut_buf_next, size_t *pOut_buf_size, const us_mz_uint32 decomp_flags)
{
  static const int s_length_base[31] = { 3,4,5,6,7,8,9,10,11,13, 15,17,19,23,27,31,35,43,51,59, 67,83,99,115,131,163,195,227,258,0,0 };
  static const int s_length_extra[31]= { 0,0,0,0,0,0,0,0,1,1,1,1,2,2,2,2,3,3,3,3,4,4,4,4,5,5,5,5,0,0,0 };
  static const int s_dist_base[32] = { 1,2,3,4,5,7,9,13,17,25,33,49,65,97,129,193, 257,385,513,769,1025,1537,2049,3073,4097,6145,8193,12289,16385,24577,0,0};
  static const int s_dist_extra[32] = { 0,0,0,0,1,1,2,2,3,3,4,4,5,5,6,6,7,7,8,8,9,9,10,10,11,11,12,12,13,13};
  static const us_mz_uint8 s_length_dezigzag[19] = { 16,17,18,0,8,7,9,6,10,5,11,4,12,3,13,2,14,1,15 };
  static const int s_min_table_sizes[3] = { 257, 1, 4 };

  us_tinfl_status status = US_TINFL_STATUS_FAILED; us_mz_uint32 num_bits, dist, counter, num_extra; us_tinfl_bit_buf_t bit_buf;
  const us_mz_uint8 *pIn_buf_cur = pIn_buf_next, *const pIn_buf_end = pIn_buf_next + *pIn_buf_size;
  us_mz_uint8 *pOut_buf_cur = pOut_buf_next, *const pOut_buf_end = pOut_buf_next + *pOut_buf_size;
  size_t out_buf_size_mask = (decomp_flags & US_TINFL_FLAG_USING_NON_WRAPPING_OUTPUT_BUF) ? (size_t)-1 : ((pOut_buf_next - pOut_buf_start) + *pOut_buf_size) - 1, dist_from_out_buf_start;

  // Ensure the output buffer's size is a power of 2, unless the output buffer is large enough to hold the entire output file (in which case it doesn't matter).
  if (((out_buf_size_mask + 1) & out_buf_size_mask) || (pOut_buf_next < pOut_buf_start)) { *pIn_buf_size = *pOut_buf_size = 0; return US_TINFL_STATUS_BAD_PARAM; }

  num_bits = r->m_num_bits; bit_buf = r->m_bit_buf; dist = r->m_dist; counter = r->m_counter; num_extra = r->m_num_extra; dist_from_out_buf_start = r->m_dist_from_out_buf_start;
  US_TINFL_CR_BEGIN

  bit_buf = num_bits = dist = counter = num_extra = r->m_zhdr0 = r->m_zhdr1 = 0; r->m_z_adler32 = r->m_check_adler32 = 1;
  if (decomp_flags & US_TINFL_FLAG_PARSE_ZLIB_HEADER)
  {
    US_TINFL_GET_BYTE(1, r->m_zhdr0); US_TINFL_GET_BYTE(2, r->m_zhdr1);
    counter = (((r->m_zhdr0 * 256 + r->m_zhdr1) % 31 != 0) || (r->m_zhdr1 & 32) || ((r->m_zhdr0 & 15) != 8));
    if (!(decomp_flags & US_TINFL_FLAG_USING_NON_WRAPPING_OUTPUT_BUF)) counter |= (((1U << (8U + (r->m_zhdr0 >> 4))) > 32768U) || ((out_buf_size_mask + 1) < (size_t)(1U << (8U + (r->m_zhdr0 >> 4)))));
    if (counter) { US_TINFL_CR_RETURN_FOREVER(36, US_TINFL_STATUS_FAILED); }
  }

  do
  {
    US_TINFL_GET_BITS(3, r->m_final, 3); r->m_type = r->m_final >> 1;
    if (r->m_type == 0)
    {
      US_TINFL_SKIP_BITS(5, num_bits & 7);
      for (counter = 0; counter < 4; ++counter) { if (num_bits) US_TINFL_GET_BITS(6, r->m_raw_header[counter], 8); else US_TINFL_GET_BYTE(7, r->m_raw_header[counter]); }
      if ((counter = (r->m_raw_header[0] | (r->m_raw_header[1] << 8))) != (us_mz_uint)(0xFFFF ^ (r->m_raw_header[2] | (r->m_raw_header[3] << 8)))) { US_TINFL_CR_RETURN_FOREVER(39, US_TINFL_STATUS_FAILED); }
      while ((counter) && (num_bits))
      {
        US_TINFL_GET_BITS(51, dist, 8);
        while (pOut_buf_cur >= pOut_buf_end) { US_TINFL_CR_RETURN(52, US_TINFL_STATUS_HAS_MORE_OUTPUT); }
        *pOut_buf_cur++ = (us_mz_uint8)dist;
        counter--;
      }
      while (counter)
      {
        size_t n; while (pOut_buf_cur >= pOut_buf_end) { US_TINFL_CR_RETURN(9, US_TINFL_STATUS_HAS_MORE_OUTPUT); }
        while (pIn_buf_cur >= pIn_buf_end)
        {
          if (decomp_flags & US_TINFL_FLAG_HAS_MORE_INPUT)
          {
            US_TINFL_CR_RETURN(38, US_TINFL_STATUS_NEEDS_MORE_INPUT);
          }
          else
          {
            US_TINFL_CR_RETURN_FOREVER(40, US_TINFL_STATUS_FAILED);
          }
        }
        n = US_MZ_MIN(US_MZ_MIN((size_t)(pOut_buf_end - pOut_buf_cur), (size_t)(pIn_buf_end - pIn_buf_cur)), counter);
        US_TINFL_MEMCPY(pOut_buf_cur, pIn_buf_cur, n); pIn_buf_cur += n; pOut_buf_cur += n; counter -= (us_mz_uint)n;
      }
    }
    else if (r->m_type == 3)
    {
      US_TINFL_CR_RETURN_FOREVER(10, US_TINFL_STATUS_FAILED);
    }
    else
    {
      if (r->m_type == 1)
      {
        us_mz_uint8 *p = r->m_tables[0].m_code_size; us_mz_uint i;
        r->m_table_sizes[0] = 288; r->m_table_sizes[1] = 32; US_TINFL_MEMSET(r->m_tables[1].m_code_size, 5, 32);
        for ( i = 0; i <= 143; ++i) *p++ = 8;
        for ( ; i <= 255; ++i) *p++ = 9;
        for ( ; i <= 279; ++i) *p++ = 7;
        for ( ; i <= 287; ++i) *p++ = 8;
      }
      else
      {
        for (counter = 0; counter < 3; counter++) { US_TINFL_GET_BITS(11, r->m_table_sizes[counter], "\05\05\04"[counter]); r->m_table_sizes[counter] += s_min_table_sizes[counter]; }
        US_MZ_CLEAR_OBJ(r->m_tables[2].m_code_size); for (counter = 0; counter < r->m_table_sizes[2]; counter++) { us_mz_uint s; US_TINFL_GET_BITS(14, s, 3); r->m_tables[2].m_code_size[s_length_dezigzag[counter]] = (us_mz_uint8)s; }
        r->m_table_sizes[2] = 19;
      }
      for ( ; (int)r->m_type >= 0; r->m_type--)
      {
        int tree_next, tree_cur; us_tinfl_huff_table *pTable;
        us_mz_uint i, j, used_syms, total, sym_index, next_code[17], total_syms[16]; pTable = &r->m_tables[r->m_type]; US_MZ_CLEAR_OBJ(total_syms); US_MZ_CLEAR_OBJ(pTable->m_look_up); US_MZ_CLEAR_OBJ(pTable->m_tree);
        for (i = 0; i < r->m_table_sizes[r->m_type]; ++i) total_syms[pTable->m_code_size[i]]++;
        used_syms = 0, total = 0; next_code[0] = next_code[1] = 0;
        for (i = 1; i <= 15; ++i) { used_syms += total_syms[i]; next_code[i + 1] = (total = ((total + total_syms[i]) << 1)); }
        if ((65536 != total) && (used_syms > 1))
        {
          US_TINFL_CR_RETURN_FOREVER(35, US_TINFL_STATUS_FAILED);
        }
        for (tree_next = -1, sym_index = 0; sym_index < r->m_table_sizes[r->m_type]; ++sym_index)
        {
          us_mz_uint rev_code = 0, l, cur_code, code_size = pTable->m_code_size[sym_index]; if (!code_size) continue;
          cur_code = next_code[code_size]++; for (l = code_size; l > 0; l--, cur_code >>= 1) rev_code = (rev_code << 1) | (cur_code & 1);
          if (code_size <= US_TINFL_FAST_LOOKUP_BITS) { us_mz_int16 k = (us_mz_int16)((code_size << 9) | sym_index); while (rev_code < US_TINFL_FAST_LOOKUP_SIZE) { pTable->m_look_up[rev_code] = k; rev_code += (1 << code_size); } continue; }
          if (0 == (tree_cur = pTable->m_look_up[rev_code & (US_TINFL_FAST_LOOKUP_SIZE - 1)])) { pTable->m_look_up[rev_code & (US_TINFL_FAST_LOOKUP_SIZE - 1)] = (us_mz_int16)tree_next; tree_cur = tree_next; tree_next -= 2; }
          rev_code >>= (US_TINFL_FAST_LOOKUP_BITS - 1);
          for (j = code_size; j > (US_TINFL_FAST_LOOKUP_BITS + 1); j--)
          {
            tree_cur -= ((rev_code >>= 1) & 1);
            if (!pTable->m_tree[-tree_cur - 1]) { pTable->m_tree[-tree_cur - 1] = (us_mz_int16)tree_next; tree_cur = tree_next; tree_next -= 2; } else tree_cur = pTable->m_tree[-tree_cur - 1];
          }
          tree_cur -= ((rev_code >>= 1) & 1); pTable->m_tree[-tree_cur - 1] = (us_mz_int16)sym_index;
        }
        if (r->m_type == 2)
        {
          for (counter = 0; counter < (r->m_table_sizes[0] + r->m_table_sizes[1]); )
          {
            us_mz_uint s; US_TINFL_HUFF_DECODE(16, dist, &r->m_tables[2]); if (dist < 16) { r->m_len_codes[counter++] = (us_mz_uint8)dist; continue; }
            if ((dist == 16) && (!counter))
            {
              US_TINFL_CR_RETURN_FOREVER(17, US_TINFL_STATUS_FAILED);
            }
            num_extra = "\02\03\07"[dist - 16]; US_TINFL_GET_BITS(18, s, num_extra); s += "\03\03\013"[dist - 16];
            US_TINFL_MEMSET(r->m_len_codes + counter, (dist == 16) ? r->m_len_codes[counter - 1] : 0, s); counter += s;
          }
          if ((r->m_table_sizes[0] + r->m_table_sizes[1]) != counter)
          {
            US_TINFL_CR_RETURN_FOREVER(21, US_TINFL_STATUS_FAILED);
          }
          US_TINFL_MEMCPY(r->m_tables[0].m_code_size, r->m_len_codes, r->m_table_sizes[0]); US_TINFL_MEMCPY(r->m_tables[1].m_code_size, r->m_len_codes + r->m_table_sizes[0], r->m_table_sizes[1]);
        }
      }
      for ( ; ; )
      {
        us_mz_uint8 *pSrc;
        for ( ; ; )
        {
          if (((pIn_buf_end - pIn_buf_cur) < 4) || ((pOut_buf_end - pOut_buf_cur) < 2))
          {
            US_TINFL_HUFF_DECODE(23, counter, &r->m_tables[0]);
            if (counter >= 256)
              break;
            while (pOut_buf_cur >= pOut_buf_end) { US_TINFL_CR_RETURN(24, US_TINFL_STATUS_HAS_MORE_OUTPUT); }
            *pOut_buf_cur++ = (us_mz_uint8)counter;
          }
          else
          {
            int sym2; us_mz_uint code_len;
#if US_TINFL_USE_64BIT_BITBUF
            if (num_bits < 30) { bit_buf |= (((us_tinfl_bit_buf_t)US_MZ_READ_LE32(pIn_buf_cur)) << num_bits); pIn_buf_cur += 4; num_bits += 32; }
#else
            if (num_bits < 15) { bit_buf |= (((us_tinfl_bit_buf_t)US_MZ_READ_LE16(pIn_buf_cur)) << num_bits); pIn_buf_cur += 2; num_bits += 16; }
#endif
            if ((sym2 = r->m_tables[0].m_look_up[bit_buf & (US_TINFL_FAST_LOOKUP_SIZE - 1)]) >= 0)
              code_len = sym2 >> 9;
            else
            {
              code_len = US_TINFL_FAST_LOOKUP_BITS; do { sym2 = r->m_tables[0].m_tree[~sym2 + ((bit_buf >> code_len++) & 1)]; } while (sym2 < 0);
            }
            counter = sym2; bit_buf >>= code_len; num_bits -= code_len;
            if (counter & 256)
              break;

#if !US_TINFL_USE_64BIT_BITBUF
            if (num_bits < 15) { bit_buf |= (((us_tinfl_bit_buf_t)US_MZ_READ_LE16(pIn_buf_cur)) << num_bits); pIn_buf_cur += 2; num_bits += 16; }
#endif
            if ((sym2 = r->m_tables[0].m_look_up[bit_buf & (US_TINFL_FAST_LOOKUP_SIZE - 1)]) >= 0)
              code_len = sym2 >> 9;
            else
            {
              code_len = US_TINFL_FAST_LOOKUP_BITS; do { sym2 = r->m_tables[0].m_tree[~sym2 + ((bit_buf >> code_len++) & 1)]; } while (sym2 < 0);
            }
            bit_buf >>= code_len; num_bits -= code_len;

            pOut_buf_cur[0] = (us_mz_uint8)counter;
            if (sym2 & 256)
            {
              pOut_buf_cur++;
              counter = sym2;
              break;
            }
            pOut_buf_cur[1] = (us_mz_uint8)sym2;
            pOut_buf_cur += 2;
          }
        }
        if ((counter &= 511) == 256) break;

        num_extra = s_length_extra[counter - 257]; counter = s_length_base[counter - 257];
        if (num_extra) { us_mz_uint extra_bits; US_TINFL_GET_BITS(25, extra_bits, num_extra); counter += extra_bits; }

        US_TINFL_HUFF_DECODE(26, dist, &r->m_tables[1]);
        num_extra = s_dist_extra[dist]; dist = s_dist_base[dist];
        if (num_extra) { us_mz_uint extra_bits; US_TINFL_GET_BITS(27, extra_bits, num_extra); dist += extra_bits; }

        dist_from_out_buf_start = pOut_buf_cur - pOut_buf_start;
        if ((dist > dist_from_out_buf_start) && (decomp_flags & US_TINFL_FLAG_USING_NON_WRAPPING_OUTPUT_BUF))
        {
          US_TINFL_CR_RETURN_FOREVER(37, US_TINFL_STATUS_FAILED);
        }

        pSrc = pOut_buf_start + ((dist_from_out_buf_start - dist) & out_buf_size_mask);

        if ((US_MZ_MAX(pOut_buf_cur, pSrc) + counter) > pOut_buf_end)
        {
          while (counter--)
          {
            while (pOut_buf_cur >= pOut_buf_end) { US_TINFL_CR_RETURN(53, US_TINFL_STATUS_HAS_MORE_OUTPUT); }
            *pOut_buf_cur++ = pOut_buf_start[(dist_from_out_buf_start++ - dist) & out_buf_size_mask];
          }
          continue;
        }
#if US_MZ_USE_UNALIGNED_LOADS_AND_STORES
        else if ((counter >= 9) && (counter <= dist))
        {
          const us_mz_uint8 *pSrc_end = pSrc + (counter & ~7);
          do
          {
            ((us_mz_uint32 *)pOut_buf_cur)[0] = ((const us_mz_uint32 *)pSrc)[0];
            ((us_mz_uint32 *)pOut_buf_cur)[1] = ((const us_mz_uint32 *)pSrc)[1];
            pOut_buf_cur += 8;
          } while ((pSrc += 8) < pSrc_end);
          if ((counter &= 7) < 3)
          {
            if (counter)
            {
              pOut_buf_cur[0] = pSrc[0];
              if (counter > 1)
                pOut_buf_cur[1] = pSrc[1];
              pOut_buf_cur += counter;
            }
            continue;
          }
        }
#endif
        do
        {
          pOut_buf_cur[0] = pSrc[0];
          pOut_buf_cur[1] = pSrc[1];
          pOut_buf_cur[2] = pSrc[2];
          pOut_buf_cur += 3; pSrc += 3;
        } while ((int)(counter -= 3) > 2);
        if ((int)counter > 0)
        {
          pOut_buf_cur[0] = pSrc[0];
          if ((int)counter > 1)
            pOut_buf_cur[1] = pSrc[1];
          pOut_buf_cur += counter;
        }
      }
    }
  } while (!(r->m_final & 1));
  if (decomp_flags & US_TINFL_FLAG_PARSE_ZLIB_HEADER)
  {
    US_TINFL_SKIP_BITS(32, num_bits & 7); for (counter = 0; counter < 4; ++counter) { us_mz_uint s; if (num_bits) US_TINFL_GET_BITS(41, s, 8); else US_TINFL_GET_BYTE(42, s); r->m_z_adler32 = (r->m_z_adler32 << 8) | s; }
  }
  US_TINFL_CR_RETURN_FOREVER(34, US_TINFL_STATUS_DONE);
  US_TINFL_CR_FINISH

common_exit:
  r->m_num_bits = num_bits; r->m_bit_buf = bit_buf; r->m_dist = dist; r->m_counter = counter; r->m_num_extra = num_extra; r->m_dist_from_out_buf_start = dist_from_out_buf_start;
  *pIn_buf_size = pIn_buf_cur - pIn_buf_next; *pOut_buf_size = pOut_buf_cur - pOut_buf_next;
  if ((decomp_flags & (US_TINFL_FLAG_PARSE_ZLIB_HEADER | US_TINFL_FLAG_COMPUTE_ADLER32)) && (status >= 0))
  {
    const us_mz_uint8 *ptr = pOut_buf_next; size_t buf_len = *pOut_buf_size;
    us_mz_uint32 i, s1 = r->m_check_adler32 & 0xffff, s2 = r->m_check_adler32 >> 16; size_t block_len = buf_len % 5552;
    while (buf_len)
    {
      for (i = 0; i + 7 < block_len; i += 8, ptr += 8)
      {
        s1 += ptr[0], s2 += s1; s1 += ptr[1], s2 += s1; s1 += ptr[2], s2 += s1; s1 += ptr[3], s2 += s1;
        s1 += ptr[4], s2 += s1; s1 += ptr[5], s2 += s1; s1 += ptr[6], s2 += s1; s1 += ptr[7], s2 += s1;
      }
      for ( ; i < block_len; ++i) s1 += *ptr++, s2 += s1;
      s1 %= 65521U, s2 %= 65521U; buf_len -= block_len; block_len = 5552;
    }
    r->m_check_adler32 = (s2 << 16) + s1; if ((status == US_TINFL_STATUS_DONE) && (decomp_flags & US_TINFL_FLAG_PARSE_ZLIB_HEADER) && (r->m_check_adler32 != r->m_z_adler32)) status = US_TINFL_STATUS_ADLER32_MISMATCH;
  }
  return status;
}

// Higher level helper functions.
void *us_tinfl_decompress_mem_to_heap(const void *pSrc_buf, size_t src_buf_len, size_t *pOut_len, int flags)
{
  us_tinfl_decompressor decomp; void *pBuf = NULL, *pNew_buf; size_t src_buf_ofs = 0, out_buf_capacity = 0;
  *pOut_len = 0;
  us_tinfl_init(&decomp);
  for ( ; ; )
  {
    size_t src_buf_size = src_buf_len - src_buf_ofs, dst_buf_size = out_buf_capacity - *pOut_len, new_out_buf_capacity;
    us_tinfl_status status = us_tinfl_decompress(&decomp, (const us_mz_uint8*)pSrc_buf + src_buf_ofs, &src_buf_size, (us_mz_uint8*)pBuf, pBuf ? (us_mz_uint8*)pBuf + *pOut_len : NULL, &dst_buf_size,
      (flags & ~US_TINFL_FLAG_HAS_MORE_INPUT) | US_TINFL_FLAG_USING_NON_WRAPPING_OUTPUT_BUF);
    if ((status < 0) || (status == US_TINFL_STATUS_NEEDS_MORE_INPUT))
    {
      US_MZ_FREE(pBuf); *pOut_len = 0; return NULL;
    }
    src_buf_ofs += src_buf_size;
    *pOut_len += dst_buf_size;
    if (status == US_TINFL_STATUS_DONE) break;
    new_out_buf_capacity = out_buf_capacity * 2; if (new_out_buf_capacity < 128) new_out_buf_capacity = 128;
    pNew_buf = US_MZ_REALLOC(pBuf, new_out_buf_capacity);
    if (!pNew_buf)
    {
      US_MZ_FREE(pBuf); *pOut_len = 0; return NULL;
    }
    pBuf = pNew_buf; out_buf_capacity = new_out_buf_capacity;
  }
  return pBuf;
}

size_t us_tinfl_decompress_mem_to_mem(void *pOut_buf, size_t out_buf_len, const void *pSrc_buf, size_t src_buf_len, int flags)
{
  us_tinfl_decompressor decomp; us_tinfl_status status; us_tinfl_init(&decomp);
  status = us_tinfl_decompress(&decomp, (const us_mz_uint8*)pSrc_buf, &src_buf_len, (us_mz_uint8*)pOut_buf, (us_mz_uint8*)pOut_buf, &out_buf_len, (flags & ~US_TINFL_FLAG_HAS_MORE_INPUT) | US_TINFL_FLAG_USING_NON_WRAPPING_OUTPUT_BUF);
  return (status != US_TINFL_STATUS_DONE) ? US_TINFL_DECOMPRESS_MEM_TO_MEM_FAILED : out_buf_len;
}

int us_tinfl_decompress_mem_to_callback(const void *pIn_buf, size_t *pIn_buf_size, us_tinfl_put_buf_func_ptr pPut_buf_func, void *pPut_buf_user, int flags)
{
  int result = 0;
  us_tinfl_decompressor decomp;
  us_mz_uint8 *pDict = (us_mz_uint8*)US_MZ_MALLOC(US_TINFL_LZ_DICT_SIZE); size_t in_buf_ofs = 0, dict_ofs = 0;
  if (!pDict)
    return US_TINFL_STATUS_FAILED;
  us_tinfl_init(&decomp);
  for ( ; ; )
  {
    size_t in_buf_size = *pIn_buf_size - in_buf_ofs, dst_buf_size = US_TINFL_LZ_DICT_SIZE - dict_ofs;
    us_tinfl_status status = us_tinfl_decompress(&decomp, (const us_mz_uint8*)pIn_buf + in_buf_ofs, &in_buf_size, pDict, pDict + dict_ofs, &dst_buf_size,
      (flags & ~(US_TINFL_FLAG_HAS_MORE_INPUT | US_TINFL_FLAG_USING_NON_WRAPPING_OUTPUT_BUF)));
    in_buf_ofs += in_buf_size;
    if ((dst_buf_size) && (!(*pPut_buf_func)(pDict + dict_ofs, (int)dst_buf_size, pPut_buf_user)))
      break;
    if (status != US_TINFL_STATUS_HAS_MORE_OUTPUT)
    {
      result = (status == US_TINFL_STATUS_DONE);
      break;
    }
    dict_ofs = (dict_ofs + dst_buf_size) & (US_TINFL_LZ_DICT_SIZE - 1);
  }
  US_MZ_FREE(pDict);
  *pIn_buf_size = in_buf_ofs;
  return result;
}

// ------------------- Low-level Compression (independent from all decompression API's)

// Purposely making these tables static for faster init and thread safety.
static const us_mz_uint16 s_us_tdefl_len_sym[256] = {
  257,258,259,260,261,262,263,264,265,265,266,266,267,267,268,268,269,269,269,269,270,270,270,270,271,271,271,271,272,272,272,272,
  273,273,273,273,273,273,273,273,274,274,274,274,274,274,274,274,275,275,275,275,275,275,275,275,276,276,276,276,276,276,276,276,
  277,277,277,277,277,277,277,277,277,277,277,277,277,277,277,277,278,278,278,278,278,278,278,278,278,278,278,278,278,278,278,278,
  279,279,279,279,279,279,279,279,279,279,279,279,279,279,279,279,280,280,280,280,280,280,280,280,280,280,280,280,280,280,280,280,
  281,281,281,281,281,281,281,281,281,281,281,281,281,281,281,281,281,281,281,281,281,281,281,281,281,281,281,281,281,281,281,281,
  282,282,282,282,282,282,282,282,282,282,282,282,282,282,282,282,282,282,282,282,282,282,282,282,282,282,282,282,282,282,282,282,
  283,283,283,283,283,283,283,283,283,283,283,283,283,283,283,283,283,283,283,283,283,283,283,283,283,283,283,283,283,283,283,283,
  284,284,284,284,284,284,284,284,284,284,284,284,284,284,284,284,284,284,284,284,284,284,284,284,284,284,284,284,284,284,284,285 };

static const us_mz_uint8 s_us_tdefl_len_extra[256] = {
  0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,
  4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
  5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
  5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,0 };

static const us_mz_uint8 s_us_tdefl_small_dist_sym[512] = {
  0,1,2,3,4,4,5,5,6,6,6,6,7,7,7,7,8,8,8,8,8,8,8,8,9,9,9,9,9,9,9,9,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,11,11,11,11,11,11,
  11,11,11,11,11,11,11,11,11,11,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,13,
  13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,14,14,14,14,14,14,14,14,14,14,14,14,
  14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,
  14,14,14,14,14,14,14,14,14,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,
  15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,16,16,16,16,16,16,16,16,16,16,16,16,16,
  16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,
  16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,
  16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,17,17,17,17,17,17,17,17,17,17,17,17,17,17,
  17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,
  17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,
  17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17 };

static const us_mz_uint8 s_us_tdefl_small_dist_extra[512] = {
  0,0,0,0,1,1,1,1,2,2,2,2,2,2,2,2,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,5,5,5,5,5,5,5,5,
  5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
  6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
  6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
  7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
  7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
  7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
  7,7,7,7,7,7,7,7 };

static const us_mz_uint8 s_us_tdefl_large_dist_sym[128] = {
  0,0,18,19,20,20,21,21,22,22,22,22,23,23,23,23,24,24,24,24,24,24,24,24,25,25,25,25,25,25,25,25,26,26,26,26,26,26,26,26,26,26,26,26,
  26,26,26,26,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,
  28,28,28,28,28,28,28,28,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29 };

static const us_mz_uint8 s_us_tdefl_large_dist_extra[128] = {
  0,0,8,8,9,9,9,9,10,10,10,10,10,10,10,10,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,
  12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,
  13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13 };

// Radix sorts us_tdefl_sym_freq[] array by 16-bit key m_key. Returns ptr to sorted values.
typedef struct { us_mz_uint16 m_key, m_sym_index; } us_tdefl_sym_freq;
static us_tdefl_sym_freq* us_tdefl_radix_sort_syms(us_mz_uint num_syms, us_tdefl_sym_freq* pSyms0, us_tdefl_sym_freq* pSyms1)
{
  us_mz_uint32 total_passes = 2, pass_shift, pass, i, hist[256 * 2]; us_tdefl_sym_freq* pCur_syms = pSyms0, *pNew_syms = pSyms1; US_MZ_CLEAR_OBJ(hist);
  for (i = 0; i < num_syms; i++) { us_mz_uint freq = pSyms0[i].m_key; hist[freq & 0xFF]++; hist[256 + ((freq >> 8) & 0xFF)]++; }
  while ((total_passes > 1) && (num_syms == hist[(total_passes - 1) * 256])) total_passes--;
  for (pass_shift = 0, pass = 0; pass < total_passes; pass++, pass_shift += 8)
  {
    const us_mz_uint32* pHist = &hist[pass << 8];
    us_mz_uint offsets[256], cur_ofs = 0;
    for (i = 0; i < 256; i++) { offsets[i] = cur_ofs; cur_ofs += pHist[i]; }
    for (i = 0; i < num_syms; i++) pNew_syms[offsets[(pCur_syms[i].m_key >> pass_shift) & 0xFF]++] = pCur_syms[i];
    { us_tdefl_sym_freq* t = pCur_syms; pCur_syms = pNew_syms; pNew_syms = t; }
  }
  return pCur_syms;
}

// us_tdefl_calculate_minimum_redundancy() originally written by: Alistair Moffat, alistair@cs.mu.oz.au, Jyrki Katajainen, jyrki@diku.dk, November 1996.
static void us_tdefl_calculate_minimum_redundancy(us_tdefl_sym_freq *A, int n)
{
  int root, leaf, next, avbl, used, dpth;
  if (n==0) return; else if (n==1) { A[0].m_key = 1; return; }
  A[0].m_key += A[1].m_key; root = 0; leaf = 2;
  for (next=1; next < n-1; next++)
  {
    if (leaf>=n || A[root].m_key<A[leaf].m_key) { A[next].m_key = A[root].m_key; A[root++].m_key = (us_mz_uint16)next; } else A[next].m_key = A[leaf++].m_key;
    if (leaf>=n || (root<next && A[root].m_key<A[leaf].m_key)) { A[next].m_key = (us_mz_uint16)(A[next].m_key + A[root].m_key); A[root++].m_key = (us_mz_uint16)next; } else A[next].m_key = (us_mz_uint16)(A[next].m_key + A[leaf++].m_key);
  }
  A[n-2].m_key = 0; for (next=n-3; next>=0; next--) A[next].m_key = A[A[next].m_key].m_key+1;
  avbl = 1; used = dpth = 0; root = n-2; next = n-1;
  while (avbl>0)
  {
    while (root>=0 && (int)A[root].m_key==dpth) { used++; root--; }
    while (avbl>used) { A[next--].m_key = (us_mz_uint16)(dpth); avbl--; }
    avbl = 2*used; dpth++; used = 0;
  }
}

// Limits canonical Huffman code table's max code size.
enum { US_TDEFL_MAX_SUPPORTED_HUFF_CODESIZE = 32 };
static void us_tdefl_huffman_enforce_max_code_size(int *pNum_codes, int code_list_len, int max_code_size)
{
  int i; us_mz_uint32 total = 0; if (code_list_len <= 1) return;
  for (i = max_code_size + 1; i <= US_TDEFL_MAX_SUPPORTED_HUFF_CODESIZE; i++) pNum_codes[max_code_size] += pNum_codes[i];
  for (i = max_code_size; i > 0; i--) total += (((us_mz_uint32)pNum_codes[i]) << (max_code_size - i));
  while (total != (1UL << max_code_size))
  {
    pNum_codes[max_code_size]--;
    for (i = max_code_size - 1; i > 0; i--) if (pNum_codes[i]) { pNum_codes[i]--; pNum_codes[i + 1] += 2; break; }
    total--;
  }
}

static void us_tdefl_optimize_huffman_table(us_tdefl_compressor *d, int table_num, int table_len, int code_size_limit, int static_table)
{
  int i, j, l, num_codes[1 + US_TDEFL_MAX_SUPPORTED_HUFF_CODESIZE]; us_mz_uint next_code[US_TDEFL_MAX_SUPPORTED_HUFF_CODESIZE + 1]; US_MZ_CLEAR_OBJ(num_codes);
  if (static_table)
  {
    for (i = 0; i < table_len; i++) num_codes[d->m_huff_code_sizes[table_num][i]]++;
  }
  else
  {
    us_tdefl_sym_freq syms0[US_TDEFL_MAX_HUFF_SYMBOLS], syms1[US_TDEFL_MAX_HUFF_SYMBOLS], *pSyms;
    int num_used_syms = 0;
    const us_mz_uint16 *pSym_count = &d->m_huff_count[table_num][0];
    for (i = 0; i < table_len; i++) if (pSym_count[i]) { syms0[num_used_syms].m_key = (us_mz_uint16)pSym_count[i]; syms0[num_used_syms++].m_sym_index = (us_mz_uint16)i; }

    pSyms = us_tdefl_radix_sort_syms(num_used_syms, syms0, syms1); us_tdefl_calculate_minimum_redundancy(pSyms, num_used_syms);

    for (i = 0; i < num_used_syms; i++) num_codes[pSyms[i].m_key]++;

    us_tdefl_huffman_enforce_max_code_size(num_codes, num_used_syms, code_size_limit);

    US_MZ_CLEAR_OBJ(d->m_huff_code_sizes[table_num]); US_MZ_CLEAR_OBJ(d->m_huff_codes[table_num]);
    for (i = 1, j = num_used_syms; i <= code_size_limit; i++)
      for (l = num_codes[i]; l > 0; l--) d->m_huff_code_sizes[table_num][pSyms[--j].m_sym_index] = (us_mz_uint8)(i);
  }

  next_code[1] = 0; for (j = 0, i = 2; i <= code_size_limit; i++) next_code[i] = j = ((j + num_codes[i - 1]) << 1);

  for (i = 0; i < table_len; i++)
  {
    us_mz_uint rev_code = 0, code, code_size; if ((code_size = d->m_huff_code_sizes[table_num][i]) == 0) continue;
    code = next_code[code_size]++; for (l = code_size; l > 0; l--, code >>= 1) rev_code = (rev_code << 1) | (code & 1);
    d->m_huff_codes[table_num][i] = (us_mz_uint16)rev_code;
  }
}

#define US_TDEFL_PUT_BITS(b, l) do { \
  us_mz_uint bits = b; us_mz_uint len = l; US_MZ_ASSERT(bits <= ((1U << len) - 1U)); \
  d->m_bit_buffer |= (bits << d->m_bits_in); d->m_bits_in += len; \
  while (d->m_bits_in >= 8) { \
    if (d->m_pOutput_buf < d->m_pOutput_buf_end) \
      *d->m_pOutput_buf++ = (us_mz_uint8)(d->m_bit_buffer); \
    d->m_bit_buffer >>= 8; \
    d->m_bits_in -= 8; \
  } \
} US_MZ_MACRO_END

#define US_TDEFL_RLE_PREV_CODE_SIZE() { if (rle_repeat_count) { \
  if (rle_repeat_count < 3) { \
    d->m_huff_count[2][prev_code_size] = (us_mz_uint16)(d->m_huff_count[2][prev_code_size] + rle_repeat_count); \
    while (rle_repeat_count--) packed_code_sizes[num_packed_code_sizes++] = prev_code_size; \
  } else { \
    d->m_huff_count[2][16] = (us_mz_uint16)(d->m_huff_count[2][16] + 1); packed_code_sizes[num_packed_code_sizes++] = 16; packed_code_sizes[num_packed_code_sizes++] = (us_mz_uint8)(rle_repeat_count - 3); \
} rle_repeat_count = 0; } }

#define US_TDEFL_RLE_ZERO_CODE_SIZE() { if (rle_z_count) { \
  if (rle_z_count < 3) { \
    d->m_huff_count[2][0] = (us_mz_uint16)(d->m_huff_count[2][0] + rle_z_count); while (rle_z_count--) packed_code_sizes[num_packed_code_sizes++] = 0; \
  } else if (rle_z_count <= 10) { \
    d->m_huff_count[2][17] = (us_mz_uint16)(d->m_huff_count[2][17] + 1); packed_code_sizes[num_packed_code_sizes++] = 17; packed_code_sizes[num_packed_code_sizes++] = (us_mz_uint8)(rle_z_count - 3); \
  } else { \
    d->m_huff_count[2][18] = (us_mz_uint16)(d->m_huff_count[2][18] + 1); packed_code_sizes[num_packed_code_sizes++] = 18; packed_code_sizes[num_packed_code_sizes++] = (us_mz_uint8)(rle_z_count - 11); \
} rle_z_count = 0; } }

static us_mz_uint8 s_us_tdefl_packed_code_size_syms_swizzle[] = { 16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15 };

static void us_tdefl_start_dynamic_block(us_tdefl_compressor *d)
{
  int num_lit_codes, num_dist_codes, num_bit_lengths; us_mz_uint i, total_code_sizes_to_pack, num_packed_code_sizes, rle_z_count, rle_repeat_count, packed_code_sizes_index;
  us_mz_uint8 code_sizes_to_pack[US_TDEFL_MAX_HUFF_SYMBOLS_0 + US_TDEFL_MAX_HUFF_SYMBOLS_1], packed_code_sizes[US_TDEFL_MAX_HUFF_SYMBOLS_0 + US_TDEFL_MAX_HUFF_SYMBOLS_1], prev_code_size = 0xFF;

  d->m_huff_count[0][256] = 1;

  us_tdefl_optimize_huffman_table(d, 0, US_TDEFL_MAX_HUFF_SYMBOLS_0, 15, US_MZ_FALSE);
  us_tdefl_optimize_huffman_table(d, 1, US_TDEFL_MAX_HUFF_SYMBOLS_1, 15, US_MZ_FALSE);

  for (num_lit_codes = 286; num_lit_codes > 257; num_lit_codes--) if (d->m_huff_code_sizes[0][num_lit_codes - 1]) break;
  for (num_dist_codes = 30; num_dist_codes > 1; num_dist_codes--) if (d->m_huff_code_sizes[1][num_dist_codes - 1]) break;

  memcpy(code_sizes_to_pack, &d->m_huff_code_sizes[0][0], num_lit_codes);
  memcpy(code_sizes_to_pack + num_lit_codes, &d->m_huff_code_sizes[1][0], num_dist_codes);
  total_code_sizes_to_pack = num_lit_codes + num_dist_codes; num_packed_code_sizes = 0; rle_z_count = 0; rle_repeat_count = 0;

  memset(&d->m_huff_count[2][0], 0, sizeof(d->m_huff_count[2][0]) * US_TDEFL_MAX_HUFF_SYMBOLS_2);
  for (i = 0; i < total_code_sizes_to_pack; i++)
  {
    us_mz_uint8 code_size = code_sizes_to_pack[i];
    if (!code_size)
    {
      US_TDEFL_RLE_PREV_CODE_SIZE();
      if (++rle_z_count == 138) { US_TDEFL_RLE_ZERO_CODE_SIZE(); }
    }
    else
    {
      US_TDEFL_RLE_ZERO_CODE_SIZE();
      if (code_size != prev_code_size)
      {
        US_TDEFL_RLE_PREV_CODE_SIZE();
        d->m_huff_count[2][code_size] = (us_mz_uint16)(d->m_huff_count[2][code_size] + 1); packed_code_sizes[num_packed_code_sizes++] = code_size;
      }
      else if (++rle_repeat_count == 6)
      {
        US_TDEFL_RLE_PREV_CODE_SIZE();
      }
    }
    prev_code_size = code_size;
  }
  if (rle_repeat_count) { US_TDEFL_RLE_PREV_CODE_SIZE(); } else { US_TDEFL_RLE_ZERO_CODE_SIZE(); }

  us_tdefl_optimize_huffman_table(d, 2, US_TDEFL_MAX_HUFF_SYMBOLS_2, 7, US_MZ_FALSE);

  US_TDEFL_PUT_BITS(2, 2);

  US_TDEFL_PUT_BITS(num_lit_codes - 257, 5);
  US_TDEFL_PUT_BITS(num_dist_codes - 1, 5);

  for (num_bit_lengths = 18; num_bit_lengths >= 0; num_bit_lengths--) if (d->m_huff_code_sizes[2][s_us_tdefl_packed_code_size_syms_swizzle[num_bit_lengths]]) break;
  num_bit_lengths = US_MZ_MAX(4, (num_bit_lengths + 1)); US_TDEFL_PUT_BITS(num_bit_lengths - 4, 4);
  for (i = 0; (int)i < num_bit_lengths; i++) US_TDEFL_PUT_BITS(d->m_huff_code_sizes[2][s_us_tdefl_packed_code_size_syms_swizzle[i]], 3);

  for (packed_code_sizes_index = 0; packed_code_sizes_index < num_packed_code_sizes; )
  {
    us_mz_uint code = packed_code_sizes[packed_code_sizes_index++]; US_MZ_ASSERT(code < US_TDEFL_MAX_HUFF_SYMBOLS_2);
    US_TDEFL_PUT_BITS(d->m_huff_codes[2][code], d->m_huff_code_sizes[2][code]);
    if (code >= 16) US_TDEFL_PUT_BITS(packed_code_sizes[packed_code_sizes_index++], "\02\03\07"[code - 16]);
  }
}

static void us_tdefl_start_static_block(us_tdefl_compressor *d)
{
  us_mz_uint i;
  us_mz_uint8 *p = &d->m_huff_code_sizes[0][0];

  for (i = 0; i <= 143; ++i) *p++ = 8;
  for ( ; i <= 255; ++i) *p++ = 9;
  for ( ; i <= 279; ++i) *p++ = 7;
  for ( ; i <= 287; ++i) *p++ = 8;

  memset(d->m_huff_code_sizes[1], 5, 32);

  us_tdefl_optimize_huffman_table(d, 0, 288, 15, US_MZ_TRUE);
  us_tdefl_optimize_huffman_table(d, 1, 32, 15, US_MZ_TRUE);

  US_TDEFL_PUT_BITS(1, 2);
}

static const us_mz_uint us_mz_bitmasks[17] = { 0x0000, 0x0001, 0x0003, 0x0007, 0x000F, 0x001F, 0x003F, 0x007F, 0x00FF, 0x01FF, 0x03FF, 0x07FF, 0x0FFF, 0x1FFF, 0x3FFF, 0x7FFF, 0xFFFF };

#if US_MZ_USE_UNALIGNED_LOADS_AND_STORES && US_MZ_LITTLE_ENDIAN && US_MZ_HAS_64BIT_REGISTERS
static us_mz_bool us_tdefl_compress_lz_codes(us_tdefl_compressor *d)
{
  us_mz_uint flags;
  us_mz_uint8 *pLZ_codes;
  us_mz_uint8 *pOutput_buf = d->m_pOutput_buf;
  us_mz_uint8 *pLZ_code_buf_end = d->m_pLZ_code_buf;
  us_mz_uint64 bit_buffer = d->m_bit_buffer;
  us_mz_uint bits_in = d->m_bits_in;

#define US_TDEFL_PUT_BITS_FAST(b, l) { bit_buffer |= (((us_mz_uint64)(b)) << bits_in); bits_in += (l); }

  flags = 1;
  for (pLZ_codes = d->m_lz_code_buf; pLZ_codes < pLZ_code_buf_end; flags >>= 1)
  {
    if (flags == 1)
      flags = *pLZ_codes++ | 0x100;

    if (flags & 1)
    {
      us_mz_uint s0, s1, n0, n1, sym, num_extra_bits;
      us_mz_uint match_len = pLZ_codes[0], match_dist = *(const us_mz_uint16 *)(pLZ_codes + 1); pLZ_codes += 3;

      US_MZ_ASSERT(d->m_huff_code_sizes[0][s_us_tdefl_len_sym[match_len]]);
      US_TDEFL_PUT_BITS_FAST(d->m_huff_codes[0][s_us_tdefl_len_sym[match_len]], d->m_huff_code_sizes[0][s_us_tdefl_len_sym[match_len]]);
      US_TDEFL_PUT_BITS_FAST(match_len & us_mz_bitmasks[s_us_tdefl_len_extra[match_len]], s_us_tdefl_len_extra[match_len]);

      // This sequence coaxes MSVC into using cmov's vs. jmp's.
      s0 = s_us_tdefl_small_dist_sym[match_dist & 511];
      n0 = s_us_tdefl_small_dist_extra[match_dist & 511];
      s1 = s_us_tdefl_large_dist_sym[match_dist >> 8];
      n1 = s_us_tdefl_large_dist_extra[match_dist >> 8];
      sym = (match_dist < 512) ? s0 : s1;
      num_extra_bits = (match_dist < 512) ? n0 : n1;

      US_MZ_ASSERT(d->m_huff_code_sizes[1][sym]);
      US_TDEFL_PUT_BITS_FAST(d->m_huff_codes[1][sym], d->m_huff_code_sizes[1][sym]);
      US_TDEFL_PUT_BITS_FAST(match_dist & us_mz_bitmasks[num_extra_bits], num_extra_bits);
    }
    else
    {
      us_mz_uint lit = *pLZ_codes++;
      US_MZ_ASSERT(d->m_huff_code_sizes[0][lit]);
      US_TDEFL_PUT_BITS_FAST(d->m_huff_codes[0][lit], d->m_huff_code_sizes[0][lit]);

      if (((flags & 2) == 0) && (pLZ_codes < pLZ_code_buf_end))
      {
        flags >>= 1;
        lit = *pLZ_codes++;
        US_MZ_ASSERT(d->m_huff_code_sizes[0][lit]);
        US_TDEFL_PUT_BITS_FAST(d->m_huff_codes[0][lit], d->m_huff_code_sizes[0][lit]);

        if (((flags & 2) == 0) && (pLZ_codes < pLZ_code_buf_end))
        {
          flags >>= 1;
          lit = *pLZ_codes++;
          US_MZ_ASSERT(d->m_huff_code_sizes[0][lit]);
          US_TDEFL_PUT_BITS_FAST(d->m_huff_codes[0][lit], d->m_huff_code_sizes[0][lit]);
        }
      }
    }

    if (pOutput_buf >= d->m_pOutput_buf_end)
      return US_MZ_FALSE;

    *(us_mz_uint64*)pOutput_buf = bit_buffer;
    pOutput_buf += (bits_in >> 3);
    bit_buffer >>= (bits_in & ~7);
    bits_in &= 7;
  }

#undef US_TDEFL_PUT_BITS_FAST

  d->m_pOutput_buf = pOutput_buf;
  d->m_bits_in = 0;
  d->m_bit_buffer = 0;

  while (bits_in)
  {
    us_mz_uint32 n = US_MZ_MIN(bits_in, 16);
    US_TDEFL_PUT_BITS((us_mz_uint)bit_buffer & us_mz_bitmasks[n], n);
    bit_buffer >>= n;
    bits_in -= n;
  }

  US_TDEFL_PUT_BITS(d->m_huff_codes[0][256], d->m_huff_code_sizes[0][256]);

  return (d->m_pOutput_buf < d->m_pOutput_buf_end);
}
#else
static us_mz_bool us_tdefl_compress_lz_codes(us_tdefl_compressor *d)
{
  us_mz_uint flags;
  us_mz_uint8 *pLZ_codes;

  flags = 1;
  for (pLZ_codes = d->m_lz_code_buf; pLZ_codes < d->m_pLZ_code_buf; flags >>= 1)
  {
    if (flags == 1)
      flags = *pLZ_codes++ | 0x100;
    if (flags & 1)
    {
      us_mz_uint sym, num_extra_bits;
      us_mz_uint match_len = pLZ_codes[0], match_dist = (pLZ_codes[1] | (pLZ_codes[2] << 8)); pLZ_codes += 3;

      US_MZ_ASSERT(d->m_huff_code_sizes[0][s_us_tdefl_len_sym[match_len]]);
      US_TDEFL_PUT_BITS(d->m_huff_codes[0][s_us_tdefl_len_sym[match_len]], d->m_huff_code_sizes[0][s_us_tdefl_len_sym[match_len]]);
      US_TDEFL_PUT_BITS(match_len & us_mz_bitmasks[s_us_tdefl_len_extra[match_len]], s_us_tdefl_len_extra[match_len]);

      if (match_dist < 512)
      {
        sym = s_us_tdefl_small_dist_sym[match_dist]; num_extra_bits = s_us_tdefl_small_dist_extra[match_dist];
      }
      else
      {
        sym = s_us_tdefl_large_dist_sym[match_dist >> 8]; num_extra_bits = s_us_tdefl_large_dist_extra[match_dist >> 8];
      }
      US_MZ_ASSERT(d->m_huff_code_sizes[1][sym]);
      US_TDEFL_PUT_BITS(d->m_huff_codes[1][sym], d->m_huff_code_sizes[1][sym]);
      US_TDEFL_PUT_BITS(match_dist & us_mz_bitmasks[num_extra_bits], num_extra_bits);
    }
    else
    {
      us_mz_uint lit = *pLZ_codes++;
      US_MZ_ASSERT(d->m_huff_code_sizes[0][lit]);
      US_TDEFL_PUT_BITS(d->m_huff_codes[0][lit], d->m_huff_code_sizes[0][lit]);
    }
  }

  US_TDEFL_PUT_BITS(d->m_huff_codes[0][256], d->m_huff_code_sizes[0][256]);

  return (d->m_pOutput_buf < d->m_pOutput_buf_end);
}
#endif // US_MZ_USE_UNALIGNED_LOADS_AND_STORES && US_MZ_LITTLE_ENDIAN && US_MZ_HAS_64BIT_REGISTERS

static us_mz_bool us_tdefl_compress_block(us_tdefl_compressor *d, us_mz_bool static_block)
{
  if (static_block)
    us_tdefl_start_static_block(d);
  else
    us_tdefl_start_dynamic_block(d);
  return us_tdefl_compress_lz_codes(d);
}

static int us_tdefl_flush_block(us_tdefl_compressor *d, int flush)
{
  us_mz_uint saved_bit_buf, saved_bits_in;
  us_mz_uint8 *pSaved_output_buf;
  us_mz_bool comp_block_succeeded = US_MZ_FALSE;
  int n, use_raw_block = ((d->m_flags & US_TDEFL_FORCE_ALL_RAW_BLOCKS) != 0) && (d->m_lookahead_pos - d->m_lz_code_buf_dict_pos) <= d->m_dict_size;
  us_mz_uint8 *pOutput_buf_start = ((d->m_pPut_buf_func == NULL) && ((*d->m_pOut_buf_size - d->m_out_buf_ofs) >= US_TDEFL_OUT_BUF_SIZE)) ? ((us_mz_uint8 *)d->m_pOut_buf + d->m_out_buf_ofs) : d->m_output_buf;

  d->m_pOutput_buf = pOutput_buf_start;
  d->m_pOutput_buf_end = d->m_pOutput_buf + US_TDEFL_OUT_BUF_SIZE - 16;

  US_MZ_ASSERT(!d->m_output_flush_remaining);
  d->m_output_flush_ofs = 0;
  d->m_output_flush_remaining = 0;

  *d->m_pLZ_flags = (us_mz_uint8)(*d->m_pLZ_flags >> d->m_num_flags_left);
  d->m_pLZ_code_buf -= (d->m_num_flags_left == 8);

  if ((d->m_flags & US_TDEFL_WRITE_ZLIB_HEADER) && (!d->m_block_index))
  {
    US_TDEFL_PUT_BITS(0x78, 8); US_TDEFL_PUT_BITS(0x01, 8);
  }

  US_TDEFL_PUT_BITS(flush == US_TDEFL_FINISH, 1);

  pSaved_output_buf = d->m_pOutput_buf; saved_bit_buf = d->m_bit_buffer; saved_bits_in = d->m_bits_in;

  if (!use_raw_block)
    comp_block_succeeded = us_tdefl_compress_block(d, (d->m_flags & US_TDEFL_FORCE_ALL_STATIC_BLOCKS) || (d->m_total_lz_bytes < 48));

  // If the block gets expanded, forget the current contents of the output buffer and send a raw block instead.
  if ( ((use_raw_block) || ((d->m_total_lz_bytes) && ((d->m_pOutput_buf - pSaved_output_buf + 1U) >= d->m_total_lz_bytes))) &&
       ((d->m_lookahead_pos - d->m_lz_code_buf_dict_pos) <= d->m_dict_size) )
  {
    us_mz_uint i; d->m_pOutput_buf = pSaved_output_buf; d->m_bit_buffer = saved_bit_buf, d->m_bits_in = saved_bits_in;
    US_TDEFL_PUT_BITS(0, 2);
    if (d->m_bits_in) { US_TDEFL_PUT_BITS(0, 8 - d->m_bits_in); }
    for (i = 2; i; --i, d->m_total_lz_bytes ^= 0xFFFF)
    {
      US_TDEFL_PUT_BITS(d->m_total_lz_bytes & 0xFFFF, 16);
    }
    for (i = 0; i < d->m_total_lz_bytes; ++i)
    {
      US_TDEFL_PUT_BITS(d->m_dict[(d->m_lz_code_buf_dict_pos + i) & US_TDEFL_LZ_DICT_SIZE_MASK], 8);
    }
  }
  // Check for the extremely unlikely (if not impossible) case of the compressed block not fitting into the output buffer when using dynamic codes.
  else if (!comp_block_succeeded)
  {
    d->m_pOutput_buf = pSaved_output_buf; d->m_bit_buffer = saved_bit_buf, d->m_bits_in = saved_bits_in;
    us_tdefl_compress_block(d, US_MZ_TRUE);
  }

  if (flush)
  {
    if (flush == US_TDEFL_FINISH)
    {
      if (d->m_bits_in) { US_TDEFL_PUT_BITS(0, 8 - d->m_bits_in); }
      if (d->m_flags & US_TDEFL_WRITE_ZLIB_HEADER) { us_mz_uint i, a = d->m_adler32; for (i = 0; i < 4; i++) { US_TDEFL_PUT_BITS((a >> 24) & 0xFF, 8); a <<= 8; } }
    }
    else
    {
      us_mz_uint i, z = 0; US_TDEFL_PUT_BITS(0, 3); if (d->m_bits_in) { US_TDEFL_PUT_BITS(0, 8 - d->m_bits_in); } for (i = 2; i; --i, z ^= 0xFFFF) { US_TDEFL_PUT_BITS(z & 0xFFFF, 16); }
    }
  }

  US_MZ_ASSERT(d->m_pOutput_buf < d->m_pOutput_buf_end);

  memset(&d->m_huff_count[0][0], 0, sizeof(d->m_huff_count[0][0]) * US_TDEFL_MAX_HUFF_SYMBOLS_0);
  memset(&d->m_huff_count[1][0], 0, sizeof(d->m_huff_count[1][0]) * US_TDEFL_MAX_HUFF_SYMBOLS_1);

  d->m_pLZ_code_buf = d->m_lz_code_buf + 1; d->m_pLZ_flags = d->m_lz_code_buf; d->m_num_flags_left = 8; d->m_lz_code_buf_dict_pos += d->m_total_lz_bytes; d->m_total_lz_bytes = 0; d->m_block_index++;

  if ((n = (int)(d->m_pOutput_buf - pOutput_buf_start)) != 0)
  {
    if (d->m_pPut_buf_func)
    {
      *d->m_pIn_buf_size = d->m_pSrc - (const us_mz_uint8 *)d->m_pIn_buf;
      if (!(*d->m_pPut_buf_func)(d->m_output_buf, n, d->m_pPut_buf_user))
        return (d->m_prev_return_status = US_TDEFL_STATUS_PUT_BUF_FAILED);
    }
    else if (pOutput_buf_start == d->m_output_buf)
    {
      int bytes_to_copy = (int)US_MZ_MIN((size_t)n, (size_t)(*d->m_pOut_buf_size - d->m_out_buf_ofs));
      memcpy((us_mz_uint8 *)d->m_pOut_buf + d->m_out_buf_ofs, d->m_output_buf, bytes_to_copy);
      d->m_out_buf_ofs += bytes_to_copy;
      if ((n -= bytes_to_copy) != 0)
      {
        d->m_output_flush_ofs = bytes_to_copy;
        d->m_output_flush_remaining = n;
      }
    }
    else
    {
      d->m_out_buf_ofs += n;
    }
  }

  return d->m_output_flush_remaining;
}

#if US_MZ_USE_UNALIGNED_LOADS_AND_STORES
#define US_TDEFL_READ_UNALIGNED_WORD(p) *(const us_mz_uint16*)(p)
static US_MZ_FORCEINLINE void us_tdefl_find_match(us_tdefl_compressor *d, us_mz_uint lookahead_pos, us_mz_uint max_dist, us_mz_uint max_match_len, us_mz_uint *pMatch_dist, us_mz_uint *pMatch_len)
{
  us_mz_uint dist, pos = lookahead_pos & US_TDEFL_LZ_DICT_SIZE_MASK, match_len = *pMatch_len, probe_pos = pos, next_probe_pos, probe_len;
  us_mz_uint num_probes_left = d->m_max_probes[match_len >= 32];
  const us_mz_uint16 *s = (const us_mz_uint16*)(d->m_dict + pos), *p, *q;
  us_mz_uint16 c01 = US_TDEFL_READ_UNALIGNED_WORD(&d->m_dict[pos + match_len - 1]), s01 = US_TDEFL_READ_UNALIGNED_WORD(s);
  US_MZ_ASSERT(max_match_len <= US_TDEFL_MAX_MATCH_LEN); if (max_match_len <= match_len) return;
  for ( ; ; )
  {
    for ( ; ; )
    {
      if (--num_probes_left == 0) return;
      #define US_TDEFL_PROBE \
        next_probe_pos = d->m_next[probe_pos]; \
        if ((!next_probe_pos) || ((dist = (us_mz_uint16)(lookahead_pos - next_probe_pos)) > max_dist)) return; \
        probe_pos = next_probe_pos & US_TDEFL_LZ_DICT_SIZE_MASK; \
        if (US_TDEFL_READ_UNALIGNED_WORD(&d->m_dict[probe_pos + match_len - 1]) == c01) break;
      US_TDEFL_PROBE; US_TDEFL_PROBE; US_TDEFL_PROBE;
    }
    if (!dist) break;
    q = (const us_mz_uint16*)(d->m_dict + probe_pos);
    if (US_TDEFL_READ_UNALIGNED_WORD(q) != s01) continue;
    p = s; probe_len = 32;
    do { } while ( (US_TDEFL_READ_UNALIGNED_WORD(++p) == US_TDEFL_READ_UNALIGNED_WORD(++q)) && (US_TDEFL_READ_UNALIGNED_WORD(++p) == US_TDEFL_READ_UNALIGNED_WORD(++q)) &&
                   (US_TDEFL_READ_UNALIGNED_WORD(++p) == US_TDEFL_READ_UNALIGNED_WORD(++q)) && (US_TDEFL_READ_UNALIGNED_WORD(++p) == US_TDEFL_READ_UNALIGNED_WORD(++q)) && (--probe_len > 0) );
    if (!probe_len)
    {
      *pMatch_dist = dist; *pMatch_len = US_MZ_MIN(max_match_len, (us_mz_uint)US_TDEFL_MAX_MATCH_LEN); break;
    }
    else if ((probe_len = ((us_mz_uint)(p - s) * 2) + (us_mz_uint)(*(const us_mz_uint8*)p == *(const us_mz_uint8*)q)) > match_len)
    {
      *pMatch_dist = dist; if ((*pMatch_len = match_len = US_MZ_MIN(max_match_len, probe_len)) == max_match_len) break;
      c01 = US_TDEFL_READ_UNALIGNED_WORD(&d->m_dict[pos + match_len - 1]);
    }
  }
}
#else
static US_MZ_FORCEINLINE void us_tdefl_find_match(us_tdefl_compressor *d, us_mz_uint lookahead_pos, us_mz_uint max_dist, us_mz_uint max_match_len, us_mz_uint *pMatch_dist, us_mz_uint *pMatch_len)
{
  us_mz_uint dist, pos = lookahead_pos & US_TDEFL_LZ_DICT_SIZE_MASK, match_len = *pMatch_len, probe_pos = pos, next_probe_pos, probe_len;
  us_mz_uint num_probes_left = d->m_max_probes[match_len >= 32];
  const us_mz_uint8 *s = d->m_dict + pos, *p, *q;
  us_mz_uint8 c0 = d->m_dict[pos + match_len], c1 = d->m_dict[pos + match_len - 1];
  US_MZ_ASSERT(max_match_len <= US_TDEFL_MAX_MATCH_LEN); if (max_match_len <= match_len) return;
  for ( ; ; )
  {
    for ( ; ; )
    {
      if (--num_probes_left == 0) return;
      #define US_TDEFL_PROBE \
        next_probe_pos = d->m_next[probe_pos]; \
        if ((!next_probe_pos) || ((dist = (us_mz_uint16)(lookahead_pos - next_probe_pos)) > max_dist)) return; \
        probe_pos = next_probe_pos & US_TDEFL_LZ_DICT_SIZE_MASK; \
        if ((d->m_dict[probe_pos + match_len] == c0) && (d->m_dict[probe_pos + match_len - 1] == c1)) break;
      US_TDEFL_PROBE; US_TDEFL_PROBE; US_TDEFL_PROBE;
    }
    if (!dist) break; p = s; q = d->m_dict + probe_pos; for (probe_len = 0; probe_len < max_match_len; probe_len++) if (*p++ != *q++) break;
    if (probe_len > match_len)
    {
      *pMatch_dist = dist; if ((*pMatch_len = match_len = probe_len) == max_match_len) return;
      c0 = d->m_dict[pos + match_len]; c1 = d->m_dict[pos + match_len - 1];
    }
  }
}
#endif // #if US_MZ_USE_UNALIGNED_LOADS_AND_STORES

#if US_MZ_USE_UNALIGNED_LOADS_AND_STORES && US_MZ_LITTLE_ENDIAN
static us_mz_bool us_tdefl_compress_fast(us_tdefl_compressor *d)
{
  // Faster, minimally featured LZRW1-style match+parse loop with better register utilization. Intended for applications where raw throughput is valued more highly than ratio.
  us_mz_uint lookahead_pos = d->m_lookahead_pos, lookahead_size = d->m_lookahead_size, dict_size = d->m_dict_size, total_lz_bytes = d->m_total_lz_bytes, num_flags_left = d->m_num_flags_left;
  us_mz_uint8 *pLZ_code_buf = d->m_pLZ_code_buf, *pLZ_flags = d->m_pLZ_flags;
  us_mz_uint cur_pos = lookahead_pos & US_TDEFL_LZ_DICT_SIZE_MASK;

  while ((d->m_src_buf_left) || ((d->m_flush) && (lookahead_size)))
  {
    const us_mz_uint US_TDEFL_COMP_FAST_LOOKAHEAD_SIZE = 4096;
    us_mz_uint dst_pos = (lookahead_pos + lookahead_size) & US_TDEFL_LZ_DICT_SIZE_MASK;
    us_mz_uint num_bytes_to_process = (us_mz_uint)US_MZ_MIN(d->m_src_buf_left, US_TDEFL_COMP_FAST_LOOKAHEAD_SIZE - lookahead_size);
    d->m_src_buf_left -= num_bytes_to_process;
    lookahead_size += num_bytes_to_process;

    while (num_bytes_to_process)
    {
      us_mz_uint32 n = US_MZ_MIN(US_TDEFL_LZ_DICT_SIZE - dst_pos, num_bytes_to_process);
      memcpy(d->m_dict + dst_pos, d->m_pSrc, n);
      if (dst_pos < (US_TDEFL_MAX_MATCH_LEN - 1))
        memcpy(d->m_dict + US_TDEFL_LZ_DICT_SIZE + dst_pos, d->m_pSrc, US_MZ_MIN(n, (US_TDEFL_MAX_MATCH_LEN - 1) - dst_pos));
      d->m_pSrc += n;
      dst_pos = (dst_pos + n) & US_TDEFL_LZ_DICT_SIZE_MASK;
      num_bytes_to_process -= n;
    }

    dict_size = US_MZ_MIN(US_TDEFL_LZ_DICT_SIZE - lookahead_size, dict_size);
    if ((!d->m_flush) && (lookahead_size < US_TDEFL_COMP_FAST_LOOKAHEAD_SIZE)) break;

    while (lookahead_size >= 4)
    {
      us_mz_uint cur_match_dist, cur_match_len = 1;
      us_mz_uint8 *pCur_dict = d->m_dict + cur_pos;
      us_mz_uint first_trigram = (*(const us_mz_uint32 *)pCur_dict) & 0xFFFFFF;
      us_mz_uint hash = (first_trigram ^ (first_trigram >> (24 - (US_TDEFL_LZ_HASH_BITS - 8)))) & US_TDEFL_LEVEL1_HASH_SIZE_MASK;
      us_mz_uint probe_pos = d->m_hash[hash];
      d->m_hash[hash] = (us_mz_uint16)lookahead_pos;

      if (((cur_match_dist = (us_mz_uint16)(lookahead_pos - probe_pos)) <= dict_size) && ((*(const us_mz_uint32 *)(d->m_dict + (probe_pos &= US_TDEFL_LZ_DICT_SIZE_MASK)) & 0xFFFFFF) == first_trigram))
      {
        const us_mz_uint16 *p = (const us_mz_uint16 *)pCur_dict;
        const us_mz_uint16 *q = (const us_mz_uint16 *)(d->m_dict + probe_pos);
        us_mz_uint32 probe_len = 32;
        do { } while ( (US_TDEFL_READ_UNALIGNED_WORD(++p) == US_TDEFL_READ_UNALIGNED_WORD(++q)) && (US_TDEFL_READ_UNALIGNED_WORD(++p) == US_TDEFL_READ_UNALIGNED_WORD(++q)) &&
          (US_TDEFL_READ_UNALIGNED_WORD(++p) == US_TDEFL_READ_UNALIGNED_WORD(++q)) && (US_TDEFL_READ_UNALIGNED_WORD(++p) == US_TDEFL_READ_UNALIGNED_WORD(++q)) && (--probe_len > 0) );
        cur_match_len = ((us_mz_uint)(p - (const us_mz_uint16 *)pCur_dict) * 2) + (us_mz_uint)(*(const us_mz_uint8 *)p == *(const us_mz_uint8 *)q);
        if (!probe_len)
          cur_match_len = cur_match_dist ? US_TDEFL_MAX_MATCH_LEN : 0;

        if ((cur_match_len < US_TDEFL_MIN_MATCH_LEN) || ((cur_match_len == US_TDEFL_MIN_MATCH_LEN) && (cur_match_dist >= 8U*1024U)))
        {
          cur_match_len = 1;
          *pLZ_code_buf++ = (us_mz_uint8)first_trigram;
          *pLZ_flags = (us_mz_uint8)(*pLZ_flags >> 1);
          d->m_huff_count[0][(us_mz_uint8)first_trigram]++;
        }
        else
        {
          us_mz_uint32 s0, s1;
          cur_match_len = US_MZ_MIN(cur_match_len, lookahead_size);

          US_MZ_ASSERT((cur_match_len >= US_TDEFL_MIN_MATCH_LEN) && (cur_match_dist >= 1) && (cur_match_dist <= US_TDEFL_LZ_DICT_SIZE));

          cur_match_dist--;

          pLZ_code_buf[0] = (us_mz_uint8)(cur_match_len - US_TDEFL_MIN_MATCH_LEN);
          *(us_mz_uint16 *)(&pLZ_code_buf[1]) = (us_mz_uint16)cur_match_dist;
          pLZ_code_buf += 3;
          *pLZ_flags = (us_mz_uint8)((*pLZ_flags >> 1) | 0x80);

          s0 = s_us_tdefl_small_dist_sym[cur_match_dist & 511];
          s1 = s_us_tdefl_large_dist_sym[cur_match_dist >> 8];
          d->m_huff_count[1][(cur_match_dist < 512) ? s0 : s1]++;

          d->m_huff_count[0][s_us_tdefl_len_sym[cur_match_len - US_TDEFL_MIN_MATCH_LEN]]++;
        }
      }
      else
      {
        *pLZ_code_buf++ = (us_mz_uint8)first_trigram;
        *pLZ_flags = (us_mz_uint8)(*pLZ_flags >> 1);
        d->m_huff_count[0][(us_mz_uint8)first_trigram]++;
      }

      if (--num_flags_left == 0) { num_flags_left = 8; pLZ_flags = pLZ_code_buf++; }

      total_lz_bytes += cur_match_len;
      lookahead_pos += cur_match_len;
      dict_size = US_MZ_MIN(dict_size + cur_match_len, (us_mz_uint)US_TDEFL_LZ_DICT_SIZE);
      cur_pos = (cur_pos + cur_match_len) & US_TDEFL_LZ_DICT_SIZE_MASK;
      US_MZ_ASSERT(lookahead_size >= cur_match_len);
      lookahead_size -= cur_match_len;

      if (pLZ_code_buf > &d->m_lz_code_buf[US_TDEFL_LZ_CODE_BUF_SIZE - 8])
      {
        int n;
        d->m_lookahead_pos = lookahead_pos; d->m_lookahead_size = lookahead_size; d->m_dict_size = dict_size;
        d->m_total_lz_bytes = total_lz_bytes; d->m_pLZ_code_buf = pLZ_code_buf; d->m_pLZ_flags = pLZ_flags; d->m_num_flags_left = num_flags_left;
        if ((n = us_tdefl_flush_block(d, 0)) != 0)
          return (n < 0) ? US_MZ_FALSE : US_MZ_TRUE;
        total_lz_bytes = d->m_total_lz_bytes; pLZ_code_buf = d->m_pLZ_code_buf; pLZ_flags = d->m_pLZ_flags; num_flags_left = d->m_num_flags_left;
      }
    }

    while (lookahead_size)
    {
      us_mz_uint8 lit = d->m_dict[cur_pos];

      total_lz_bytes++;
      *pLZ_code_buf++ = lit;
      *pLZ_flags = (us_mz_uint8)(*pLZ_flags >> 1);
      if (--num_flags_left == 0) { num_flags_left = 8; pLZ_flags = pLZ_code_buf++; }

      d->m_huff_count[0][lit]++;

      lookahead_pos++;
      dict_size = US_MZ_MIN(dict_size + 1, (us_mz_uint)US_TDEFL_LZ_DICT_SIZE);
      cur_pos = (cur_pos + 1) & US_TDEFL_LZ_DICT_SIZE_MASK;
      lookahead_size--;

      if (pLZ_code_buf > &d->m_lz_code_buf[US_TDEFL_LZ_CODE_BUF_SIZE - 8])
      {
        int n;
        d->m_lookahead_pos = lookahead_pos; d->m_lookahead_size = lookahead_size; d->m_dict_size = dict_size;
        d->m_total_lz_bytes = total_lz_bytes; d->m_pLZ_code_buf = pLZ_code_buf; d->m_pLZ_flags = pLZ_flags; d->m_num_flags_left = num_flags_left;
        if ((n = us_tdefl_flush_block(d, 0)) != 0)
          return (n < 0) ? US_MZ_FALSE : US_MZ_TRUE;
        total_lz_bytes = d->m_total_lz_bytes; pLZ_code_buf = d->m_pLZ_code_buf; pLZ_flags = d->m_pLZ_flags; num_flags_left = d->m_num_flags_left;
      }
    }
  }

  d->m_lookahead_pos = lookahead_pos; d->m_lookahead_size = lookahead_size; d->m_dict_size = dict_size;
  d->m_total_lz_bytes = total_lz_bytes; d->m_pLZ_code_buf = pLZ_code_buf; d->m_pLZ_flags = pLZ_flags; d->m_num_flags_left = num_flags_left;
  return US_MZ_TRUE;
}
#endif // US_MZ_USE_UNALIGNED_LOADS_AND_STORES && US_MZ_LITTLE_ENDIAN

static US_MZ_FORCEINLINE void us_tdefl_record_literal(us_tdefl_compressor *d, us_mz_uint8 lit)
{
  d->m_total_lz_bytes++;
  *d->m_pLZ_code_buf++ = lit;
  *d->m_pLZ_flags = (us_mz_uint8)(*d->m_pLZ_flags >> 1); if (--d->m_num_flags_left == 0) { d->m_num_flags_left = 8; d->m_pLZ_flags = d->m_pLZ_code_buf++; }
  d->m_huff_count[0][lit]++;
}

static US_MZ_FORCEINLINE void us_tdefl_record_match(us_tdefl_compressor *d, us_mz_uint match_len, us_mz_uint match_dist)
{
  us_mz_uint32 s0, s1;

  US_MZ_ASSERT((match_len >= US_TDEFL_MIN_MATCH_LEN) && (match_dist >= 1) && (match_dist <= US_TDEFL_LZ_DICT_SIZE));

  d->m_total_lz_bytes += match_len;

  d->m_pLZ_code_buf[0] = (us_mz_uint8)(match_len - US_TDEFL_MIN_MATCH_LEN);

  match_dist -= 1;
  d->m_pLZ_code_buf[1] = (us_mz_uint8)(match_dist & 0xFF);
  d->m_pLZ_code_buf[2] = (us_mz_uint8)(match_dist >> 8); d->m_pLZ_code_buf += 3;

  *d->m_pLZ_flags = (us_mz_uint8)((*d->m_pLZ_flags >> 1) | 0x80); if (--d->m_num_flags_left == 0) { d->m_num_flags_left = 8; d->m_pLZ_flags = d->m_pLZ_code_buf++; }

  s0 = s_us_tdefl_small_dist_sym[match_dist & 511]; s1 = s_us_tdefl_large_dist_sym[(match_dist >> 8) & 127];
  d->m_huff_count[1][(match_dist < 512) ? s0 : s1]++;

  if (match_len >= US_TDEFL_MIN_MATCH_LEN) d->m_huff_count[0][s_us_tdefl_len_sym[match_len - US_TDEFL_MIN_MATCH_LEN]]++;
}

static us_mz_bool us_tdefl_compress_normal(us_tdefl_compressor *d)
{
  const us_mz_uint8 *pSrc = d->m_pSrc; size_t src_buf_left = d->m_src_buf_left;
  us_tdefl_flush flush = d->m_flush;

  while ((src_buf_left) || ((flush) && (d->m_lookahead_size)))
  {
    us_mz_uint len_to_move, cur_match_dist, cur_match_len, cur_pos;
    // Update dictionary and hash chains. Keeps the lookahead size equal to US_TDEFL_MAX_MATCH_LEN.
    if ((d->m_lookahead_size + d->m_dict_size) >= (US_TDEFL_MIN_MATCH_LEN - 1))
    {
      us_mz_uint dst_pos = (d->m_lookahead_pos + d->m_lookahead_size) & US_TDEFL_LZ_DICT_SIZE_MASK, ins_pos = d->m_lookahead_pos + d->m_lookahead_size - 2;
      us_mz_uint hash = (d->m_dict[ins_pos & US_TDEFL_LZ_DICT_SIZE_MASK] << US_TDEFL_LZ_HASH_SHIFT) ^ d->m_dict[(ins_pos + 1) & US_TDEFL_LZ_DICT_SIZE_MASK];
      us_mz_uint num_bytes_to_process = (us_mz_uint)US_MZ_MIN(src_buf_left, US_TDEFL_MAX_MATCH_LEN - d->m_lookahead_size);
      const us_mz_uint8 *pSrc_end = pSrc + num_bytes_to_process;
      src_buf_left -= num_bytes_to_process;
      d->m_lookahead_size += num_bytes_to_process;
      while (pSrc != pSrc_end)
      {
        us_mz_uint8 c = *pSrc++; d->m_dict[dst_pos] = c; if (dst_pos < (US_TDEFL_MAX_MATCH_LEN - 1)) d->m_dict[US_TDEFL_LZ_DICT_SIZE + dst_pos] = c;
        hash = ((hash << US_TDEFL_LZ_HASH_SHIFT) ^ c) & (US_TDEFL_LZ_HASH_SIZE - 1);
        d->m_next[ins_pos & US_TDEFL_LZ_DICT_SIZE_MASK] = d->m_hash[hash]; d->m_hash[hash] = (us_mz_uint16)(ins_pos);
        dst_pos = (dst_pos + 1) & US_TDEFL_LZ_DICT_SIZE_MASK; ins_pos++;
      }
    }
    else
    {
      while ((src_buf_left) && (d->m_lookahead_size < US_TDEFL_MAX_MATCH_LEN))
      {
        us_mz_uint8 c = *pSrc++;
        us_mz_uint dst_pos = (d->m_lookahead_pos + d->m_lookahead_size) & US_TDEFL_LZ_DICT_SIZE_MASK;
        src_buf_left--;
        d->m_dict[dst_pos] = c;
        if (dst_pos < (US_TDEFL_MAX_MATCH_LEN - 1))
          d->m_dict[US_TDEFL_LZ_DICT_SIZE + dst_pos] = c;
        if ((++d->m_lookahead_size + d->m_dict_size) >= US_TDEFL_MIN_MATCH_LEN)
        {
          us_mz_uint ins_pos = d->m_lookahead_pos + (d->m_lookahead_size - 1) - 2;
          us_mz_uint hash = ((d->m_dict[ins_pos & US_TDEFL_LZ_DICT_SIZE_MASK] << (US_TDEFL_LZ_HASH_SHIFT * 2)) ^ (d->m_dict[(ins_pos + 1) & US_TDEFL_LZ_DICT_SIZE_MASK] << US_TDEFL_LZ_HASH_SHIFT) ^ c) & (US_TDEFL_LZ_HASH_SIZE - 1);
          d->m_next[ins_pos & US_TDEFL_LZ_DICT_SIZE_MASK] = d->m_hash[hash]; d->m_hash[hash] = (us_mz_uint16)(ins_pos);
        }
      }
    }
    d->m_dict_size = US_MZ_MIN(US_TDEFL_LZ_DICT_SIZE - d->m_lookahead_size, d->m_dict_size);
    if ((!flush) && (d->m_lookahead_size < US_TDEFL_MAX_MATCH_LEN))
      break;

    // Simple lazy/greedy parsing state machine.
    len_to_move = 1; cur_match_dist = 0; cur_match_len = d->m_saved_match_len ? d->m_saved_match_len : (US_TDEFL_MIN_MATCH_LEN - 1); cur_pos = d->m_lookahead_pos & US_TDEFL_LZ_DICT_SIZE_MASK;
    if (d->m_flags & (US_TDEFL_RLE_MATCHES | US_TDEFL_FORCE_ALL_RAW_BLOCKS))
    {
      if ((d->m_dict_size) && (!(d->m_flags & US_TDEFL_FORCE_ALL_RAW_BLOCKS)))
      {
        us_mz_uint8 c = d->m_dict[(cur_pos - 1) & US_TDEFL_LZ_DICT_SIZE_MASK];
        cur_match_len = 0; while (cur_match_len < d->m_lookahead_size) { if (d->m_dict[cur_pos + cur_match_len] != c) break; cur_match_len++; }
        if (cur_match_len < US_TDEFL_MIN_MATCH_LEN) cur_match_len = 0; else cur_match_dist = 1;
      }
    }
    else
    {
      us_tdefl_find_match(d, d->m_lookahead_pos, d->m_dict_size, d->m_lookahead_size, &cur_match_dist, &cur_match_len);
    }
    if (((cur_match_len == US_TDEFL_MIN_MATCH_LEN) && (cur_match_dist >= 8U*1024U)) || (cur_pos == cur_match_dist) || ((d->m_flags & US_TDEFL_FILTER_MATCHES) && (cur_match_len <= 5)))
    {
      cur_match_dist = cur_match_len = 0;
    }
    if (d->m_saved_match_len)
    {
      if (cur_match_len > d->m_saved_match_len)
      {
        us_tdefl_record_literal(d, (us_mz_uint8)d->m_saved_lit);
        if (cur_match_len >= 128)
        {
          us_tdefl_record_match(d, cur_match_len, cur_match_dist);
          d->m_saved_match_len = 0; len_to_move = cur_match_len;
        }
        else
        {
          d->m_saved_lit = d->m_dict[cur_pos]; d->m_saved_match_dist = cur_match_dist; d->m_saved_match_len = cur_match_len;
        }
      }
      else
      {
        us_tdefl_record_match(d, d->m_saved_match_len, d->m_saved_match_dist);
        len_to_move = d->m_saved_match_len - 1; d->m_saved_match_len = 0;
      }
    }
    else if (!cur_match_dist)
      us_tdefl_record_literal(d, d->m_dict[US_MZ_MIN(cur_pos, sizeof(d->m_dict) - 1)]);
    else if ((d->m_greedy_parsing) || (d->m_flags & US_TDEFL_RLE_MATCHES) || (cur_match_len >= 128))
    {
      us_tdefl_record_match(d, cur_match_len, cur_match_dist);
      len_to_move = cur_match_len;
    }
    else
    {
      d->m_saved_lit = d->m_dict[US_MZ_MIN(cur_pos, sizeof(d->m_dict) - 1)]; d->m_saved_match_dist = cur_match_dist; d->m_saved_match_len = cur_match_len;
    }
    // Move the lookahead forward by len_to_move bytes.
    d->m_lookahead_pos += len_to_move;
    US_MZ_ASSERT(d->m_lookahead_size >= len_to_move);
    d->m_lookahead_size -= len_to_move;
    d->m_dict_size = US_MZ_MIN(d->m_dict_size + len_to_move, (us_mz_uint)US_TDEFL_LZ_DICT_SIZE);
    // Check if it's time to flush the current LZ codes to the internal output buffer.
    if ( (d->m_pLZ_code_buf > &d->m_lz_code_buf[US_TDEFL_LZ_CODE_BUF_SIZE - 8]) ||
         ( (d->m_total_lz_bytes > 31*1024) && (((((us_mz_uint)(d->m_pLZ_code_buf - d->m_lz_code_buf) * 115) >> 7) >= d->m_total_lz_bytes) || (d->m_flags & US_TDEFL_FORCE_ALL_RAW_BLOCKS))) )
    {
      int n;
      d->m_pSrc = pSrc; d->m_src_buf_left = src_buf_left;
      if ((n = us_tdefl_flush_block(d, 0)) != 0)
        return (n < 0) ? US_MZ_FALSE : US_MZ_TRUE;
    }
  }

  d->m_pSrc = pSrc; d->m_src_buf_left = src_buf_left;
  return US_MZ_TRUE;
}

static us_tdefl_status us_tdefl_flush_output_buffer(us_tdefl_compressor *d)
{
  if (d->m_pIn_buf_size)
  {
    *d->m_pIn_buf_size = d->m_pSrc - (const us_mz_uint8 *)d->m_pIn_buf;
  }

  if (d->m_pOut_buf_size)
  {
    size_t n = US_MZ_MIN(*d->m_pOut_buf_size - d->m_out_buf_ofs, d->m_output_flush_remaining);
    memcpy((us_mz_uint8 *)d->m_pOut_buf + d->m_out_buf_ofs, d->m_output_buf + d->m_output_flush_ofs, n);
    d->m_output_flush_ofs += (us_mz_uint)n;
    d->m_output_flush_remaining -= (us_mz_uint)n;
    d->m_out_buf_ofs += n;

    *d->m_pOut_buf_size = d->m_out_buf_ofs;
  }

  return (d->m_finished && !d->m_output_flush_remaining) ? US_TDEFL_STATUS_DONE : US_TDEFL_STATUS_OKAY;
}

us_tdefl_status us_tdefl_compress(us_tdefl_compressor *d, const void *pIn_buf, size_t *pIn_buf_size, void *pOut_buf, size_t *pOut_buf_size, us_tdefl_flush flush)
{
  if (!d)
  {
    if (pIn_buf_size) *pIn_buf_size = 0;
    if (pOut_buf_size) *pOut_buf_size = 0;
    return US_TDEFL_STATUS_BAD_PARAM;
  }

  d->m_pIn_buf = pIn_buf; d->m_pIn_buf_size = pIn_buf_size;
  d->m_pOut_buf = pOut_buf; d->m_pOut_buf_size = pOut_buf_size;
  d->m_pSrc = (const us_mz_uint8 *)(pIn_buf); d->m_src_buf_left = pIn_buf_size ? *pIn_buf_size : 0;
  d->m_out_buf_ofs = 0;
  d->m_flush = flush;

  if ( ((d->m_pPut_buf_func != NULL) == ((pOut_buf != NULL) || (pOut_buf_size != NULL))) || (d->m_prev_return_status != US_TDEFL_STATUS_OKAY) ||
        (d->m_wants_to_finish && (flush != US_TDEFL_FINISH)) || (pIn_buf_size && *pIn_buf_size && !pIn_buf) || (pOut_buf_size && *pOut_buf_size && !pOut_buf) )
  {
    if (pIn_buf_size) *pIn_buf_size = 0;
    if (pOut_buf_size) *pOut_buf_size = 0;
    return (d->m_prev_return_status = US_TDEFL_STATUS_BAD_PARAM);
  }
  d->m_wants_to_finish |= (flush == US_TDEFL_FINISH);

  if ((d->m_output_flush_remaining) || (d->m_finished))
    return (d->m_prev_return_status = us_tdefl_flush_output_buffer(d));

#if US_MZ_USE_UNALIGNED_LOADS_AND_STORES && US_MZ_LITTLE_ENDIAN
  if (((d->m_flags & US_TDEFL_MAX_PROBES_MASK) == 1) &&
      ((d->m_flags & US_TDEFL_GREEDY_PARSING_FLAG) != 0) &&
      ((d->m_flags & (US_TDEFL_FILTER_MATCHES | US_TDEFL_FORCE_ALL_RAW_BLOCKS | US_TDEFL_RLE_MATCHES)) == 0))
  {
    if (!us_tdefl_compress_fast(d))
      return d->m_prev_return_status;
  }
  else
#endif // #if US_MZ_USE_UNALIGNED_LOADS_AND_STORES && US_MZ_LITTLE_ENDIAN
  {
    if (!us_tdefl_compress_normal(d))
      return d->m_prev_return_status;
  }

  if ((d->m_flags & (US_TDEFL_WRITE_ZLIB_HEADER | US_TDEFL_COMPUTE_ADLER32)) && (pIn_buf))
    d->m_adler32 = (us_mz_uint32)us_mz_adler32(d->m_adler32, (const us_mz_uint8 *)pIn_buf, d->m_pSrc - (const us_mz_uint8 *)pIn_buf);

  if ((flush) && (!d->m_lookahead_size) && (!d->m_src_buf_left) && (!d->m_output_flush_remaining))
  {
    if (us_tdefl_flush_block(d, flush) < 0)
      return d->m_prev_return_status;
    d->m_finished = (flush == US_TDEFL_FINISH);
    if (flush == US_TDEFL_FULL_FLUSH) { US_MZ_CLEAR_OBJ(d->m_hash); US_MZ_CLEAR_OBJ(d->m_next); d->m_dict_size = 0; }
  }

  return (d->m_prev_return_status = us_tdefl_flush_output_buffer(d));
}

us_tdefl_status us_tdefl_compress_buffer(us_tdefl_compressor *d, const void *pIn_buf, size_t in_buf_size, us_tdefl_flush flush)
{
  US_MZ_ASSERT(d->m_pPut_buf_func); return us_tdefl_compress(d, pIn_buf, &in_buf_size, NULL, NULL, flush);
}

us_tdefl_status us_tdefl_init(us_tdefl_compressor *d, us_tdefl_put_buf_func_ptr pPut_buf_func, void *pPut_buf_user, int flags)
{
  d->m_pPut_buf_func = pPut_buf_func; d->m_pPut_buf_user = pPut_buf_user;
  d->m_flags = (us_mz_uint)(flags); d->m_max_probes[0] = 1 + ((flags & 0xFFF) + 2) / 3; d->m_greedy_parsing = (flags & US_TDEFL_GREEDY_PARSING_FLAG) != 0;
  d->m_max_probes[1] = 1 + (((flags & 0xFFF) >> 2) + 2) / 3;
  if (!(flags & US_TDEFL_NONDETERMINISTIC_PARSING_FLAG)) US_MZ_CLEAR_OBJ(d->m_hash);
  d->m_lookahead_pos = d->m_lookahead_size = d->m_dict_size = d->m_total_lz_bytes = d->m_lz_code_buf_dict_pos = d->m_bits_in = 0;
  d->m_output_flush_ofs = d->m_output_flush_remaining = d->m_finished = d->m_block_index = d->m_bit_buffer = d->m_wants_to_finish = 0;
  d->m_pLZ_code_buf = d->m_lz_code_buf + 1; d->m_pLZ_flags = d->m_lz_code_buf; d->m_num_flags_left = 8;
  d->m_pOutput_buf = d->m_output_buf; d->m_pOutput_buf_end = d->m_output_buf; d->m_prev_return_status = US_TDEFL_STATUS_OKAY;
  d->m_saved_match_dist = d->m_saved_match_len = d->m_saved_lit = 0; d->m_adler32 = 1;
  d->m_pIn_buf = NULL; d->m_pOut_buf = NULL;
  d->m_pIn_buf_size = NULL; d->m_pOut_buf_size = NULL;
  d->m_flush = US_TDEFL_NO_FLUSH; d->m_pSrc = NULL; d->m_src_buf_left = 0; d->m_out_buf_ofs = 0;
  memset(&d->m_huff_count[0][0], 0, sizeof(d->m_huff_count[0][0]) * US_TDEFL_MAX_HUFF_SYMBOLS_0);
  memset(&d->m_huff_count[1][0], 0, sizeof(d->m_huff_count[1][0]) * US_TDEFL_MAX_HUFF_SYMBOLS_1);
  return US_TDEFL_STATUS_OKAY;
}

us_tdefl_status us_tdefl_get_prev_return_status(us_tdefl_compressor *d)
{
  return d->m_prev_return_status;
}

us_mz_uint32 us_tdefl_get_adler32(us_tdefl_compressor *d)
{
  return d->m_adler32;
}

us_mz_bool us_tdefl_compress_mem_to_output(const void *pBuf, size_t buf_len, us_tdefl_put_buf_func_ptr pPut_buf_func, void *pPut_buf_user, int flags)
{
  us_tdefl_compressor *pComp; us_mz_bool succeeded; if (((buf_len) && (!pBuf)) || (!pPut_buf_func)) return US_MZ_FALSE;
  pComp = (us_tdefl_compressor*)US_MZ_MALLOC(sizeof(us_tdefl_compressor)); if (!pComp) return US_MZ_FALSE;
  succeeded = (us_tdefl_init(pComp, pPut_buf_func, pPut_buf_user, flags) == US_TDEFL_STATUS_OKAY);
  succeeded = succeeded && (us_tdefl_compress_buffer(pComp, pBuf, buf_len, US_TDEFL_FINISH) == US_TDEFL_STATUS_DONE);
  US_MZ_FREE(pComp); return succeeded;
}

typedef struct
{
  size_t m_size, m_capacity;
  us_mz_uint8 *m_pBuf;
  us_mz_bool m_expandable;
} us_tdefl_output_buffer;

static us_mz_bool us_tdefl_output_buffer_putter(const void *pBuf, int len, void *pUser)
{
  us_tdefl_output_buffer *p = (us_tdefl_output_buffer *)pUser;
  size_t new_size = p->m_size + len;
  if (new_size > p->m_capacity)
  {
    size_t new_capacity = p->m_capacity; us_mz_uint8 *pNew_buf; if (!p->m_expandable) return US_MZ_FALSE;
    do { new_capacity = US_MZ_MAX(128U, new_capacity << 1U); } while (new_size > new_capacity);
    pNew_buf = (us_mz_uint8*)US_MZ_REALLOC(p->m_pBuf, new_capacity); if (!pNew_buf) return US_MZ_FALSE;
    p->m_pBuf = pNew_buf; p->m_capacity = new_capacity;
  }
  memcpy((us_mz_uint8*)p->m_pBuf + p->m_size, pBuf, len); p->m_size = new_size;
  return US_MZ_TRUE;
}

void *us_tdefl_compress_mem_to_heap(const void *pSrc_buf, size_t src_buf_len, size_t *pOut_len, int flags)
{
  us_tdefl_output_buffer out_buf; US_MZ_CLEAR_OBJ(out_buf);
  if (!pOut_len) return US_MZ_FALSE; else *pOut_len = 0;
  out_buf.m_expandable = US_MZ_TRUE;
  if (!us_tdefl_compress_mem_to_output(pSrc_buf, src_buf_len, us_tdefl_output_buffer_putter, &out_buf, flags)) return NULL;
  *pOut_len = out_buf.m_size; return out_buf.m_pBuf;
}

size_t us_tdefl_compress_mem_to_mem(void *pOut_buf, size_t out_buf_len, const void *pSrc_buf, size_t src_buf_len, int flags)
{
  us_tdefl_output_buffer out_buf; US_MZ_CLEAR_OBJ(out_buf);
  if (!pOut_buf) return 0;
  out_buf.m_pBuf = (us_mz_uint8*)pOut_buf; out_buf.m_capacity = out_buf_len;
  if (!us_tdefl_compress_mem_to_output(pSrc_buf, src_buf_len, us_tdefl_output_buffer_putter, &out_buf, flags)) return 0;
  return out_buf.m_size;
}

#ifndef US_MZ_NO_ZLIB_APIS
static const us_mz_uint s_us_tdefl_num_probes[11] = { 0, 1, 6, 32,  16, 32, 128, 256,  512, 768, 1500 };

// level may actually range from [0,10] (10 is a "hidden" max level, where we want a bit more compression and it's fine if throughput to fall off a cliff on some files).
us_mz_uint us_tdefl_create_comp_flags_from_zip_params(int level, int window_bits, int strategy)
{
  us_mz_uint comp_flags = s_us_tdefl_num_probes[(level >= 0) ? US_MZ_MIN(10, level) : US_MZ_DEFAULT_LEVEL] | ((level <= 3) ? US_TDEFL_GREEDY_PARSING_FLAG : 0);
  if (window_bits > 0) comp_flags |= US_TDEFL_WRITE_ZLIB_HEADER;

  if (!level) comp_flags |= US_TDEFL_FORCE_ALL_RAW_BLOCKS;
  else if (strategy == US_MZ_FILTERED) comp_flags |= US_TDEFL_FILTER_MATCHES;
  else if (strategy == US_MZ_HUFFMAN_ONLY) comp_flags &= ~US_TDEFL_MAX_PROBES_MASK;
  else if (strategy == US_MZ_FIXED) comp_flags |= US_TDEFL_FORCE_ALL_STATIC_BLOCKS;
  else if (strategy == US_MZ_RLE) comp_flags |= US_TDEFL_RLE_MATCHES;

  return comp_flags;
}
#endif //US_MZ_NO_ZLIB_APIS

#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable:4204) // nonstandard extension used : non-constant aggregate initializer (also supported by GNU C and C99, so no big deal)
#endif

// Simple PNG writer function by Alex Evans, 2011. Released into the public domain: https://gist.github.com/908299
// This is actually a modification of Alex's original code so PNG files generated by this function pass pngcheck.
void *us_tdefl_write_image_to_png_file_in_memory_ex(const void *pImage, int w, int h, int num_chans, size_t *pLen_out, us_mz_uint level, us_mz_bool flip)
{
  // Using a local copy of this array here in case US_MZ_NO_ZLIB_APIS was defined.
  static const us_mz_uint s_us_tdefl_png_num_probes[11] = { 0, 1, 6, 32,  16, 32, 128, 256,  512, 768, 1500 };
  us_tdefl_compressor *pComp = (us_tdefl_compressor *)US_MZ_MALLOC(sizeof(us_tdefl_compressor)); us_tdefl_output_buffer out_buf; int i, bpl = w * num_chans, y, z; us_mz_uint32 c; *pLen_out = 0;
  if (!pComp) return NULL;
  US_MZ_CLEAR_OBJ(out_buf); out_buf.m_expandable = US_MZ_TRUE; out_buf.m_capacity = 57+US_MZ_MAX(64, (1+bpl)*h); if (NULL == (out_buf.m_pBuf = (us_mz_uint8*)US_MZ_MALLOC(out_buf.m_capacity))) { US_MZ_FREE(pComp); return NULL; }
  // write dummy header
  for (z = 41; z; --z) us_tdefl_output_buffer_putter(&z, 1, &out_buf);
  // compress image data
  us_tdefl_init(pComp, us_tdefl_output_buffer_putter, &out_buf, s_us_tdefl_png_num_probes[US_MZ_MIN(10, level)] | US_TDEFL_WRITE_ZLIB_HEADER);
  for (y = 0; y < h; ++y) { us_tdefl_compress_buffer(pComp, &z, 1, US_TDEFL_NO_FLUSH); us_tdefl_compress_buffer(pComp, (us_mz_uint8*)pImage + (flip ? (h - 1 - y) : y) * bpl, bpl, US_TDEFL_NO_FLUSH); }
  if (us_tdefl_compress_buffer(pComp, NULL, 0, US_TDEFL_FINISH) != US_TDEFL_STATUS_DONE) { US_MZ_FREE(pComp); US_MZ_FREE(out_buf.m_pBuf); return NULL; }
  // write real header
  *pLen_out = out_buf.m_size-41;
  {
    static const us_mz_uint8 chans[] = {0x00, 0x00, 0x04, 0x02, 0x06};
    us_mz_uint8 pnghdr[41]={0x89,0x50,0x4e,0x47,0x0d,0x0a,0x1a,0x0a,0x00,0x00,0x00,0x0d,0x49,0x48,0x44,0x52,
      0,0,(us_mz_uint8)(w>>8),(us_mz_uint8)w,0,0,(us_mz_uint8)(h>>8),(us_mz_uint8)h,8,chans[num_chans],0,0,0,0,0,0,0,
      (us_mz_uint8)(*pLen_out>>24),(us_mz_uint8)(*pLen_out>>16),(us_mz_uint8)(*pLen_out>>8),(us_mz_uint8)*pLen_out,0x49,0x44,0x41,0x54};
    c=(us_mz_uint32)us_mz_crc32(US_MZ_CRC32_INIT,pnghdr+12,17); for (i=0; i<4; ++i, c<<=8) ((us_mz_uint8*)(pnghdr+29))[i]=(us_mz_uint8)(c>>24);
    memcpy(out_buf.m_pBuf, pnghdr, 41);
  }
  // write footer (IDAT CRC-32, followed by IEND chunk)
  if (!us_tdefl_output_buffer_putter("\0\0\0\0\0\0\0\0\x49\x45\x4e\x44\xae\x42\x60\x82", 16, &out_buf)) { *pLen_out = 0; US_MZ_FREE(pComp); US_MZ_FREE(out_buf.m_pBuf); return NULL; }
  c = (us_mz_uint32)us_mz_crc32(US_MZ_CRC32_INIT,out_buf.m_pBuf+41-4, *pLen_out+4); for (i=0; i<4; ++i, c<<=8) (out_buf.m_pBuf+out_buf.m_size-16)[i] = (us_mz_uint8)(c >> 24);
  // compute final size of file, grab compressed data buffer and return
  *pLen_out += 57; US_MZ_FREE(pComp); return out_buf.m_pBuf;
}
void *us_tdefl_write_image_to_png_file_in_memory(const void *pImage, int w, int h, int num_chans, size_t *pLen_out)
{
  // Level 6 corresponds to US_TDEFL_DEFAULT_MAX_PROBES or US_MZ_DEFAULT_LEVEL (but we can't depend on US_MZ_DEFAULT_LEVEL being available in case the zlib API's where #defined out)
  return us_tdefl_write_image_to_png_file_in_memory_ex(pImage, w, h, num_chans, pLen_out, 6, US_MZ_FALSE);
}

#ifdef _MSC_VER
#pragma warning (pop)
#endif

// ------------------- .ZIP archive reading

#ifndef US_MZ_NO_ARCHIVE_APIS

#ifdef US_MZ_NO_STDIO
  #define US_MZ_FILE void *
#else
  #include <stdio.h>
  #include <sys/stat.h>

  #if defined(_MSC_VER) || defined(__MINGW64__)
    static FILE *us_mz_fopen(const char *pFilename, const char *pMode)
    {
      FILE* pFile = NULL;
      fopen_s(&pFile, pFilename, pMode);
      return pFile;
    }
    static FILE *us_mz_freopen(const char *pPath, const char *pMode, FILE *pStream)
    {
      FILE* pFile = NULL;
      if (freopen_s(&pFile, pPath, pMode, pStream))
        return NULL;
      return pFile;
    }
    #ifndef US_MZ_NO_TIME
      #include <sys/utime.h>
    #endif
    #define US_MZ_FILE FILE
    #define US_MZ_FOPEN us_mz_fopen
    #define US_MZ_FCLOSE fclose
    #define US_MZ_FREAD fread
    #define US_MZ_FWRITE fwrite
    #define US_MZ_FTELL64 _ftelli64
    #define US_MZ_FSEEK64 _fseeki64
    #define US_MZ_FILE_STAT_STRUCT _stat
    #define US_MZ_FILE_STAT _stat
    #define US_MZ_FFLUSH fflush
    #define US_MZ_FREOPEN us_mz_freopen
    #define US_MZ_DELETE_FILE remove
  #elif defined(__MINGW32__)
    #ifndef US_MZ_NO_TIME
      #include <sys/utime.h>
    #endif
    #define US_MZ_FILE FILE
    #define US_MZ_FOPEN(f, m) fopen(f, m)
    #define US_MZ_FCLOSE fclose
    #define US_MZ_FREAD fread
    #define US_MZ_FWRITE fwrite
    #define US_MZ_FTELL64 ftello64
    #define US_MZ_FSEEK64 fseeko64
    #define US_MZ_FILE_STAT_STRUCT _stat
    #define US_MZ_FILE_STAT _stat
    #define US_MZ_FFLUSH fflush
    #define US_MZ_FREOPEN(f, m, s) freopen(f, m, s)
    #define US_MZ_DELETE_FILE remove
  #elif defined(__TINYC__)
    #ifndef US_MZ_NO_TIME
      #include <sys/utime.h>
    #endif
    #define US_MZ_FILE FILE
    #define US_MZ_FOPEN(f, m) fopen(f, m)
    #define US_MZ_FCLOSE fclose
    #define US_MZ_FREAD fread
    #define US_MZ_FWRITE fwrite
    #define US_MZ_FTELL64 ftell
    #define US_MZ_FSEEK64 fseek
    #define US_MZ_FILE_STAT_STRUCT stat
    #define US_MZ_FILE_STAT stat
    #define US_MZ_FFLUSH fflush
    #define US_MZ_FREOPEN(f, m, s) freopen(f, m, s)
    #define US_MZ_DELETE_FILE remove
  #elif defined(__GNUC__) && _LARGEFILE64_SOURCE
    #ifndef US_MZ_NO_TIME
      #include <utime.h>
    #endif
    #define US_MZ_FILE FILE
    #define US_MZ_FOPEN(f, m) fopen64(f, m)
    #define US_MZ_FCLOSE fclose
    #define US_MZ_FREAD fread
    #define US_MZ_FWRITE fwrite
    #define US_MZ_FTELL64 ftello64
    #define US_MZ_FSEEK64 fseeko64
    #define US_MZ_FILE_STAT_STRUCT stat64
    #define US_MZ_FILE_STAT stat64
    #define US_MZ_FFLUSH fflush
    #define US_MZ_FREOPEN(p, m, s) freopen64(p, m, s)
    #define US_MZ_DELETE_FILE remove
  #else
    #ifndef US_MZ_NO_TIME
      #include <utime.h>
    #endif
    #define US_MZ_FILE FILE
    #define US_MZ_FOPEN(f, m) fopen(f, m)
    #define US_MZ_FCLOSE fclose
    #define US_MZ_FREAD fread
    #define US_MZ_FWRITE fwrite
    #define US_MZ_FTELL64 ftello
    #define US_MZ_FSEEK64 fseeko
    #define US_MZ_FILE_STAT_STRUCT stat
    #define US_MZ_FILE_STAT stat
    #define US_MZ_FFLUSH fflush
    #define US_MZ_FREOPEN(f, m, s) freopen(f, m, s)
    #define US_MZ_DELETE_FILE remove
  #endif // #ifdef _MSC_VER
#endif // #ifdef US_MZ_NO_STDIO

#define US_MZ_TOLOWER(c) ((((c) >= 'A') && ((c) <= 'Z')) ? ((c) - 'A' + 'a') : (c))

// Various ZIP archive enums. To completely avoid cross platform compiler alignment and platform endian issues, miniz.c doesn't use structs for any of this stuff.
enum
{
  // ZIP archive identifiers and record sizes
  US_MZ_ZIP_END_OF_CENTRAL_DIR_HEADER_SIG = 0x06054b50, US_MZ_ZIP_CENTRAL_DIR_HEADER_SIG = 0x02014b50, US_MZ_ZIP_LOCAL_DIR_HEADER_SIG = 0x04034b50,
  US_MZ_ZIP_LOCAL_DIR_HEADER_SIZE = 30, US_MZ_ZIP_CENTRAL_DIR_HEADER_SIZE = 46, US_MZ_ZIP_END_OF_CENTRAL_DIR_HEADER_SIZE = 22,
  // Central directory header record offsets
  US_MZ_ZIP_CDH_SIG_OFS = 0, US_MZ_ZIP_CDH_VERSION_MADE_BY_OFS = 4, US_MZ_ZIP_CDH_VERSION_NEEDED_OFS = 6, US_MZ_ZIP_CDH_BIT_FLAG_OFS = 8,
  US_MZ_ZIP_CDH_METHOD_OFS = 10, US_MZ_ZIP_CDH_FILE_TIME_OFS = 12, US_MZ_ZIP_CDH_FILE_DATE_OFS = 14, US_MZ_ZIP_CDH_CRC32_OFS = 16,
  US_MZ_ZIP_CDH_COMPRESSED_SIZE_OFS = 20, US_MZ_ZIP_CDH_DECOMPRESSED_SIZE_OFS = 24, US_MZ_ZIP_CDH_FILENAME_LEN_OFS = 28, US_MZ_ZIP_CDH_EXTRA_LEN_OFS = 30,
  US_MZ_ZIP_CDH_COMMENT_LEN_OFS = 32, US_MZ_ZIP_CDH_DISK_START_OFS = 34, US_MZ_ZIP_CDH_INTERNAL_ATTR_OFS = 36, US_MZ_ZIP_CDH_EXTERNAL_ATTR_OFS = 38, US_MZ_ZIP_CDH_LOCAL_HEADER_OFS = 42,
  // Local directory header offsets
  US_MZ_ZIP_LDH_SIG_OFS = 0, US_MZ_ZIP_LDH_VERSION_NEEDED_OFS = 4, US_MZ_ZIP_LDH_BIT_FLAG_OFS = 6, US_MZ_ZIP_LDH_METHOD_OFS = 8, US_MZ_ZIP_LDH_FILE_TIME_OFS = 10,
  US_MZ_ZIP_LDH_FILE_DATE_OFS = 12, US_MZ_ZIP_LDH_CRC32_OFS = 14, US_MZ_ZIP_LDH_COMPRESSED_SIZE_OFS = 18, US_MZ_ZIP_LDH_DECOMPRESSED_SIZE_OFS = 22,
  US_MZ_ZIP_LDH_FILENAME_LEN_OFS = 26, US_MZ_ZIP_LDH_EXTRA_LEN_OFS = 28,
  // End of central directory offsets
  US_MZ_ZIP_ECDH_SIG_OFS = 0, US_MZ_ZIP_ECDH_NUM_THIS_DISK_OFS = 4, US_MZ_ZIP_ECDH_NUM_DISK_CDIR_OFS = 6, US_MZ_ZIP_ECDH_CDIR_NUM_ENTRIES_ON_DISK_OFS = 8,
  US_MZ_ZIP_ECDH_CDIR_TOTAL_ENTRIES_OFS = 10, US_MZ_ZIP_ECDH_CDIR_SIZE_OFS = 12, US_MZ_ZIP_ECDH_CDIR_OFS_OFS = 16, US_MZ_ZIP_ECDH_COMMENT_SIZE_OFS = 20,
};

typedef struct
{
  void *m_p;
  size_t m_size, m_capacity;
  us_mz_uint m_element_size;
} us_mz_zip_array;

struct us_mz_zip_internal_state_tag
{
  us_mz_zip_array m_central_dir;
  us_mz_zip_array m_central_dir_offsets;
  us_mz_zip_array m_sorted_central_dir_offsets;
  US_MZ_FILE *m_pFile;
  void *m_pMem;
  size_t m_mem_size;
  size_t m_mem_capacity;
};

#define US_MZ_ZIP_ARRAY_SET_ELEMENT_SIZE(array_ptr, element_size) (array_ptr)->m_element_size = element_size
#define US_MZ_ZIP_ARRAY_ELEMENT(array_ptr, element_type, index) ((element_type *)((array_ptr)->m_p))[index]

static US_MZ_FORCEINLINE void us_mz_zip_array_clear(us_mz_zip_archive *pZip, us_mz_zip_array *pArray)
{
  pZip->m_pFree(pZip->m_pAlloc_opaque, pArray->m_p);
  memset(pArray, 0, sizeof(us_mz_zip_array));
}

static us_mz_bool us_mz_zip_array_ensure_capacity(us_mz_zip_archive *pZip, us_mz_zip_array *pArray, size_t min_new_capacity, us_mz_uint growing)
{
  void *pNew_p; size_t new_capacity = min_new_capacity; US_MZ_ASSERT(pArray->m_element_size); if (pArray->m_capacity >= min_new_capacity) return US_MZ_TRUE;
  if (growing) { new_capacity = US_MZ_MAX(1, pArray->m_capacity); while (new_capacity < min_new_capacity) new_capacity *= 2; }
  if (NULL == (pNew_p = pZip->m_pRealloc(pZip->m_pAlloc_opaque, pArray->m_p, pArray->m_element_size, new_capacity))) return US_MZ_FALSE;
  pArray->m_p = pNew_p; pArray->m_capacity = new_capacity;
  return US_MZ_TRUE;
}

static US_MZ_FORCEINLINE us_mz_bool us_mz_zip_array_reserve(us_mz_zip_archive *pZip, us_mz_zip_array *pArray, size_t new_capacity, us_mz_uint growing)
{
  if (new_capacity > pArray->m_capacity) { if (!us_mz_zip_array_ensure_capacity(pZip, pArray, new_capacity, growing)) return US_MZ_FALSE; }
  return US_MZ_TRUE;
}

static US_MZ_FORCEINLINE us_mz_bool us_mz_zip_array_resize(us_mz_zip_archive *pZip, us_mz_zip_array *pArray, size_t new_size, us_mz_uint growing)
{
  if (new_size > pArray->m_capacity) { if (!us_mz_zip_array_ensure_capacity(pZip, pArray, new_size, growing)) return US_MZ_FALSE; }
  pArray->m_size = new_size;
  return US_MZ_TRUE;
}

static US_MZ_FORCEINLINE us_mz_bool us_mz_zip_array_ensure_room(us_mz_zip_archive *pZip, us_mz_zip_array *pArray, size_t n)
{
  return us_mz_zip_array_reserve(pZip, pArray, pArray->m_size + n, US_MZ_TRUE);
}

static US_MZ_FORCEINLINE us_mz_bool us_mz_zip_array_push_back(us_mz_zip_archive *pZip, us_mz_zip_array *pArray, const void *pElements, size_t n)
{
  size_t orig_size = pArray->m_size; if (!us_mz_zip_array_resize(pZip, pArray, orig_size + n, US_MZ_TRUE)) return US_MZ_FALSE;
  memcpy((us_mz_uint8*)pArray->m_p + orig_size * pArray->m_element_size, pElements, n * pArray->m_element_size);
  return US_MZ_TRUE;
}

#ifndef US_MZ_NO_TIME
static time_t us_mz_zip_dos_to_time_t(int dos_time, int dos_date)
{
  struct tm tm;
  memset(&tm, 0, sizeof(tm)); tm.tm_isdst = -1;
  tm.tm_year = ((dos_date >> 9) & 127) + 1980 - 1900; tm.tm_mon = ((dos_date >> 5) & 15) - 1; tm.tm_mday = dos_date & 31;
  tm.tm_hour = (dos_time >> 11) & 31; tm.tm_min = (dos_time >> 5) & 63; tm.tm_sec = (dos_time << 1) & 62;
  return mktime(&tm);
}

static void us_mz_zip_time_to_dos_time(time_t time, us_mz_uint16 *pDOS_time, us_mz_uint16 *pDOS_date)
{
#ifdef _MSC_VER
  struct tm tm_struct;
  struct tm *tm = &tm_struct;
  errno_t err = localtime_s(tm, &time);
  if (err)
  {
    *pDOS_date = 0; *pDOS_time = 0;
    return;
  }
#else
  struct tm *tm = localtime(&time);
#endif
  *pDOS_time = (us_mz_uint16)(((tm->tm_hour) << 11) + ((tm->tm_min) << 5) + ((tm->tm_sec) >> 1));
  *pDOS_date = (us_mz_uint16)(((tm->tm_year + 1900 - 1980) << 9) + ((tm->tm_mon + 1) << 5) + tm->tm_mday);
}
#endif

#ifndef US_MZ_NO_STDIO
static us_mz_bool us_mz_zip_get_file_modified_time(const char *pFilename, us_mz_uint16 *pDOS_time, us_mz_uint16 *pDOS_date)
{
#ifdef US_MZ_NO_TIME
  (void)pFilename; *pDOS_date = *pDOS_time = 0;
#else
  struct US_MZ_FILE_STAT_STRUCT file_stat;
  // On Linux with x86 glibc, this call will fail on large files (>= 0x80000000 bytes) unless you compiled with _LARGEFILE64_SOURCE. Argh.
  if (US_MZ_FILE_STAT(pFilename, &file_stat) != 0)
    return US_MZ_FALSE;
  us_mz_zip_time_to_dos_time(file_stat.st_mtime, pDOS_time, pDOS_date);
#endif // #ifdef US_MZ_NO_TIME
  return US_MZ_TRUE;
}

#ifndef US_MZ_NO_TIME
static us_mz_bool us_mz_zip_set_file_times(const char *pFilename, time_t access_time, time_t modified_time)
{
  struct utimbuf t; t.actime = access_time; t.modtime = modified_time;
  return !utime(pFilename, &t);
}
#endif // #ifndef US_MZ_NO_TIME
#endif // #ifndef US_MZ_NO_STDIO

static us_mz_bool us_mz_zip_reader_init_internal(us_mz_zip_archive *pZip, us_mz_uint32 flags)
{
  (void)flags;
  if ((!pZip) || (pZip->m_pState) || (pZip->m_zip_mode != US_MZ_ZIP_MODE_INVALID))
    return US_MZ_FALSE;

  if (!pZip->m_pAlloc) pZip->m_pAlloc = def_alloc_func;
  if (!pZip->m_pFree) pZip->m_pFree = def_free_func;
  if (!pZip->m_pRealloc) pZip->m_pRealloc = def_realloc_func;

  pZip->m_zip_mode = US_MZ_ZIP_MODE_READING;
  pZip->m_archive_size = 0;
  pZip->m_central_directory_file_ofs = 0;
  pZip->m_total_files = 0;

  if (NULL == (pZip->m_pState = (us_mz_zip_internal_state *)pZip->m_pAlloc(pZip->m_pAlloc_opaque, 1, sizeof(us_mz_zip_internal_state))))
    return US_MZ_FALSE;
  memset(pZip->m_pState, 0, sizeof(us_mz_zip_internal_state));
  US_MZ_ZIP_ARRAY_SET_ELEMENT_SIZE(&pZip->m_pState->m_central_dir, sizeof(us_mz_uint8));
  US_MZ_ZIP_ARRAY_SET_ELEMENT_SIZE(&pZip->m_pState->m_central_dir_offsets, sizeof(us_mz_uint32));
  US_MZ_ZIP_ARRAY_SET_ELEMENT_SIZE(&pZip->m_pState->m_sorted_central_dir_offsets, sizeof(us_mz_uint32));
  return US_MZ_TRUE;
}

static US_MZ_FORCEINLINE us_mz_bool us_mz_zip_reader_filename_less(const us_mz_zip_array *pCentral_dir_array, const us_mz_zip_array *pCentral_dir_offsets, us_mz_uint l_index, us_mz_uint r_index)
{
  const us_mz_uint8 *pL = &US_MZ_ZIP_ARRAY_ELEMENT(pCentral_dir_array, us_mz_uint8, US_MZ_ZIP_ARRAY_ELEMENT(pCentral_dir_offsets, us_mz_uint32, l_index)), *pE;
  const us_mz_uint8 *pR = &US_MZ_ZIP_ARRAY_ELEMENT(pCentral_dir_array, us_mz_uint8, US_MZ_ZIP_ARRAY_ELEMENT(pCentral_dir_offsets, us_mz_uint32, r_index));
  us_mz_uint l_len = US_MZ_READ_LE16(pL + US_MZ_ZIP_CDH_FILENAME_LEN_OFS), r_len = US_MZ_READ_LE16(pR + US_MZ_ZIP_CDH_FILENAME_LEN_OFS);
  us_mz_uint8 l = 0, r = 0;
  pL += US_MZ_ZIP_CENTRAL_DIR_HEADER_SIZE; pR += US_MZ_ZIP_CENTRAL_DIR_HEADER_SIZE;
  pE = pL + US_MZ_MIN(l_len, r_len);
  while (pL < pE)
  {
    if ((l = US_MZ_TOLOWER(*pL)) != (r = US_MZ_TOLOWER(*pR)))
      break;
    pL++; pR++;
  }
  return (pL == pE) ? (l_len < r_len) : (l < r);
}

#define US_MZ_SWAP_UINT32(a, b) do { us_mz_uint32 t = a; a = b; b = t; } US_MZ_MACRO_END

// Heap sort of lowercased filenames, used to help accelerate plain central directory searches by us_mz_zip_reader_locate_file(). (Could also use qsort(), but it could allocate memory.)
static void us_mz_zip_reader_sort_central_dir_offsets_by_filename(us_mz_zip_archive *pZip)
{
  us_mz_zip_internal_state *pState = pZip->m_pState;
  const us_mz_zip_array *pCentral_dir_offsets = &pState->m_central_dir_offsets;
  const us_mz_zip_array *pCentral_dir = &pState->m_central_dir;
  us_mz_uint32 *pIndices = &US_MZ_ZIP_ARRAY_ELEMENT(&pState->m_sorted_central_dir_offsets, us_mz_uint32, 0);
  const int size = pZip->m_total_files;
  int start = (size - 2) >> 1, end;
  while (start >= 0)
  {
    int child, root = start;
    for ( ; ; )
    {
      if ((child = (root << 1) + 1) >= size)
        break;
      child += (((child + 1) < size) && (us_mz_zip_reader_filename_less(pCentral_dir, pCentral_dir_offsets, pIndices[child], pIndices[child + 1])));
      if (!us_mz_zip_reader_filename_less(pCentral_dir, pCentral_dir_offsets, pIndices[root], pIndices[child]))
        break;
      US_MZ_SWAP_UINT32(pIndices[root], pIndices[child]); root = child;
    }
    start--;
  }

  end = size - 1;
  while (end > 0)
  {
    int child, root = 0;
    US_MZ_SWAP_UINT32(pIndices[end], pIndices[0]);
    for ( ; ; )
    {
      if ((child = (root << 1) + 1) >= end)
        break;
      child += (((child + 1) < end) && us_mz_zip_reader_filename_less(pCentral_dir, pCentral_dir_offsets, pIndices[child], pIndices[child + 1]));
      if (!us_mz_zip_reader_filename_less(pCentral_dir, pCentral_dir_offsets, pIndices[root], pIndices[child]))
        break;
      US_MZ_SWAP_UINT32(pIndices[root], pIndices[child]); root = child;
    }
    end--;
  }
}

static us_mz_bool us_mz_zip_reader_read_central_dir(us_mz_zip_archive *pZip, us_mz_uint32 flags)
{
  us_mz_uint cdir_size, num_this_disk, cdir_disk_index;
  us_mz_uint64 cdir_ofs;
  us_mz_int64 cur_file_ofs;
  const us_mz_uint8 *p;
  us_mz_uint32 buf_u32[4096 / sizeof(us_mz_uint32)]; us_mz_uint8 *pBuf = (us_mz_uint8 *)buf_u32;
  us_mz_bool sort_central_dir = ((flags & US_MZ_ZIP_FLAG_DO_NOT_SORT_CENTRAL_DIRECTORY) == 0);
  us_mz_bool zip_signature_found = 0;
  // Basic sanity checks - reject files which are too small, and check the first 4 bytes of the file to make sure a local header is there.
  if (pZip->m_archive_size < US_MZ_ZIP_END_OF_CENTRAL_DIR_HEADER_SIZE)
    return US_MZ_FALSE;
  // Find the end of central directory record by scanning the file from the end towards the beginning.
  cur_file_ofs = US_MZ_MAX((us_mz_int64)pZip->m_archive_size - (us_mz_int64)sizeof(buf_u32), 0);
  for ( ; ; )
  {
    int i, n = (int)US_MZ_MIN(sizeof(buf_u32), pZip->m_archive_size - cur_file_ofs);
    if (pZip->m_pRead(pZip->m_pIO_opaque, cur_file_ofs, pBuf, n) != (us_mz_uint)n)
      return US_MZ_FALSE;
    for (i = n - 4; i >= 0; --i)
    {
      if (US_MZ_READ_LE32(pBuf + i) == US_MZ_ZIP_END_OF_CENTRAL_DIR_HEADER_SIG)
      {
        // Read and verify the end of central directory record.
        if (pZip->m_pRead(pZip->m_pIO_opaque, cur_file_ofs + i, pBuf, US_MZ_ZIP_END_OF_CENTRAL_DIR_HEADER_SIZE) != US_MZ_ZIP_END_OF_CENTRAL_DIR_HEADER_SIZE)
          continue;
        if ((US_MZ_READ_LE32(pBuf + US_MZ_ZIP_ECDH_SIG_OFS) != US_MZ_ZIP_END_OF_CENTRAL_DIR_HEADER_SIG) ||
        ((pZip->m_total_files = US_MZ_READ_LE16(pBuf + US_MZ_ZIP_ECDH_CDIR_TOTAL_ENTRIES_OFS)) != US_MZ_READ_LE16(pBuf + US_MZ_ZIP_ECDH_CDIR_NUM_ENTRIES_ON_DISK_OFS)))
          continue;
        zip_signature_found = 1;
        break;
      }
    }

    if (zip_signature_found)
    {
      cur_file_ofs += i;
      break;
    }
    if ((!cur_file_ofs) || (cur_file_ofs < US_MZ_ZIP_END_OF_CENTRAL_DIR_HEADER_SIZE))
      return US_MZ_FALSE;

    cur_file_ofs = US_MZ_MAX(cur_file_ofs - (us_mz_int64)(sizeof(buf_u32) - 3), 0);
  }

  num_this_disk = US_MZ_READ_LE16(pBuf + US_MZ_ZIP_ECDH_NUM_THIS_DISK_OFS);
  cdir_disk_index = US_MZ_READ_LE16(pBuf + US_MZ_ZIP_ECDH_NUM_DISK_CDIR_OFS);
  if (((num_this_disk | cdir_disk_index) != 0) && ((num_this_disk != 1) || (cdir_disk_index != 1)))
    return US_MZ_FALSE;

  if ((cdir_size = US_MZ_READ_LE32(pBuf + US_MZ_ZIP_ECDH_CDIR_SIZE_OFS)) < pZip->m_total_files * US_MZ_ZIP_CENTRAL_DIR_HEADER_SIZE)
    return US_MZ_FALSE;

  cdir_ofs = cur_file_ofs - cdir_size;
  if ((cdir_ofs + (us_mz_uint64)cdir_size) > pZip->m_archive_size)
    return US_MZ_FALSE;

  pZip->m_central_directory_file_ofs = cdir_ofs;

  pZip->m_archive_file_ofs = cdir_ofs - US_MZ_READ_LE32(pBuf + US_MZ_ZIP_ECDH_CDIR_OFS_OFS);
  if (pZip->m_archive_file_ofs > pZip->m_archive_size)
    return US_MZ_FALSE;

  if (pZip->m_total_files)
  {
     us_mz_uint i, n;

    // Read the entire central directory into a heap block, and allocate another heap block to hold the unsorted central dir file record offsets, and another to hold the sorted indices.
    if ((!us_mz_zip_array_resize(pZip, &pZip->m_pState->m_central_dir, cdir_size, US_MZ_FALSE)) ||
        (!us_mz_zip_array_resize(pZip, &pZip->m_pState->m_central_dir_offsets, pZip->m_total_files, US_MZ_FALSE)))
      return US_MZ_FALSE;

    if (sort_central_dir)
    {
      if (!us_mz_zip_array_resize(pZip, &pZip->m_pState->m_sorted_central_dir_offsets, pZip->m_total_files, US_MZ_FALSE))
        return US_MZ_FALSE;
    }

    if (pZip->m_pRead(pZip->m_pIO_opaque, cdir_ofs, pZip->m_pState->m_central_dir.m_p, cdir_size) != cdir_size)
      return US_MZ_FALSE;

    // Now create an index into the central directory file records, do some basic sanity checking on each record, and check for zip64 entries (which are not yet supported).
    p = (const us_mz_uint8 *)pZip->m_pState->m_central_dir.m_p;
    for (n = cdir_size, i = 0; i < pZip->m_total_files; ++i)
    {
      us_mz_uint total_header_size, comp_size, decomp_size, disk_index;
      if ((n < US_MZ_ZIP_CENTRAL_DIR_HEADER_SIZE) || (US_MZ_READ_LE32(p) != US_MZ_ZIP_CENTRAL_DIR_HEADER_SIG))
        return US_MZ_FALSE;
      US_MZ_ZIP_ARRAY_ELEMENT(&pZip->m_pState->m_central_dir_offsets, us_mz_uint32, i) = (us_mz_uint32)(p - (const us_mz_uint8 *)pZip->m_pState->m_central_dir.m_p);
      if (sort_central_dir)
        US_MZ_ZIP_ARRAY_ELEMENT(&pZip->m_pState->m_sorted_central_dir_offsets, us_mz_uint32, i) = i;
      comp_size = US_MZ_READ_LE32(p + US_MZ_ZIP_CDH_COMPRESSED_SIZE_OFS);
      decomp_size = US_MZ_READ_LE32(p + US_MZ_ZIP_CDH_DECOMPRESSED_SIZE_OFS);
      if (((!US_MZ_READ_LE32(p + US_MZ_ZIP_CDH_METHOD_OFS)) && (decomp_size != comp_size)) || (decomp_size && !comp_size) || (decomp_size == 0xFFFFFFFF) || (comp_size == 0xFFFFFFFF))
        return US_MZ_FALSE;
      disk_index = US_MZ_READ_LE16(p + US_MZ_ZIP_CDH_DISK_START_OFS);
      if ((disk_index != num_this_disk) && (disk_index != 1))
        return US_MZ_FALSE;
      if (((us_mz_uint64)US_MZ_READ_LE32(p + US_MZ_ZIP_CDH_LOCAL_HEADER_OFS) + US_MZ_ZIP_LOCAL_DIR_HEADER_SIZE + comp_size) > pZip->m_archive_size)
        return US_MZ_FALSE;
      if ((total_header_size = US_MZ_ZIP_CENTRAL_DIR_HEADER_SIZE + US_MZ_READ_LE16(p + US_MZ_ZIP_CDH_FILENAME_LEN_OFS) + US_MZ_READ_LE16(p + US_MZ_ZIP_CDH_EXTRA_LEN_OFS) + US_MZ_READ_LE16(p + US_MZ_ZIP_CDH_COMMENT_LEN_OFS)) > n)
        return US_MZ_FALSE;
      n -= total_header_size; p += total_header_size;
    }
  }

  if (sort_central_dir)
    us_mz_zip_reader_sort_central_dir_offsets_by_filename(pZip);

  return US_MZ_TRUE;
}

us_mz_bool us_mz_zip_reader_init(us_mz_zip_archive *pZip, us_mz_uint64 size, us_mz_uint32 flags)
{
  if ((!pZip) || (!pZip->m_pRead))
    return US_MZ_FALSE;
  if (!us_mz_zip_reader_init_internal(pZip, flags))
    return US_MZ_FALSE;
  pZip->m_archive_size = size;
  if (!us_mz_zip_reader_read_central_dir(pZip, flags))
  {
    us_mz_zip_reader_end(pZip);
    return US_MZ_FALSE;
  }
  return US_MZ_TRUE;
}

static size_t us_mz_zip_mem_read_func(void *pOpaque, us_mz_uint64 file_ofs, void *pBuf, size_t n)
{
  us_mz_zip_archive *pZip = (us_mz_zip_archive *)pOpaque;
  size_t s = (file_ofs >= pZip->m_archive_size) ? 0 : (size_t)US_MZ_MIN(pZip->m_archive_size - file_ofs, n);
  memcpy(pBuf, (const us_mz_uint8 *)pZip->m_pState->m_pMem + file_ofs, s);
  return s;
}

us_mz_bool us_mz_zip_reader_init_mem(us_mz_zip_archive *pZip, const void *pMem, size_t size, us_mz_uint32 flags)
{
  if (!us_mz_zip_reader_init_internal(pZip, flags))
    return US_MZ_FALSE;
  pZip->m_archive_size = size;
  pZip->m_pRead = us_mz_zip_mem_read_func;
  pZip->m_pIO_opaque = pZip;
#ifdef __cplusplus
  pZip->m_pState->m_pMem = const_cast<void *>(pMem);
#else
  pZip->m_pState->m_pMem = (void *)pMem;
#endif
  pZip->m_pState->m_mem_size = size;
  if (!us_mz_zip_reader_read_central_dir(pZip, flags))
  {
    us_mz_zip_reader_end(pZip);
    return US_MZ_FALSE;
  }
  return US_MZ_TRUE;
}

#ifndef US_MZ_NO_STDIO
static size_t us_mz_zip_file_read_func(void *pOpaque, us_mz_uint64 file_ofs, void *pBuf, size_t n)
{
  us_mz_zip_archive *pZip = (us_mz_zip_archive *)pOpaque;
  us_mz_int64 cur_ofs = US_MZ_FTELL64(pZip->m_pState->m_pFile);
  if (((us_mz_int64)file_ofs < 0) || (((cur_ofs != (us_mz_int64)file_ofs)) && (US_MZ_FSEEK64(pZip->m_pState->m_pFile, (us_mz_int64)file_ofs, SEEK_SET))))
    return 0;
  return US_MZ_FREAD(pBuf, 1, n, pZip->m_pState->m_pFile);
}

us_mz_bool us_mz_zip_reader_init_file(us_mz_zip_archive *pZip, const char *pFilename, us_mz_uint32 flags)
{
  us_mz_uint64 file_size;
  US_MZ_FILE *pFile = US_MZ_FOPEN(pFilename, "rb");
  if (!pFile)
    return US_MZ_FALSE;
  if (US_MZ_FSEEK64(pFile, 0, SEEK_END))
  {
    US_MZ_FCLOSE(pFile);
    return US_MZ_FALSE;
  }
  file_size = US_MZ_FTELL64(pFile);
  if (!us_mz_zip_reader_init_internal(pZip, flags))
  {
    US_MZ_FCLOSE(pFile);
    return US_MZ_FALSE;
  }
  pZip->m_pRead = us_mz_zip_file_read_func;
  pZip->m_pIO_opaque = pZip;
  pZip->m_pState->m_pFile = pFile;
  pZip->m_archive_size = file_size;
  if (!us_mz_zip_reader_read_central_dir(pZip, flags))
  {
    us_mz_zip_reader_end(pZip);
    return US_MZ_FALSE;
  }
  return US_MZ_TRUE;
}
#endif // #ifndef US_MZ_NO_STDIO

us_mz_uint us_mz_zip_reader_get_num_files(us_mz_zip_archive *pZip)
{
  return pZip ? pZip->m_total_files : 0;
}

static US_MZ_FORCEINLINE const us_mz_uint8 *us_mz_zip_reader_get_cdh(us_mz_zip_archive *pZip, us_mz_uint file_index)
{
  if ((!pZip) || (!pZip->m_pState) || (file_index >= pZip->m_total_files) || (pZip->m_zip_mode != US_MZ_ZIP_MODE_READING))
    return NULL;
  return &US_MZ_ZIP_ARRAY_ELEMENT(&pZip->m_pState->m_central_dir, us_mz_uint8, US_MZ_ZIP_ARRAY_ELEMENT(&pZip->m_pState->m_central_dir_offsets, us_mz_uint32, file_index));
}

us_mz_bool us_mz_zip_reader_is_file_encrypted(us_mz_zip_archive *pZip, us_mz_uint file_index)
{
  us_mz_uint m_bit_flag;
  const us_mz_uint8 *p = us_mz_zip_reader_get_cdh(pZip, file_index);
  if (!p)
    return US_MZ_FALSE;
  m_bit_flag = US_MZ_READ_LE16(p + US_MZ_ZIP_CDH_BIT_FLAG_OFS);
  return (m_bit_flag & 1);
}

us_mz_bool us_mz_zip_reader_is_file_a_directory(us_mz_zip_archive *pZip, us_mz_uint file_index)
{
  us_mz_uint filename_len, external_attr;
  const us_mz_uint8 *p = us_mz_zip_reader_get_cdh(pZip, file_index);
  if (!p)
    return US_MZ_FALSE;

  // First see if the filename ends with a '/' character.
  filename_len = US_MZ_READ_LE16(p + US_MZ_ZIP_CDH_FILENAME_LEN_OFS);
  if (filename_len)
  {
    if (*(p + US_MZ_ZIP_CENTRAL_DIR_HEADER_SIZE + filename_len - 1) == '/')
      return US_MZ_TRUE;
  }

  // Bugfix: This code was also checking if the internal attribute was non-zero, which wasn't correct.
  // Most/all zip writers (hopefully) set DOS file/directory attributes in the low 16-bits, so check for the DOS directory flag and ignore the source OS ID in the created by field.
  // FIXME: Remove this check? Is it necessary - we already check the filename.
  external_attr = US_MZ_READ_LE32(p + US_MZ_ZIP_CDH_EXTERNAL_ATTR_OFS);
  if ((external_attr & 0x10) != 0)
    return US_MZ_TRUE;

  return US_MZ_FALSE;
}

us_mz_bool us_mz_zip_reader_file_stat(us_mz_zip_archive *pZip, us_mz_uint file_index, us_mz_zip_archive_file_stat *pStat)
{
  us_mz_uint n;
  const us_mz_uint8 *p = us_mz_zip_reader_get_cdh(pZip, file_index);
  if ((!p) || (!pStat))
    return US_MZ_FALSE;

  // Unpack the central directory record.
  pStat->m_file_index = file_index;
  pStat->m_central_dir_ofs = US_MZ_ZIP_ARRAY_ELEMENT(&pZip->m_pState->m_central_dir_offsets, us_mz_uint32, file_index);
  pStat->m_version_made_by = US_MZ_READ_LE16(p + US_MZ_ZIP_CDH_VERSION_MADE_BY_OFS);
  pStat->m_version_needed = US_MZ_READ_LE16(p + US_MZ_ZIP_CDH_VERSION_NEEDED_OFS);
  pStat->m_bit_flag = US_MZ_READ_LE16(p + US_MZ_ZIP_CDH_BIT_FLAG_OFS);
  pStat->m_method = US_MZ_READ_LE16(p + US_MZ_ZIP_CDH_METHOD_OFS);
#ifndef US_MZ_NO_TIME
  pStat->m_time = us_mz_zip_dos_to_time_t(US_MZ_READ_LE16(p + US_MZ_ZIP_CDH_FILE_TIME_OFS), US_MZ_READ_LE16(p + US_MZ_ZIP_CDH_FILE_DATE_OFS));
#endif
  pStat->m_crc32 = US_MZ_READ_LE32(p + US_MZ_ZIP_CDH_CRC32_OFS);
  pStat->m_comp_size = US_MZ_READ_LE32(p + US_MZ_ZIP_CDH_COMPRESSED_SIZE_OFS);
  pStat->m_uncomp_size = US_MZ_READ_LE32(p + US_MZ_ZIP_CDH_DECOMPRESSED_SIZE_OFS);
  pStat->m_internal_attr = US_MZ_READ_LE16(p + US_MZ_ZIP_CDH_INTERNAL_ATTR_OFS);
  pStat->m_external_attr = US_MZ_READ_LE32(p + US_MZ_ZIP_CDH_EXTERNAL_ATTR_OFS);
  pStat->m_local_header_ofs = US_MZ_READ_LE32(p + US_MZ_ZIP_CDH_LOCAL_HEADER_OFS);

  // Copy as much of the filename and comment as possible.
  n = US_MZ_READ_LE16(p + US_MZ_ZIP_CDH_FILENAME_LEN_OFS); n = US_MZ_MIN(n, US_MZ_ZIP_MAX_ARCHIVE_FILENAME_SIZE - 1);
  memcpy(pStat->m_filename, p + US_MZ_ZIP_CENTRAL_DIR_HEADER_SIZE, n); pStat->m_filename[n] = '\0';

  n = US_MZ_READ_LE16(p + US_MZ_ZIP_CDH_COMMENT_LEN_OFS); n = US_MZ_MIN(n, US_MZ_ZIP_MAX_ARCHIVE_FILE_COMMENT_SIZE - 1);
  pStat->m_comment_size = n;
  memcpy(pStat->m_comment, p + US_MZ_ZIP_CENTRAL_DIR_HEADER_SIZE + US_MZ_READ_LE16(p + US_MZ_ZIP_CDH_FILENAME_LEN_OFS) + US_MZ_READ_LE16(p + US_MZ_ZIP_CDH_EXTRA_LEN_OFS), n); pStat->m_comment[n] = '\0';

  return US_MZ_TRUE;
}

us_mz_uint us_mz_zip_reader_get_filename(us_mz_zip_archive *pZip, us_mz_uint file_index, char *pFilename, us_mz_uint filename_buf_size)
{
  us_mz_uint n;
  const us_mz_uint8 *p = us_mz_zip_reader_get_cdh(pZip, file_index);
  if (!p) { if (filename_buf_size) pFilename[0] = '\0'; return 0; }
  n = US_MZ_READ_LE16(p + US_MZ_ZIP_CDH_FILENAME_LEN_OFS);
  if (filename_buf_size)
  {
    n = US_MZ_MIN(n, filename_buf_size - 1);
    memcpy(pFilename, p + US_MZ_ZIP_CENTRAL_DIR_HEADER_SIZE, n);
    pFilename[n] = '\0';
  }
  return n + 1;
}

static US_MZ_FORCEINLINE us_mz_bool us_mz_zip_reader_string_equal(const char *pA, const char *pB, us_mz_uint len, us_mz_uint flags)
{
  us_mz_uint i;
  if (flags & US_MZ_ZIP_FLAG_CASE_SENSITIVE)
    return 0 == memcmp(pA, pB, len);
  for (i = 0; i < len; ++i)
    if (US_MZ_TOLOWER(pA[i]) != US_MZ_TOLOWER(pB[i]))
      return US_MZ_FALSE;
  return US_MZ_TRUE;
}

static US_MZ_FORCEINLINE int us_mz_zip_reader_filename_compare(const us_mz_zip_array *pCentral_dir_array, const us_mz_zip_array *pCentral_dir_offsets, us_mz_uint l_index, const char *pR, us_mz_uint r_len)
{
  const us_mz_uint8 *pL = &US_MZ_ZIP_ARRAY_ELEMENT(pCentral_dir_array, us_mz_uint8, US_MZ_ZIP_ARRAY_ELEMENT(pCentral_dir_offsets, us_mz_uint32, l_index)), *pE;
  us_mz_uint l_len = US_MZ_READ_LE16(pL + US_MZ_ZIP_CDH_FILENAME_LEN_OFS);
  us_mz_uint8 l = 0, r = 0;
  pL += US_MZ_ZIP_CENTRAL_DIR_HEADER_SIZE;
  pE = pL + US_MZ_MIN(l_len, r_len);
  while (pL < pE)
  {
    if ((l = US_MZ_TOLOWER(*pL)) != (r = US_MZ_TOLOWER(*pR)))
      break;
    pL++; pR++;
  }
  return (pL == pE) ? (int)(l_len - r_len) : (l - r);
}

static int us_mz_zip_reader_locate_file_binary_search(us_mz_zip_archive *pZip, const char *pFilename)
{
  us_mz_zip_internal_state *pState = pZip->m_pState;
  const us_mz_zip_array *pCentral_dir_offsets = &pState->m_central_dir_offsets;
  const us_mz_zip_array *pCentral_dir = &pState->m_central_dir;
  us_mz_uint32 *pIndices = &US_MZ_ZIP_ARRAY_ELEMENT(&pState->m_sorted_central_dir_offsets, us_mz_uint32, 0);
  const int size = pZip->m_total_files;
  const us_mz_uint filename_len = (us_mz_uint)strlen(pFilename);
  int l = 0, h = size - 1;
  while (l <= h)
  {
    int m = (l + h) >> 1, file_index = pIndices[m], comp = us_mz_zip_reader_filename_compare(pCentral_dir, pCentral_dir_offsets, file_index, pFilename, filename_len);
    if (!comp)
      return file_index;
    else if (comp < 0)
      l = m + 1;
    else
      h = m - 1;
  }
  return -1;
}

int us_mz_zip_reader_locate_file(us_mz_zip_archive *pZip, const char *pName, const char *pComment, us_mz_uint flags)
{
  us_mz_uint file_index; size_t name_len, comment_len;
  if ((!pZip) || (!pZip->m_pState) || (!pName) || (pZip->m_zip_mode != US_MZ_ZIP_MODE_READING))
    return -1;
  if (((flags & (US_MZ_ZIP_FLAG_IGNORE_PATH | US_MZ_ZIP_FLAG_CASE_SENSITIVE)) == 0) && (!pComment) && (pZip->m_pState->m_sorted_central_dir_offsets.m_size))
    return us_mz_zip_reader_locate_file_binary_search(pZip, pName);
  name_len = strlen(pName); if (name_len > 0xFFFF) return -1;
  comment_len = pComment ? strlen(pComment) : 0; if (comment_len > 0xFFFF) return -1;
  for (file_index = 0; file_index < pZip->m_total_files; file_index++)
  {
    const us_mz_uint8 *pHeader = &US_MZ_ZIP_ARRAY_ELEMENT(&pZip->m_pState->m_central_dir, us_mz_uint8, US_MZ_ZIP_ARRAY_ELEMENT(&pZip->m_pState->m_central_dir_offsets, us_mz_uint32, file_index));
    us_mz_uint filename_len = US_MZ_READ_LE16(pHeader + US_MZ_ZIP_CDH_FILENAME_LEN_OFS);
    const char *pFilename = (const char *)pHeader + US_MZ_ZIP_CENTRAL_DIR_HEADER_SIZE;
    if (filename_len < name_len)
      continue;
    if (comment_len)
    {
      us_mz_uint file_extra_len = US_MZ_READ_LE16(pHeader + US_MZ_ZIP_CDH_EXTRA_LEN_OFS), file_comment_len = US_MZ_READ_LE16(pHeader + US_MZ_ZIP_CDH_COMMENT_LEN_OFS);
      const char *pFile_comment = pFilename + filename_len + file_extra_len;
      if ((file_comment_len != comment_len) || (!us_mz_zip_reader_string_equal(pComment, pFile_comment, file_comment_len, flags)))
        continue;
    }
    if ((flags & US_MZ_ZIP_FLAG_IGNORE_PATH) && (filename_len))
    {
      int ofs = filename_len - 1;
      do
      {
        if ((pFilename[ofs] == '/') || (pFilename[ofs] == '\\') || (pFilename[ofs] == ':'))
          break;
      } while (--ofs >= 0);
      ofs++;
      pFilename += ofs; filename_len -= ofs;
    }
    if ((filename_len == name_len) && (us_mz_zip_reader_string_equal(pName, pFilename, filename_len, flags)))
      return file_index;
  }
  return -1;
}

us_mz_bool us_mz_zip_reader_extract_to_mem_no_alloc(us_mz_zip_archive *pZip, us_mz_uint file_index, void *pBuf, size_t buf_size, us_mz_uint flags, void *pUser_read_buf, size_t user_read_buf_size)
{
  int status = US_TINFL_STATUS_DONE;
  us_mz_uint64 needed_size, cur_file_ofs, comp_remaining, out_buf_ofs = 0, read_buf_size, read_buf_ofs = 0, read_buf_avail;
  us_mz_zip_archive_file_stat file_stat;
  void *pRead_buf;
  us_mz_uint32 local_header_u32[(US_MZ_ZIP_LOCAL_DIR_HEADER_SIZE + sizeof(us_mz_uint32) - 1) / sizeof(us_mz_uint32)]; us_mz_uint8 *pLocal_header = (us_mz_uint8 *)local_header_u32;
  us_tinfl_decompressor inflator;

  if ((buf_size) && (!pBuf))
    return US_MZ_FALSE;

  if (!us_mz_zip_reader_file_stat(pZip, file_index, &file_stat))
    return US_MZ_FALSE;

  // Empty file, or a directory (but not always a directory - I've seen odd zips with directories that have compressed data which inflates to 0 bytes)
  if (!file_stat.m_comp_size)
    return US_MZ_TRUE;

  // Entry is a subdirectory (I've seen old zips with dir entries which have compressed deflate data which inflates to 0 bytes, but these entries claim to uncompress to 512 bytes in the headers).
  // I'm torn how to handle this case - should it fail instead?
  if (us_mz_zip_reader_is_file_a_directory(pZip, file_index))
    return US_MZ_TRUE;

  // Encryption and patch files are not supported.
  if (file_stat.m_bit_flag & (1 | 32))
    return US_MZ_FALSE;

  // This function only supports stored and deflate.
  if ((!(flags & US_MZ_ZIP_FLAG_COMPRESSED_DATA)) && (file_stat.m_method != 0) && (file_stat.m_method != US_MZ_DEFLATED))
    return US_MZ_FALSE;

  // Ensure supplied output buffer is large enough.
  needed_size = (flags & US_MZ_ZIP_FLAG_COMPRESSED_DATA) ? file_stat.m_comp_size : file_stat.m_uncomp_size;
  if (buf_size < needed_size)
    return US_MZ_FALSE;

  // Read and parse the local directory entry.
  cur_file_ofs = pZip->m_archive_file_ofs + file_stat.m_local_header_ofs;
  if (pZip->m_pRead(pZip->m_pIO_opaque, cur_file_ofs, pLocal_header, US_MZ_ZIP_LOCAL_DIR_HEADER_SIZE) != US_MZ_ZIP_LOCAL_DIR_HEADER_SIZE)
    return US_MZ_FALSE;
  if (US_MZ_READ_LE32(pLocal_header) != US_MZ_ZIP_LOCAL_DIR_HEADER_SIG)
    return US_MZ_FALSE;

  cur_file_ofs += US_MZ_ZIP_LOCAL_DIR_HEADER_SIZE + US_MZ_READ_LE16(pLocal_header + US_MZ_ZIP_LDH_FILENAME_LEN_OFS) + US_MZ_READ_LE16(pLocal_header + US_MZ_ZIP_LDH_EXTRA_LEN_OFS);
  if ((cur_file_ofs + file_stat.m_comp_size) > pZip->m_archive_size)
    return US_MZ_FALSE;

  if ((flags & US_MZ_ZIP_FLAG_COMPRESSED_DATA) || (!file_stat.m_method))
  {
    // The file is stored or the caller has requested the compressed data.
    if (pZip->m_pRead(pZip->m_pIO_opaque, cur_file_ofs, pBuf, (size_t)needed_size) != needed_size)
      return US_MZ_FALSE;
    return ((flags & US_MZ_ZIP_FLAG_COMPRESSED_DATA) != 0) || (us_mz_crc32(US_MZ_CRC32_INIT, (const us_mz_uint8 *)pBuf, (size_t)file_stat.m_uncomp_size) == file_stat.m_crc32);
  }

  // Decompress the file either directly from memory or from a file input buffer.
  us_tinfl_init(&inflator);

  if (pZip->m_pState->m_pMem)
  {
    // Read directly from the archive in memory.
    pRead_buf = (us_mz_uint8 *)pZip->m_pState->m_pMem + cur_file_ofs;
    read_buf_size = read_buf_avail = file_stat.m_comp_size;
    comp_remaining = 0;
  }
  else if (pUser_read_buf)
  {
    // Use a user provided read buffer.
    if (!user_read_buf_size)
      return US_MZ_FALSE;
    pRead_buf = (us_mz_uint8 *)pUser_read_buf;
    read_buf_size = user_read_buf_size;
    read_buf_avail = 0;
    comp_remaining = file_stat.m_comp_size;
  }
  else
  {
    // Temporarily allocate a read buffer.
    read_buf_size = US_MZ_MIN(file_stat.m_comp_size, (us_mz_uint64)US_MZ_ZIP_MAX_IO_BUF_SIZE);
#ifdef _MSC_VER
    if (((0, sizeof(size_t) == sizeof(us_mz_uint32))) && (read_buf_size > 0x7FFFFFFF))
#else
    if (((sizeof(size_t) == sizeof(us_mz_uint32))) && (read_buf_size > 0x7FFFFFFF))
#endif
      return US_MZ_FALSE;
    if (NULL == (pRead_buf = pZip->m_pAlloc(pZip->m_pAlloc_opaque, 1, (size_t)read_buf_size)))
      return US_MZ_FALSE;
    read_buf_avail = 0;
    comp_remaining = file_stat.m_comp_size;
  }

  do
  {
    size_t in_buf_size, out_buf_size = (size_t)(file_stat.m_uncomp_size - out_buf_ofs);
    if ((!read_buf_avail) && (!pZip->m_pState->m_pMem))
    {
      read_buf_avail = US_MZ_MIN(read_buf_size, comp_remaining);
      if (pZip->m_pRead(pZip->m_pIO_opaque, cur_file_ofs, pRead_buf, (size_t)read_buf_avail) != read_buf_avail)
      {
        status = US_TINFL_STATUS_FAILED;
        break;
      }
      cur_file_ofs += read_buf_avail;
      comp_remaining -= read_buf_avail;
      read_buf_ofs = 0;
    }
    in_buf_size = (size_t)read_buf_avail;
    status = us_tinfl_decompress(&inflator, (us_mz_uint8 *)pRead_buf + read_buf_ofs, &in_buf_size, (us_mz_uint8 *)pBuf, (us_mz_uint8 *)pBuf + out_buf_ofs, &out_buf_size, US_TINFL_FLAG_USING_NON_WRAPPING_OUTPUT_BUF | (comp_remaining ? US_TINFL_FLAG_HAS_MORE_INPUT : 0));
    read_buf_avail -= in_buf_size;
    read_buf_ofs += in_buf_size;
    out_buf_ofs += out_buf_size;
  } while (status == US_TINFL_STATUS_NEEDS_MORE_INPUT);

  if (status == US_TINFL_STATUS_DONE)
  {
    // Make sure the entire file was decompressed, and check its CRC.
    if ((out_buf_ofs != file_stat.m_uncomp_size) || (us_mz_crc32(US_MZ_CRC32_INIT, (const us_mz_uint8 *)pBuf, (size_t)file_stat.m_uncomp_size) != file_stat.m_crc32))
      status = US_TINFL_STATUS_FAILED;
  }

  if ((!pZip->m_pState->m_pMem) && (!pUser_read_buf))
    pZip->m_pFree(pZip->m_pAlloc_opaque, pRead_buf);

  return status == US_TINFL_STATUS_DONE;
}

us_mz_bool us_mz_zip_reader_extract_file_to_mem_no_alloc(us_mz_zip_archive *pZip, const char *pFilename, void *pBuf, size_t buf_size, us_mz_uint flags, void *pUser_read_buf, size_t user_read_buf_size)
{
  int file_index = us_mz_zip_reader_locate_file(pZip, pFilename, NULL, flags);
  if (file_index < 0)
    return US_MZ_FALSE;
  return us_mz_zip_reader_extract_to_mem_no_alloc(pZip, file_index, pBuf, buf_size, flags, pUser_read_buf, user_read_buf_size);
}

us_mz_bool us_mz_zip_reader_extract_to_mem(us_mz_zip_archive *pZip, us_mz_uint file_index, void *pBuf, size_t buf_size, us_mz_uint flags)
{
  return us_mz_zip_reader_extract_to_mem_no_alloc(pZip, file_index, pBuf, buf_size, flags, NULL, 0);
}

us_mz_bool us_mz_zip_reader_extract_file_to_mem(us_mz_zip_archive *pZip, const char *pFilename, void *pBuf, size_t buf_size, us_mz_uint flags)
{
  return us_mz_zip_reader_extract_file_to_mem_no_alloc(pZip, pFilename, pBuf, buf_size, flags, NULL, 0);
}

void *us_mz_zip_reader_extract_to_heap(us_mz_zip_archive *pZip, us_mz_uint file_index, size_t *pSize, us_mz_uint flags)
{
  us_mz_uint64 comp_size, uncomp_size, alloc_size;
  const us_mz_uint8 *p = us_mz_zip_reader_get_cdh(pZip, file_index);
  void *pBuf;

  if (pSize)
    *pSize = 0;
  if (!p)
    return NULL;

  comp_size = US_MZ_READ_LE32(p + US_MZ_ZIP_CDH_COMPRESSED_SIZE_OFS);
  uncomp_size = US_MZ_READ_LE32(p + US_MZ_ZIP_CDH_DECOMPRESSED_SIZE_OFS);

  alloc_size = (flags & US_MZ_ZIP_FLAG_COMPRESSED_DATA) ? comp_size : uncomp_size;
#ifdef _MSC_VER
  if (((0, sizeof(size_t) == sizeof(us_mz_uint32))) && (alloc_size > 0x7FFFFFFF))
#else
  if (((sizeof(size_t) == sizeof(us_mz_uint32))) && (alloc_size > 0x7FFFFFFF))
#endif
    return NULL;
  if (NULL == (pBuf = pZip->m_pAlloc(pZip->m_pAlloc_opaque, 1, (size_t)alloc_size)))
    return NULL;

  if (!us_mz_zip_reader_extract_to_mem(pZip, file_index, pBuf, (size_t)alloc_size, flags))
  {
    pZip->m_pFree(pZip->m_pAlloc_opaque, pBuf);
    return NULL;
  }

  if (pSize) *pSize = (size_t)alloc_size;
  return pBuf;
}

void *us_mz_zip_reader_extract_file_to_heap(us_mz_zip_archive *pZip, const char *pFilename, size_t *pSize, us_mz_uint flags)
{
  int file_index = us_mz_zip_reader_locate_file(pZip, pFilename, NULL, flags);
  if (file_index < 0)
  {
    if (pSize) *pSize = 0;
    return US_MZ_FALSE;
  }
  return us_mz_zip_reader_extract_to_heap(pZip, file_index, pSize, flags);
}

us_mz_bool us_mz_zip_reader_extract_to_callback(us_mz_zip_archive *pZip, us_mz_uint file_index, us_mz_file_write_func pCallback, void *pOpaque, us_mz_uint flags)
{
  int status = US_TINFL_STATUS_DONE; us_mz_uint file_crc32 = US_MZ_CRC32_INIT;
  us_mz_uint64 read_buf_size, read_buf_ofs = 0, read_buf_avail, comp_remaining, out_buf_ofs = 0, cur_file_ofs;
  us_mz_zip_archive_file_stat file_stat;
  void *pRead_buf = NULL; void *pWrite_buf = NULL;
  us_mz_uint32 local_header_u32[(US_MZ_ZIP_LOCAL_DIR_HEADER_SIZE + sizeof(us_mz_uint32) - 1) / sizeof(us_mz_uint32)]; us_mz_uint8 *pLocal_header = (us_mz_uint8 *)local_header_u32;

  if (!us_mz_zip_reader_file_stat(pZip, file_index, &file_stat))
    return US_MZ_FALSE;

  // Empty file, or a directory (but not always a directory - I've seen odd zips with directories that have compressed data which inflates to 0 bytes)
  if (!file_stat.m_comp_size)
    return US_MZ_TRUE;

  // Entry is a subdirectory (I've seen old zips with dir entries which have compressed deflate data which inflates to 0 bytes, but these entries claim to uncompress to 512 bytes in the headers).
  // I'm torn how to handle this case - should it fail instead?
  if (us_mz_zip_reader_is_file_a_directory(pZip, file_index))
    return US_MZ_TRUE;

  // Encryption and patch files are not supported.
  if (file_stat.m_bit_flag & (1 | 32))
    return US_MZ_FALSE;

  // This function only supports stored and deflate.
  if ((!(flags & US_MZ_ZIP_FLAG_COMPRESSED_DATA)) && (file_stat.m_method != 0) && (file_stat.m_method != US_MZ_DEFLATED))
    return US_MZ_FALSE;

  // Read and parse the local directory entry.
  cur_file_ofs = file_stat.m_local_header_ofs;
  if (pZip->m_pRead(pZip->m_pIO_opaque, cur_file_ofs, pLocal_header, US_MZ_ZIP_LOCAL_DIR_HEADER_SIZE) != US_MZ_ZIP_LOCAL_DIR_HEADER_SIZE)
    return US_MZ_FALSE;
  if (US_MZ_READ_LE32(pLocal_header) != US_MZ_ZIP_LOCAL_DIR_HEADER_SIG)
    return US_MZ_FALSE;

  cur_file_ofs += US_MZ_ZIP_LOCAL_DIR_HEADER_SIZE + US_MZ_READ_LE16(pLocal_header + US_MZ_ZIP_LDH_FILENAME_LEN_OFS) + US_MZ_READ_LE16(pLocal_header + US_MZ_ZIP_LDH_EXTRA_LEN_OFS);
  if ((cur_file_ofs + file_stat.m_comp_size) > pZip->m_archive_size)
    return US_MZ_FALSE;

  // Decompress the file either directly from memory or from a file input buffer.
  if (pZip->m_pState->m_pMem)
  {
    pRead_buf = (us_mz_uint8 *)pZip->m_pState->m_pMem + cur_file_ofs;
    read_buf_size = read_buf_avail = file_stat.m_comp_size;
    comp_remaining = 0;
  }
  else
  {
    read_buf_size = US_MZ_MIN(file_stat.m_comp_size, (us_mz_uint64)US_MZ_ZIP_MAX_IO_BUF_SIZE);
    if (NULL == (pRead_buf = pZip->m_pAlloc(pZip->m_pAlloc_opaque, 1, (size_t)read_buf_size)))
      return US_MZ_FALSE;
    read_buf_avail = 0;
    comp_remaining = file_stat.m_comp_size;
  }

  if ((flags & US_MZ_ZIP_FLAG_COMPRESSED_DATA) || (!file_stat.m_method))
  {
    // The file is stored or the caller has requested the compressed data.
    if (pZip->m_pState->m_pMem)
    {
#ifdef _MSC_VER
      if (((0, sizeof(size_t) == sizeof(us_mz_uint32))) && (file_stat.m_comp_size > 0xFFFFFFFF))
#else
      if (((sizeof(size_t) == sizeof(us_mz_uint32))) && (file_stat.m_comp_size > 0xFFFFFFFF))
#endif
        return US_MZ_FALSE;
      if (pCallback(pOpaque, out_buf_ofs, pRead_buf, (size_t)file_stat.m_comp_size) != file_stat.m_comp_size)
        status = US_TINFL_STATUS_FAILED;
      else if (!(flags & US_MZ_ZIP_FLAG_COMPRESSED_DATA))
        file_crc32 = (us_mz_uint32)us_mz_crc32(file_crc32, (const us_mz_uint8 *)pRead_buf, (size_t)file_stat.m_comp_size);
      cur_file_ofs += file_stat.m_comp_size;
      out_buf_ofs += file_stat.m_comp_size;
      comp_remaining = 0;
    }
    else
    {
      while (comp_remaining)
      {
        read_buf_avail = US_MZ_MIN(read_buf_size, comp_remaining);
        if (pZip->m_pRead(pZip->m_pIO_opaque, cur_file_ofs, pRead_buf, (size_t)read_buf_avail) != read_buf_avail)
        {
          status = US_TINFL_STATUS_FAILED;
          break;
        }

        if (!(flags & US_MZ_ZIP_FLAG_COMPRESSED_DATA))
          file_crc32 = (us_mz_uint32)us_mz_crc32(file_crc32, (const us_mz_uint8 *)pRead_buf, (size_t)read_buf_avail);

        if (pCallback(pOpaque, out_buf_ofs, pRead_buf, (size_t)read_buf_avail) != read_buf_avail)
        {
          status = US_TINFL_STATUS_FAILED;
          break;
        }
        cur_file_ofs += read_buf_avail;
        out_buf_ofs += read_buf_avail;
        comp_remaining -= read_buf_avail;
      }
    }
  }
  else
  {
    us_tinfl_decompressor inflator;
    us_tinfl_init(&inflator);

    if (NULL == (pWrite_buf = pZip->m_pAlloc(pZip->m_pAlloc_opaque, 1, US_TINFL_LZ_DICT_SIZE)))
      status = US_TINFL_STATUS_FAILED;
    else
    {
      do
      {
        us_mz_uint8 *pWrite_buf_cur = (us_mz_uint8 *)pWrite_buf + (out_buf_ofs & (US_TINFL_LZ_DICT_SIZE - 1));
        size_t in_buf_size, out_buf_size = US_TINFL_LZ_DICT_SIZE - (out_buf_ofs & (US_TINFL_LZ_DICT_SIZE - 1));
        if ((!read_buf_avail) && (!pZip->m_pState->m_pMem))
        {
          read_buf_avail = US_MZ_MIN(read_buf_size, comp_remaining);
          if (pZip->m_pRead(pZip->m_pIO_opaque, cur_file_ofs, pRead_buf, (size_t)read_buf_avail) != read_buf_avail)
          {
            status = US_TINFL_STATUS_FAILED;
            break;
          }
          cur_file_ofs += read_buf_avail;
          comp_remaining -= read_buf_avail;
          read_buf_ofs = 0;
        }

        in_buf_size = (size_t)read_buf_avail;
        status = us_tinfl_decompress(&inflator, (const us_mz_uint8 *)pRead_buf + read_buf_ofs, &in_buf_size, (us_mz_uint8 *)pWrite_buf, pWrite_buf_cur, &out_buf_size, comp_remaining ? US_TINFL_FLAG_HAS_MORE_INPUT : 0);
        read_buf_avail -= in_buf_size;
        read_buf_ofs += in_buf_size;

        if (out_buf_size)
        {
          if (pCallback(pOpaque, out_buf_ofs, pWrite_buf_cur, out_buf_size) != out_buf_size)
          {
            status = US_TINFL_STATUS_FAILED;
            break;
          }
          file_crc32 = (us_mz_uint32)us_mz_crc32(file_crc32, pWrite_buf_cur, out_buf_size);
          if ((out_buf_ofs += out_buf_size) > file_stat.m_uncomp_size)
          {
            status = US_TINFL_STATUS_FAILED;
            break;
          }
        }
      } while ((status == US_TINFL_STATUS_NEEDS_MORE_INPUT) || (status == US_TINFL_STATUS_HAS_MORE_OUTPUT));
    }
  }

  if ((status == US_TINFL_STATUS_DONE) && (!(flags & US_MZ_ZIP_FLAG_COMPRESSED_DATA)))
  {
    // Make sure the entire file was decompressed, and check its CRC.
    if ((out_buf_ofs != file_stat.m_uncomp_size) || (file_crc32 != file_stat.m_crc32))
      status = US_TINFL_STATUS_FAILED;
  }

  if (!pZip->m_pState->m_pMem)
    pZip->m_pFree(pZip->m_pAlloc_opaque, pRead_buf);
  if (pWrite_buf)
    pZip->m_pFree(pZip->m_pAlloc_opaque, pWrite_buf);

  return status == US_TINFL_STATUS_DONE;
}

us_mz_bool us_mz_zip_reader_extract_file_to_callback(us_mz_zip_archive *pZip, const char *pFilename, us_mz_file_write_func pCallback, void *pOpaque, us_mz_uint flags)
{
  int file_index = us_mz_zip_reader_locate_file(pZip, pFilename, NULL, flags);
  if (file_index < 0)
    return US_MZ_FALSE;
  return us_mz_zip_reader_extract_to_callback(pZip, file_index, pCallback, pOpaque, flags);
}

#ifndef US_MZ_NO_STDIO
static size_t us_mz_zip_file_write_callback(void *pOpaque, us_mz_uint64 ofs, const void *pBuf, size_t n)
{
  (void)ofs; return US_MZ_FWRITE(pBuf, 1, n, (US_MZ_FILE*)pOpaque);
}

us_mz_bool us_mz_zip_reader_extract_to_file(us_mz_zip_archive *pZip, us_mz_uint file_index, const char *pDst_filename, us_mz_uint flags)
{
  us_mz_bool status;
  us_mz_zip_archive_file_stat file_stat;
  US_MZ_FILE *pFile;
  if (!us_mz_zip_reader_file_stat(pZip, file_index, &file_stat))
    return US_MZ_FALSE;
  pFile = US_MZ_FOPEN(pDst_filename, "wb");
  if (!pFile)
    return US_MZ_FALSE;
  status = us_mz_zip_reader_extract_to_callback(pZip, file_index, us_mz_zip_file_write_callback, pFile, flags);
  if (US_MZ_FCLOSE(pFile) == EOF)
    return US_MZ_FALSE;
#ifndef US_MZ_NO_TIME
  if (status)
    us_mz_zip_set_file_times(pDst_filename, file_stat.m_time, file_stat.m_time);
#endif
  return status;
}
#endif // #ifndef US_MZ_NO_STDIO

us_mz_bool us_mz_zip_reader_end(us_mz_zip_archive *pZip)
{
  if ((!pZip) || (!pZip->m_pState) || (!pZip->m_pAlloc) || (!pZip->m_pFree) || (pZip->m_zip_mode != US_MZ_ZIP_MODE_READING))
    return US_MZ_FALSE;

  if (pZip->m_pState)
  {
    us_mz_zip_internal_state *pState = pZip->m_pState; pZip->m_pState = NULL;
    us_mz_zip_array_clear(pZip, &pState->m_central_dir);
    us_mz_zip_array_clear(pZip, &pState->m_central_dir_offsets);
    us_mz_zip_array_clear(pZip, &pState->m_sorted_central_dir_offsets);

#ifndef US_MZ_NO_STDIO
    if (pState->m_pFile)
    {
      US_MZ_FCLOSE(pState->m_pFile);
      pState->m_pFile = NULL;
    }
#endif // #ifndef US_MZ_NO_STDIO

    pZip->m_pFree(pZip->m_pAlloc_opaque, pState);
  }
  pZip->m_zip_mode = US_MZ_ZIP_MODE_INVALID;

  return US_MZ_TRUE;
}

#ifndef US_MZ_NO_STDIO
us_mz_bool us_mz_zip_reader_extract_file_to_file(us_mz_zip_archive *pZip, const char *pArchive_filename, const char *pDst_filename, us_mz_uint flags)
{
  int file_index = us_mz_zip_reader_locate_file(pZip, pArchive_filename, NULL, flags);
  if (file_index < 0)
    return US_MZ_FALSE;
  return us_mz_zip_reader_extract_to_file(pZip, file_index, pDst_filename, flags);
}
#endif

// ------------------- .ZIP archive writing

#ifndef US_MZ_NO_ARCHIVE_WRITING_APIS

static void us_mz_write_le16(us_mz_uint8 *p, us_mz_uint16 v) { p[0] = (us_mz_uint8)v; p[1] = (us_mz_uint8)(v >> 8); }
static void us_mz_write_le32(us_mz_uint8 *p, us_mz_uint32 v) { p[0] = (us_mz_uint8)v; p[1] = (us_mz_uint8)(v >> 8); p[2] = (us_mz_uint8)(v >> 16); p[3] = (us_mz_uint8)(v >> 24); }
#define US_MZ_WRITE_LE16(p, v) us_mz_write_le16((us_mz_uint8 *)(p), (us_mz_uint16)(v))
#define US_MZ_WRITE_LE32(p, v) us_mz_write_le32((us_mz_uint8 *)(p), (us_mz_uint32)(v))

us_mz_bool us_mz_zip_writer_init(us_mz_zip_archive *pZip, us_mz_uint64 existing_size)
{
  if ((!pZip) || (pZip->m_pState) || (!pZip->m_pWrite) || (pZip->m_zip_mode != US_MZ_ZIP_MODE_INVALID))
    return US_MZ_FALSE;

  if (pZip->m_file_offset_alignment)
  {
    // Ensure user specified file offset alignment is a power of 2.
    if (pZip->m_file_offset_alignment & (pZip->m_file_offset_alignment - 1))
      return US_MZ_FALSE;
  }

  if (!pZip->m_pAlloc) pZip->m_pAlloc = def_alloc_func;
  if (!pZip->m_pFree) pZip->m_pFree = def_free_func;
  if (!pZip->m_pRealloc) pZip->m_pRealloc = def_realloc_func;

  pZip->m_zip_mode = US_MZ_ZIP_MODE_WRITING;
  pZip->m_archive_size = existing_size;
  pZip->m_central_directory_file_ofs = 0;
  pZip->m_total_files = 0;

  if (NULL == (pZip->m_pState = (us_mz_zip_internal_state *)pZip->m_pAlloc(pZip->m_pAlloc_opaque, 1, sizeof(us_mz_zip_internal_state))))
    return US_MZ_FALSE;
  memset(pZip->m_pState, 0, sizeof(us_mz_zip_internal_state));
  US_MZ_ZIP_ARRAY_SET_ELEMENT_SIZE(&pZip->m_pState->m_central_dir, sizeof(us_mz_uint8));
  US_MZ_ZIP_ARRAY_SET_ELEMENT_SIZE(&pZip->m_pState->m_central_dir_offsets, sizeof(us_mz_uint32));
  US_MZ_ZIP_ARRAY_SET_ELEMENT_SIZE(&pZip->m_pState->m_sorted_central_dir_offsets, sizeof(us_mz_uint32));
  return US_MZ_TRUE;
}

static size_t us_mz_zip_heap_write_func(void *pOpaque, us_mz_uint64 file_ofs, const void *pBuf, size_t n)
{
  us_mz_zip_archive *pZip = (us_mz_zip_archive *)pOpaque;
  us_mz_zip_internal_state *pState = pZip->m_pState;
  us_mz_uint64 new_size = US_MZ_MAX(file_ofs + n, pState->m_mem_size);
#ifdef _MSC_VER
  if ((!n) || ((0, sizeof(size_t) == sizeof(us_mz_uint32)) && (new_size > 0x7FFFFFFF)))
#else
  if ((!n) || ((sizeof(size_t) == sizeof(us_mz_uint32)) && (new_size > 0x7FFFFFFF)))
#endif
    return 0;
  if (new_size > pState->m_mem_capacity)
  {
    void *pNew_block;
    size_t new_capacity = US_MZ_MAX(64, pState->m_mem_capacity); while (new_capacity < new_size) new_capacity *= 2;
    if (NULL == (pNew_block = pZip->m_pRealloc(pZip->m_pAlloc_opaque, pState->m_pMem, 1, new_capacity)))
      return 0;
    pState->m_pMem = pNew_block; pState->m_mem_capacity = new_capacity;
  }
  memcpy((us_mz_uint8 *)pState->m_pMem + file_ofs, pBuf, n);
  pState->m_mem_size = (size_t)new_size;
  return n;
}

us_mz_bool us_mz_zip_writer_init_heap(us_mz_zip_archive *pZip, size_t size_to_reserve_at_beginning, size_t initial_allocation_size)
{
  pZip->m_pWrite = us_mz_zip_heap_write_func;
  pZip->m_pIO_opaque = pZip;
  if (!us_mz_zip_writer_init(pZip, size_to_reserve_at_beginning))
    return US_MZ_FALSE;
  if (0 != (initial_allocation_size = US_MZ_MAX(initial_allocation_size, size_to_reserve_at_beginning)))
  {
    if (NULL == (pZip->m_pState->m_pMem = pZip->m_pAlloc(pZip->m_pAlloc_opaque, 1, initial_allocation_size)))
    {
      us_mz_zip_writer_end(pZip);
      return US_MZ_FALSE;
    }
    pZip->m_pState->m_mem_capacity = initial_allocation_size;
  }
  return US_MZ_TRUE;
}

#ifndef US_MZ_NO_STDIO
static size_t us_mz_zip_file_write_func(void *pOpaque, us_mz_uint64 file_ofs, const void *pBuf, size_t n)
{
  us_mz_zip_archive *pZip = (us_mz_zip_archive *)pOpaque;
  us_mz_int64 cur_ofs = US_MZ_FTELL64(pZip->m_pState->m_pFile);
  if (((us_mz_int64)file_ofs < 0) || (((cur_ofs != (us_mz_int64)file_ofs)) && (US_MZ_FSEEK64(pZip->m_pState->m_pFile, (us_mz_int64)file_ofs, SEEK_SET))))
    return 0;
  return US_MZ_FWRITE(pBuf, 1, n, pZip->m_pState->m_pFile);
}

us_mz_bool us_mz_zip_writer_init_file(us_mz_zip_archive *pZip, const char *pFilename, us_mz_uint64 size_to_reserve_at_beginning)
{
  US_MZ_FILE *pFile;
  pZip->m_pWrite = us_mz_zip_file_write_func;
  pZip->m_pIO_opaque = pZip;
  if (!us_mz_zip_writer_init(pZip, size_to_reserve_at_beginning))
    return US_MZ_FALSE;
  if (NULL == (pFile = US_MZ_FOPEN(pFilename, "wb")))
  {
    us_mz_zip_writer_end(pZip);
    return US_MZ_FALSE;
  }
  pZip->m_pState->m_pFile = pFile;
  if (size_to_reserve_at_beginning)
  {
    us_mz_uint64 cur_ofs = 0; char buf[4096]; US_MZ_CLEAR_OBJ(buf);
    do
    {
      size_t n = (size_t)US_MZ_MIN(sizeof(buf), size_to_reserve_at_beginning);
      if (pZip->m_pWrite(pZip->m_pIO_opaque, cur_ofs, buf, n) != n)
      {
        us_mz_zip_writer_end(pZip);
        return US_MZ_FALSE;
      }
      cur_ofs += n; size_to_reserve_at_beginning -= n;
    } while (size_to_reserve_at_beginning);
  }
  return US_MZ_TRUE;
}
#endif // #ifndef US_MZ_NO_STDIO

us_mz_bool us_mz_zip_writer_init_from_reader(us_mz_zip_archive *pZip, const char *pFilename)
{
  us_mz_zip_internal_state *pState;
  if ((!pZip) || (!pZip->m_pState) || (pZip->m_zip_mode != US_MZ_ZIP_MODE_READING))
    return US_MZ_FALSE;
  // No sense in trying to write to an archive that's already at the support max size
  if ((pZip->m_total_files == 0xFFFF) || ((pZip->m_archive_size + US_MZ_ZIP_CENTRAL_DIR_HEADER_SIZE + US_MZ_ZIP_LOCAL_DIR_HEADER_SIZE) > 0xFFFFFFFF))
    return US_MZ_FALSE;

  pState = pZip->m_pState;

  if (pState->m_pFile)
  {
#ifdef US_MZ_NO_STDIO
    pFilename; return US_MZ_FALSE;
#else
    // Archive is being read from stdio - try to reopen as writable.
    if (pZip->m_pIO_opaque != pZip)
      return US_MZ_FALSE;
    if (!pFilename)
      return US_MZ_FALSE;
    pZip->m_pWrite = us_mz_zip_file_write_func;
    if (NULL == (pState->m_pFile = US_MZ_FREOPEN(pFilename, "r+b", pState->m_pFile)))
    {
      // The us_mz_zip_archive is now in a bogus state because pState->m_pFile is NULL, so just close it.
      us_mz_zip_reader_end(pZip);
      return US_MZ_FALSE;
    }
#endif // #ifdef US_MZ_NO_STDIO
  }
  else if (pState->m_pMem)
  {
    // Archive lives in a memory block. Assume it's from the heap that we can resize using the realloc callback.
    if (pZip->m_pIO_opaque != pZip)
      return US_MZ_FALSE;
    pState->m_mem_capacity = pState->m_mem_size;
    pZip->m_pWrite = us_mz_zip_heap_write_func;
  }
  // Archive is being read via a user provided read function - make sure the user has specified a write function too.
  else if (!pZip->m_pWrite)
    return US_MZ_FALSE;

  // Start writing new files at the archive's current central directory location.
  pZip->m_archive_size = pZip->m_central_directory_file_ofs;
  pZip->m_zip_mode = US_MZ_ZIP_MODE_WRITING;
  pZip->m_central_directory_file_ofs = 0;

  return US_MZ_TRUE;
}

us_mz_bool us_mz_zip_writer_add_mem(us_mz_zip_archive *pZip, const char *pArchive_name, const void *pBuf, size_t buf_size, us_mz_uint level_and_flags)
{
  return us_mz_zip_writer_add_mem_ex(pZip, pArchive_name, pBuf, buf_size, NULL, 0, level_and_flags, 0, 0);
}

typedef struct
{
  us_mz_zip_archive *m_pZip;
  us_mz_uint64 m_cur_archive_file_ofs;
  us_mz_uint64 m_comp_size;
} us_mz_zip_writer_add_state;

static us_mz_bool us_mz_zip_writer_add_put_buf_callback(const void* pBuf, int len, void *pUser)
{
  us_mz_zip_writer_add_state *pState = (us_mz_zip_writer_add_state *)pUser;
  if ((int)pState->m_pZip->m_pWrite(pState->m_pZip->m_pIO_opaque, pState->m_cur_archive_file_ofs, pBuf, len) != len)
    return US_MZ_FALSE;
  pState->m_cur_archive_file_ofs += len;
  pState->m_comp_size += len;
  return US_MZ_TRUE;
}

static us_mz_bool us_mz_zip_writer_create_local_dir_header(us_mz_zip_archive *pZip, us_mz_uint8 *pDst, us_mz_uint16 filename_size, us_mz_uint16 extra_size, us_mz_uint64 uncomp_size, us_mz_uint64 comp_size, us_mz_uint32 uncomp_crc32, us_mz_uint16 method, us_mz_uint16 bit_flags, us_mz_uint16 dos_time, us_mz_uint16 dos_date)
{
  (void)pZip;
  memset(pDst, 0, US_MZ_ZIP_LOCAL_DIR_HEADER_SIZE);
  US_MZ_WRITE_LE32(pDst + US_MZ_ZIP_LDH_SIG_OFS, US_MZ_ZIP_LOCAL_DIR_HEADER_SIG);
  US_MZ_WRITE_LE16(pDst + US_MZ_ZIP_LDH_VERSION_NEEDED_OFS, method ? 20 : 0);
  US_MZ_WRITE_LE16(pDst + US_MZ_ZIP_LDH_BIT_FLAG_OFS, bit_flags);
  US_MZ_WRITE_LE16(pDst + US_MZ_ZIP_LDH_METHOD_OFS, method);
  US_MZ_WRITE_LE16(pDst + US_MZ_ZIP_LDH_FILE_TIME_OFS, dos_time);
  US_MZ_WRITE_LE16(pDst + US_MZ_ZIP_LDH_FILE_DATE_OFS, dos_date);
  US_MZ_WRITE_LE32(pDst + US_MZ_ZIP_LDH_CRC32_OFS, uncomp_crc32);
  US_MZ_WRITE_LE32(pDst + US_MZ_ZIP_LDH_COMPRESSED_SIZE_OFS, comp_size);
  US_MZ_WRITE_LE32(pDst + US_MZ_ZIP_LDH_DECOMPRESSED_SIZE_OFS, uncomp_size);
  US_MZ_WRITE_LE16(pDst + US_MZ_ZIP_LDH_FILENAME_LEN_OFS, filename_size);
  US_MZ_WRITE_LE16(pDst + US_MZ_ZIP_LDH_EXTRA_LEN_OFS, extra_size);
  return US_MZ_TRUE;
}

static us_mz_bool us_mz_zip_writer_create_central_dir_header(us_mz_zip_archive *pZip, us_mz_uint8 *pDst, us_mz_uint16 filename_size, us_mz_uint16 extra_size, us_mz_uint16 comment_size, us_mz_uint64 uncomp_size, us_mz_uint64 comp_size, us_mz_uint32 uncomp_crc32, us_mz_uint16 method, us_mz_uint16 bit_flags, us_mz_uint16 dos_time, us_mz_uint16 dos_date, us_mz_uint64 local_header_ofs, us_mz_uint32 ext_attributes)
{
  (void)pZip;
  memset(pDst, 0, US_MZ_ZIP_CENTRAL_DIR_HEADER_SIZE);
  US_MZ_WRITE_LE32(pDst + US_MZ_ZIP_CDH_SIG_OFS, US_MZ_ZIP_CENTRAL_DIR_HEADER_SIG);
  US_MZ_WRITE_LE16(pDst + US_MZ_ZIP_CDH_VERSION_NEEDED_OFS, method ? 20 : 0);
  US_MZ_WRITE_LE16(pDst + US_MZ_ZIP_CDH_BIT_FLAG_OFS, bit_flags);
  US_MZ_WRITE_LE16(pDst + US_MZ_ZIP_CDH_METHOD_OFS, method);
  US_MZ_WRITE_LE16(pDst + US_MZ_ZIP_CDH_FILE_TIME_OFS, dos_time);
  US_MZ_WRITE_LE16(pDst + US_MZ_ZIP_CDH_FILE_DATE_OFS, dos_date);
  US_MZ_WRITE_LE32(pDst + US_MZ_ZIP_CDH_CRC32_OFS, uncomp_crc32);
  US_MZ_WRITE_LE32(pDst + US_MZ_ZIP_CDH_COMPRESSED_SIZE_OFS, comp_size);
  US_MZ_WRITE_LE32(pDst + US_MZ_ZIP_CDH_DECOMPRESSED_SIZE_OFS, uncomp_size);
  US_MZ_WRITE_LE16(pDst + US_MZ_ZIP_CDH_FILENAME_LEN_OFS, filename_size);
  US_MZ_WRITE_LE16(pDst + US_MZ_ZIP_CDH_EXTRA_LEN_OFS, extra_size);
  US_MZ_WRITE_LE16(pDst + US_MZ_ZIP_CDH_COMMENT_LEN_OFS, comment_size);
  US_MZ_WRITE_LE32(pDst + US_MZ_ZIP_CDH_EXTERNAL_ATTR_OFS, ext_attributes);
  US_MZ_WRITE_LE32(pDst + US_MZ_ZIP_CDH_LOCAL_HEADER_OFS, local_header_ofs);
  return US_MZ_TRUE;
}

static us_mz_bool us_mz_zip_writer_add_to_central_dir(us_mz_zip_archive *pZip, const char *pFilename, us_mz_uint16 filename_size, const void *pExtra, us_mz_uint16 extra_size, const void *pComment, us_mz_uint16 comment_size, us_mz_uint64 uncomp_size, us_mz_uint64 comp_size, us_mz_uint32 uncomp_crc32, us_mz_uint16 method, us_mz_uint16 bit_flags, us_mz_uint16 dos_time, us_mz_uint16 dos_date, us_mz_uint64 local_header_ofs, us_mz_uint32 ext_attributes)
{
  us_mz_zip_internal_state *pState = pZip->m_pState;
  us_mz_uint32 central_dir_ofs = (us_mz_uint32)pState->m_central_dir.m_size;
  size_t orig_central_dir_size = pState->m_central_dir.m_size;
  us_mz_uint8 central_dir_header[US_MZ_ZIP_CENTRAL_DIR_HEADER_SIZE];

  // No zip64 support yet
  if ((local_header_ofs > 0xFFFFFFFF) || (((us_mz_uint64)pState->m_central_dir.m_size + US_MZ_ZIP_CENTRAL_DIR_HEADER_SIZE + filename_size + extra_size + comment_size) > 0xFFFFFFFF))
    return US_MZ_FALSE;

  if (!us_mz_zip_writer_create_central_dir_header(pZip, central_dir_header, filename_size, extra_size, comment_size, uncomp_size, comp_size, uncomp_crc32, method, bit_flags, dos_time, dos_date, local_header_ofs, ext_attributes))
    return US_MZ_FALSE;

  if ((!us_mz_zip_array_push_back(pZip, &pState->m_central_dir, central_dir_header, US_MZ_ZIP_CENTRAL_DIR_HEADER_SIZE)) ||
      (!us_mz_zip_array_push_back(pZip, &pState->m_central_dir, pFilename, filename_size)) ||
      (!us_mz_zip_array_push_back(pZip, &pState->m_central_dir, pExtra, extra_size)) ||
      (!us_mz_zip_array_push_back(pZip, &pState->m_central_dir, pComment, comment_size)) ||
      (!us_mz_zip_array_push_back(pZip, &pState->m_central_dir_offsets, &central_dir_ofs, 1)))
  {
    // Try to push the central directory array back into its original state.
    us_mz_zip_array_resize(pZip, &pState->m_central_dir, orig_central_dir_size, US_MZ_FALSE);
    return US_MZ_FALSE;
  }

  return US_MZ_TRUE;
}

static us_mz_bool us_mz_zip_writer_validate_archive_name(const char *pArchive_name)
{
  // Basic ZIP archive filename validity checks: Valid filenames cannot start with a forward slash, cannot contain a drive letter, and cannot use DOS-style backward slashes.
  if (*pArchive_name == '/')
    return US_MZ_FALSE;
  while (*pArchive_name)
  {
    if ((*pArchive_name == '\\') || (*pArchive_name == ':'))
      return US_MZ_FALSE;
    pArchive_name++;
  }
  return US_MZ_TRUE;
}

static us_mz_uint us_mz_zip_writer_compute_padding_needed_for_file_alignment(us_mz_zip_archive *pZip)
{
  us_mz_uint32 n;
  if (!pZip->m_file_offset_alignment)
    return 0;
  n = (us_mz_uint32)(pZip->m_archive_size & (pZip->m_file_offset_alignment - 1));
  return (pZip->m_file_offset_alignment - n) & (pZip->m_file_offset_alignment - 1);
}

static us_mz_bool us_mz_zip_writer_write_zeros(us_mz_zip_archive *pZip, us_mz_uint64 cur_file_ofs, us_mz_uint32 n)
{
  char buf[4096];
  memset(buf, 0, US_MZ_MIN(sizeof(buf), n));
  while (n)
  {
    us_mz_uint32 s = US_MZ_MIN(sizeof(buf), n);
    if (pZip->m_pWrite(pZip->m_pIO_opaque, cur_file_ofs, buf, s) != s)
      return US_MZ_FALSE;
    cur_file_ofs += s; n -= s;
  }
  return US_MZ_TRUE;
}

us_mz_bool us_mz_zip_writer_add_mem_ex(us_mz_zip_archive *pZip, const char *pArchive_name, const void *pBuf, size_t buf_size, const void *pComment, us_mz_uint16 comment_size, us_mz_uint level_and_flags, us_mz_uint64 uncomp_size, us_mz_uint32 uncomp_crc32)
{
  us_mz_uint16 method = 0, dos_time = 0, dos_date = 0;
  us_mz_uint level, ext_attributes = 0, num_alignment_padding_bytes;
  us_mz_uint64 local_dir_header_ofs = pZip->m_archive_size, cur_archive_file_ofs = pZip->m_archive_size, comp_size = 0;
  size_t archive_name_size;
  us_mz_uint8 local_dir_header[US_MZ_ZIP_LOCAL_DIR_HEADER_SIZE];
  us_tdefl_compressor *pComp = NULL;
  us_mz_bool store_data_uncompressed;
  us_mz_zip_internal_state *pState;

  if ((int)level_and_flags < 0)
    level_and_flags = US_MZ_DEFAULT_LEVEL;
  level = level_and_flags & 0xF;
  store_data_uncompressed = ((!level) || (level_and_flags & US_MZ_ZIP_FLAG_COMPRESSED_DATA));

  if ((!pZip) || (!pZip->m_pState) || (pZip->m_zip_mode != US_MZ_ZIP_MODE_WRITING) || ((buf_size) && (!pBuf)) || (!pArchive_name) || ((comment_size) && (!pComment)) || (pZip->m_total_files == 0xFFFF) || (level > US_MZ_UBER_COMPRESSION))
    return US_MZ_FALSE;

  pState = pZip->m_pState;

  if ((!(level_and_flags & US_MZ_ZIP_FLAG_COMPRESSED_DATA)) && (uncomp_size))
    return US_MZ_FALSE;
  // No zip64 support yet
  if ((buf_size > 0xFFFFFFFF) || (uncomp_size > 0xFFFFFFFF))
    return US_MZ_FALSE;
  if (!us_mz_zip_writer_validate_archive_name(pArchive_name))
    return US_MZ_FALSE;

#ifndef US_MZ_NO_TIME
  {
    time_t cur_time; time(&cur_time);
    us_mz_zip_time_to_dos_time(cur_time, &dos_time, &dos_date);
  }
#endif // #ifndef US_MZ_NO_TIME

  archive_name_size = strlen(pArchive_name);
  if (archive_name_size > 0xFFFF)
    return US_MZ_FALSE;

  num_alignment_padding_bytes = us_mz_zip_writer_compute_padding_needed_for_file_alignment(pZip);

  // no zip64 support yet
  if ((pZip->m_total_files == 0xFFFF) || ((pZip->m_archive_size + num_alignment_padding_bytes + US_MZ_ZIP_LOCAL_DIR_HEADER_SIZE + US_MZ_ZIP_CENTRAL_DIR_HEADER_SIZE + comment_size + archive_name_size) > 0xFFFFFFFF))
    return US_MZ_FALSE;

  if ((archive_name_size) && (pArchive_name[archive_name_size - 1] == '/'))
  {
    // Set DOS Subdirectory attribute bit.
    ext_attributes |= 0x10;
    // Subdirectories cannot contain data.
    if ((buf_size) || (uncomp_size))
      return US_MZ_FALSE;
  }

  // Try to do any allocations before writing to the archive, so if an allocation fails the file remains unmodified. (A good idea if we're doing an in-place modification.)
  if ((!us_mz_zip_array_ensure_room(pZip, &pState->m_central_dir, US_MZ_ZIP_CENTRAL_DIR_HEADER_SIZE + archive_name_size + comment_size)) || (!us_mz_zip_array_ensure_room(pZip, &pState->m_central_dir_offsets, 1)))
    return US_MZ_FALSE;

  if ((!store_data_uncompressed) && (buf_size))
  {
    if (NULL == (pComp = (us_tdefl_compressor *)pZip->m_pAlloc(pZip->m_pAlloc_opaque, 1, sizeof(us_tdefl_compressor))))
      return US_MZ_FALSE;
  }

  if (!us_mz_zip_writer_write_zeros(pZip, cur_archive_file_ofs, num_alignment_padding_bytes + sizeof(local_dir_header)))
  {
    pZip->m_pFree(pZip->m_pAlloc_opaque, pComp);
    return US_MZ_FALSE;
  }
  local_dir_header_ofs += num_alignment_padding_bytes;
  if (pZip->m_file_offset_alignment) { US_MZ_ASSERT((local_dir_header_ofs & (pZip->m_file_offset_alignment - 1)) == 0); }
  cur_archive_file_ofs += num_alignment_padding_bytes + sizeof(local_dir_header);

  US_MZ_CLEAR_OBJ(local_dir_header);
  if (pZip->m_pWrite(pZip->m_pIO_opaque, cur_archive_file_ofs, pArchive_name, archive_name_size) != archive_name_size)
  {
    pZip->m_pFree(pZip->m_pAlloc_opaque, pComp);
    return US_MZ_FALSE;
  }
  cur_archive_file_ofs += archive_name_size;

  if (!(level_and_flags & US_MZ_ZIP_FLAG_COMPRESSED_DATA))
  {
    uncomp_crc32 = (us_mz_uint32)us_mz_crc32(US_MZ_CRC32_INIT, (const us_mz_uint8*)pBuf, buf_size);
    uncomp_size = buf_size;
    if (uncomp_size <= 3)
    {
      level = 0;
      store_data_uncompressed = US_MZ_TRUE;
    }
  }

  if (store_data_uncompressed)
  {
    if (pZip->m_pWrite(pZip->m_pIO_opaque, cur_archive_file_ofs, pBuf, buf_size) != buf_size)
    {
      pZip->m_pFree(pZip->m_pAlloc_opaque, pComp);
      return US_MZ_FALSE;
    }

    cur_archive_file_ofs += buf_size;
    comp_size = buf_size;

    if (level_and_flags & US_MZ_ZIP_FLAG_COMPRESSED_DATA)
      method = US_MZ_DEFLATED;
  }
  else if (buf_size)
  {
    us_mz_zip_writer_add_state state;

    state.m_pZip = pZip;
    state.m_cur_archive_file_ofs = cur_archive_file_ofs;
    state.m_comp_size = 0;

    if ((us_tdefl_init(pComp, us_mz_zip_writer_add_put_buf_callback, &state, us_tdefl_create_comp_flags_from_zip_params(level, -15, US_MZ_DEFAULT_STRATEGY)) != US_TDEFL_STATUS_OKAY) ||
        (us_tdefl_compress_buffer(pComp, pBuf, buf_size, US_TDEFL_FINISH) != US_TDEFL_STATUS_DONE))
    {
      pZip->m_pFree(pZip->m_pAlloc_opaque, pComp);
      return US_MZ_FALSE;
    }

    comp_size = state.m_comp_size;
    cur_archive_file_ofs = state.m_cur_archive_file_ofs;

    method = US_MZ_DEFLATED;
  }

  pZip->m_pFree(pZip->m_pAlloc_opaque, pComp);
  pComp = NULL;

  // no zip64 support yet
  if ((comp_size > 0xFFFFFFFF) || (cur_archive_file_ofs > 0xFFFFFFFF))
    return US_MZ_FALSE;

  if (!us_mz_zip_writer_create_local_dir_header(pZip, local_dir_header, (us_mz_uint16)archive_name_size, 0, uncomp_size, comp_size, uncomp_crc32, method, 0, dos_time, dos_date))
    return US_MZ_FALSE;

  if (pZip->m_pWrite(pZip->m_pIO_opaque, local_dir_header_ofs, local_dir_header, sizeof(local_dir_header)) != sizeof(local_dir_header))
    return US_MZ_FALSE;

  if (!us_mz_zip_writer_add_to_central_dir(pZip, pArchive_name, (us_mz_uint16)archive_name_size, NULL, 0, pComment, comment_size, uncomp_size, comp_size, uncomp_crc32, method, 0, dos_time, dos_date, local_dir_header_ofs, ext_attributes))
    return US_MZ_FALSE;

  pZip->m_total_files++;
  pZip->m_archive_size = cur_archive_file_ofs;

  return US_MZ_TRUE;
}

#ifndef US_MZ_NO_STDIO
us_mz_bool us_mz_zip_writer_add_file(us_mz_zip_archive *pZip, const char *pArchive_name, const char *pSrc_filename, const void *pComment, us_mz_uint16 comment_size, us_mz_uint level_and_flags)
{
  us_mz_uint uncomp_crc32 = US_MZ_CRC32_INIT, level, num_alignment_padding_bytes;
  us_mz_uint16 method = 0, dos_time = 0, dos_date = 0, ext_attributes = 0;
  us_mz_uint64 local_dir_header_ofs = pZip->m_archive_size, cur_archive_file_ofs = pZip->m_archive_size, uncomp_size = 0, comp_size = 0;
  size_t archive_name_size;
  us_mz_uint8 local_dir_header[US_MZ_ZIP_LOCAL_DIR_HEADER_SIZE];
  US_MZ_FILE *pSrc_file = NULL;

  if ((int)level_and_flags < 0)
    level_and_flags = US_MZ_DEFAULT_LEVEL;
  level = level_and_flags & 0xF;

  if ((!pZip) || (!pZip->m_pState) || (pZip->m_zip_mode != US_MZ_ZIP_MODE_WRITING) || (!pArchive_name) || ((comment_size) && (!pComment)) || (level > US_MZ_UBER_COMPRESSION))
    return US_MZ_FALSE;
  if (level_and_flags & US_MZ_ZIP_FLAG_COMPRESSED_DATA)
    return US_MZ_FALSE;
  if (!us_mz_zip_writer_validate_archive_name(pArchive_name))
    return US_MZ_FALSE;

  archive_name_size = strlen(pArchive_name);
  if (archive_name_size > 0xFFFF)
    return US_MZ_FALSE;

  num_alignment_padding_bytes = us_mz_zip_writer_compute_padding_needed_for_file_alignment(pZip);

  // no zip64 support yet
  if ((pZip->m_total_files == 0xFFFF) || ((pZip->m_archive_size + num_alignment_padding_bytes + US_MZ_ZIP_LOCAL_DIR_HEADER_SIZE + US_MZ_ZIP_CENTRAL_DIR_HEADER_SIZE + comment_size + archive_name_size) > 0xFFFFFFFF))
    return US_MZ_FALSE;

  if (!us_mz_zip_get_file_modified_time(pSrc_filename, &dos_time, &dos_date))
    return US_MZ_FALSE;

  pSrc_file = US_MZ_FOPEN(pSrc_filename, "rb");
  if (!pSrc_file)
    return US_MZ_FALSE;
  US_MZ_FSEEK64(pSrc_file, 0, SEEK_END);
  uncomp_size = US_MZ_FTELL64(pSrc_file);
  US_MZ_FSEEK64(pSrc_file, 0, SEEK_SET);

  if (uncomp_size > 0xFFFFFFFF)
  {
    // No zip64 support yet
    US_MZ_FCLOSE(pSrc_file);
    return US_MZ_FALSE;
  }
  if (uncomp_size <= 3)
    level = 0;

  if (!us_mz_zip_writer_write_zeros(pZip, cur_archive_file_ofs, num_alignment_padding_bytes + sizeof(local_dir_header)))
  {
    US_MZ_FCLOSE(pSrc_file);
    return US_MZ_FALSE;
  }
  local_dir_header_ofs += num_alignment_padding_bytes;
  if (pZip->m_file_offset_alignment) { US_MZ_ASSERT((local_dir_header_ofs & (pZip->m_file_offset_alignment - 1)) == 0); }
  cur_archive_file_ofs += num_alignment_padding_bytes + sizeof(local_dir_header);

  US_MZ_CLEAR_OBJ(local_dir_header);
  if (pZip->m_pWrite(pZip->m_pIO_opaque, cur_archive_file_ofs, pArchive_name, archive_name_size) != archive_name_size)
  {
    US_MZ_FCLOSE(pSrc_file);
    return US_MZ_FALSE;
  }
  cur_archive_file_ofs += archive_name_size;

  if (uncomp_size)
  {
    us_mz_uint64 uncomp_remaining = uncomp_size;
    void *pRead_buf = pZip->m_pAlloc(pZip->m_pAlloc_opaque, 1, US_MZ_ZIP_MAX_IO_BUF_SIZE);
    if (!pRead_buf)
    {
      US_MZ_FCLOSE(pSrc_file);
      return US_MZ_FALSE;
    }

    if (!level)
    {
      while (uncomp_remaining)
      {
        us_mz_uint n = (us_mz_uint)US_MZ_MIN((us_mz_uint64)US_MZ_ZIP_MAX_IO_BUF_SIZE, uncomp_remaining);
        if ((US_MZ_FREAD(pRead_buf, 1, n, pSrc_file) != n) || (pZip->m_pWrite(pZip->m_pIO_opaque, cur_archive_file_ofs, pRead_buf, n) != n))
        {
          pZip->m_pFree(pZip->m_pAlloc_opaque, pRead_buf);
          US_MZ_FCLOSE(pSrc_file);
          return US_MZ_FALSE;
        }
        uncomp_crc32 = (us_mz_uint32)us_mz_crc32(uncomp_crc32, (const us_mz_uint8 *)pRead_buf, n);
        uncomp_remaining -= n;
        cur_archive_file_ofs += n;
      }
      comp_size = uncomp_size;
    }
    else
    {
      us_mz_bool result = US_MZ_FALSE;
      us_mz_zip_writer_add_state state;
      us_tdefl_compressor *pComp = (us_tdefl_compressor *)pZip->m_pAlloc(pZip->m_pAlloc_opaque, 1, sizeof(us_tdefl_compressor));
      if (!pComp)
      {
        pZip->m_pFree(pZip->m_pAlloc_opaque, pRead_buf);
        US_MZ_FCLOSE(pSrc_file);
        return US_MZ_FALSE;
      }

      state.m_pZip = pZip;
      state.m_cur_archive_file_ofs = cur_archive_file_ofs;
      state.m_comp_size = 0;

      if (us_tdefl_init(pComp, us_mz_zip_writer_add_put_buf_callback, &state, us_tdefl_create_comp_flags_from_zip_params(level, -15, US_MZ_DEFAULT_STRATEGY)) != US_TDEFL_STATUS_OKAY)
      {
        pZip->m_pFree(pZip->m_pAlloc_opaque, pComp);
        pZip->m_pFree(pZip->m_pAlloc_opaque, pRead_buf);
        US_MZ_FCLOSE(pSrc_file);
        return US_MZ_FALSE;
      }

      for ( ; ; )
      {
        size_t in_buf_size = (us_mz_uint32)US_MZ_MIN(uncomp_remaining, (us_mz_uint64)US_MZ_ZIP_MAX_IO_BUF_SIZE);
        us_tdefl_status status;

        if (US_MZ_FREAD(pRead_buf, 1, in_buf_size, pSrc_file) != in_buf_size)
          break;

        uncomp_crc32 = (us_mz_uint32)us_mz_crc32(uncomp_crc32, (const us_mz_uint8 *)pRead_buf, in_buf_size);
        uncomp_remaining -= in_buf_size;

        status = us_tdefl_compress_buffer(pComp, pRead_buf, in_buf_size, uncomp_remaining ? US_TDEFL_NO_FLUSH : US_TDEFL_FINISH);
        if (status == US_TDEFL_STATUS_DONE)
        {
          result = US_MZ_TRUE;
          break;
        }
        else if (status != US_TDEFL_STATUS_OKAY)
          break;
      }

      pZip->m_pFree(pZip->m_pAlloc_opaque, pComp);

      if (!result)
      {
        pZip->m_pFree(pZip->m_pAlloc_opaque, pRead_buf);
        US_MZ_FCLOSE(pSrc_file);
        return US_MZ_FALSE;
      }

      comp_size = state.m_comp_size;
      cur_archive_file_ofs = state.m_cur_archive_file_ofs;

      method = US_MZ_DEFLATED;
    }

    pZip->m_pFree(pZip->m_pAlloc_opaque, pRead_buf);
  }

  US_MZ_FCLOSE(pSrc_file); pSrc_file = NULL;

  // no zip64 support yet
  if ((comp_size > 0xFFFFFFFF) || (cur_archive_file_ofs > 0xFFFFFFFF))
    return US_MZ_FALSE;

  if (!us_mz_zip_writer_create_local_dir_header(pZip, local_dir_header, (us_mz_uint16)archive_name_size, 0, uncomp_size, comp_size, uncomp_crc32, method, 0, dos_time, dos_date))
    return US_MZ_FALSE;

  if (pZip->m_pWrite(pZip->m_pIO_opaque, local_dir_header_ofs, local_dir_header, sizeof(local_dir_header)) != sizeof(local_dir_header))
    return US_MZ_FALSE;

  if (!us_mz_zip_writer_add_to_central_dir(pZip, pArchive_name, (us_mz_uint16)archive_name_size, NULL, 0, pComment, comment_size, uncomp_size, comp_size, uncomp_crc32, method, 0, dos_time, dos_date, local_dir_header_ofs, ext_attributes))
    return US_MZ_FALSE;

  pZip->m_total_files++;
  pZip->m_archive_size = cur_archive_file_ofs;

  return US_MZ_TRUE;
}
#endif // #ifndef US_MZ_NO_STDIO

us_mz_bool us_mz_zip_writer_add_from_zip_reader(us_mz_zip_archive *pZip, us_mz_zip_archive *pSource_zip, us_mz_uint file_index)
{
  us_mz_uint n, bit_flags, num_alignment_padding_bytes;
  us_mz_uint64 comp_bytes_remaining, local_dir_header_ofs;
  us_mz_uint64 cur_src_file_ofs, cur_dst_file_ofs;
  us_mz_uint32 local_header_u32[(US_MZ_ZIP_LOCAL_DIR_HEADER_SIZE + sizeof(us_mz_uint32) - 1) / sizeof(us_mz_uint32)]; us_mz_uint8 *pLocal_header = (us_mz_uint8 *)local_header_u32;
  us_mz_uint8 central_header[US_MZ_ZIP_CENTRAL_DIR_HEADER_SIZE];
  size_t orig_central_dir_size;
  us_mz_zip_internal_state *pState;
  void *pBuf; const us_mz_uint8 *pSrc_central_header;

  if ((!pZip) || (!pZip->m_pState) || (pZip->m_zip_mode != US_MZ_ZIP_MODE_WRITING))
    return US_MZ_FALSE;
  if (NULL == (pSrc_central_header = us_mz_zip_reader_get_cdh(pSource_zip, file_index)))
    return US_MZ_FALSE;
  pState = pZip->m_pState;

  num_alignment_padding_bytes = us_mz_zip_writer_compute_padding_needed_for_file_alignment(pZip);

  // no zip64 support yet
  if ((pZip->m_total_files == 0xFFFF) || ((pZip->m_archive_size + num_alignment_padding_bytes + US_MZ_ZIP_LOCAL_DIR_HEADER_SIZE + US_MZ_ZIP_CENTRAL_DIR_HEADER_SIZE) > 0xFFFFFFFF))
    return US_MZ_FALSE;

  cur_src_file_ofs = pSource_zip->m_archive_file_ofs + US_MZ_READ_LE32(pSrc_central_header + US_MZ_ZIP_CDH_LOCAL_HEADER_OFS);
  cur_dst_file_ofs = pZip->m_archive_size;

  if (pSource_zip->m_pRead(pSource_zip->m_pIO_opaque, cur_src_file_ofs, pLocal_header, US_MZ_ZIP_LOCAL_DIR_HEADER_SIZE) != US_MZ_ZIP_LOCAL_DIR_HEADER_SIZE)
    return US_MZ_FALSE;
  if (US_MZ_READ_LE32(pLocal_header) != US_MZ_ZIP_LOCAL_DIR_HEADER_SIG)
    return US_MZ_FALSE;
  cur_src_file_ofs += US_MZ_ZIP_LOCAL_DIR_HEADER_SIZE;

  if (!us_mz_zip_writer_write_zeros(pZip, cur_dst_file_ofs, num_alignment_padding_bytes))
    return US_MZ_FALSE;
  cur_dst_file_ofs += num_alignment_padding_bytes;
  local_dir_header_ofs = cur_dst_file_ofs;
  if (pZip->m_file_offset_alignment) { US_MZ_ASSERT((local_dir_header_ofs & (pZip->m_file_offset_alignment - 1)) == 0); }

  if (pZip->m_pWrite(pZip->m_pIO_opaque, cur_dst_file_ofs, pLocal_header, US_MZ_ZIP_LOCAL_DIR_HEADER_SIZE) != US_MZ_ZIP_LOCAL_DIR_HEADER_SIZE)
    return US_MZ_FALSE;
  cur_dst_file_ofs += US_MZ_ZIP_LOCAL_DIR_HEADER_SIZE;

  n = US_MZ_READ_LE16(pLocal_header + US_MZ_ZIP_LDH_FILENAME_LEN_OFS) + US_MZ_READ_LE16(pLocal_header + US_MZ_ZIP_LDH_EXTRA_LEN_OFS);
  comp_bytes_remaining = n + US_MZ_READ_LE32(pSrc_central_header + US_MZ_ZIP_CDH_COMPRESSED_SIZE_OFS);

  if (NULL == (pBuf = pZip->m_pAlloc(pZip->m_pAlloc_opaque, 1, (size_t)US_MZ_MAX(sizeof(us_mz_uint32) * 4, US_MZ_MIN((us_mz_uint64)US_MZ_ZIP_MAX_IO_BUF_SIZE, comp_bytes_remaining)))))
    return US_MZ_FALSE;

  while (comp_bytes_remaining)
  {
    n = (us_mz_uint)US_MZ_MIN((us_mz_uint64)US_MZ_ZIP_MAX_IO_BUF_SIZE, comp_bytes_remaining);
    if (pSource_zip->m_pRead(pSource_zip->m_pIO_opaque, cur_src_file_ofs, pBuf, n) != n)
    {
      pZip->m_pFree(pZip->m_pAlloc_opaque, pBuf);
      return US_MZ_FALSE;
    }
    cur_src_file_ofs += n;

    if (pZip->m_pWrite(pZip->m_pIO_opaque, cur_dst_file_ofs, pBuf, n) != n)
    {
      pZip->m_pFree(pZip->m_pAlloc_opaque, pBuf);
      return US_MZ_FALSE;
    }
    cur_dst_file_ofs += n;

    comp_bytes_remaining -= n;
  }

  bit_flags = US_MZ_READ_LE16(pLocal_header + US_MZ_ZIP_LDH_BIT_FLAG_OFS);
  if (bit_flags & 8)
  {
    // Copy data descriptor
    if (pSource_zip->m_pRead(pSource_zip->m_pIO_opaque, cur_src_file_ofs, pBuf, sizeof(us_mz_uint32) * 4) != sizeof(us_mz_uint32) * 4)
    {
      pZip->m_pFree(pZip->m_pAlloc_opaque, pBuf);
      return US_MZ_FALSE;
    }

    n = sizeof(us_mz_uint32) * ((US_MZ_READ_LE32(pBuf) == 0x08074b50) ? 4 : 3);
    if (pZip->m_pWrite(pZip->m_pIO_opaque, cur_dst_file_ofs, pBuf, n) != n)
    {
      pZip->m_pFree(pZip->m_pAlloc_opaque, pBuf);
      return US_MZ_FALSE;
    }

    cur_src_file_ofs += n;
    cur_dst_file_ofs += n;
  }
  pZip->m_pFree(pZip->m_pAlloc_opaque, pBuf);

  // no zip64 support yet
  if (cur_dst_file_ofs > 0xFFFFFFFF)
    return US_MZ_FALSE;

  orig_central_dir_size = pState->m_central_dir.m_size;

  memcpy(central_header, pSrc_central_header, US_MZ_ZIP_CENTRAL_DIR_HEADER_SIZE);
  US_MZ_WRITE_LE32(central_header + US_MZ_ZIP_CDH_LOCAL_HEADER_OFS, local_dir_header_ofs);
  if (!us_mz_zip_array_push_back(pZip, &pState->m_central_dir, central_header, US_MZ_ZIP_CENTRAL_DIR_HEADER_SIZE))
    return US_MZ_FALSE;

  n = US_MZ_READ_LE16(pSrc_central_header + US_MZ_ZIP_CDH_FILENAME_LEN_OFS) + US_MZ_READ_LE16(pSrc_central_header + US_MZ_ZIP_CDH_EXTRA_LEN_OFS) + US_MZ_READ_LE16(pSrc_central_header + US_MZ_ZIP_CDH_COMMENT_LEN_OFS);
  if (!us_mz_zip_array_push_back(pZip, &pState->m_central_dir, pSrc_central_header + US_MZ_ZIP_CENTRAL_DIR_HEADER_SIZE, n))
  {
    us_mz_zip_array_resize(pZip, &pState->m_central_dir, orig_central_dir_size, US_MZ_FALSE);
    return US_MZ_FALSE;
  }

  if (pState->m_central_dir.m_size > 0xFFFFFFFF)
    return US_MZ_FALSE;
  n = (us_mz_uint32)orig_central_dir_size;
  if (!us_mz_zip_array_push_back(pZip, &pState->m_central_dir_offsets, &n, 1))
  {
    us_mz_zip_array_resize(pZip, &pState->m_central_dir, orig_central_dir_size, US_MZ_FALSE);
    return US_MZ_FALSE;
  }

  pZip->m_total_files++;
  pZip->m_archive_size = cur_dst_file_ofs;

  return US_MZ_TRUE;
}

us_mz_bool us_mz_zip_writer_finalize_archive(us_mz_zip_archive *pZip)
{
  us_mz_zip_internal_state *pState;
  us_mz_uint64 central_dir_ofs, central_dir_size;
  us_mz_uint8 hdr[US_MZ_ZIP_END_OF_CENTRAL_DIR_HEADER_SIZE];

  if ((!pZip) || (!pZip->m_pState) || (pZip->m_zip_mode != US_MZ_ZIP_MODE_WRITING))
    return US_MZ_FALSE;

  pState = pZip->m_pState;

  // no zip64 support yet
  if ((pZip->m_total_files > 0xFFFF) || ((pZip->m_archive_size + pState->m_central_dir.m_size + US_MZ_ZIP_END_OF_CENTRAL_DIR_HEADER_SIZE) > 0xFFFFFFFF))
    return US_MZ_FALSE;

  central_dir_ofs = 0;
  central_dir_size = 0;
  if (pZip->m_total_files)
  {
    // Write central directory
    central_dir_ofs = pZip->m_archive_size;
    central_dir_size = pState->m_central_dir.m_size;
    pZip->m_central_directory_file_ofs = central_dir_ofs;
    if (pZip->m_pWrite(pZip->m_pIO_opaque, central_dir_ofs, pState->m_central_dir.m_p, (size_t)central_dir_size) != central_dir_size)
      return US_MZ_FALSE;
    pZip->m_archive_size += central_dir_size;
  }

  // Write end of central directory record
  US_MZ_CLEAR_OBJ(hdr);
  US_MZ_WRITE_LE32(hdr + US_MZ_ZIP_ECDH_SIG_OFS, US_MZ_ZIP_END_OF_CENTRAL_DIR_HEADER_SIG);
  US_MZ_WRITE_LE16(hdr + US_MZ_ZIP_ECDH_CDIR_NUM_ENTRIES_ON_DISK_OFS, pZip->m_total_files);
  US_MZ_WRITE_LE16(hdr + US_MZ_ZIP_ECDH_CDIR_TOTAL_ENTRIES_OFS, pZip->m_total_files);
  US_MZ_WRITE_LE32(hdr + US_MZ_ZIP_ECDH_CDIR_SIZE_OFS, central_dir_size);
  US_MZ_WRITE_LE32(hdr + US_MZ_ZIP_ECDH_CDIR_OFS_OFS, central_dir_ofs);

  if (pZip->m_pWrite(pZip->m_pIO_opaque, pZip->m_archive_size, hdr, sizeof(hdr)) != sizeof(hdr))
    return US_MZ_FALSE;
#ifndef US_MZ_NO_STDIO
  if ((pState->m_pFile) && (US_MZ_FFLUSH(pState->m_pFile) == EOF))
    return US_MZ_FALSE;
#endif // #ifndef US_MZ_NO_STDIO

  pZip->m_archive_size += sizeof(hdr);

  pZip->m_zip_mode = US_MZ_ZIP_MODE_WRITING_HAS_BEEN_FINALIZED;
  return US_MZ_TRUE;
}

us_mz_bool us_mz_zip_writer_finalize_heap_archive(us_mz_zip_archive *pZip, void **pBuf, size_t *pSize)
{
  if ((!pZip) || (!pZip->m_pState) || (!pBuf) || (!pSize))
    return US_MZ_FALSE;
  if (pZip->m_pWrite != us_mz_zip_heap_write_func)
    return US_MZ_FALSE;
  if (!us_mz_zip_writer_finalize_archive(pZip))
    return US_MZ_FALSE;

  *pBuf = pZip->m_pState->m_pMem;
  *pSize = pZip->m_pState->m_mem_size;
  pZip->m_pState->m_pMem = NULL;
  pZip->m_pState->m_mem_size = pZip->m_pState->m_mem_capacity = 0;
  return US_MZ_TRUE;
}

us_mz_bool us_mz_zip_writer_end(us_mz_zip_archive *pZip)
{
  us_mz_zip_internal_state *pState;
  us_mz_bool status = US_MZ_TRUE;
  if ((!pZip) || (!pZip->m_pState) || (!pZip->m_pAlloc) || (!pZip->m_pFree) || ((pZip->m_zip_mode != US_MZ_ZIP_MODE_WRITING) && (pZip->m_zip_mode != US_MZ_ZIP_MODE_WRITING_HAS_BEEN_FINALIZED)))
    return US_MZ_FALSE;

  pState = pZip->m_pState;
  pZip->m_pState = NULL;
  us_mz_zip_array_clear(pZip, &pState->m_central_dir);
  us_mz_zip_array_clear(pZip, &pState->m_central_dir_offsets);
  us_mz_zip_array_clear(pZip, &pState->m_sorted_central_dir_offsets);

#ifndef US_MZ_NO_STDIO
  if (pState->m_pFile)
  {
    US_MZ_FCLOSE(pState->m_pFile);
    pState->m_pFile = NULL;
  }
#endif // #ifndef US_MZ_NO_STDIO

  if ((pZip->m_pWrite == us_mz_zip_heap_write_func) && (pState->m_pMem))
  {
    pZip->m_pFree(pZip->m_pAlloc_opaque, pState->m_pMem);
    pState->m_pMem = NULL;
  }

  pZip->m_pFree(pZip->m_pAlloc_opaque, pState);
  pZip->m_zip_mode = US_MZ_ZIP_MODE_INVALID;
  return status;
}

#ifndef US_MZ_NO_STDIO
us_mz_bool us_mz_zip_add_mem_to_archive_file_in_place(const char *pZip_filename, const char *pArchive_name, const void *pBuf, size_t buf_size, const void *pComment, us_mz_uint16 comment_size, us_mz_uint level_and_flags)
{
  us_mz_bool status, created_new_archive = US_MZ_FALSE;
  us_mz_zip_archive zip_archive;
  struct US_MZ_FILE_STAT_STRUCT file_stat;
  US_MZ_CLEAR_OBJ(zip_archive);
  if ((int)level_and_flags < 0)
     level_and_flags = US_MZ_DEFAULT_LEVEL;
  if ((!pZip_filename) || (!pArchive_name) || ((buf_size) && (!pBuf)) || ((comment_size) && (!pComment)) || ((level_and_flags & 0xF) > US_MZ_UBER_COMPRESSION))
    return US_MZ_FALSE;
  if (!us_mz_zip_writer_validate_archive_name(pArchive_name))
    return US_MZ_FALSE;
  if (US_MZ_FILE_STAT(pZip_filename, &file_stat) != 0)
  {
    // Create a new archive.
    if (!us_mz_zip_writer_init_file(&zip_archive, pZip_filename, 0))
      return US_MZ_FALSE;
    created_new_archive = US_MZ_TRUE;
  }
  else
  {
    // Append to an existing archive.
    if (!us_mz_zip_reader_init_file(&zip_archive, pZip_filename, level_and_flags | US_MZ_ZIP_FLAG_DO_NOT_SORT_CENTRAL_DIRECTORY))
      return US_MZ_FALSE;
    if (!us_mz_zip_writer_init_from_reader(&zip_archive, pZip_filename))
    {
      us_mz_zip_reader_end(&zip_archive);
      return US_MZ_FALSE;
    }
  }
  status = us_mz_zip_writer_add_mem_ex(&zip_archive, pArchive_name, pBuf, buf_size, pComment, comment_size, level_and_flags, 0, 0);
  // Always finalize, even if adding failed for some reason, so we have a valid central directory. (This may not always succeed, but we can try.)
  if (!us_mz_zip_writer_finalize_archive(&zip_archive))
    status = US_MZ_FALSE;
  if (!us_mz_zip_writer_end(&zip_archive))
    status = US_MZ_FALSE;
  if ((!status) && (created_new_archive))
  {
    // It's a new archive and something went wrong, so just delete it.
    int ignoredStatus = US_MZ_DELETE_FILE(pZip_filename);
    (void)ignoredStatus;
  }
  return status;
}

void *us_mz_zip_extract_archive_file_to_heap(const char *pZip_filename, const char *pArchive_name, size_t *pSize, us_mz_uint flags)
{
  int file_index;
  us_mz_zip_archive zip_archive;
  void *p = NULL;

  if (pSize)
    *pSize = 0;

  if ((!pZip_filename) || (!pArchive_name))
    return NULL;

  US_MZ_CLEAR_OBJ(zip_archive);
  if (!us_mz_zip_reader_init_file(&zip_archive, pZip_filename, flags | US_MZ_ZIP_FLAG_DO_NOT_SORT_CENTRAL_DIRECTORY))
    return NULL;

  if ((file_index = us_mz_zip_reader_locate_file(&zip_archive, pArchive_name, NULL, flags)) >= 0)
    p = us_mz_zip_reader_extract_to_heap(&zip_archive, file_index, pSize, flags);

  us_mz_zip_reader_end(&zip_archive);
  return p;
}

#endif // #ifndef US_MZ_NO_STDIO

#endif // #ifndef US_MZ_NO_ARCHIVE_WRITING_APIS

#endif // #ifndef US_MZ_NO_ARCHIVE_APIS

#ifdef __cplusplus
}
#endif

#endif // US_MZ_HEADER_FILE_ONLY

/*
  This is free and unencumbered software released into the public domain.

  Anyone is free to copy, modify, publish, use, compile, sell, or
  distribute this software, either in source code form or as a compiled
  binary, for any purpose, commercial or non-commercial, and by any
  means.

  In jurisdictions that recognize copyright laws, the author or authors
  of this software dedicate any and all copyright interest in the
  software to the public domain. We make this dedication for the benefit
  of the public at large and to the detriment of our heirs and
  successors. We intend this dedication to be an overt act of
  relinquishment in perpetuity of all present and future rights to this
  software under copyright law.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
  IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
  OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
  ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
  OTHER DEALINGS IN THE SOFTWARE.

  For more information, please refer to <https://unlicense.org/>
*/
