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

#ifdef JVT_P029
#include "ReadYUVFile.h"
double getEqm(Pel* rec, Pel* org, UInt h,UInt w,UInt stride)
{
    unsigned char*  pOrg  = org;//.getBuffer();
    unsigned char*  pRec  = rec;//.getBuffer();
  double          ssd   = 0;
  int             diff;

  for  ( int r = 0; r < h; r++ )
  {
    for( int c = 0; c < w;  c++ )
    {
      diff  = pRec[c] - pOrg[c];
      ssd  += (double)( diff * diff );
    }
    pRec   += stride;
    pOrg   += stride;
  }

  return (ssd);///((double)rec.width * (double)rec.height)); 
}
#endif

H264AVCDecoderTest::H264AVCDecoderTest() :
  m_pcH264AVCDecoder( NULL ),
  m_pcReadBitstream( NULL ),
  m_pcWriteYuv( NULL ),
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


ErrVal H264AVCDecoderTest::init( DecoderParameter *pcDecoderParameter )
{
  ROT( NULL == pcDecoderParameter );

  m_pcParameter = pcDecoderParameter;
  m_pcParameter->nResult = -1;

  RNOKS( WriteYuvToFile::create( m_pcWriteYuv, m_pcParameter->cYuvFile ) );

  ReadBitstreamFile *pcReadBitstreamFile;
  RNOKS( ReadBitstreamFile::create( pcReadBitstreamFile ) ); 
  RNOKS( pcReadBitstreamFile->init( m_pcParameter->cBitstreamFile ) );  
  m_pcReadBitstream = (ReadBitstreamIf*)pcReadBitstreamFile;

  RNOK( h264::CreaterH264AVCDecoder::create( m_pcH264AVCDecoder ) );

  return Err::m_nOK;
}



ErrVal H264AVCDecoderTest::destroy()
{

  if( NULL != m_pcH264AVCDecoder )       
  {
    RNOK( m_pcH264AVCDecoder->destroy() );       
  }

  if( NULL != m_pcWriteYuv )              
  {
    RNOK( m_pcWriteYuv->destroy() );  
  }

  if( NULL != m_pcReadBitstream )     
  {
    RNOK( m_pcReadBitstream->uninit() );  
    RNOK( m_pcReadBitstream->destroy() );  
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


ErrVal H264AVCDecoderTest::go()
{
  PicBuffer*    pcPicBuffer = NULL;
  PicBufferList cPicBufferOutputList; 
  PicBufferList cPicBufferUnusedList;
  PicBufferList cPicBufferReleaseList;
#ifdef JVT_P029
  PicBuffer*    pcPicOrigBuffer = NULL;
  ReadYuvFile*  pcReaderYUV = NULL;
  Double eqm = 0.0;
  Double eqmLast = 0.0;
  Double eqmGop = 0.0;
  UInt seq_length;
  //UInt uiFrameDeb, uiFrameEnd,intraFrame;
  UInt uiSizeGop;
  FILE *fEqm;
  UInt uiNbFrames = 0;
  UInt uiEnd;
  UInt uiDeb;
  FILE *fDisto = fopen("disto","wt");
#endif
  UInt      uiMbX           = 0;
  UInt      uiMbY           = 0;
  UInt      uiNalUnitType   = 0;
  UInt      uiSize          = 0;
#if NON_REQUIRED_SEI_ENABLE  //shenqiu
  UInt      uiNonRequiredPic= 0;
#endif
  UInt      uiLumOffset     = 0;
  UInt      uiCbOffset      = 0;
  UInt      uiCrOffset      = 0;
  UInt      uiFrame;
  
  Bool      bEOS            = false;
  Bool      bYuvDimSet      = false;

  // HS: packet trace
  UInt   uiMaxPocDiff = m_pcParameter->uiMaxPocDiff;
  UInt   uiLastPoc    = MSYS_UINT_MAX;
  UChar* pcLastFrame  = 0;

  cPicBufferOutputList.clear();
  cPicBufferUnusedList.clear();
  
  RNOK( m_pcH264AVCDecoder->init() );
#ifdef JVT_P029
  if(m_pcParameter->m_bQL == true)
  {
  ReadYuvFile::create(pcReaderYUV);
  seq_length = m_pcParameter->seq_length;
  uiSizeGop = m_pcParameter->sizeGop;
  if(m_pcParameter->InterGopFrame == false)
      uiEnd = 1;
  else
  {
      //intergop frames are cut
      if(m_pcParameter->FirstInterGopFrame == 0)
      {
          uiDeb = 0;
          uiEnd = 1+uiSizeGop;
      }
      else
      {
          uiDeb = 1;
          uiEnd = 1+uiSizeGop*2;
      }
  }
  fEqm = fopen("eqp","wt");
  pcReaderYUV->init(m_pcParameter->cOrigYUV,m_pcParameter->h,m_pcParameter->w);
  }
#endif
  Bool bToDecode = false; //JVT-P031
  for( uiFrame = 0; ( uiFrame <= MSYS_UINT_MAX && ! bEOS); )
  {
    BinData* pcBinData;
    BinDataAccessor cBinDataAccessor;

    Int  iPos;
    Bool bFinishChecking;

    RNOK( m_pcReadBitstream->getPosition(iPos) );

    do 
    {
      // analyze the dependency information
      RNOK( m_pcReadBitstream->extractPacket( pcBinData, bEOS ) );
      
      pcBinData->setMemAccessor( cBinDataAccessor );

      // open the NAL Unit, determine the type and if it's a slice get the frame size
      bFinishChecking = false;
      //JVT-P031
      if(m_pcH264AVCDecoder->getNumOfNALInAU() == 0)
      {
        m_pcH264AVCDecoder->setDependencyInitialized(false);
        m_pcH264AVCDecoder->initNumberOfFragment();
      }
      //~JVT-P031
      RNOK( m_pcH264AVCDecoder->checkSliceLayerDependency( &cBinDataAccessor, bFinishChecking ) );

      RNOK( m_pcReadBitstream->releasePacket( pcBinData ) );
    } while( ! bFinishChecking );

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
    uiFragNb = 0;
    bEOS = false;
    while(!bStart && !bEOS)
    {
      if(bFirst)
      {
          RNOK( m_pcReadBitstream->setPosition(iPos) );
          bFirst = false;
      }

      RNOK( m_pcReadBitstream->extractPacket( pcBinDataTmp[uiFragNb], bEOS ) );
    
      pcBinDataTmp[uiFragNb]->setMemAccessor( cBinDataAccessorTmp[uiFragNb] );
      // open the NAL Unit, determine the type and if it's a slice get the frame size
#if NON_REQUIRED_SEI_ENABLE  //shenqiu 05-10-01
    RNOK( m_pcH264AVCDecoder->initPacket( &cBinDataAccessorTmp[uiFragNb], uiNalUnitType, uiMbX, uiMbY, uiSize, uiNonRequiredPic, bStart, auiStartPos[uiFragNb], auiEndPos[uiFragNb], bFragmented, bDiscardable ) );
    if(uiNonRequiredPic)
		continue;
#else
    RNOK( m_pcH264AVCDecoder->initPacket( &cBinDataAccessorTmp[uiFragNb], uiNalUnitType, uiMbX, uiMbY, uiSize, bStart, auiStartPos[uiFragNb], auiEndPos[uiFragNb], bFragmented, bDiscardable ) );
#endif
      uiTotalLength += auiEndPos[uiFragNb] - auiStartPos[uiFragNb];

      if(!bStart)
      {
        uiFragNb++;
      }
      else
      {
        if(pcBinDataTmp[0]->size() != 0)
        {
          pcBinData = new BinData;
          pcBinData->set( new UChar[uiTotalLength], uiTotalLength );
          // append fragments
          UInt uiOffset = 0;
          for(UInt uiFrag = 0; uiFrag<uiFragNb+1; uiFrag++)
          {
              memcpy(pcBinData->data()+uiOffset, pcBinDataTmp[uiFrag]->data() + auiStartPos[uiFrag], auiEndPos[uiFrag]-auiStartPos[uiFrag]);
              uiOffset += auiEndPos[uiFrag]-auiStartPos[uiFrag];
              RNOK( m_pcReadBitstream->releasePacket( pcBinDataTmp[uiFrag] ) );
              pcBinDataTmp[uiFrag] = NULL;
              m_pcH264AVCDecoder->decreaseNumOfNALInAU();
          }
          
          pcBinData->setMemAccessor( cBinDataAccessor );
          bToDecode = false;
          if((uiTotalLength != 0) && (!bDiscardable || bFragmented))
          {
              m_pcH264AVCDecoder->initPacket( &cBinDataAccessor );
              bToDecode = true;
          }
        }
      }

    }
    //~JVT-P031
    
if(bToDecode)//JVT-P031
{
    // get new picture buffer if required if coded Slice || coded IDR slice
    pcPicBuffer = NULL;
    
    if( uiNalUnitType == 1 || uiNalUnitType == 5 || uiNalUnitType == 20 || uiNalUnitType == 21 )
    {
      RNOK( xGetNewPicBuffer( pcPicBuffer, uiSize ) );
#ifdef JVT_P029
	  if(m_pcParameter->m_bQL == true)
  {
      if(pcPicOrigBuffer == NULL && (m_pcParameter->w == (uiMbX << 4)))
      {
          pcPicOrigBuffer = new PicBuffer( new UChar[ uiSize ] );
      }
  }
#endif 
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
    
    // decode the NAL unit
    RNOK( m_pcH264AVCDecoder->process( pcPicBuffer, cPicBufferOutputList, cPicBufferUnusedList, cPicBufferReleaseList ) );
    
    // picture output
    while( ! cPicBufferOutputList.empty() )
    {
      PicBuffer* pcPicBuffer = cPicBufferOutputList.front();
      cPicBufferOutputList.pop_front();
      if( pcPicBuffer != NULL )
      {
        // HS: decoder robustness
        while( uiLastPoc + uiMaxPocDiff < (UInt)pcPicBuffer->getCts() )
        {
          RNOK( m_pcWriteYuv->writeFrame( pcLastFrame + uiLumOffset, 
                                          pcLastFrame + uiCbOffset, 
                                          pcLastFrame + uiCrOffset,
                                           uiMbY << 4,
                                           uiMbX << 4,
                                          (uiMbX << 4)+ YUV_X_MARGIN*2 ) );
          printf("REPEAT FRAME\n");
          uiFrame   ++;
          uiLastPoc += uiMaxPocDiff;
        }

#ifdef JVT_P029
		if(m_pcParameter->m_bQL == true)
  {
        RNOK( pcReaderYUV->readFrame(*pcPicOrigBuffer+uiLumOffset,
                                    *pcPicOrigBuffer+uiCbOffset,
                                    *pcPicOrigBuffer+uiCrOffset,
                                    uiMbY << 4,
                                    uiMbX << 4,
                                    (uiMbX << 4)+ YUV_X_MARGIN*2 ) );
        //calculate eqm
            eqm = getEqm(*pcPicOrigBuffer+uiLumOffset,
            *pcPicBuffer+uiLumOffset,
            uiMbY << 4,
            uiMbX << 4,
            (uiMbX << 4)+ YUV_X_MARGIN*2 );
			if(m_pcParameter->InterGopFrame == true)
			{
                if(uiFrame >= uiDeb && uiFrame < uiEnd)
				{
                    printf(" Frame %d eqm %f \n",uiFrame,eqm);
                    eqmGop += eqm;
                    eqmLast = eqm;
                    uiNbFrames++;
                }
                else
                {
                    if(uiFrame > uiDeb)
                    {
                    printf("Save eqm uiEnd %d eqmGop %f nbFrames %d \n", uiEnd, eqmGop, uiNbFrames);
					printf(" Frame %d eqm %f \n",uiFrame,eqm);
					fprintf(fDisto,"%d \n",(int)( eqmGop / (double)seq_length + 0.5 ));
					eqmGop = eqm;//eqmLast+eqm;
					eqmLast = eqm;
                    uiDeb = uiEnd;
					uiEnd += uiSizeGop*2;
					uiNbFrames = 1;//2;
                    }
                }
			}
			else
			{
				if(uiFrame < uiEnd)
				{
					printf(" Frame %d eqm %f \n",uiFrame,eqm);
                    eqmGop += eqm;
                    eqmLast = eqm;
					uiNbFrames++;
				}
				else
				{
					printf("Save eqm uiEnd %d eqmGop %f nbFrames %d \n", uiEnd, eqmGop, uiNbFrames);
					printf(" Frame %d eqm %f \n",uiFrame,eqm);
					fprintf(fDisto,"%d \n",(int)( eqmGop / (double)seq_length + 0.5 ));
					eqmGop = eqm;//eqmLast+eqm;
					eqmLast = eqm;
					uiEnd += uiSizeGop;
					uiNbFrames = 1;//2;
				}
			}
        
        fprintf(fEqm,"%f\n",eqm);
     
  }
  if(m_pcParameter->m_bQL == false)
  {
#endif          
        RNOK( m_pcWriteYuv->writeFrame( *pcPicBuffer + uiLumOffset, 
                                        *pcPicBuffer + uiCbOffset, 
                                        *pcPicBuffer + uiCrOffset,
                                         uiMbY << 4,
                                         uiMbX << 4,
                                        (uiMbX << 4)+ YUV_X_MARGIN*2 ) );
#ifdef JVT_P029
  }
#endif
        uiFrame++;
      
    
        // HS: decoder robustness
        uiLastPoc = pcPicBuffer->getCts();
        ::memcpy( pcLastFrame, *pcPicBuffer+0, uiSize*sizeof(UChar) );
      }
    }
   } 
    RNOK( xRemovePicBuffer( cPicBufferReleaseList ) );
    RNOK( xRemovePicBuffer( cPicBufferUnusedList ) );
    RNOK( m_pcReadBitstream->releasePacket( pcBinData ) );
  }

  printf("\n %d frames decoded\n", uiFrame );
#ifdef JVT_P029
  if(m_pcParameter->m_bQL == true)
  {
  //FILE *fDisto = fopen("disto","wt");
  //fprintf(fDisto,"%d",(int)( eqm / (double)seq_length + 0.5 ));
  printf("Save eqm uiEnd %d eqmGop %f nbFrames %d \n", uiEnd, eqmGop, uiNbFrames);
  fprintf(fDisto,"%d",(int)( eqmGop / (double)seq_length + 0.5 ));
  fclose(fDisto);
  pcReaderYUV->uninit();
  pcReaderYUV->destroy();
  fclose(fEqm);
  }
#endif
  delete [] pcLastFrame; // HS: decoder robustness
  
  RNOK( m_pcH264AVCDecoder->uninit() );
  
  m_pcParameter->nFrames  = uiFrame;
  m_pcParameter->nResult  = 0;

  return Err::m_nOK;
}





