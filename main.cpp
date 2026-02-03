#include <QtWidgets/QApplication>

#include "backend.cpp"
#include "gui.cpp"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    MainWindow window;
    window.show();
    return app.exec();
}
