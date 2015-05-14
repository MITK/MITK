#include "mitkPlanarComment.h"
#include "mitkPlaneGeometry.h"

#include "mitkPlanarLine.h"
#include "mitkPlaneGeometry.h"


mitk::PlanarComment::PlanarComment()
: FEATURE_ID_LENGTH( this->AddFeature( "Length", "mm" ) )
{
  this->ResetNumberOfControlPoints(1);
  this->SetNumberOfPolyLines(1);
}


mitk::PlanarComment::~PlanarComment()
{
}

void mitk::PlanarComment::setText(const std::string& comment)
{
  m_commentText = comment;
}

void mitk::PlanarComment::GeneratePolyLine()
{
  this->ClearPolyLines();

  this->AppendPointToPolyLine(0, this->GetControlPoint(0));
}

void mitk::PlanarComment::GenerateHelperPolyLine(double /*mmPerDisplayUnit*/, unsigned int /*displayHeight*/)
{
}

std::string mitk::PlanarComment::EvaluateAnnotation()
{
  return m_commentText;
}

void mitk::PlanarComment::EvaluateFeaturesInternal()
{
  // NOT USED.
}


void mitk::PlanarComment::PrintSelf( std::ostream& os, itk::Indent indent) const
{
  Superclass::PrintSelf( os, indent );
}

 bool mitk::PlanarComment::Equals(const PlanarFigure &other) const
 {
   const mitk::PlanarComment* otherLine = dynamic_cast<const mitk::PlanarComment*>(&other);
   if ( otherLine )
   {
     return Superclass::Equals(other);
   }
   else
   {
     return false;
   }
 }