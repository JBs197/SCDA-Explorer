#include "mainwindow.h"
#include <QApplication>

mutex m_err;
const string sroot{ "F:" };
const string scroot{ "www12.statcan.gc.ca/datasets/index-eng.cfm" };

int main(int argc, char* argv[])
{
    setlocale(LC_ALL, ".UTF8");
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}