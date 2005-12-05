#ifndef QMITK_DATATREE_LISTVIEW_EXPANDER_H_INCLUDED_23hjb
#define QMITK_DATATREE_LISTVIEW_EXPANDER_H_INCLUDED_23hjb

#include<qlabel.h>
#include<list>

class QGridLayout;

class QmitkListViewExpanderIcon : public QLabel
{
  Q_OBJECT

  public:
    
    QmitkListViewExpanderIcon( const QGridLayout* childContainer, QWidget* parent, const char* name = 0 );
    virtual ~QmitkListViewExpanderIcon();

    void addWidget(QWidget*);
    
    bool expanded();
    void setExpanded(bool);
    
  protected:
   
    virtual void mouseReleaseEvent (QMouseEvent*);

    virtual void showEvent(QShowEvent*);
    virtual void hideEvent(QHideEvent*);
    
  private:

    void setAllChildrenVisible(bool);

    bool m_Expanded;
    const QGridLayout* m_ChildContainer;

    std::list<QWidget*> m_Children;

};


#endif

