#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "RDTree.h"


static Double sdScaleSlope = 1e6;

void RDTree::HierarchyPrint(UInt uiTab)
{
  UInt i;
  for(i=0; i<uiTab; i++)
    printf(" ");
  printf("Frame: %3d Rate: %6d Disto: %f slope: %f\n", 
    m_uiFrameNum, m_uiDeltaRate, m_dDeltaDisto, sdScaleSlope*m_dLocalSlopeToNextRDNode);
  if (m_pcChild1)
    m_pcChild1->HierarchyPrint(uiTab+2);
  if (m_pcChild2)
    m_pcChild2->HierarchyPrint(uiTab+2);
}

void RDTree::MergeRDLists(RDTree *pcList1, RDTree *pcList2)
{
  RDTree *pcCurrent = this;
  if (pcList1)
  {
    if (pcList2)
    {
      while ( (pcList1) || (pcList2) )
      {
        if (pcList1)
        {
          if (pcList2)
          {
            if (pcList1->m_dGlobalSlopeToNextRDNode > pcList2->m_dGlobalSlopeToNextRDNode)
              pcCurrent->ProgressRDNodeList(pcList1);
            else
              pcCurrent->ProgressRDNodeList(pcList2);
          } else
            pcCurrent->ProgressRDNodeList(pcList1);
        } else
          pcCurrent->ProgressRDNodeList(pcList2);
        pcCurrent = pcCurrent->m_pcNextRDNode;
      }
    }
    else
    {
      m_pcNextRDNode = pcList1;
    }
  }
}

void RDTree::PrintRDList()
{
  RDTree *pcList = this;
  while (pcList)
  {
    printf("(%3d, %6.3f, %6.3f) ", pcList->m_uiFrameNum, 
      sdScaleSlope*pcList->m_dGlobalSlopeToNextRDNode,
      sdScaleSlope*pcList->m_dLocalSlopeToNextRDNode);
    pcList = pcList->m_pcNextRDNode;
  }
  printf("\n");
}

void RDTree::RDOptim(UInt uiDepth)
{
  // optimize in childs first
  if (m_pcChild1)
    m_pcChild1->RDOptim(uiDepth+1);
  if (m_pcChild2)
    m_pcChild2->RDOptim(uiDepth+1);

  m_dLocalSlopeToNextRDNode = (m_uiDeltaRate==0 ? 0 : m_dDeltaDisto/m_uiDeltaRate );
  m_dGlobalSlopeToNextRDNode = m_dLocalSlopeToNextRDNode;

  //if (uiDepth<2)
  //  return;
  // merge RD list of childs
  MergeRDLists(m_pcChild1, m_pcChild2);
#ifdef DO_GLOBAL_SLOPE_UPDATE
  if (uiDepth!=0)
    UpdateGlobalRDSlopeOfList();
#endif

  printf("--- generated list\n");
  PrintRDList();
 }

void RDTree::UpdateGlobalRDSlopeOfList()
{
  if (m_pcNextRDNode==0)
  {
    m_dGlobalSlopeToNextRDNode = m_dLocalSlopeToNextRDNode;
    return;
  }
  //-- search for the next node on convex-hull
  Double dBestSlope = m_dLocalSlopeToNextRDNode;
  RDTree *pcBestNode = this;
  UInt uiSumRate;
  Double dSumDisto, dSlope;
  RDTree *pcNode;
  pcNode = this;
  uiSumRate = m_uiDeltaRate;
  dSumDisto = m_dDeltaDisto;
  while (pcNode->GetNextRDNode())
  {
    pcNode = pcNode->GetNextRDNode();
    uiSumRate += pcNode->GetDeltaRate();
    dSumDisto += pcNode->GetDeltaDisto();
    dSlope = (uiSumRate==0 ? 0 : dSumDisto/uiSumRate );
    if (dSlope>dBestSlope)
    {
      dBestSlope = dSlope;
      pcBestNode = pcNode;
    }
  }
  printf("localSlope: %6.3f globalSlope: %6.3f (%d to %d)\n", sdScaleSlope*m_dLocalSlopeToNextRDNode,
    sdScaleSlope*dBestSlope,
    m_uiFrameNum, pcBestNode->m_uiFrameNum);

  // update Global RD slope
  if (pcBestNode==this)
    m_dGlobalSlopeToNextRDNode = dBestSlope;
  else
  {
    pcNode = this;
    pcNode->m_dGlobalSlopeToNextRDNode = dBestSlope;
    do
    {
      pcNode = pcNode->GetNextRDNode();
      pcNode->m_dGlobalSlopeToNextRDNode = dBestSlope;
    } while (pcNode != pcBestNode);
  }
  // perform slope update on remaining tail
  if (pcBestNode->GetNextRDNode())
    pcBestNode->GetNextRDNode()->UpdateGlobalRDSlopeOfList();


}