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





#include "ReadBitstreamFile.h"


ReadBitstreamFile::ReadBitstreamFile()
{
}


ReadBitstreamFile::~ReadBitstreamFile()
{
} 

ErrVal ReadBitstreamFile::releasePacket( BinData* pcBinData )
{
  ROT( NULL == pcBinData );
  pcBinData->deleteData();
  delete pcBinData;
  return Err::m_nOK;
}

ErrVal ReadBitstreamFile::extractPacket( BinData*& rpcBinData, Bool& rbEOS )
{
	UInt	 dwBytesRead;
	UInt	 dwLength = 0;
  Int64  iPos;
	UInt	 n;
	UChar  Buffer[0x400];
  UChar  *puc = NULL;
  UInt   uiCond;
  UInt  uiZeros;

  ROT( NULL == ( rpcBinData = new BinData ) );

  rbEOS     = false;
  // exit if there's no bitstream
  ROFS( m_cFile.is_open());

  // we max read any number of zeros
  uiZeros = 0;
  do
  {
    m_cFile.read( Buffer, 1, dwBytesRead );
    uiZeros++;
  } while ((dwBytesRead==1)&&(Buffer[0]==0));

  if( 0 == dwBytesRead )
  {
    rbEOS = true;
    return Err::m_nOK;
  }

  // next we expect "0x01"
  ROTS(Buffer[0]!=0x01);

  // the is a min of two zeros in a startcode
  ROTS(uiZeros<2);

  // get the current position
  iPos = m_cFile.tell(); 

 	// read at first 0x400 bytes
  m_cFile.read( Buffer, 0x400, dwBytesRead );

  ROFS( dwBytesRead );

	do
	{
    puc = Buffer;
    uiCond = 0;

    for( n = 0; n < dwBytesRead-2; n++, puc++)
    {
      uiCond = (puc[0] == 0 && puc[1] == 0 && puc[2] == 1 );
      if( uiCond )
      {
         break;
      }
    }

		if( uiCond ) 
		{
			break;
		}

    // found no synch so go on
		dwLength += dwBytesRead-2 ; 
    // step 3 bytes back in the stream
    RNOK( m_cFile.seek( -2, SEEK_CUR ) );

    // read the next chunk or return out of bytes
    m_cFile.read( Buffer, 0x400, dwBytesRead );

    if( 2 == dwBytesRead )
    {
      n = 2;
      // end of stream cause we former stepped 4 bytes back
      break;      // this is the last pack
    }
	}
	while( true );

  // calc the complete length
  dwLength += n;

  if( 0 == dwLength )
  {
    rbEOS = true;
    return Err::m_nOK;
  }

  rpcBinData->set( new UChar[dwLength], dwLength );
  ROT( NULL == rpcBinData->data() );
  
  // seek the bitstream to the prev start position
  RNOK( m_cFile.seek( iPos, SEEK_SET ) );

  // read the access unit into the transport buffer
  RNOK( m_cFile.read( rpcBinData->data(), dwLength, dwBytesRead ) );

  return Err::m_nOK;
}


ErrVal ReadBitstreamFile::init( const std::string& rcFileName )
{
	// try to open the bitstream binary
  if ( Err::m_nOK != m_cFile.open( rcFileName, LargeFile::OM_READONLY ) )
  {
    std::cerr << "failed to open input bitstream file " << rcFileName.data() << std::endl;
    return Err::m_nERR;
  }

  UChar  aucBuffer[0x4];
  UInt uiBytesRead;
  RNOK( m_cFile.read( aucBuffer, 4, uiBytesRead ) );

  UInt uiStartPos = (aucBuffer[0] == 0 && aucBuffer[1] == 0 && aucBuffer[2] == 1) ? 1 : 0;
  
  RNOK( m_cFile.seek( uiStartPos, SEEK_SET ) );

  return Err::m_nOK;
}

ErrVal ReadBitstreamFile::create( ReadBitstreamFile*& rpcReadBitstreamFile )
{
  rpcReadBitstreamFile = new ReadBitstreamFile;
  ROT( NULL == rpcReadBitstreamFile );
  return Err::m_nOK;
}


ErrVal ReadBitstreamFile::getPosition( Int& iPos )
{
  ROFS( m_cFile.is_open());

  iPos = (Int)m_cFile.tell(); 

  return Err::m_nOK;
}


ErrVal ReadBitstreamFile::setPosition( Int  iPos )
{
  ROFS( m_cFile.is_open());

  // seek the bitstream to the prev start position
  RNOK( m_cFile.seek( iPos, SEEK_SET ) );

  return Err::m_nOK;
}


ErrVal ReadBitstreamFile::uninit()
{
  if( m_cFile.is_open() )
  { 
    RNOK( m_cFile.close() );
  }
  return Err::m_nOK;
}

ErrVal ReadBitstreamFile::destroy()
{
  AOT_DBG( m_cFile.is_open() );

  RNOK( uninit() );
  
  delete this;
  return Err::m_nOK;
}
