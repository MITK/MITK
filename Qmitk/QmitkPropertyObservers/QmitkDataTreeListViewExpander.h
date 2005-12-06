#ifndef QMITK_DATATREE_LISTVIEW_EXPANDER_H_INCLUDED_23hjb
#define QMITK_DATATREE_LISTVIEW_EXPANDER_H_INCLUDED_23hjb

#include<qlabel.h>
#include<list>
#include<mitkDataTreeFilter.h>

class QGridLayout;

class QmitkListViewItemIndex
{
  public:

    QmitkListViewItemIndex(QGridLayout* layout);
    ~QmitkListViewItemIndex();

    void addWidget(QWidget*);
    
    int rowAt(int y);  // y coordinate -> row index
    
    QmitkListViewItemIndex* childrenIndexAt(int row); // index for sub-items in a row
    mitk::DataTreeFilter::Item* itemAt(int row);   // mitkDataTreeFilter::Item of a row
    
  protected:

    QmitkListViewItemIndex();
    
    QGridLayout* m_Grid;
    std::list<QWidget*> m_Children;

  private:
};

class QmitkListViewExpanderIcon : public QLabel, public QmitkListViewItemIndex
{
  Q_OBJECT

  public:
    
    QmitkListViewExpanderIcon( QGridLayout* childContainer, QWidget* parent, const char* name = 0 );
    virtual ~QmitkListViewExpanderIcon();

    bool expanded();
    void setExpanded(bool);
    
  protected:
   
    virtual void mouseReleaseEvent (QMouseEvent*);

    virtual void showEvent(QShowEvent*);
    virtual void hideEvent(QHideEvent*);
    
  private:

    void setAllChildrenVisible(bool);

    bool m_Expanded;
};


#endif

