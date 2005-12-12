#ifndef QMITK_DATATREE_LISTVIEW_EXPANDER_H_INCLUDED_23hjb
#define QMITK_DATATREE_LISTVIEW_EXPANDER_H_INCLUDED_23hjb

#include<qlabel.h>
#include<list>
#include<mitkDataTreeFilter.h>

class QGridLayout;

class QmitkListViewItemIndex
{
  public:

    QmitkListViewItemIndex(QGridLayout* layout, QmitkListViewItemIndex* parentIndex);
    ~QmitkListViewItemIndex();

    void addIndex(QmitkListViewItemIndex*, int row);
    void addWidget(QWidget*, int row, int col, int alignment = 0);
    void addMultiCellWidget(QWidget*, int fromRow, int toRow, int fromCol, int toCol, int alignment = 0);
    void addItem(mitk::DataTreeFilter::Item*, int row);
    
    int rowAt(int y);  // y coordinate -> row index

    QmitkListViewItemIndex* indexAt(int row);       // index for sub-items in a row
    mitk::DataTreeFilter::Item* itemAt(int row);   // mitkDataTreeFilter::Item of a row
    std::list<QWidget*>& widgetsAt(int row);      // widgets of a row
    
    void lockBecauseOfSelection(bool);
    QmitkListViewItemIndex* parentIndex();

    QGridLayout* m_Grid;
    
  protected:

    typedef std::vector< std::pair< mitk::DataTreeFilter::Item*, std::list<QWidget*> > > RowStructureType;

    QmitkListViewItemIndex();
    void clearIndex();
    
    RowStructureType m_Rows;
    std::vector<QmitkListViewItemIndex*> m_Indices;

    int m_Locked;
    QmitkListViewItemIndex* m_ParentIndex;

  private:
};

class QmitkListViewExpanderIcon : public QLabel, public QmitkListViewItemIndex
{
  Q_OBJECT

  public:
    
    QmitkListViewExpanderIcon( QGridLayout* childContainer, QmitkListViewItemIndex* parentIndex, QWidget* parent, const char* name = 0 );
    virtual ~QmitkListViewExpanderIcon();

    bool expanded();
    void setExpanded(bool);

    virtual QSize sizeHint();
    
  protected:
   
    virtual void mouseReleaseEvent (QMouseEvent*);

    virtual void showEvent(QShowEvent*);
    virtual void hideEvent(QHideEvent*);

  protected slots:

    void displayCorrectIcon();
    
  private:

    void setAllChildrenVisible(bool);

    bool m_Expanded;
};


#endif

