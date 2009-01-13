#include "MantidMatrix.h"
#include "MantidUI.h"
#include "../Graph3D.h"
#include "../ApplicationWindow.h"
#include "../Spectrogram.h"
#include "MantidDataObjects/Workspace2D.h"

#include <QtGlobal>
#include <QTextStream>
#include <QList>
#include <QEvent>
#include <QContextMenuEvent>
#include <QVBoxLayout>
#include <QMouseEvent>
#include <QHeaderView>
#include <QApplication>
#include <QVarLengthArray>
#include <QClipboard>
#include <QShortcut>
#include <QPrinter>
#include <QPrintDialog>
#include <QPainter>
#include <QLocale>
#include <QItemDelegate>
#include <QLabel>
#include <QStackedWidget>
#include <QImageWriter>
#include <QSvgGenerator>
#include <QFile>
#include <QUndoStack>
#include <QCheckBox>
#include <QTabWidget>

#include <stdlib.h>
#include <iostream>
#include <algorithm>

MantidMatrix::MantidMatrix(Mantid::API::Workspace_sptr ws, ApplicationWindow* parent, const QString& label, const QString& name, int start, int end)
: MdiSubWindow(label, parent, name, 0),m_funct(this),m_min(0),m_max(0),m_are_min_max_set(false),
m_replaceObserver(*this,&MantidMatrix::handleReplaceWorkspace),
m_deleteObserver(*this,&MantidMatrix::handleDeleteWorkspace),
m_rowBegin(-1), m_rowEnd(-1), m_colBegin(-1), m_colEnd(-1)
{
    m_appWindow = parent;
    m_strName = name.toStdString();
    setup(ws,start,end);
	setWindowTitle(name);
	setName(name); 
	setIcon( QPixmap(matrixIcon()) );

    m_modelY = new MantidMatrixModel(this,ws,m_rows,m_cols,m_startRow,MantidMatrixModel::Y);
    m_table_viewY = new QTableView();
    connectTableView(m_table_viewY,m_modelY);

    m_modelX = new MantidMatrixModel(this,ws,m_rows,m_cols,m_startRow,MantidMatrixModel::X);
    m_table_viewX = new QTableView();
    connectTableView(m_table_viewX,m_modelX);

    m_modelE = new MantidMatrixModel(this,ws,m_rows,m_cols,m_startRow,MantidMatrixModel::E);
    m_table_viewE = new QTableView();
    connectTableView(m_table_viewE,m_modelE);

    m_tabs = new QTabWidget(this);
    m_tabs->insertTab(0,m_table_viewY,"Y values");
    m_tabs->insertTab(1,m_table_viewX,"X values"); 
    m_tabs->insertTab(2,m_table_viewE,"Errors");
    setWidget(m_tabs);

    setGeometry(50, 50, QMIN(5, numCols())*m_table_viewY->horizontalHeader()->sectionSize(0) + 55,
                (QMIN(10,numRows())+1)*m_table_viewY->verticalHeader()->sectionSize(0)+100);
 
    Mantid::API::AnalysisDataService::Instance().notificationCenter.addObserver(m_replaceObserver);
    Mantid::API::AnalysisDataService::Instance().notificationCenter.addObserver(m_deleteObserver);
    static bool Workspace_sptr_qRegistered = false;
    if (!Workspace_sptr_qRegistered)
    {
        Workspace_sptr_qRegistered = true;
        qRegisterMetaType<Mantid::API::Workspace_sptr>();
    }
    connect(this,SIGNAL(needChangeWorkspace(Mantid::API::Workspace_sptr)),this,SLOT(changeWorkspace(Mantid::API::Workspace_sptr)));
    connect(this,SIGNAL(needDeleteWorkspace()),this,SLOT(deleteWorkspace()));
    connect(this, SIGNAL(closedWindow(MdiSubWindow*)), this, SLOT(selfClosed(MdiSubWindow*)));

    askOnCloseEvent(false);
}

MantidMatrix::~MantidMatrix()
{
    Mantid::API::AnalysisDataService::Instance().notificationCenter.removeObserver(m_replaceObserver);
    Mantid::API::AnalysisDataService::Instance().notificationCenter.removeObserver(m_deleteObserver);
    delete m_modelY;
    delete m_modelX;
    delete m_modelE;
}

void MantidMatrix::setup(Mantid::API::Workspace_sptr ws, int start, int end)
{
    if (!ws.get())
    {
        QMessageBox::critical(0,"WorkspaceMatrixModel error","2D workspace expected.");
        m_rows = 0;
    	m_cols = 0; 
        m_startRow = 0;
        m_endRow = 0;
        return;
    }
    m_workspace = ws;
    m_startRow = (start<0 || start>=ws->getNumberHistograms())?0:start;
    m_endRow   = (end<0 || end>=ws->getNumberHistograms() || end < start)?ws->getNumberHistograms()-1:end;
    m_rows = m_endRow - m_startRow + 1;
	m_cols = ws->blocksize(); 
    m_histogram = false;
    if ( m_workspace->blocksize() || m_workspace->readX(0).size() != m_workspace->readY(0).size() ) m_histogram = true;
    connect(this,SIGNAL(needsUpdating()),this,SLOT(repaintAll()));

    x_start = ws->readX(0)[0];
    if (ws->readX(0).size() != ws->readY(0).size()) x_end = ws->readX(0)[ws->blocksize()];
    else
        x_end = ws->readX(0)[ws->blocksize()-1];
    // What if y is not a spectrum number?
    y_start = double(m_startRow);
    y_end = double(m_endRow);

    m_bk_color = QColor(128, 255, 255);
    m_matrix_icon = mantid_matrix_xpm;
    m_column_width = 100;

    m_funct.init();

}

void MantidMatrix::connectTableView(QTableView* view,MantidMatrixModel*model)
{
    view->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));
    view->setSelectionMode(QAbstractItemView::ContiguousSelection);// only one contiguous selection supported
    view->setModel(model);
    view->setCornerButtonEnabled(false);
    view->setFocusPolicy(Qt::StrongFocus);
    
    
    QPalette pal = view->palette();
    pal.setColor(QColorGroup::Base, m_bk_color);
    view->setPalette(pal);
    
    // set header properties
    QHeaderView* hHeader = (QHeaderView*)view->horizontalHeader();
    hHeader->setMovable(false);
    hHeader->setResizeMode(QHeaderView::Fixed);
    hHeader->setDefaultSectionSize(m_column_width);
    
    view->resizeRowToContents(0);
    int row_height = view->rowHeight(0);

	QHeaderView* vHeader = (QHeaderView*)view->verticalHeader();
    vHeader->setDefaultSectionSize(row_height);
    vHeader->setResizeMode(QHeaderView::Fixed);
    vHeader->setMovable(false);

}

double MantidMatrix::cell(int row, int col)
{
	return m_modelY->data(row, col);
}

QString MantidMatrix::text(int row, int col)
{
    return QString::number(activeModel()->data(row, col));
}

void MantidMatrix::setColumnsWidth(int width)
{
	if (m_column_width == width)
		return;

    m_column_width = width;
    m_table_viewY->horizontalHeader()->setDefaultSectionSize(m_column_width);
    m_table_viewX->horizontalHeader()->setDefaultSectionSize(m_column_width);
    m_table_viewE->horizontalHeader()->setDefaultSectionSize(m_column_width);

    int cols = numCols();
    for(int i=0; i<cols; i++)
    {
        m_table_viewY->setColumnWidth(i, width);
        m_table_viewX->setColumnWidth(i, width);
        m_table_viewE->setColumnWidth(i, width);
    }

	emit modifiedWindow(this);
}

QTableView *MantidMatrix::activeView()
{
    switch (m_tabs->currentIndex())
    {
    case 0: return m_table_viewY;
    case 1: return m_table_viewX;
    case 2: return m_table_viewE;
    }
    return m_table_viewY;
}

MantidMatrixModel *MantidMatrix::activeModel()
{
    switch (m_tabs->currentIndex())
    {
    case 0: return m_modelY;
    case 1: return m_modelX;
    case 2: return m_modelE;
    }
    return m_modelY;
}

void MantidMatrix::copySelection()
{
    QItemSelectionModel *selModel = activeView()->selectionModel();
	QString s = "";
	QString eol = applicationWindow()->endOfLine();
	if (!selModel->hasSelection()){
		QModelIndex index = selModel->currentIndex();
		s = text(index.row(), index.column());
	} else {
		QItemSelection sel = selModel->selection();
		QListIterator<QItemSelectionRange> it(sel);
		if(!it.hasNext())
			return;

		QItemSelectionRange cur = it.next();
		int top = cur.top();
		int bottom = cur.bottom();
		int left = cur.left();
		int right = cur.right();
		for(int i=top; i<=bottom; i++){
			for(int j=left; j<right; j++)
				s += text(i, j) + "\t";
			s += text(i,right) + eol;
		}
	}
	// Copy text into the clipboard
	QApplication::clipboard()->setText(s.trimmed());
}

void MantidMatrix::range(double *min, double *max)
{
    if (!m_are_min_max_set)
    {
	    m_min = cell(0, 0);
	    m_max = m_min;
	    int rows = numRows();
	    int cols = numCols();

	    for(int i=0; i<rows; i++){
		    for(int j=0; j<cols; j++){
			    double aux = cell(i, j);
			    if (aux <= m_min)
				    m_min = aux;

			    if (aux >= m_max)
				    m_max = aux;
		    }
	    }
        m_are_min_max_set = true;
    }

    *min = m_min;
    *max = m_max;
}

void MantidMatrix::setRange(double min, double max)
{
    m_min = min;
    m_max = max;
    m_are_min_max_set = true;
}

double** MantidMatrix::allocateMatrixData(int rows, int columns)
{
	double** data = (double **)malloc(rows * sizeof (double*));
	if(!data){
		QMessageBox::critical(0, tr("QtiPlot") + " - " + tr("Memory Allocation Error"),
		tr("Not enough memory, operation aborted!"));
		return NULL;
	}

	for ( int i = 0; i < rows; ++i){
		data[i] = (double *)malloc(columns * sizeof (double));
		if(!data[i]){
		    for ( int j = 0; j < i; j++)
                free(data[j]);
		    free(data);

			QMessageBox::critical(0, tr("QtiPlot") + " - " + tr("Memory Allocation Error"),
			tr("Not enough memory, operation aborted!"));
			return NULL;
		}
	}
	return data;
}

void MantidMatrix::freeMatrixData(double **data, int rows)
{
	for ( int i = 0; i < rows; i++)
		free(data[i]);

	free(data);
}

void MantidMatrix::goTo(int row,int col)
{
	if(row < 1 || row > numRows())
		return;
	if(col < 1 || col > numCols())
		return;

	activeView()->scrollTo(activeModel()->index(row - 1, col - 1), QAbstractItemView::PositionAtTop);
}

void MantidMatrix::goToRow(int row)
{
	if(row < 1 || row > numRows())
		return;

	activeView()->selectRow(row - 1);
	activeView()->scrollTo(activeModel()->index(row - 1, 0), QAbstractItemView::PositionAtTop);
}

void MantidMatrix::goToColumn(int col)
{
	if(col < 1 || col > numCols())
		return;

	activeView()->selectColumn(col - 1);
	activeView()->scrollTo(activeModel()->index(0, col - 1), QAbstractItemView::PositionAtCenter);
}

double MantidMatrix::dataX(int row, int col) const
{
    if (!m_workspace || row >= numRows() || col >= m_workspace->readX(row + m_startRow).size()) return 0.;
    double res = m_workspace->readX(row + m_startRow)[col];
    return res;

}

double MantidMatrix::dataY(int row, int col) const
{
    if (!m_workspace || row >= numRows() || col >= numCols()) return 0.;
    double res = m_workspace->readY(row + m_startRow)[col];
    return res;

}

double MantidMatrix::dataE(int row, int col) const
{
    if (!m_workspace || row >= numRows() || col >= numCols()) return 0.;
    double res = m_workspace->readE(row + m_startRow)[col];
    if (res == 0.) res = 1.;//  quick fix of the fitting problem
    return res;

}

int MantidMatrix::indexX(double s)const
{
    int n = m_workspace->blocksize();

    if (n == 0 || s < m_workspace->readX(0)[0] || s > m_workspace->readX(0)[n-1]) return -1;

    int i = 0, j = n-1, k = n/2;
    double ss;
    int it;
    for(it=0;it<n;it++)
    {
        ss = m_workspace->readX(0)[k];
        if (ss == s || abs(i - j) <2) break;
        if (s > ss) i = k;
        else
            j = k;
        k = i + (j - i)/2;
    }

    return i;
}

/*void MantidMatrix::copy(Matrix *m)
{
	if (!m)
        return;

    MatrixModel *mModel = m->matrixModel();
	if (!mModel)
		return;

	x_start = m->xStart();
	x_end = m->xEnd();
	y_start = m->yStart();
	y_end = m->yEnd();

	int rows = numRows();
	int cols = numCols();

    txt_format = m->textFormat();
	num_precision = m->precision();

    double *data = d_matrix_model->dataVector();
    double *m_data = mModel->dataVector();
    int size = rows*cols;
    for (int i=0; i<size; i++)
        data[i] = m_data[i];

	d_header_view_type = m->headerViewType();
    d_view_type = m->viewType();
	setColumnsWidth(m->columnsWidth());
	formula_str = m->formula();
    d_color_map_type = m->colorMapType();
    d_color_map = m->colorMap();

    if (d_view_type == ImageView){
	    if (d_table_view)
            delete d_table_view;
        if (d_select_all_shortcut)
            delete d_select_all_shortcut;
	    initImageView();
		d_stack->setCurrentWidget(imageLabel);
	}
	resetView();
}*/

QwtDoubleRect MantidMatrix::boundingRect()
{
    int rows = numRows();
    int cols = numCols();
    double dx = fabs(x_end - x_start)/(double)(cols - 1);
    double dy = fabs(y_end - y_start)/(double)(rows - 1);

    return QwtDoubleRect(QMIN(x_start, x_end) - 0.5*dx, QMIN(y_start, y_end) - 0.5*dy,
						 fabs(x_end - x_start) + dx, fabs(y_end - y_start) + dy).normalized();
}

//----------------------------------------------------------------------------
void MantidMatrixFunction::init()
{
    int nx = m_matrix->numCols();
    int ny = m_matrix->numRows();

    m_dx = (m_matrix->xEnd() - m_matrix->xStart()) / (nx > 1? nx - 1 : 1);
    m_dy = (m_matrix->yEnd() - m_matrix->yStart()) / (ny > 1? ny - 1 : 1);

    if (m_dx == 0.) m_dx = 1.;//?
    if (m_dy == 0.) m_dy = 1.;//?
}

double MantidMatrixFunction::operator()(double x, double y)
{
    x += 0.5*m_dx;
    y -= 0.5*m_dy;
    	
    int i = abs((y - m_matrix->yStart())/m_dy);
    int j = abs((x - m_matrix->xStart())/m_dx);

    int jj = m_matrix->indexX(x);
    if (jj >= 0) j = jj;

    if (i >= 0 && i < m_matrix->numRows() && j >=0 && j < m_matrix->numCols())
	    return m_matrix->dataY(i,j);
    else
	    return 0.0;
}
//----------------------------------------------------------------------------

Graph3D * MantidMatrix::plotGraph3D(int style)
{
	QApplication::setOverrideCursor(Qt::WaitCursor);

    ApplicationWindow* a = applicationWindow();
	QString labl = a->generateUniqueName(tr("Graph"));

	Graph3D *plot = new Graph3D("", a);
	plot->resize(500,400);
	plot->setWindowTitle(labl);
	plot->setName(labl);
    plot->setTitle(tr("Workspace ")+name());
	a->customPlot3D(plot);
	plot->customPlotStyle(style);
    int resCol = numCols() / 200;
    int resRow = numRows() / 200;
    plot->setResolution( qMax(resCol,resRow) );

    double zMin =  1e300;
    double zMax = -1e300;
    for(int i=0;i<numRows();i++)
    for(int j=0;j<numCols();j++)
    {
        if (cell(i,j) < zMin) zMin = cell(i,j);
        if (cell(i,j) > zMax) zMax = cell(i,j);
    }
    
	plot->addFunction("", xStart(), xEnd(), yStart(), yEnd(), zMin, zMax, numCols(), numRows(), static_cast<UserHelperFunction*>(&m_funct));
	
    Mantid::API::Axis* ax = m_workspace->getAxis(0);
    std::string s;
    if (ax->unit().get()) s = ax->unit()->caption() + " / " + ax->unit()->label();
    else
        s = "X Axis";
    plot->setXAxisLabel(tr(s.c_str()));
    
    ax = m_workspace->getAxis(1);
    if (ax->isNumeric()) 
    {
        if (ax->unit().get()) s = ax->unit()->caption() + " / " + ax->unit()->label();
        else
            s = "Y Axis";
        plot->setYAxisLabel(tr(s.c_str())); 
    }
    else
        plot->setYAxisLabel(tr("Spectrum")); 

    plot->setZAxisLabel(tr(m_workspace->YUnit().c_str())); 

    a->initPlot3D(plot);
    plot->askOnCloseEvent(false);
	QApplication::restoreOverrideCursor();

    return plot;
}

MultiLayer* MantidMatrix::plotGraph2D(Graph::CurveType type)
{
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    ApplicationWindow *a = applicationWindow();
	MultiLayer* g = a->multilayerPlot(a->generateUniqueName(tr("Graph")));
    m_plots2D<<g;
    connect(g, SIGNAL(closedWindow(MdiSubWindow*)), this, SLOT(dependantClosed(MdiSubWindow*)));
    a->connectMultilayerPlot(g);
    Graph* plot = g->activeGraph();
	a->setPreferences(plot);
    plot->setTitle(tr("Workspace ") + name());
    Mantid::API::Axis* ax;
    ax = m_workspace->getAxis(0);
    std::string s;
    if (ax->unit().get()) s = ax->unit()->caption() + " / " + ax->unit()->label();
    else
        s = "X axis";
    plot->setXAxisTitle(tr(s.c_str()));
    ax = m_workspace->getAxis(1);
    if (ax->isNumeric()) 
    {
        if (ax->unit().get()) s = ax->unit()->caption() + " / " + ax->unit()->label();
        else
            s = "Y axis";
        plot->setYAxisTitle(tr(s.c_str())); 
    }
    else
        plot->setYAxisTitle(tr("Spectrum")); 

    double minz, maxz;
    range(&minz,&maxz);
	plot->plotSpectrogram(&m_funct, numRows(), numCols(), boundingRect(), minz, maxz, type);

    plot->setAutoScale();
    g->askOnCloseEvent(false);

	QApplication::restoreOverrideCursor();
	return g;
}

void MantidMatrix::setGraph1D(MultiLayer *ml, Table* t)
{
    Graph* g = ml->activeGraph();
    g->setTitle(tr("Workspace ")+name());
    Mantid::API::Axis* ax;
    ax = m_workspace->getAxis(0);
    std::string s;
    if (ax->unit().get()) s = ax->unit()->caption() + " / " + ax->unit()->label();
    else
        s = "X axis";
    g->setXAxisTitle(tr(s.c_str()));
    g->setYAxisTitle(tr(m_workspace->YUnit().c_str()));
    connect(ml, SIGNAL(closedWindow(MdiSubWindow*)), this, SLOT(dependantClosed(MdiSubWindow*)));
    if (t) 
    {
        m_plots1D[ml] = t;
        connect(t, SIGNAL(closedWindow(MdiSubWindow*)), this, SLOT(dependantClosed(MdiSubWindow*)));
    }
    else
        m_plots2D<<ml;
}

// Remove all references to the MantidMatrix
void MantidMatrix::removeWindow()
{
	QList<MdiSubWindow *> windows = applicationWindow()->windowsList();
	foreach(MdiSubWindow *w, windows){
		if (w->isA("Graph3D") && ((Graph3D*)w)->userFunction()->hlpFun() == &m_funct)
			((Graph3D*)w)->clearData();
		else if (w->isA("MultiLayer")){
			QList<Graph *> layers = ((MultiLayer*)w)->layersList();
			foreach(Graph *g, layers){
				for (int i=0; i<g->curves(); i++){
                    Spectrogram *sp = (Spectrogram *)g->plotItem(i);
                    if (sp && sp->rtti() == QwtPlotItem::Rtti_PlotSpectrogram && sp->funct() == &m_funct)
                        g->removeCurve(i);
				}
			}
		}
	}
}

void MantidMatrix::getSelectedRows(int& i0,int& i1)
{
  i0 = m_rowBegin;
  i1 = m_rowEnd;
}

/*
   Returns indices of the first and last selected rows in i0 and i1.
*/
bool MantidMatrix::setSelectedRows()
{
  QTableView *tv = activeView();
  QItemSelectionModel *selModel = tv->selectionModel();
  if( !selModel  ) return false;

  QPoint localCursor = tv->mapFromGlobal(QCursor::pos());
  //This is due to what I think is a bug in Qt where it seems to include
  //the height  of the horizontalHeader bar in determining the row that the mouse pointer
  //is pointing at. This results in the last row having an undefined index so we need to trick
  // the function by asking for a different postion
  localCursor.ry() -=  tv->horizontalHeader()->height();

  if( localCursor.x() > tv->verticalHeader()->width() ) return false;
  QModelIndex cursorIndex = tv->indexAt(localCursor);

  if( selModel->selection().contains(cursorIndex)  && 
      selModel->selection().front().left() == 0 && 
      selModel->selection().front().right() == tv->horizontalHeader()->count() - 1 )
  {
    m_rowBegin = selModel->selection().front().top();
    m_rowEnd = selModel->selection().front().bottom();
  }
  else
  {
    m_rowBegin = m_rowEnd = cursorIndex.row();
    tv->selectRow(m_rowBegin);
  }
  if( m_rowBegin == -1 || m_rowEnd == -1 ) return false;

  return true;
}

void MantidMatrix::getSelectedColumns(int& i0,int& i1)
{
  i0 = m_colBegin;
  i1 = m_colEnd;
}

/*
   Returns indices of the first and last selected column in i0 and i1.
*/
bool MantidMatrix::setSelectedColumns()
{
  QTableView *tv = activeView();
  QItemSelectionModel *selModel = tv->selectionModel();
  if( !selModel ) return false;

  QPoint localCursor = tv->mapFromGlobal(QCursor::pos());
  //This is due to what I think is a bug in Qt where it seems to include
  //the width of the verticalHeader bar in determining the column that the mouse pointer
  //is pointing at
  localCursor.rx() -=  tv->verticalHeader()->width();
  if( localCursor.y() > tv->horizontalHeader()->height() ) return false;

  QModelIndex index = tv->indexAt(localCursor);
  QModelIndex cursorIndex = index.sibling(index.row(), index.column()); 

  if( selModel->selection().contains(cursorIndex) )
  {
    m_colBegin = selModel->selection().front().left();
    m_colEnd = selModel->selection().front().right();
  }
  else
  {
    m_colBegin = m_colEnd = tv->indexAt(localCursor).column();
    tv->selectColumn(m_colBegin);
  }

  if( m_colBegin == 0 && m_colEnd == tv->horizontalHeader()->count() - 1 ) return false;

  return true;
}

void MantidMatrix::tst()
{
    std::cerr<<"2D plots: "<<m_plots2D.size()<<'\n';
    std::cerr<<"1D plots: "<<m_plots1D.size()<<'\n';
}

void MantidMatrix::dependantClosed(MdiSubWindow* w)
{
    if (w->isA("MultiLayer")) 
    {
        int i = m_plots2D.indexOf((MultiLayer*)w);
        if (i >= 0) m_plots2D.remove(i);
        else
        {
            QMap<MultiLayer*,Table*>::iterator i = m_plots1D.find((MultiLayer*)w);
            if (i != m_plots1D.end())
            {
                if (i.value() != 0) 
                {
                    i.value()->askOnCloseEvent(false);
                    i.value()->close();
                }
                m_plots1D.erase(i);
            }
        }
    }
}

void MantidMatrix::repaintAll()
{
    repaint();
    QVector<MultiLayer*>::iterator vEnd  = m_plots2D.end();
    for( QVector<MultiLayer*>::iterator vItr = m_plots2D.begin(); vItr != vEnd; ++vItr )
    {
      (*vItr)->activeGraph()->replot();
    }
    QMap<MultiLayer*,Table*>::iterator mEnd = m_plots1D.end();
    for(QMap<MultiLayer*,Table*>::iterator mItr = m_plots1D.begin(); mItr != mEnd;  ++mItr)
    {
        Table* t = mItr.value();
        if ( !t ) continue;
        int charsToRemove = t->name().size() + 1;
	int nTableCols(t->numCols());
        for(int col = 1; col < nTableCols; ++col)
        {
	  QString colName = t->colName(col).remove(0,charsToRemove);
	  if( colName.isEmpty() ) break;
	  //Need to determine whether the table was created from plotting a spectrum
	  //or a time bin. A spectrum has a Y column name YS and a bin YB
	  QString ident = colName.left(2);
	  colName.remove(0,2); //This now contains the number in the MantidMatrix
	  int matrixNumber = colName.toInt();
	  if( matrixNumber < 0 ) break;
	  bool errs = (ident[0] == QChar('E')) ? true : false;
	  if( ident[1] == QChar('S') )
	  {
	    if( matrixNumber >= numRows() ) break;
	    int endCount = numCols();
	    for(int j = 0; j < endCount; ++j)
	    {
	      if( errs ) t->setCell(j, col, dataE(matrixNumber, j));
	      else t->setCell(j, col, dataY(matrixNumber, j));
	    }	    
	  }
	  else
	  {
	    if( matrixNumber >= numCols() ) break;
	    int endCount = numRows();
	    for(int j = 0; j < endCount; ++j)
	    {
	      if( errs ) t->setCell(j, col, dataE(j, matrixNumber));
	      else t->setCell(j, col, dataY(j, matrixNumber));
	    }
	  }
        }
        t->notifyChanges();
        Graph *g = mItr.key()->activeGraph();
        if (g) g->setAutoScale();
    }
}

void MantidMatrix::handleReplaceWorkspace(const Poco::AutoPtr<Mantid::Kernel::DataService<Mantid::API::Workspace>::BeforeReplaceNotification>& pNf)
{
  if( !pNf->object() || !pNf->new_object() ) return;

  if (pNf->new_object() != m_workspace && pNf->object_name() == m_strName)
    {
        emit needChangeWorkspace(pNf->new_object());
    }
}

void MantidMatrix::changeWorkspace(Mantid::API::Workspace_sptr ws)
{
    if (m_workspace->blocksize() != ws->blocksize() ||
        m_workspace->getNumberHistograms() != ws->getNumberHistograms())
      { 
	closeDependants();
      }

    // Save selection
    QItemSelectionModel *oldSelModel = activeView()->selectionModel();
    QModelIndexList indexList = oldSelModel->selectedIndexes();
    QModelIndex curIndex = activeView()->currentIndex();
    
    setup(ws,m_startRow,m_endRow);
    
    delete m_modelY;
    m_modelY = new MantidMatrixModel(this,ws,m_rows,m_cols,m_startRow,MantidMatrixModel::Y);
    connectTableView(m_table_viewY,m_modelY);

    delete m_modelX;
    m_modelX = new MantidMatrixModel(this,ws,m_rows,m_cols,m_startRow,MantidMatrixModel::X);
    connectTableView(m_table_viewX,m_modelX);

    delete m_modelE;
    m_modelE = new MantidMatrixModel(this,ws,m_rows,m_cols,m_startRow,MantidMatrixModel::E);
    connectTableView(m_table_viewE,m_modelE);

    // Restore selection
    activeView()->setCurrentIndex(curIndex);
    if (indexList.size())
    {
        QItemSelection sel(indexList.first(),indexList.last());
        QItemSelectionModel *selModel = activeView()->selectionModel();
        selModel->select(sel,QItemSelectionModel::Select);
    }

    repaintAll();

} 

void MantidMatrix::closeDependants()
{
  while(m_plots2D.size())
  {
      MultiLayer* ml = m_plots2D.front();
      ml->askOnCloseEvent(false);
      ml->close();// this calls slot dependantClosed() which removes the pointer from m_plots2D
  }
  while(m_plots1D.size())
  {
      MultiLayer* ml = m_plots1D.begin().key();
      ml->askOnCloseEvent(false);
      ml->close();// this calls slot dependantClosed() which removes the pointer from m_plots1D
  }
}

void MantidMatrix::handleDeleteWorkspace(const Poco::AutoPtr<Mantid::Kernel::DataService<Mantid::API::Workspace>::DeleteNotification>& pNf)
{
    if (pNf->object() == m_workspace)
    {
        emit needDeleteWorkspace();
    }
}

void MantidMatrix::deleteWorkspace()
{
    askOnCloseEvent(false);
    close();
}

void MantidMatrix::selfClosed(MdiSubWindow* w)
{
  closeDependants();
}


QVariant MantidMatrixModel::data(const QModelIndex &index, int role) const
{
    if (role != Qt::DisplayRole) return QVariant();
    double val = data(index.row(),index.column());
    return QVariant(m_locale.toString(val,'f',6));
}
