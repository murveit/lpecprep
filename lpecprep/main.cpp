#include <stdio.h>
#include "mainwindow.h"
#include <QApplication>

#include "phdconvert.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
#if defined(__linux__)
    setlocale(LC_NUMERIC, "C");
#endif
    MainWindow *win = new MainWindow();
    app.exec();

    delete win;

    return 0;
}
