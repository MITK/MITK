#ifndef QmitkMonaiLabelTool2DGUI_h_Included
#define QmitkMonaiLabelTool2DGUI_h_Included

#include <MitkSegmentationUIExports.h>
#include "QmitkMonaiLabelToolGUI.h"

class MITKSEGMENTATIONUI_EXPORT QmitkMonaiLabel2DToolGUI : public QmitkMonaiLabelToolGUI
{
  Q_OBJECT

public:
  mitkClassMacro(QmitkMonaiLabel2DToolGUI, QmitkMonaiLabelToolGUI);
  itkFactorylessNewMacro(Self);
  itkCloneMacro(Self);

protected:
  QmitkMonaiLabel2DToolGUI();
  ~QmitkMonaiLabel2DToolGUI() = default;
};

#endif
