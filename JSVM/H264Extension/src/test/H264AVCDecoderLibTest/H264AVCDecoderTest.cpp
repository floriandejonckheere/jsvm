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



#include "H264AVCDecoderLibTest.h"
#include "H264AVCDecoderTest.h"


H264AVCDecoderTest::H264AVCDecoderTest() :
  m_pcH264AVCDecoder( NULL ),
  m_pcH264AVCDecoderSuffix( NULL ), //JVT-S036 lsj
  m_pcParameter( NULL ),
  m_cActivePicBufferList( ),
  m_cUnusedPicBufferList( )
{
}


H264AVCDecoderTest::~H264AVCDecoderTest()
{
}


ErrVal H264AVCDecoderTest::create( H264AVCDecoderTest*& rpcH264AVCDecoderTest )
{
  rpcH264AVCDecoderTest = new H264AVCDecoderTest;
  ROT( NULL == rpcH264AVCDecoderTest );
  return Err::m_nOK;
}


ErrVal H264AVCDecoderTest::init( DecoderParameter *pcDecoderParameter, WriteYuvToFile *pcWriterYuv, ReadBitstreamFile *pcReadBitstreamFile ) //TMM_EC
{
  ROT( NULL == pcDecoderParameter );

  m_pcParameter = pcDecoderParameter;
  m_pcParameter->nResult = -1;

	m_pcWriteYuv	=	pcWriterYuv;
  m_pcReadBitstream = (ReadBitstreamIf*)pcReadBitstreamFile;

  RNOK( h264::CreaterH264AVCDecoder::create( m_pcH264AVCDecoder ) );
	m_pcH264AVCDecoder->setec( m_pcParameter->uiErrorConceal);

	RNOK( h264::CreaterH264AVCDecoder::create( m_pcH264AVCDecoderSuffix ) );  //JVT-S036 lsj
  return Err::m_nOK;
}

ErrVal	H264AVCDecoderTest::setec( UInt uiErrorConceal)
{
	return	m_pcH264AVCDecoder->setec( uiErrorConceal);
}

ErrVal H264AVCDecoderTest::destroy()
{

  if( NULL != m_pcH264AVCDecoder )       
  {
    RNOK( m_pcH264AVCDecoder->destroy() );       
  }

  if( NULL != m_pcH264AVCDecoderSuffix )       
  {//JVT-S036 lsj
    RNOK( m_pcH264AVCDecoderSuffix->destroy() );       
  }

  AOF( m_cActivePicBufferList.empty() );
  
  //===== delete picture buffer =====
  PicBufferList::iterator iter;
  for( iter = m_cUnusedPicBufferList.begin(); iter != m_cUnusedPicBufferList.end(); iter++ )
  {
    delete (*iter)->getBuffer();
    delete (*iter);
  }
  for( iter = m_cActivePicBufferList.begin(); iter != m_cActivePicBufferList.end(); iter++ )
  {
    delete (*iter)->getBuffer();
    delete (*iter);
  }

  delete this;
  return Err::m_nOK;
}


ErrVal H264AVCDecoderTest::xGetNewPicBuffer ( PicBuffer*& rpcPicBuffer, UInt uiSize )
{
  if( m_cUnusedPicBufferList.empty() )
  {
    rpcPicBuffer = new PicBuffer( new UChar[ uiSize ] );
  }
  else
  {
    rpcPicBuffer = m_cUnusedPicBufferList.popFront();
  }

  m_cActivePicBufferList.push_back( rpcPicBuffer );
  return Err::m_nOK;
}


ErrVal H264AVCDecoderTest::xRemovePicBuffer( PicBufferList& rcPicBufferUnusedList )
{
  while( ! rcPicBufferUnusedList.empty() )
  {
    PicBuffer* pcBuffer = rcPicBufferUnusedList.popFront();

    if( NULL != pcBuffer )
    {
      PicBufferList::iterator  begin = m_cActivePicBufferList.begin();
      PicBufferList::iterator  end   = m_cActivePicBufferList.end  ();
      PicBufferList::iterator  iter  = std::find( begin, end, pcBuffer );

      ROT( iter == end ); // there is something wrong if the address is not in the active list

      AOT( pcBuffer->isUsed() )
      m_cUnusedPicBufferList.push_back( pcBuffer );
      m_cActivePicBufferList.erase    (  iter );
    }
  }
  return Err::m_nOK;
}

ErrVal H264AVCDecoderTest::appendFragment(UInt uiTotalLength, UInt uiFragNb, BinData* pcBinDataTmp[10], UInt *auiStartPos, 
                                          UInt * auiEndPos, Bool &bConcatenated, BinData* pcBinData,
                                          UChar*  apucFragBuffers [] )
{
    // append fragments
    UInt uiOffset = 0;
    UInt uiFrag = 0;
    for( uiFrag = 0; uiFrag<uiFragNb+1; uiFrag++)
    {
      apucFragBuffers[uiFrag] = pcBinData->data() + uiOffset;
      memcpy(pcBinData->data()+uiOffset, pcBinDataTmp[uiFrag]->data() + auiStartPos[uiFrag], auiEndPos[uiFrag]-auiStartPos[uiFrag]);
      uiOffset += auiEndPos[uiFrag]-auiStartPos[uiFrag];
      RNOK( m_pcReadBitstream->releasePacket( pcBinDataTmp[uiFrag] ) );
      pcBinDataTmp[uiFrag] = NULL;
      m_pcH264AVCDecoder->decreaseNumOfNALInAU();
		  if(uiFrag > 0) 
	  	  bConcatenated = true;
    }
    apucFragBuffers[uiFrag] = pcBinData->data()+uiOffset;

    return Err::m_nOK;
}

ErrVal H264AVCDecoderTest::initPacketToDecode(Bool &bDecode, BinData*& pcBinData, BinDataAccessor* cBinDataAccessor, Bool& bEOS, UInt& uiNalUnitType, 
                                              UInt& uiMbX, UInt& uiMbY, UInt& uiSize, UInt uiPreNalUnitType, Int iPos, 
                                              MyList<BinData*>&	cVirtualSliceList, Bool bVirtualSlice)
{
  //JVT-P031
  Bool bFragmented = false;
  Bool bDiscardable = false;
  Bool bStart = false;
  Bool bFirst = true;
  UInt uiTotalLength = 0;
#define MAX_FRAGMENTS 10 // hard-coded
  BinData* pcBinDataTmp[MAX_FRAGMENTS];
  BinDataAccessor cBinDataAccessorTmp[MAX_FRAGMENTS];
  UInt uiFragNb, auiStartPos[MAX_FRAGMENTS], auiEndPos[MAX_FRAGMENTS];
	Bool bConcatenated = false; //FRAG_FIX_3
  uiFragNb = 0;
  bEOS = false;
  pcBinData = 0;
  // set once for all the fragments
  Bool  bFgsParallelDecoding = false;

  while(!bStart && !bEOS)
  {
    if(bFirst)
    {
      if(!bVirtualSlice)
        RNOK( m_pcReadBitstream->setPosition(iPos) );
      bFirst = false;
    }
    if(bVirtualSlice)
    {
      pcBinDataTmp[uiFragNb]	=	cVirtualSliceList.front();
			cVirtualSliceList.pop_front();
    }
    else
    {
      RNOK( m_pcReadBitstream->extractPacket( pcBinDataTmp[uiFragNb], bEOS ) );
//TMM_EC {{
		  if( !bEOS && ((pcBinDataTmp[uiFragNb]->data())[0] & 0x1f )== 0x0b)
		  {
			  printf("end of stream\n");
			  bEOS=true;
			  uiNalUnitType= uiPreNalUnitType;
        RNOK( m_pcReadBitstream->releasePacket( pcBinDataTmp[uiFragNb] ) );
        pcBinDataTmp[uiFragNb] = new BinData;
			  uiTotalLength	=	0;
        pcBinDataTmp[uiFragNb]->set( new UChar[uiTotalLength], uiTotalLength );
		  }
//TMM_EC }}
    }
     pcBinDataTmp[uiFragNb]->setMemAccessor( cBinDataAccessorTmp[uiFragNb] );
     // open the NAL Unit, determine the type and if it's a slice get the frame size
     RNOK( m_pcH264AVCDecoder->initPacket( &cBinDataAccessorTmp[uiFragNb], 
                                            uiNalUnitType, uiMbX, uiMbY, uiSize,  true, 
		 false, //FRAG_FIX_3
        bStart, auiStartPos[uiFragNb], auiEndPos[uiFragNb], bFragmented, bDiscardable, & bFgsParallelDecoding ) );

      uiTotalLength += auiEndPos[uiFragNb] - auiStartPos[uiFragNb];

      if(!bStart)
      {
        ROT( bEOS) ; //jerome.vieron@thomson.net
        uiFragNb++;
      }
      else
      {
        if(pcBinDataTmp[0]->size() != 0)
        {
          pcBinData = new BinData;
          pcBinData->set( new UChar[uiTotalLength], uiTotalLength );
          UChar*  apucFragBuffers [MAX_NUM_PD_FRAGMENTS + 1];
          UInt    uiNumFragments = 1;

          uiNumFragments = uiFragNb + 1;
          RNOK(appendFragment(uiTotalLength, uiFragNb, pcBinDataTmp, auiStartPos, auiEndPos, bConcatenated, pcBinData, apucFragBuffers ));

          pcBinData->setMemAccessor( *cBinDataAccessor );
          bDecode = false;
          if((uiTotalLength != 0) && (!bDiscardable || bFragmented))
          {
              if( (uiNalUnitType == 20) || (uiNalUnitType == 21) || (uiNalUnitType == 1) || (uiNalUnitType == 5) )
              {
                uiPreNalUnitType=uiNalUnitType;

                RNOK( m_pcH264AVCDecoder->initPacket( 
                cBinDataAccessor, 
                uiNalUnitType, 
                uiMbX, 
                uiMbY, 
                uiSize, 
                //uiNonRequiredPic, //NonRequired JVT-Q066
                false, 
                bConcatenated, //FRAG_FIX_3
                bStart, 
                auiStartPos[uiFragNb+1], 
                auiEndPos[uiFragNb+1], 
                bFragmented, 
                bDiscardable, 
                0,
                &uiNumFragments,
                apucFragBuffers ) );
              }
              else
                  m_pcH264AVCDecoder->initPacket( cBinDataAccessor );
              bDecode = true;
          }
        }
      }
  }

  return Err::m_nOK;
}

ErrVal H264AVCDecoderTest::go()
{
  PicBuffer*    pcPicBuffer = NULL;
  PicBufferList cPicBufferOutputList; 
  PicBufferList cPicBufferUnusedList;
  PicBufferList cPicBufferReleaseList;

  UInt      uiMbX           = 0;
  UInt      uiMbY           = 0;
  UInt      uiNalUnitType   = 0;
  UInt      uiSize          = 0;
  //UInt      uiNonRequiredPic= 0; //NonRequired JVT-Q066
  UInt      uiLumOffset     = 0;
  UInt      uiCbOffset      = 0;
  UInt      uiCrOffset      = 0;
  UInt      uiFrame;
  
  Bool      bEOS            = false;
  Bool      bYuvDimSet      = false;
  Bool 	    bFinish         = false;

  Bool		SuffixEnable;  //JVT-S036 lsj 

  // HS: packet trace
  Int   iMaxPocDiff = (Int)m_pcParameter->uiMaxPocDiff;
  Int   iLastPoc    = MSYS_INT_MAX;
  UChar* pcLastFrame  = 0;
  UInt   uiPreNalUnitType = 0;

  cPicBufferOutputList.clear();
  cPicBufferUnusedList.clear();

  RNOK( m_pcH264AVCDecoder->init( true ) );

  Bool bToDecode = false; //JVT-P031
  Bool bVirtual = false; //TMM_EC_FIX

  for( uiFrame = 0; ( uiFrame <= MSYS_UINT_MAX && ! bEOS); )
  {
    BinData* pcBinData;
    BinDataAccessor cBinDataAccessor;

    Int  iPos;
    Bool bFinishChecking  = false;
    RNOK( m_pcReadBitstream->getPosition(iPos) );
    do 
    {
      // analyze the dependency information
      RNOK( m_pcReadBitstream->extractPacket( pcBinData, bEOS ) );
      //TMM_EC {{
      {
         pcBinData->setMemAccessor( cBinDataAccessor );
         bFinish = false;
         RNOK( removeRedundencySlice( &cBinDataAccessor,  bFinish ) );
         if ( bFinish )
         {
           RNOK( m_pcReadBitstream->releasePacket( pcBinData ) );
           continue;
         }
      }
      //TMM_EC }}
      
//TMM_EC {{
			if ( !bEOS && ((pcBinData->data())[0] & 0x1f) == 0x0b)
			{
				bEOS	=	true;
        RNOK( m_pcReadBitstream->releasePacket( pcBinData ) );
        pcBinData	= new BinData;
				UInt	uiTotalLength	=	0;
        pcBinData->set( new UChar[uiTotalLength], uiTotalLength );
			}
//TMM_EC }}      
      pcBinData->setMemAccessor( cBinDataAccessor );

      // open the NAL Unit, determine the type and if it's a slice get the frame size
      bFinishChecking = false;
      RNOK( m_pcH264AVCDecoder->checkSliceLayerDependency( &cBinDataAccessor, bFinishChecking ) );

      RNOK( m_pcReadBitstream->releasePacket( pcBinData ) );
      pcBinData = NULL;
    } while( ! bFinishChecking );

    RNOK( m_pcReadBitstream->setPosition(iPos) );

    //TMM_EC {{	check slice gap and process virtual slice

    if ( m_pcParameter->uiErrorConceal != 0)
    {
      RNOK( m_pcReadBitstream->extractPacket( pcBinData, bEOS ) );
      pcBinData->setMemAccessor( cBinDataAccessor );
      //TMM_EC {{
      bFinish = false;
      RNOK( removeRedundencySlice( &cBinDataAccessor,  bFinish ) );
      if ( bFinish )
      {
         RNOK( m_pcReadBitstream->releasePacket( pcBinData ) );
         continue;
      }
      //TMM_EC }}
      do
		  {
			  BinDataAccessor	lcBinDataAccessor;
			  lcBinDataAccessor	=	cBinDataAccessor;

			  MyList<BinData*>	cVirtualSliceList;
			  while ( Err::m_nERR == m_pcH264AVCDecoder->checkSliceGap( &lcBinDataAccessor, cVirtualSliceList))
			  {
	        RNOK( m_pcReadBitstream->releasePacket( pcBinData ) );
		      RNOK( m_pcReadBitstream->getPosition(iPos) );
				  RNOK( m_pcReadBitstream->extractPacket( pcBinData, bEOS ) );
				  pcBinData->setMemAccessor( cBinDataAccessor );
				  lcBinDataAccessor	=	cBinDataAccessor;
			  }

			  if ( cVirtualSliceList.empty())
			  {
	        RNOK( m_pcReadBitstream->releasePacket( pcBinData ) );
				  RNOK( m_pcReadBitstream->setPosition(iPos) );
				  break;
			  }

        bVirtual = false;
			  while( !cVirtualSliceList.empty())
			  {
				  Bool bStart = false;
          bVirtual = true;//TMM_EC_FIX
          BinData	*pcBinDataS = 0;
          RNOK(initPacketToDecode(bToDecode, pcBinDataS, &lcBinDataAccessor, bEOS,
                   uiNalUnitType, uiMbX, uiMbY, uiSize, uiPreNalUnitType, iPos, cVirtualSliceList, true));

  //NonRequired JVT-Q066{
				  if(m_pcH264AVCDecoder->isNonRequiredPic())
					  continue;
  //NonRequired JVT-Q066}
				  if(bToDecode)//JVT-P031
				  {
					  // get new picture buffer if required if coded Slice || coded IDR slice
					  pcPicBuffer = NULL;
			      
					  if( uiNalUnitType == 1 || uiNalUnitType == 5 || uiNalUnitType == 20 || uiNalUnitType == 21)
					  {
						  RNOK( xGetNewPicBuffer( pcPicBuffer, uiSize ) );
			        
						  if( ! bYuvDimSet )
						  {
							  UInt uiLumSize  = ((uiMbX<<3)+  YUV_X_MARGIN) * ((uiMbY<<3)    + YUV_Y_MARGIN ) * 4;
							  uiLumOffset     = ((uiMbX<<4)+2*YUV_X_MARGIN) * YUV_Y_MARGIN   + YUV_X_MARGIN;  
							  uiCbOffset      = ((uiMbX<<3)+  YUV_X_MARGIN) * YUV_Y_MARGIN/2 + YUV_X_MARGIN/2 + uiLumSize; 
							  uiCrOffset      = ((uiMbX<<3)+  YUV_X_MARGIN) * YUV_Y_MARGIN/2 + YUV_X_MARGIN/2 + 5*uiLumSize/4;
							  bYuvDimSet = true;

							  // HS: decoder robustness
							  pcLastFrame = new UChar [uiSize];
							  ROF( pcLastFrame );
						  }
					  }

				    //JVT-S036 lsj start
					  if( (uiNalUnitType == 1 || uiNalUnitType == 5 )  && !bVirtual) //TMM_EC_FIX
					  {
						  SuffixEnable = true;
					//	  RNOK(m_pcH264AVCDecoderSuffix->init( false ));     // THAT'S REALLY BAD !!!
						  RNOK( m_pcReadBitstream->getPosition( iPos ) );
						  RNOK( m_pcReadBitstream->extractPacket( pcBinData, bEOS ) );
						  pcBinData->setMemAccessor( cBinDataAccessor );
              //EMM_TC {{
              bFinish = false;
              RNOK( removeRedundencySlice( &cBinDataAccessor,  bFinish ) );
              if ( bFinish )
              {
                RNOK( m_pcReadBitstream->releasePacket( pcBinData ) );
                pcBinData = NULL;
              }
              else
              {
              //TMM_EC
                 RNOK(m_pcH264AVCDecoderSuffix->init( false ));     // THAT'S REALLY BAD !!!
						     RNOK( m_pcH264AVCDecoderSuffix->initPacketSuffix( &cBinDataAccessor, uiNalUnitType, true, 
								      false, //FRAG_FIX_3
								      bStart, m_pcH264AVCDecoder,SuffixEnable
								      )
							      );

						      RNOK( m_pcH264AVCDecoderSuffix->uninit( false ));  // THAT'S REALLY BAD !!!

						      if( !SuffixEnable )
						      {
							       RNOK( m_pcReadBitstream->setPosition( iPos ) );
        //bug-fix suffix{{
							       bEOS = false;
						       }
						       else
						       {
							        m_pcH264AVCDecoder->decreaseNumOfNALInAU(); 
        //bug-fix suffix}}
						        }
                }

						  if( pcBinData)
						  { 
                             m_pcReadBitstream->releasePacket( pcBinData ) ;
                             pcBinData=NULL;
						  }

					  }
				  //JVT-S036 lsj end
		      
					  // decode the NAL unit
					  RNOK( m_pcH264AVCDecoder->process( pcPicBuffer, cPicBufferOutputList, cPicBufferUnusedList, cPicBufferReleaseList ) );

					  // ROI DECODE ICU/ETRI
					  m_pcH264AVCDecoder->RoiDecodeInit();
			      
					  // picture output
					  while( ! cPicBufferOutputList.empty() )
					  {
                PicBuffer* pcPicBufferTmp = cPicBufferOutputList.front();
						    cPicBufferOutputList.pop_front();
                if( pcPicBufferTmp != NULL )
                {
							  // HS: decoder robustness
				          while( abs( iLastPoc + iMaxPocDiff ) < abs( (Int)pcPicBufferTmp->getCts() ) )
                 // while( uiLastPoc + uiMaxPocDiff < (UInt)pcPicBufferTmp->getCts() && (Int)pcPicBufferTmp->getCts() > 0) //EIDR bug-fix
                  {
								    RNOK( m_pcWriteYuv->writeFrame( pcLastFrame + uiLumOffset, 
																								  pcLastFrame + uiCbOffset, 
																								  pcLastFrame + uiCrOffset,
																								  uiMbY << 4,
																								  uiMbX << 4,
																								  (uiMbX << 4)+ YUV_X_MARGIN*2 ) );
								    printf("REPEAT FRAME\n");
								    uiFrame   ++;
								    iLastPoc += iMaxPocDiff;
                  }

			          
                  RNOK( m_pcWriteYuv->writeFrame( *pcPicBufferTmp + uiLumOffset, 
                    *pcPicBufferTmp + uiCbOffset, 
                    *pcPicBufferTmp + uiCrOffset,
																							  uiMbY << 4,
																							  uiMbX << 4,
																							  (uiMbX << 4)+ YUV_X_MARGIN*2 ) );
							    uiFrame++;
			        
			      
							  // HS: decoder robustness
                  iLastPoc = (Int) pcPicBufferTmp->getCts();
                  ::memcpy( pcLastFrame, *pcPicBufferTmp+0, uiSize*sizeof(UChar) );
						  }
					  }
			      
					  RNOK( xRemovePicBuffer( cPicBufferReleaseList ) );
					  RNOK( xRemovePicBuffer( cPicBufferUnusedList ) );
              if(pcBinDataS)
                RNOK( m_pcReadBitstream->releasePacket( pcBinDataS ) );
				  }
			  }
		  }	while( true);
		}
//TMM_EC }}

    Bool bStart = false;
    MyList<BinData*>	cVirtualSliceList;
    pcBinData = 0;
    RNOK(initPacketToDecode(bToDecode, pcBinData, &cBinDataAccessor, bEOS,
      uiNalUnitType, uiMbX, uiMbY, uiSize, uiPreNalUnitType, iPos, cVirtualSliceList, false));

    //JVT-T054_FIX{
    Bool bWasAVCNALUnit = (uiNalUnitType == 1 || uiNalUnitType == 5);
    //JVT-T054}

//NonRequired JVT-Q066{
	if(m_pcH264AVCDecoder->isNonRequiredPic())
		continue;
//NonRequired JVT-Q066}

// JVT-Q054 Red. Picture {
  RNOK( m_pcH264AVCDecoder->checkRedundantPic() );
  if ( m_pcH264AVCDecoder->isRedundantPic() )
//bug-fix suffix{{
  {
	if((uiNalUnitType == 1 || uiNalUnitType == 5) && !bEOS) 
	{
		SuffixEnable = true;
		RNOK(m_pcH264AVCDecoderSuffix->init( false ));   // THAT'S REALLY BAD !!!
		RNOK( m_pcReadBitstream->getPosition( iPos ) );
		RNOK( m_pcReadBitstream->extractPacket( pcBinData, bEOS ) );
		pcBinData->setMemAccessor( cBinDataAccessor );
		RNOK( m_pcH264AVCDecoderSuffix->initPacketSuffix( &cBinDataAccessor, uiNalUnitType, true, 
				false, //FRAG_FIX_3
				bStart, m_pcH264AVCDecoder,SuffixEnable
				)
			);

		RNOK( m_pcH264AVCDecoderSuffix->uninit( false ));  // THAT'S REALLY BAD !!!

		if( !SuffixEnable )
		{
			RNOK( m_pcReadBitstream->setPosition( iPos ) );
			bEOS = false; 
		}
		else
		{
			m_pcH264AVCDecoder->decreaseNumOfNALInAU(); 
		}
	}
	continue;
  }
//bug-fix suffix}}
// JVT-Q054 Red. Picture }

  if(bToDecode)//JVT-P031
  {
    // get new picture buffer if required if coded Slice || coded IDR slice
    pcPicBuffer = NULL;
    
    if( uiNalUnitType == 1 || uiNalUnitType == 5 || uiNalUnitType == 20 || uiNalUnitType == 21 )
    {
      RNOK( xGetNewPicBuffer( pcPicBuffer, uiSize ) );

      if( ! bYuvDimSet )
      {
        UInt uiLumSize  = ((uiMbX<<3)+  YUV_X_MARGIN) * ((uiMbY<<3)    + YUV_Y_MARGIN ) * 4;
        uiLumOffset     = ((uiMbX<<4)+2*YUV_X_MARGIN) * YUV_Y_MARGIN   + YUV_X_MARGIN;  
        uiCbOffset      = ((uiMbX<<3)+  YUV_X_MARGIN) * YUV_Y_MARGIN/2 + YUV_X_MARGIN/2 + uiLumSize; 
        uiCrOffset      = ((uiMbX<<3)+  YUV_X_MARGIN) * YUV_Y_MARGIN/2 + YUV_X_MARGIN/2 + 5*uiLumSize/4;
        bYuvDimSet = true;

        // HS: decoder robustness
        pcLastFrame = new UChar [uiSize];
        ROF( pcLastFrame );
      }
    }
    
//JVT-S036 lsj start
	if((uiNalUnitType == 1 || uiNalUnitType == 5) && !bEOS) 
	{
		SuffixEnable = true;
//		RNOK(m_pcH264AVCDecoderSuffix->init( false ));   // THAT'S REALLY BAD !!!
		RNOK( m_pcReadBitstream->getPosition( iPos ) );
		RNOK( m_pcReadBitstream->extractPacket( pcBinData, bEOS ) );
		pcBinData->setMemAccessor( cBinDataAccessor );
    //TMM_EC
    bFinish = false;
    RNOK( removeRedundencySlice( &cBinDataAccessor,  bFinish ) );
    if ( bFinish )
    {
       RNOK( m_pcReadBitstream->releasePacket( pcBinData ) );
       pcBinData = NULL;
    }
    else
    {
       RNOK(m_pcH264AVCDecoderSuffix->init( false ));   // THAT'S REALLY BAD !!!
		   RNOK( m_pcH264AVCDecoderSuffix->initPacketSuffix( &cBinDataAccessor, uiNalUnitType, true, 
			    false, //FRAG_FIX_3
			    bStart, m_pcH264AVCDecoder,SuffixEnable
			    )
		    );

		   RNOK( m_pcH264AVCDecoderSuffix->uninit( false ));  // THAT'S REALLY BAD !!!

	     if( !SuffixEnable )
		   {
			    RNOK( m_pcReadBitstream->setPosition( iPos ) );
			    bEOS = false; 
		   }
    //bug-fix suffix{{
		   else
		   {
			   m_pcH264AVCDecoder->decreaseNumOfNALInAU(); 
		   }
     }
//bug-fix suffix}}
	}
//JVT-S036 lsj end

    // decode the NAL unit
    RNOK( m_pcH264AVCDecoder->process( pcPicBuffer, cPicBufferOutputList, cPicBufferUnusedList, cPicBufferReleaseList ) );
    //JVT-T054_FIX{
    //add last decoded frame to SVC DPB if last frame was an AVC unit
   if(bWasAVCNALUnit && m_pcH264AVCDecoder->getBaseSVCActive())
   {
      RNOK( xGetNewPicBuffer( pcPicBuffer, uiSize ) );

      if( ! bYuvDimSet )
      {
        UInt uiLumSize  = ((uiMbX<<3)+  YUV_X_MARGIN) * ((uiMbY<<3)    + YUV_Y_MARGIN ) * 4;
        uiLumOffset     = ((uiMbX<<4)+2*YUV_X_MARGIN) * YUV_Y_MARGIN   + YUV_X_MARGIN;  
        uiCbOffset      = ((uiMbX<<3)+  YUV_X_MARGIN) * YUV_Y_MARGIN/2 + YUV_X_MARGIN/2 + uiLumSize; 
        uiCrOffset      = ((uiMbX<<3)+  YUV_X_MARGIN) * YUV_Y_MARGIN/2 + YUV_X_MARGIN/2 + 5*uiLumSize/4;
        bYuvDimSet = true;

        // HS: decoder robustness
        pcLastFrame = new UChar [uiSize];
        ROF( pcLastFrame );
      }

      RNOK( m_pcH264AVCDecoder->process( pcPicBuffer, cPicBufferOutputList, cPicBufferUnusedList, cPicBufferReleaseList ) );
   }
    //JVT-T054}

	// ROI DECODE ICU/ETRI
	m_pcH264AVCDecoder->RoiDecodeInit();
    
    // picture output
    while( ! cPicBufferOutputList.empty() )
    {
        PicBuffer* pcPicBufferTmp = cPicBufferOutputList.front();
      cPicBufferOutputList.pop_front();
        if( pcPicBufferTmp != NULL )
      {
        // HS: decoder robustness
				while( abs( iLastPoc + iMaxPocDiff ) < abs( (Int)pcPicBufferTmp->getCts() ) )
        // while( uiLastPoc + uiMaxPocDiff < (UInt)pcPicBufferTmp->getCts() && (Int)pcPicBufferTmp->getCts() > 0) //EIDR bug-fix
        {
          RNOK( m_pcWriteYuv->writeFrame( pcLastFrame + uiLumOffset, 
                                          pcLastFrame + uiCbOffset, 
                                          pcLastFrame + uiCrOffset,
                                           uiMbY << 4,
                                           uiMbX << 4,
                                          (uiMbX << 4)+ YUV_X_MARGIN*2 ) );
          printf("REPEAT FRAME\n");
          uiFrame   ++;
          iLastPoc += iMaxPocDiff;
        }

        RNOK( m_pcWriteYuv->writeFrame( *pcPicBufferTmp + uiLumOffset, 
                                        *pcPicBufferTmp + uiCbOffset, 
                                        *pcPicBufferTmp + uiCrOffset,
                                         uiMbY << 4,
                                         uiMbX << 4,
                                        (uiMbX << 4)+ YUV_X_MARGIN*2 ) );

        uiFrame++;
      
    
        // HS: decoder robustness
        iLastPoc = (UInt)pcPicBufferTmp->getCts();
        ::memcpy( pcLastFrame, *pcPicBufferTmp+0, uiSize*sizeof(UChar) );
      }
    }
   } 
    RNOK( xRemovePicBuffer( cPicBufferReleaseList ) );
    RNOK( xRemovePicBuffer( cPicBufferUnusedList ) );
    if( pcBinData )
    {
      RNOK( m_pcReadBitstream->releasePacket( pcBinData ) );
      pcBinData = 0;
    }
  }

  printf("\n %d frames decoded\n", uiFrame );

  delete [] pcLastFrame; // HS: decoder robustness
  
  RNOK( m_pcH264AVCDecoder->uninit( true ) );
  
  m_pcParameter->nFrames  = uiFrame;
  m_pcParameter->nResult  = 0;

  return Err::m_nOK;
}

//TMM_EC
ErrVal
H264AVCDecoderTest::removeRedundencySlice(BinDataAccessor*  pcBinDataAccessor,
                                     Bool&             bFinishChecking  )
{
  Bool bEos;
  bFinishChecking = false;
  UInt eNalUnitType;
  int uiLayerId;
  int uiQualityLevel;
  int m_bFGSFragFlag;

  if ( m_pcParameter->uiErrorConceal == 0)
    return Err::m_nOK; 

  ROT( NULL == pcBinDataAccessor );
  bEos = ( NULL == pcBinDataAccessor->data() ) || ( 0 == pcBinDataAccessor->size() );

  if ( bEos )
  {
    return Err::m_nOK;
  }

  ROF( pcBinDataAccessor->size() );
   ROF( pcBinDataAccessor->data() );

   UChar ucByte          = pcBinDataAccessor->data()[0];
   ROT( ucByte & 0x80 );                                     // forbidden_zero_bit ( &10000000b)
   eNalUnitType        =  ( ucByte &  0x1F  );  // nal_unit_type      ( &00011111b)

  if ( *(int*)(pcBinDataAccessor->data()+1) != 0xdeadface)
	{
		if( eNalUnitType == 20 ||
				eNalUnitType == 21 )
		{
		  ROF( pcBinDataAccessor->size() > 3 );
	
		  ucByte              = pcBinDataAccessor->data()[1];
	    ucByte              = pcBinDataAccessor->data()[2];
	    uiLayerId         = ( ucByte >> 2 ) & 7;
      uiQualityLevel    = ( ucByte      ) & 3;  

      ucByte              = pcBinDataAccessor->data()[3];
      // JVT-U116 LMI {
      //ROT( ucByte & 0x80 );
      m_bFGSFragFlag        = ( ucByte >> 4) & 1;
      // JVT-U116 LMI {
    }
		else
		{
			uiLayerId       = 0;
		}
	}
	else //TMM_EC
	{
    uiLayerId       = 0;
    return Err::m_nERR;
	}

  if (   ( eNalUnitType == 20 
      ||   eNalUnitType == 21)  
      &&   uiLayerId == 0     )
  {
    bFinishChecking = true;
  }


  return Err::m_nOK;
}
//TMM_EC }}


