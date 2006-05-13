/*
********************************************************************************

NOTE - One of the two copyright statements below may be chosen
       that applies for the software.

********************************************************************************

This software module was originally developed by

Heiko Schwarz    (Fraunhofer HHI),
Tobias Hinz      (Fraunhofer HHI),
Karsten Suehring (Fraunhofer HHI)

in the course of development of the ISO/IEC 14496-10:2005 Amd.1 (Scalable Video
Coding) for reference purposes and its performance may not have been optimized.
This software module is an implementation of one or more tools as specified by
the ISO/IEC 14496-10:2005 Amd.1 (Scalable Video Coding).

Those intending to use this software module in products are advised that its
use may infringe existing patents. ISO/IEC have no liability for use of this
software module or modifications thereof.

Assurance that the originally developed software module can be used
(1) in the ISO/IEC 14496-10:2005 Amd.1 (Scalable Video Coding) once the
ISO/IEC 14496-10:2005 Amd.1 (Scalable Video Coding) has been adopted; and
(2) to develop the ISO/IEC 14496-10:2005 Amd.1 (Scalable Video Coding): 

To the extent that Fraunhofer HHI owns patent rights that would be required to
make, use, or sell the originally developed software module or portions thereof
included in the ISO/IEC 14496-10:2005 Amd.1 (Scalable Video Coding) in a
conforming product, Fraunhofer HHI will assure the ISO/IEC that it is willing
to negotiate licenses under reasonable and non-discriminatory terms and
conditions with applicants throughout the world.

Fraunhofer HHI retains full right to modify and use the code for its own
purpose, assign or donate the code to a third party and to inhibit third
parties from using the code for products that do not conform to MPEG-related
ITU Recommendations and/or ISO/IEC International Standards. 

This copyright notice must be included in all copies or derivative works.
Copyright (c) ISO/IEC 2005. 

********************************************************************************

COPYRIGHT AND WARRANTY INFORMATION

Copyright 2005, International Telecommunications Union, Geneva

The Fraunhofer HHI hereby donate this source code to the ITU, with the following
understanding:
    1. Fraunhofer HHI retain the right to do whatever they wish with the
       contributed source code, without limit.
    2. Fraunhofer HHI retain full patent rights (if any exist) in the technical
       content of techniques and algorithms herein.
    3. The ITU shall make this code available to anyone, free of license or
       royalty fees.

DISCLAIMER OF WARRANTY

These software programs are available to the user without any license fee or
royalty on an "as is" basis. The ITU disclaims any and all warranties, whether
express, implied, or statutory, including any implied warranties of
merchantability or of fitness for a particular purpose. In no event shall the
contributor or the ITU be liable for any incidental, punitive, or consequential
damages of any kind whatsoever arising from the use of these programs.

This disclaimer of warranty extends to the user of these programs and user's
customers, employees, agents, transferees, successors, and assigns.

The ITU does not represent or warrant that the programs furnished hereunder are
free of infringement of any third-party patents. Commercial implementations of
ITU-T Recommendations, including shareware, may be subject to royalty fees to
patent holders. Information regarding the ITU-T patent policy is available from 
the ITU Web site at http://www.itu.int.

THIS IS NOT A GRANT OF PATENT RIGHTS - SEE THE ITU-T PATENT POLICY.

********************************************************************************
*/





#include "H264AVCVideoIoLib.h"
#include "ReadYuvFile.h"


ReadYuvFile::ReadYuvFile()  
: m_uiLumPicHeight( 0 )
, m_uiLumPicWidth ( 0 )
, m_uiStartLine   ( 0 )
, m_uiEndLine     ( MSYS_UINT_MAX )
, m_eFillMode     ( FILL_CLEAR )
{
}

ReadYuvFile::~ReadYuvFile()
{
}


ErrVal ReadYuvFile::create( ReadYuvFile*& rpcReadYuvFile )
{
  rpcReadYuvFile = new ReadYuvFile;
  ROT( NULL == rpcReadYuvFile );
  return Err::m_nOK;
}


ErrVal ReadYuvFile::destroy()
{
  AOT_DBG( m_cFile.is_open() );
  RNOK( uninit() )
  delete this;
  return Err::m_nOK;
}

ErrVal ReadYuvFile::uninit()
{
  if( m_cFile.is_open() )
  {
  	RNOK( m_cFile.close() );
  }
  m_uiLumPicHeight = 0;
  m_uiLumPicWidth  = 0;
  return Err::m_nOK;
}


ErrVal ReadYuvFile::init( const std::string& rcFileName, UInt uiLumPicHeight, UInt uiLumPicWidth, UInt uiStartLine, UInt uiEndLine, FillMode eFillMode )
{
  ROT( 0 == uiLumPicHeight );
  ROT( 0 == uiLumPicWidth );
  
  m_uiLumPicWidth  = uiLumPicWidth;
  m_uiLumPicHeight = uiLumPicHeight;

  m_uiStartLine = uiStartLine;
  m_uiEndLine   = uiEndLine;
  m_eFillMode   = eFillMode;

  if( Err::m_nOK != m_cFile.open( rcFileName, LargeFile::OM_READONLY ) )
  { 
    std::cerr << "failed to open YUV input file " << rcFileName.data() << std::endl;
    return Err::m_nERR;
  }

  return Err::m_nOK;
}




ErrVal ReadYuvFile::xReadPlane( UChar *pucDest, UInt uiBufHeight, UInt uiBufWidth, UInt uiBufStride, UInt uiPicHeight, UInt uiPicWidth, UInt uiStartLine, UInt uiEndLine )
{
  UInt uiClearSize = uiBufWidth - uiPicWidth;

  ROT( 0 > (Int)uiClearSize );
  ROT( uiBufHeight < uiPicHeight );

  // clear skiped buffer above reading section and skip in file
  if( 0 != uiStartLine )
  {
    UInt uiLines = uiStartLine;
    ::memset( pucDest, 0, uiBufWidth * uiLines );
    pucDest += uiBufStride * uiLines;
    RNOKRS(m_cFile.seek( uiPicWidth * uiLines, SEEK_CUR), Err::m_nEndOfFile);
  }


  UInt uiEnd = min (uiPicHeight, uiEndLine);
  
  for( UInt yR = uiStartLine; yR < uiEnd; yR++ )
  {
    UInt uiBytesRead;
	  RNOKS( m_cFile.read( pucDest, uiPicWidth, uiBytesRead ) );
    ::memset( &pucDest[uiPicWidth], 0, uiClearSize );
    pucDest += uiBufStride;
  }

  // clear skiped buffer below reading section and skip in file
  if( uiEnd != uiPicHeight )
  {
    UInt uiLines = uiPicHeight - uiEnd;
    ::memset( pucDest, 0, uiBufWidth * uiLines );
    pucDest += uiBufStride * uiLines;
    RNOKRS(m_cFile.seek( uiPicWidth * uiLines, SEEK_CUR), Err::m_nEndOfFile);
  }

  // clear remaining buffer
  if( uiPicHeight != uiBufHeight )
  {
    if( uiEnd != uiPicHeight )
    {
      UInt uiLines = uiBufHeight - uiPicHeight;
      ::memset( pucDest, 0, uiBufWidth * uiLines);
    }
    else
    {
      switch( m_eFillMode )
      {
        case FILL_CLEAR:
        {
          UInt uiLines = uiBufHeight - uiPicHeight;
          ::memset( pucDest, 0, uiBufWidth * uiLines);
        }
        break;
        case FILL_FRAME:
        {
          for( UInt y = uiPicHeight; y < uiBufHeight; y++ )
          {
            memcpy( pucDest, pucDest - uiBufStride, uiBufStride );
            pucDest += uiBufStride;
          }
        }
        break;
        case FILL_FIELD:
        {
          ROT( (uiBufHeight - uiPicHeight) & 1 );
          for( UInt y = uiPicHeight; y < uiBufHeight; y+=2 )
          {
            memcpy( pucDest, pucDest - 2*uiBufStride, 2*uiBufStride );
            pucDest += 2*uiBufStride;
          }
        }
        break;
        default:
          AF()
        break;
      }
    }
  }
  
  return Err::m_nOK;
}


ErrVal ReadYuvFile::readFrame( UChar *pLum,
                               UChar *pCb,
                               UChar *pCr,
                               UInt uiBufHeight,
                               UInt uiBufWidth,
                               UInt uiBufStride)
{
  ROT( uiBufHeight < m_uiLumPicHeight || uiBufWidth < m_uiLumPicWidth );
  
  UInt uiPicHeight = m_uiLumPicHeight;
  UInt uiPicWidth  = m_uiLumPicWidth;
  UInt uiClearSize = uiBufWidth - uiPicWidth;
  UInt uiStartLine = m_uiStartLine;
  UInt uiEndLine   = m_uiEndLine;

  RNOKS( xReadPlane( pLum, uiBufHeight, uiBufWidth, uiBufStride, uiPicHeight, uiPicWidth, uiStartLine, uiEndLine ) );

  uiPicHeight  >>= 1;
  uiPicWidth   >>= 1;
  uiClearSize  >>= 1;
  uiBufHeight  >>= 1;
  uiBufWidth   >>= 1;
  uiBufStride  >>= 1;
  uiStartLine  >>= 1;
  uiEndLine    >>= 1;

  RNOKS( xReadPlane( pCb, uiBufHeight, uiBufWidth, uiBufStride, uiPicHeight, uiPicWidth, uiStartLine, uiEndLine ) );
  RNOKS( xReadPlane( pCr, uiBufHeight, uiBufWidth, uiBufStride, uiPicHeight, uiPicWidth, uiStartLine, uiEndLine ) );

  return Err::m_nOK;

}


