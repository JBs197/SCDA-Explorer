#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLayout>
#include <QListWidget>
#include <QListWidgetItem>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QStandardItemModel>
#include <QTableView>
#include <QPlainTextEdit>
#include <QTabWidget>
#include <sqlite3.h>
#include <iostream>
#include "jmap.h"
#include "gdifunc.h"
#include "gsfunc.h"
#include "imgfunc.h"
#include "iofunc.h"
#include "jtree.h"
#include "qtfunc.h"
#include "qtpaint.h"
#include "sqlfunc.h"
#include "statscan.h"
#include "switchboard.h"
#include "mathfunc.h"
#include "winfunc.h"
#include "zipfunc.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    BINMAP bm;
    GDIFUNC gdi;
    GSFUNC gf;
    IOFUNC io;
    IMGFUNC im;
    JFUNC jf;
    JTREE jtCataLocal, jtMapLocal, jtMapDB, jtCataOnline;
    MATHFUNC mf;
    PNGMAP pngm;
    QTFUNC qf;
    SWITCHBOARD sb;
    SQLFUNC sf;
    STATSCAN sc;
    WINFUNC wf;
    ZIPFUNC zf;

signals:

public slots:

private slots:
    void mousePressEvent(QMouseEvent* event);
    void on_cB_drives_currentTextChanged(const QString& arg1);
    void on_listW_map_itemDoubleClicked(QListWidgetItem* qItem);
    void on_listW_map_itemSelectionChanged();
    void on_listW_searchresult_itemDoubleClicked(QListWidgetItem* qItem);
    void on_listW_searchresult_itemSelectionChanged();
    void on_pB_clearsearch_clicked();
    void on_pB_createmap_clicked();
    void on_pB_deletetable_clicked();
    void on_pB_download_clicked();
    void on_pB_insert_clicked();
    void on_pB_insertmap_clicked();
    void on_pB_maplocal_clicked();
    void on_pB_resolution_clicked();
    void on_pB_resume_clicked();
    void on_pB_reviewmap_clicked();
    void on_pB_search_clicked();
    void on_pB_test_clicked();
    void on_pB_usc_clicked();
    void on_pB_viewtable_clicked();
    void on_tableW_db_currentCellChanged(int RowNow, int ColNow, int RowThen, int ColThen);
    void on_tableW_maplocal_currentCellChanged(int RowNow, int ColNow, int RowThen, int ColThen);
    void on_tabW_main_currentChanged(int index);
    void on_treeW_catadb_itemSelectionChanged();
    void on_treeW_catalocal_itemSelectionChanged();
    void on_treeW_cataonline_itemSelectionChanged();
    void on_treeW_mapdb_itemSelectionChanged();
    void on_treeW_maplocal_itemDoubleClicked(QTreeWidgetItem* qFolder, int column);
    void on_treeW_maplocal_itemSelectionChanged();

private:
    Ui::MainWindow *ui;

    int comm_length = 4;  // Number of integers used in every 'comm' vector.
    const int cores = 3, treeLength = 40, mapMargin = 1;
    const long long csvMaxSize = 200000000;  // Bytes
    string db_path, projectDir, savedSettings;
    const DWORD gui_sleep = 50;  // Number of milliseconds the GUI thread will sleep between event processings.
    bool ignorePartie = 1;
    unordered_map<string, string> mapCataToYear;
    string mapRoot = sroot + "\\maps";
    mutex m_bar;
    vector<vector<string>> navSearch;
    QPlainTextEdit* pteDefault;
    QWidget* recentClick = nullptr;
    vector<int> resDesktop = { 1920, 1080 };
    vector<double> resScaling = { 1.0, 1.0 };
    int statusResume;
    long long time;

    void autoExpand(QTreeWidget*& qTree, int maxNum);
    void barMessage(string message);
    void barReset(int iMax, string message);
    void barUpdate(int iCurrent);
    void bind(string&, vector<string>&);
    void createBinMap(SWITCHBOARD& sbgui);
    void createPngMap(string& tnameGeoLayers, string& tnameGeo);
    void displayTable(QTableWidget*& qTable, string tname);
    void downloadCatalogue(string sYear, string sCata);
    void getCataMapDB(SWITCHBOARD& sbgui, SQLFUNC& sfgui, JTREE& jtgui);
    void GetDesktopResolution(int& horizontal, int& vertical);
    void getGeoLayers(string sYear, string sCata, vector<string>& geoLayers);
    void getMapDBCoord(SWITCHBOARD& sbgui, SQLFUNC& sfgui, vector<vector<vector<double>>>& coord);
    void initGUI();
    void initialize();
    void initImgFont(string fontName);
    void insertCataMaps(SWITCHBOARD& sbgui, SQLFUNC& sfgui);
    void insertGeoLayers(string sYear, string sCata);
    void judicator(SWITCHBOARD& sbgui, SQLFUNC& sfgui);
    void pauseDebugMap(string mapPath);
    void qshow(string message);
    void qshow(vector<string> message);
    void reportTable(QTableWidget*& qTable);
    void scanLocalCata(SWITCHBOARD& sbgui, JTREE& jtgui);
    void scanLocalMap(SWITCHBOARD& sbgui, JTREE& jtgui);
    void tablePopulate(QTableWidget*& qTable, vector<vector<string>>& sData);
    void tableTopper(SWITCHBOARD& sbgui, vector<vector<string>>& vvsResult);
    void thrDownload(SWITCHBOARD& sbgui);
    void thrFileSplitter(SWITCHBOARD& sbgui, int& progress, mutex& m_progress);
    void thrUnzip(SWITCHBOARD& sbgui);
    void updateDBCata();


};


#endif // MAINWINDOW_H
