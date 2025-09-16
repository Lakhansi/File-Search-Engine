#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <iostream>

int main(int argc, char *argv[])
{
    std::cout << "Starting Qt application test..." << std::endl;

    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    engine.load("qrc:/main.qml");

    if (engine.rootObjects().isEmpty()) {
        std::cout << "Failed to load QML" << std::endl;
        return -1;
    }

    std::cout << "Qt application started successfully!" << std::endl;
    return app.exec();
}
