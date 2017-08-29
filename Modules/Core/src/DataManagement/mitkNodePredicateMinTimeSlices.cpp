#include "mitkNodePredicateMinTimeSlices.h"

#include <mitkImage.h>

mitk::NodePredicateMinTimeSlices::NodePredicateMinTimeSlices(unsigned int minTime)
  : NodePredicateBase(), 
    m_MinTime(minTime)
{
}

mitk::NodePredicateMinTimeSlices::~NodePredicateMinTimeSlices()
{
}


bool mitk::NodePredicateMinTimeSlices::CheckNode(const mitk::DataNode* node) const
{
  if (node == nullptr) {
    throw std::invalid_argument("NodePredicateFirstLevel: invalid node");
  }

  mitk::Image* image = dynamic_cast<mitk::Image*>(node->GetData());
  if (!image) {
    return false;
  }

  if (image->GetTimeSteps() < m_MinTime) {
    return false;
  }

  return true;
}

