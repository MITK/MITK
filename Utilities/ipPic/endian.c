
union {
  unsigned short s;
  unsigned char b;
} t = { 0x1234 };

void main( void )
{

  /*t.s = 0x1234;*/

  if( t.b == 0x12 )
    printf( "big\n" );
  else
    printf( "little\n" );
  
}
