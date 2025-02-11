#include <QApplication>
#include "MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Set the application palette for the dark theme
    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window, QColor("#121212"));        // Dark background
    darkPalette.setColor(QPalette::WindowText, QColor("#FFFFFF"));    // White text
    darkPalette.setColor(QPalette::Base, QColor("#1E1E1E"));
    darkPalette.setColor(QPalette::AlternateBase, QColor("#1E1E1E"));
    darkPalette.setColor(QPalette::ToolTipBase, QColor("#FFFFFF"));
    darkPalette.setColor(QPalette::ToolTipText, QColor("#FFFFFF"));
    darkPalette.setColor(QPalette::Text, QColor("#FFFFFF"));
    darkPalette.setColor(QPalette::Button, QColor("#1E1E1E"));
    darkPalette.setColor(QPalette::ButtonText, QColor("#FFFFFF"));
    darkPalette.setColor(QPalette::BrightText, QColor("#FFFFFF"));
    darkPalette.setColor(QPalette::Highlight, QColor("#007BFF"));     // Blue accent
    darkPalette.setColor(QPalette::HighlightedText, QColor("#FFFFFF"));

    a.setPalette(darkPalette);

    // Load a modern font
    QFont font("Segoe UI", 10);
    a.setFont(font);

    MainWindow w;
    w.show();

    return a.exec();
}
