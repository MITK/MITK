#pragma once

#include "mitkNodePredicateBase.h"
#include "mitkDataNode.h"

namespace mitk {

class MITKCORE_EXPORT NodePredicateMinTimeSlices : public NodePredicateBase
{
public:
  mitkClassMacro(NodePredicateMinTimeSlices, NodePredicateBase);
  mitkNewMacro1Param(NodePredicateMinTimeSlices, unsigned int);

  virtual ~NodePredicateMinTimeSlices();

  virtual bool CheckNode(const mitk::DataNode* node) const override;

protected:
  NodePredicateMinTimeSlices(unsigned int minTime);

  unsigned int m_MinTime;
};

}
