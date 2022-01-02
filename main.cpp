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
mutex m_bar, m_err;

int main(int argc, char* argv[])
{
    setlocale(LC_ALL, ".UTF8");

    QApplication a(argc, argv);
    a.setAttribute(Qt::AA_UseStyleSheetPropagationInWidgetStyles, 1);

    QString qsExecFolder = a.applicationDirPath();
    string sExecFolder = qsExecFolder.toUtf8();
    string cssPath = sExecFolder + "\\css.txt";
    a.setStyleSheet(cssPath.c_str());

    JLOG::getInstance()->init(sExecFolder, "SCDA-Explorer");

    SCDA scda(sExecFolder);
    scda.show();
    scda.postRender();
    return a.exec();
}