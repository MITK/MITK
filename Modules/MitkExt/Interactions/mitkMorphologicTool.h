#ifndef MITKMORPHOLOGICTOOL_H
#define MITKMORPHOLOGICTOOL_H

#include "mitkTool.h"
#include "mitkSBExports.h"

namespace mitk
{
  class MitkExt_EXPORT MorphologicTool : public Tool
  {
  public:

    mitkClassMacro(MorphologicTool, Tool);
    const char* GetGroup() const;

    itkGetMacro(Radius, unsigned int);
    void SetRadius(unsigned int);

    itkGetMacro(Preview, bool);
    itkSetMacro(Preview, bool);
    itkBooleanMacro(Preview);

    virtual void Activated();
    virtual void Deactivated();
    void AcceptPreview(const std::string& name, const Color& color);
    virtual void CancelPreviewing();

  protected:
    MorphologicTool();
    MorphologicTool(const char*);
    virtual ~MorphologicTool();

    virtual void UpdatePreview();
    virtual mitk::Image::Pointer ApplyFilter(mitk::Image::Pointer image);
    virtual void SetupPreviewNodeFor (mitk::DataNode* nodeToProceed);
    virtual void OnRoiDataChanged();

    unsigned int m_Radius;
    bool m_Preview;

    mitk::DataNode::Pointer m_FeedbackNode;
    mitk::DataNode* m_NodeToProceed;
    mitk::DataNode* m_OriginalNode;
  };//class
}//namespace

#endif