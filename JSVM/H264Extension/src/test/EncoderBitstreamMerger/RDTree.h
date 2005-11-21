#ifndef RD_TREE__HPP
#define RD_TREE__HPP

#define DO_GLOBAL_SLOPE_UPDATE


#include "Typedefs.h"
class RDTree;
typedef RDTree *pRDTree;
typedef pRDTree *ppRDTree;
typedef ppRDTree *pppRDTree;
typedef pppRDTree *ppppRDTree;

class RDTree
{
protected:
  // rate distortion information
  UInt m_uiFrameNum;
  UInt m_uiDeltaRate;
  Double m_dDeltaDisto;
  // dependency information
  RDTree *m_pcChild1;
  RDTree *m_pcChild2;
  // RD optimisation results
  RDTree *m_pcNextRDNode;
  Double m_dGlobalSlopeToNextRDNode;
  Double m_dLocalSlopeToNextRDNode;

public:
  RDTree()
    : m_uiFrameNum(123456), m_uiDeltaRate(0), m_dDeltaDisto(0),
    m_pcChild1(0), m_pcChild2(0),
    m_pcNextRDNode(0), m_dGlobalSlopeToNextRDNode(0), m_dLocalSlopeToNextRDNode(0)
  {}

  void SetFrameNum(UInt uiFrameNum)
  { m_uiFrameNum = uiFrameNum; }
  UInt GetFrameNum() { return m_uiFrameNum; }

  void SetDeltaRate(UInt uiDeltaRate)
  { m_uiDeltaRate = uiDeltaRate; }
  UInt GetDeltaRate() { return m_uiDeltaRate; }

  void SetDeltaDisto(Double dDeltaDisto)
  { m_dDeltaDisto = dDeltaDisto; }
  Double GetDeltaDisto() { return m_dDeltaDisto; }

  void SetChild1(RDTree *pcChild1)
  { m_pcChild1 = pcChild1; }
  void SetChild2(RDTree *pcChild2)
  { m_pcChild2 = pcChild2; }

  RDTree *GetNextRDNode()
  { return m_pcNextRDNode; }

  void HierarchyPrint(UInt uiTab=0);
  void PrintRDList();

  void RDOptim(UInt uiDepth=0);

  void MergeRDLists(RDTree *pcList1, RDTree *pcList2);

  void UpdateGlobalRDSlopeOfList();

  void ProgressRDNodeList(pRDTree &rpcNextInList)
  {
    m_pcNextRDNode = rpcNextInList;
    rpcNextInList = rpcNextInList->m_pcNextRDNode;
  }

};


#endif //RD_TREE__HPP