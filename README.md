**Deutsche Version**

Aufgaben-App
Dies ist eine einfache Aufgabenverwaltung-App, entwickelt mit C++ und Qt. Diese Anwendung ermöglicht es Benutzern, ihre Aufgaben zu erstellen, zu bearbeiten und zu löschen.

Funktionen
Aufgaben erstellen

Aufgaben bearbeiten

Aufgaben löschen

Aufgaben als erledigt markieren

Installation
Klone dieses Repository:

**bash**
**git clone https://github.com/on2me/Aufgaben-App**
Öffne Qt Creator und wähle Datei -> Öffnen.

Navigiere zum geklonten Repository und wähle die .pro-Datei aus, um das Projekt zu importieren.

Drücke auf Erstellen, um das Projekt zu kompilieren und auszuführen.

App-Icon
Falls das App-Icon nicht angezeigt wird, stelle sicher, dass:

Das Icon in der resources.qrc-Datei definiert ist.

Der Pfad zum Icon korrekt ist.

Das Icon in main.cpp gesetzt wird.

cpp
QApplication app(argc, argv);
QIcon appIcon(":/icons/app_icon.png");
app.setWindowIcon(appIcon);
Lizenz
Dieses Projekt steht unter der MIT-Lizenz. Weitere Informationen findest du in der LICENSE-Datei.
**Fühle dich frei, Änderungen vorzunehmen oder dieses Projekt zu kopieren.**

**English Version**

Tasks App
This is a simple task management app developed with C++ and Qt. This application allows users to create, edit, and delete tasks.

Features
Create tasks

Edit tasks

Delete tasks

Mark tasks as completed

Installation
Clone this repository:

**bash**
**git clone https://github.com/on2me/Aufgaben-App**
Open Qt Creator and select File -> Open File or Project.

Navigate to the cloned repository and select the .pro file to import the project.

Click on Build to compile and run the project.

App Icon
If the app icon is not displayed, ensure that:

The icon is defined in the resources.qrc file.

The path to the icon is correct.

The icon is set in main.cpp.

cpp
QApplication app(argc, argv);
QIcon appIcon(":/icons/app_icon.png");
app.setWindowIcon(appIcon);
License
This project is licensed under the MIT License. For more details, see the LICENSE file.
**Feel free to make changes or copy this project.**
