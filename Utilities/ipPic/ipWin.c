
#ifdef WIN
#include <windows.h>
#include <stdio.h>

DWORD hfread(LPSTR ptr, DWORD size, DWORD n, FILE *stream)
{
  if( (size < 0xffff) && (n < 0xffff) )
    return fread( ptr, (WORD)size, (WORD)n, stream );
  else
    {
      DWORD Bytes;
      BYTE huge *lpwork;

      Bytes = size*n;

      for( lpwork = ptr; Bytes > 0xffff; Bytes-=0xffff, lpwork+=0xffff )
        fread( (char far*)lpwork, 0xffff, 1, stream );

      return fread( (char far*)lpwork, (WORD)Bytes, 1, stream );
    }
}
#endif

