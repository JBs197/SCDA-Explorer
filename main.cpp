#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_TRUETYPE_IMPLEMENTATION
#define STBI_ASSERT(x)
#define STBI_ONLY_PNG
#include "jlog.h"
#include "SCDAexplorer.h"
#include <QApplication>

JLOG* JLOG::instance = 0;
int JNODE::nextID{ 0 };
std::mutex m_bar, m_err;

int main(int argc, char* argv[])
{
    setlocale(LC_ALL, ".UTF8");

    QApplication a(argc, argv);
    a.setAttribute(Qt::AA_UseStyleSheetPropagationInWidgetStyles, 1);

    QString qsExecFolder = a.applicationDirPath();
    std::wstring wsExecFolder = qsExecFolder.toStdWString();
    QByteArray qbaTemp = qsExecFolder.toUtf8();
    string sExecFolder = qbaTemp.toStdString();

    QString qsCSS = qsExecFolder + "/css.txt";
    a.setStyleSheet(qsCSS);

    JLOG::getInstance()->init(sExecFolder, "SCDA-Explorer");

    SCDA scda(sExecFolder);
    scda.show();
    scda.postRender();
    return a.exec();
}