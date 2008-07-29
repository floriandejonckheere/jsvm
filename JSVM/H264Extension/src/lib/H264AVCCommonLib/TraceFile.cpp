
#include "H264AVCCommonLib.h"
#include "H264AVCCommonLib/TraceFile.h"



#ifdef MSYS_WIN32
#define snprintf _snprintf
#endif


H264AVC_NAMESPACE_BEGIN



TraceFile::TraceFile()
{
}


TraceFile::~TraceFile()
{
}

Bool  TraceFile::sm_bDisable = false;
UInt  TraceFile::sm_uiLayer;
FILE* TraceFile::sm_fTrace      [MAX_TRACE_LAYERS];
UInt  TraceFile::sm_uiFrameNum  [MAX_TRACE_LAYERS];
UInt  TraceFile::sm_uiSliceNum  [MAX_TRACE_LAYERS];
UInt  TraceFile::sm_uiPosCounter[MAX_TRACE_LAYERS];
Char  TraceFile::sm_acLine      [MAX_LINE_LENGTH] ;
Char  TraceFile::sm_acType      [9];
Char  TraceFile::sm_acPos       [9];
Char  TraceFile::sm_acCode      [6];
Char  TraceFile::sm_acBits      [MAX_BITS_LENGTH];


ErrVal
TraceFile::disable()
{
  sm_bDisable = true;
  return Err::m_nOK;
}

ErrVal
TraceFile::enable()
{
  sm_bDisable = false;
  return Err::m_nOK;
}


ErrVal
TraceFile::initTrace()
{
  sm_uiLayer    = 0;
  sm_acLine[0]  = '\0';
  sm_acType[0]  = '\0';
  sm_acPos [0]  = '\0';
  sm_acCode[0]  = '\0';
  sm_acBits[0]  = '\0';
  for( UInt ui = 0; ui < MAX_TRACE_LAYERS; ui++ )
  {
    sm_fTrace      [ui] = 0;
    sm_uiFrameNum  [ui] = 0;
    sm_uiSliceNum  [ui] = 0;
    sm_uiPosCounter[ui] = 0;
  }

  return Err::m_nOK;
}


ErrVal
TraceFile::openTrace( const Char* pucBaseFilename )
{
  for( UInt ui = 0; ui < MAX_TRACE_LAYERS; ui++ )
  {
    Char file[1000];
    ::snprintf( file, 1000, "%s_L%d.txt", pucBaseFilename, ui );
    ROT( NULL == ( sm_fTrace[ui] = ::fopen( file, "wt" ) ) );
  }

  return Err::m_nOK;
}


ErrVal
TraceFile::closeTrace ()
{
  for( UInt ui = 0; ui < MAX_TRACE_LAYERS; ui++ )
  {
    if( sm_fTrace[ui] )
    {
      ::fclose( sm_fTrace[ui] );
    }
  }

  return Err::m_nOK;
}


ErrVal
TraceFile::setLayer( UInt uiLayerId )
{
  ROT( uiLayerId >= MAX_TRACE_LAYERS );
  sm_uiLayer = uiLayerId;
  return Err::m_nOK;
}


ErrVal
TraceFile::startNalUnit()
{
  ROTRS( sm_bDisable, Err::m_nOK );
  RNOK( printHeading("Nal Unit") );
  return Err::m_nOK;
}


ErrVal
TraceFile::startFrame()
{
  ROTRS( sm_bDisable, Err::m_nOK );
  sm_uiFrameNum[sm_uiLayer]++;
  sm_uiSliceNum[sm_uiLayer]=0;

  return Err::m_nOK;
}


ErrVal
TraceFile::startSlice()
{
  ROTRS( sm_bDisable, Err::m_nOK );
  Char acSliceHead[100];
  ::snprintf( acSliceHead, 100, "Slice # %d Frame # %d", sm_uiSliceNum[sm_uiLayer], sm_uiFrameNum[sm_uiLayer] );

  RNOK( printHeading( acSliceHead ) );
  sm_uiSliceNum[sm_uiLayer]++;

  return Err::m_nOK;
}


ErrVal
TraceFile::startMb( Int iMbAddress )
{
  ROTRS( sm_bDisable, Err::m_nOK );
  Char acMbHead[100];
  ::snprintf( acMbHead, 100, "MB # %d", iMbAddress );
  RNOK( printHeading( acMbHead ) );

  return Err::m_nOK;
}


ErrVal
TraceFile::printHeading( const Char* pcString )
{
  ROTRS( sm_bDisable, Err::m_nOK );
  sm_uiPosCounter[sm_uiLayer] = 0;
  if( ! sm_fTrace[0] )
  {
    sm_acLine[0] = '\0';
    return Err::m_nOK;
  }

  ::snprintf( sm_acLine, MAX_LINE_LENGTH, "-------------------- %s --------------------\n", pcString );
  ::fprintf ( sm_fTrace[sm_uiLayer], sm_acLine );
  ::fflush  ( sm_fTrace[sm_uiLayer] );
  sm_acLine[0] = '\0';

  return Err::m_nOK;
}


ErrVal
TraceFile::countBits( UInt uiBitCount )
{
  ROTRS( sm_bDisable, Err::m_nOK );
  sm_uiPosCounter[sm_uiLayer] += uiBitCount;
  return Err::m_nOK;
}



ErrVal
TraceFile::printPos()
{
  ROTRS( sm_bDisable, Err::m_nOK );
  ::snprintf( sm_acPos, 8, "@%d", sm_uiPosCounter[sm_uiLayer] );
  return Err::m_nOK;
}


ErrVal
TraceFile::printString( const Char* pcString )
{
  ROTRS( sm_bDisable, Err::m_nOK );
  ::strncat( sm_acLine, pcString, MAX_LINE_LENGTH );
  return Err::m_nOK;
}


ErrVal
TraceFile::printType( const Char* pcString )
{
  ROTRS( sm_bDisable, Err::m_nOK );
  ::snprintf( sm_acType, 8, "%s", pcString);
  return Err::m_nOK;
}


ErrVal
TraceFile::printVal( UInt uiVal )
{
  ROTRS( sm_bDisable, Err::m_nOK );
  Char tmp[8];
  ::snprintf( tmp, 8, "%3u", uiVal);
  ::strncat( sm_acLine, tmp, MAX_LINE_LENGTH);
  return Err::m_nOK;
}


ErrVal
TraceFile::printVal( Int iVal )
{
  ROTRS( sm_bDisable, Err::m_nOK );
  Char tmp[8];
  ::snprintf( tmp, 8, "%3i", iVal );
  ::strncat( sm_acLine, tmp, MAX_LINE_LENGTH );
  return Err::m_nOK;
}


ErrVal
TraceFile::printXVal( UInt uiVal )
{
  ROTRS( sm_bDisable, Err::m_nOK );
  Char tmp[8];
  ::snprintf( tmp, 8, "0x%04x", uiVal );
  ::strncat( sm_acLine, tmp, MAX_LINE_LENGTH);
  return Err::m_nOK;
}


ErrVal
TraceFile::addBits( UInt uiVal,
                    UInt uiLength )
{
  ROTRS( sm_bDisable, Err::m_nOK );
  ROT( uiLength > 32 );

  Char acBuffer[33];
  UInt i;
  for ( i = 0; i < uiLength; i++ )
  {
    acBuffer[i] = '0' + ( ( uiVal & ( 1 << (uiLength - i - 1) ) ) >> (uiLength - i - 1 ) );
  }
  acBuffer[uiLength] = '\0';

  i = strlen( sm_acBits );
  if( i < 2 )
  {
    sm_acBits[0] = '[';
    sm_acBits[1] = '\0';
  }
  else
  {
    sm_acBits[i-1] = '\0';
  }
  sm_acBits[sizeof(sm_acBits)-1]='\0';
  strncat( sm_acBits, acBuffer, sizeof(sm_acBits)-1-strlen( sm_acBits ) );
  strncat( sm_acBits, "]",      sizeof(sm_acBits)-1-strlen( sm_acBits ) );

  return Err::m_nOK;
}


ErrVal
TraceFile::printBits( UInt uiVal,
                      UInt uiLength )
{
  ROTRS( sm_bDisable, Err::m_nOK );
  sm_acBits[0] = '[';
  sm_acBits[1] = ']';
  sm_acBits[2] = '\0';
  RNOK( addBits( uiVal, uiLength ) );
  return Err::m_nOK;
}


ErrVal
TraceFile::printCode( UInt uiVal )
{
  ROTRS( sm_bDisable, Err::m_nOK );
  ::snprintf( sm_acCode, MAX_LINE_LENGTH, "%u", uiVal );
  return Err::m_nOK;
}


ErrVal
TraceFile::printCode(Int iVal)
{
  ROTRS( sm_bDisable, Err::m_nOK );
  ::snprintf( sm_acCode, MAX_LINE_LENGTH, "%i", iVal );
  return Err::m_nOK;
}


ErrVal
TraceFile::newLine()
{
  ROTRS( sm_bDisable, Err::m_nOK );
  if( ! sm_fTrace[0] )
  {
    sm_acLine[0] = '\0';
    sm_acType[0] ='\0';
    sm_acCode[0] ='\0';
    sm_acBits[0] ='\0';
    sm_acPos [0] ='\0';
    return Err::m_nOK;
  }

  ::fprintf( sm_fTrace[sm_uiLayer], "%-6s",   sm_acPos  );
  ::fprintf( sm_fTrace[sm_uiLayer], " %-50s", sm_acLine );
  ::fprintf( sm_fTrace[sm_uiLayer], " %-8s",  sm_acType );
  ::fprintf( sm_fTrace[sm_uiLayer], " %5s",   sm_acCode );
  ::fprintf( sm_fTrace[sm_uiLayer], " %s",    sm_acBits );
  ::fprintf( sm_fTrace[sm_uiLayer], "\n");
  ::fflush ( sm_fTrace[sm_uiLayer] );

  sm_acLine[0] ='\0';
  sm_acType[0] ='\0';
  sm_acCode[0] ='\0';
  sm_acBits[0] ='\0';
  sm_acPos [0] ='\0';

  return Err::m_nOK;
}

H264AVC_NAMESPACE_END
