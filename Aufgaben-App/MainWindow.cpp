#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QInputDialog>
#include <QMessageBox>
#include <QFile>
#include <QLabel>
#include <QJsonDocument>
#include <QEvent>
#include <QMouseEvent>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QDebug>
#include <QMenu>
#include <QSet>
#include <QDateTimeEdit>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , totalPoints(0) // Initialisiere die Punkte, falls nicht anderswo gesetzt.
{
    ui->setupUi(this);

    setWindowTitle("Aufgaben-App");
    resize(800, 600);

    trayIcon = new QSystemTrayIcon(QIcon(":/Aufgaben-App2.png"), this);
    trayIcon->setVisible(true);  // Tray-Icon anzeigen
    trayIcon->setToolTip("Aufgaben-App");

    setupUI();
    loadStyleSheet();
    loadTasks();

    // Check for notifications every second
    notificationTimer = new QTimer(this);
    connect(notificationTimer, &QTimer::timeout, this, &MainWindow::showNotifications);
    notificationTimer->start(2000); // 1,000 ms = 2 Sekunde
}

MainWindow::~MainWindow()
{
    saveTasks();
    delete ui;
}

void MainWindow::setupUI()
{
    // Central widget und Layout
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

    // Task-Liste
    taskWidget = new QListWidget(this);
    taskWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    taskWidget->setDragEnabled(true);
    taskWidget->setAcceptDrops(true);
    taskWidget->setDropIndicatorShown(true);
    taskWidget->setDragDropMode(QAbstractItemView::InternalMove);
    taskWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(taskWidget->model(), &QAbstractItemModel::rowsMoved, this, &MainWindow::updateTaskOrder);
    connect(taskWidget, &QListWidget::customContextMenuRequested, this, &MainWindow::taskContextMenu);
    connect(taskWidget, &QListWidget::itemDoubleClicked, this, &MainWindow::taskDoubleClicked);
    mainLayout->addWidget(taskWidget);

    pointsLabel = new QLabel(this);
    pointsLabel->setText(QString("Gesamtpunkte: %1").arg(totalPoints));
    pointsLabel->setStyleSheet("font-size: 16px; color: #FFFFFF;");
    pointsLabel->setAlignment(Qt::AlignCenter);

    mainLayout->addWidget(pointsLabel);

    taskWidget->installEventFilter(this);

    // Buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout();

    QPushButton *addButton = new QPushButton("Aufgabe hinzufügen", this);
    QPushButton *removeButton = new QPushButton("Aufgabe entfernen", this);
    QPushButton *editButton = new QPushButton("Aufgabe bearbeiten", this);

    buttonLayout->addWidget(addButton);
    buttonLayout->addWidget(removeButton);
    buttonLayout->addWidget(editButton);

    mainLayout->addLayout(buttonLayout);

    // Signal-Slot-Verbindungen
    connect(addButton, &QPushButton::clicked, this, &MainWindow::addTask);
    connect(removeButton, &QPushButton::clicked, this, &MainWindow::removeTask);
    connect(editButton, &QPushButton::clicked, this, &MainWindow::editTask);
}

void MainWindow::loadStyleSheet()
{
    // Style für dunkles Theme mit blauen Akzenten
    QString style = R"(
        QListWidget {
            background-color: #1E1E1E;
            color: #FFFFFF;
            font-size: 16px;
        }
        QListWidget::item {
            padding: 10px;
            border-bottom: 1px solid #333333;
        }
        QListWidget::item:selected {
            background-color: #333333;
        }
        QListWidget::item:hover {
            background-color: #2A2A2A;
        }
        QPushButton {
            background-color: #007BFF;
            color: #FFFFFF;
            border: none;
            padding: 10px;
            margin: 5px;
            border-radius: 5px;
            font-size: 14px;
        }
        QPushButton:hover {
            background-color: #0056b3;
        }
        QPushButton:pressed {
            background-color: #00408d;
        }
    )";
    setStyleSheet(style);
}

void MainWindow::addTask()
{
    // Dialog, um eine neue Aufgabe hinzuzufügen
    bool ok;
    QString title = QInputDialog::getText(this, "Neue Aufgabe", "Titel:", QLineEdit::Normal, "", &ok);
    if (!ok || title.isEmpty()) return;

    QString description = QInputDialog::getMultiLineText(this, "Neue Aufgabe", "Beschreibung:");

    int priority = QInputDialog::getInt(this, "Priorität", "Prioritätsstufe (1-5):", 3, 1, 5);

    // Fälligkeitsdatum abfragen mit QDateTimeEdit
    QDialog dateDialog(this);
    QVBoxLayout *layout = new QVBoxLayout(&dateDialog);

    QLabel *label = new QLabel("Wählen Sie das Fälligkeitsdatum:", &dateDialog);
    layout->addWidget(label);

    QDateTimeEdit *dueDateEdit = new QDateTimeEdit(QDateTime::currentDateTime(), &dateDialog);
    dueDateEdit->setCalendarPopup(true);  // Kalender-Popup aktivieren
    layout->addWidget(dueDateEdit);

    QPushButton *okButton = new QPushButton("OK", &dateDialog);
    layout->addWidget(okButton);

    connect(okButton, &QPushButton::clicked, &dateDialog, &QDialog::accept);

    if (dateDialog.exec() == QDialog::Accepted) {
        QDateTime dueDate = dueDateEdit->dateTime();  // Das ausgewählte Fälligkeitsdatum holen

        Task task;
        task.title = title;
        task.description = description;
        task.priority = priority;
        task.completed = false;
        task.dueDate = dueDate;  // Fälligkeitsdatum setzen

        // Aufgabe zur Liste hinzufügen
        taskList.append(task);
        refreshTaskList();

        // Benachrichtigung anzeigen, wenn die Aufgabe in den nächsten 30 Minuten fällig ist
        QDateTime currentTime = QDateTime::currentDateTime();
        if (currentTime.secsTo(dueDate) <= 1800 && currentTime <= dueDate) {  // 30 Minuten Zeitfenster
            trayIcon->showMessage(
                "Aufgabe fällig",
                QString("Die Aufgabe '%1' ist in Kürze fällig!").arg(task.title),
                QSystemTrayIcon::Information,
                5000  // Benachrichtigung für 5 Sekunden anzeigen
                );
        }
    }
}

void MainWindow::removeTask()
{
    int row = taskWidget->currentRow();
    if (row >= 0 && row < taskList.size()) {
        taskList.removeAt(row);
        refreshTaskList();
    }
}

void MainWindow::editTask()
{
    int row = taskWidget->currentRow();
    if (row >= 0 && row < taskList.size()) {
        Task &task = taskList[row];

        bool ok;
        QString title = QInputDialog::getText(this, "Aufgabe bearbeiten", "Titel:", QLineEdit::Normal, task.title, &ok);
        if (!ok || title.isEmpty()) return;

        QString description = QInputDialog::getMultiLineText(this, "Aufgabe bearbeiten", "Beschreibung:", task.description);

        int priority = QInputDialog::getInt(this, "Priorität", "Prioritätsstufe (1-5):", task.priority, 1, 5);

        task.title = title;
        task.description = description;
        task.priority = priority;

        refreshTaskList();
    }
}

void MainWindow::updateTaskOrder()
{
    // Aktualisiere die Reihenfolge der Aufgaben basierend auf der Liste
    QList<Task> newTaskList;
    for (int i = 0; i < taskWidget->count(); ++i) {
        QWidget *widget = taskWidget->itemWidget(taskWidget->item(i));
        QLabel *titleLabel = widget->findChild<QLabel*>("titleLabel");

        QString title = titleLabel->text();
        for (const Task &task : taskList) {
            if (task.title == title) {
                newTaskList.append(task);
                break;
            }
        }
    }
    taskList = newTaskList;
}

void MainWindow::loadTasks()
{
    QFile loadFile("tasks.json");

    if (!loadFile.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(this, "Fehler", "Konnte 'tasks.json' nicht öffnen. Stellen Sie sicher, dass die Datei existiert.");
        totalPoints = 0; // Standardwert
        return;
    }

    QByteArray data = loadFile.readAll();
    QJsonDocument loadDoc(QJsonDocument::fromJson(data));
    QJsonObject rootObject = loadDoc.object();

    // Gesamtpunkte laden
    totalPoints = rootObject["totalPoints"].toInt();

    // Aufgaben laden
    QJsonArray jsonArray = rootObject["tasks"].toArray();
    jsonToTasks(jsonArray, taskList);
    refreshTaskList();

    // Punkteanzeige aktualisieren
    pointsLabel->setText(QString("Gesamtpunkte: %1").arg(totalPoints));
}

void MainWindow::saveTasks()
{
    QFile saveFile("tasks.json");

    if (!saveFile.open(QIODevice::WriteOnly)) {
        QMessageBox::critical(this, "Fehler", "Konnte 'tasks.json' nicht speichern.");
        return;
    }

    QJsonObject rootObject;
    rootObject["totalPoints"] = totalPoints;

    QJsonArray jsonArray = tasksToJson(taskList);
    rootObject["tasks"] = jsonArray;

    QJsonDocument saveDoc(rootObject);
    saveFile.write(saveDoc.toJson());
}


void MainWindow::refreshTaskList()
{
    taskWidget->clear();
    for (const Task &task : taskList) {
        QListWidgetItem *item = new QListWidgetItem();
        item->setSizeHint(QSize(0, 60)); // Passe die Höhe des Elements an
        taskWidget->addItem(item);
        taskWidget->setItemWidget(item, createTaskWidget(task));
    }
}

QWidget* MainWindow::createTaskWidget(const Task &task)
{
    QWidget *widget = new QWidget();

    // Titel-Label
    QLabel *titleLabel = new QLabel(task.title);
    titleLabel->setObjectName("titleLabel");
    titleLabel->setStyleSheet("font-weight: bold; font-size: 18px;");
    titleLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    // Prioritäts-Label
    QLabel *priorityLabel = new QLabel(QString("Priorität: %1").arg(task.priority));
    priorityLabel->setStyleSheet("font-size: 14px; color: #AAAAAA;");
    priorityLabel->setAlignment(Qt::AlignRight);
    priorityLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);

    // Fälligkeitsdatum-Label
    QLabel *dueDateLabel = new QLabel(task.dueDate.toString("dddd, dd MMM yyyy, HH:mm"));
    dueDateLabel->setStyleSheet("font-size: 14px; color: #007BFF;");
    dueDateLabel->setAlignment(Qt::AlignRight);
    dueDateLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);

    // Horizontales Layout für Titel, Priorität und Fälligkeitsdatum
    QHBoxLayout *topLayout = new QHBoxLayout();
    topLayout->addWidget(titleLabel);
    topLayout->addStretch(); // Adds space between titleLabel and priorityLabel
    topLayout->addWidget(priorityLabel);
    topLayout->addSpacing(10); // Adds space between priorityLabel and dueDateLabel
    topLayout->addWidget(dueDateLabel);

    // Beschreibung-Label
    QLabel *descriptionLabel = new QLabel(task.description);
    descriptionLabel->setStyleSheet("font-size: 12px; color: #CCCCCC;");
    descriptionLabel->setWordWrap(true); // Zeilenumbrüche ermöglichen

    // Vertikales Hauptlayout
    QVBoxLayout *mainLayout = new QVBoxLayout(widget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    mainLayout->addLayout(topLayout);
    mainLayout->addWidget(descriptionLabel);

    // Anpassung bei abgeschlossenen Aufgaben
    if (task.completed) {
        titleLabel->setStyleSheet("font-weight: bold; font-size: 18px; text-decoration: line-through; color: #777777;");
        descriptionLabel->setStyleSheet("font-size: 14px; color: #777777; text-decoration: line-through;");
        priorityLabel->setStyleSheet("font-size: 14px; color: #777777;");
        dueDateLabel->setStyleSheet("font-size: 14px; color: #777777; text-decoration: line-through;");
    }

    return widget;
}



QJsonArray MainWindow::tasksToJson(const QList<Task> &tasks)
{
    QJsonArray jsonArray;
    for (const Task &task : tasks) {
        QJsonObject taskObject;
        taskObject["title"] = task.title;
        taskObject["description"] = task.description;
        taskObject["priority"] = task.priority;
        taskObject["completed"] = task.completed;
        taskObject["dueDate"] = task.dueDate.toString(Qt::ISODate);
        taskObject["category"] = task.category;
        taskObject["tags"] = QJsonArray::fromStringList(task.tags);
        // Rekursives Hinzufügen von Unteraufgaben
        taskObject["subtasks"] = tasksToJson(task.subtasks);
        jsonArray.append(taskObject);
    }
    return jsonArray;
}

void MainWindow::jsonToTasks(const QJsonArray &jsonArray, QList<Task> &tasks)
{
    for (const QJsonValue &value : jsonArray) {
        QJsonObject taskObject = value.toObject();
        Task task;
        task.title = taskObject["title"].toString();
        task.description = taskObject["description"].toString();
        task.priority = taskObject["priority"].toInt();
        task.completed = taskObject["completed"].toBool();
        task.dueDate = QDateTime::fromString(taskObject["dueDate"].toString(), Qt::ISODate);
        task.category = taskObject["category"].toString();
        QJsonArray tagsArray = taskObject["tags"].toArray();
        for (const QJsonValue &tagValue : tagsArray) {
            task.tags.append(tagValue.toString());
        }
        // Rekursives Laden von Unteraufgaben
        QJsonArray subtasksArray = taskObject["subtasks"].toArray();
        jsonToTasks(subtasksArray, task.subtasks);
        tasks.append(task);
    }
}

void MainWindow::showNotifications()
{
    QDateTime currentTime = QDateTime::currentDateTime();

    // Überprüfen Sie alle Aufgaben auf fällige Aufgaben
    for (const Task &task : taskList) {
        // Wenn die Aufgabe nicht abgeschlossen ist und sie fällig ist (gleiche Zeit)
        if (!task.completed && currentTime.secsTo(task.dueDate) == 0) {
            trayIcon->showMessage(
                "Aufgabe fällig",
                QString("Die Aufgabe '%1' ist fällig!").arg(task.title),
                QSystemTrayIcon::Information,
                5000  // Benachrichtigung für 5 Sekunden anzeigen
                );
        }
    }
}



void MainWindow::taskContextMenu(const QPoint &pos)
{
    QListWidgetItem *item = taskWidget->itemAt(pos);
    if (item) {
        QMenu contextMenu;
        QAction *completeAction = new QAction("Abschließen", &contextMenu);
        QAction *deleteAction = new QAction("Löschen", &contextMenu);

        connect(completeAction, &QAction::triggered, this, &MainWindow::markTaskCompleted);
        connect(deleteAction, &QAction::triggered, this, &MainWindow::deleteTask);

        contextMenu.addAction(completeAction);
        contextMenu.addAction(deleteAction);

        contextMenu.exec(taskWidget->mapToGlobal(pos));
    }
}

// ----------------------------------------------------------------------------
// ACHIEVEMENT SYSTEM & PUNKTEVERGABE
// ----------------------------------------------------------------------------

void MainWindow::markTaskCompleted()
{
    int row = taskWidget->currentRow();
    if (row >= 0 && row < taskList.size()) {
        // Punkte nur vergeben, wenn die Aufgabe noch nicht abgeschlossen war
        if (!taskList[row].completed) {
            taskList[row].completed = true;
            int pointsToAdd = taskList[row].priority * 10; // Prio 1 = 10 Punkte, 2 = 20, …, 5 = 50
            totalPoints += pointsToAdd;
            pointsLabel->setText(QString("Gesamtpunkte: %1").arg(totalPoints));

            // Zeige eine Nachricht über das erhaltene Punkte-Bonus
            trayIcon->show();
            trayIcon->showMessage("Aufgabe abgeschlossen",
                                  QString("Du hast %1 Punkte erhalten!").arg(pointsToAdd),
                                  QSystemTrayIcon::Information, 10000);

            // Prüfe, ob ein Achievement freigeschaltet wurde
            checkAchievements();
        }
        refreshTaskList();
    }
}

void MainWindow::deleteTask()
{
    removeTask();
}

void MainWindow::taskDoubleClicked(QListWidgetItem *item)
{
    editTask();
}

// Prüft, ob basierend auf den Gesamtpunkten ein neues Achievement freigeschaltet wird.
void MainWindow::checkAchievements()
{
    // Definiere einige Achievements mit entsprechenden Punkteschwellen
    QList<QPair<int, QString>> achievements = {
        {50,   "Neuling: 50 Punkte erreicht!"},
        {100,  "Fortgeschritten: 100 Punkte erreicht!"},
        {250,  "Erfahren: 250 Punkte erreicht!"},
        {500,  "Profi: 500 Punkte erreicht!"},
        {1000, "Meister: 1000 Punkte erreicht!"}
    };

    for (const auto &pair : achievements) {
        int threshold = pair.first;
        QString achievementMsg = pair.second;
        // Prüfen, ob das Achievement noch nicht freigeschaltet wurde und die Schwelle erreicht ist
        if (totalPoints >= threshold && !unlockedAchievements.contains(achievementMsg)) {
            unlockedAchievements.insert(achievementMsg);
            trayIcon->show();
            trayIcon->showMessage("Neues Achievement freigeschaltet!",
                                  achievementMsg,
                                  QSystemTrayIcon::Information, 10000);
        }
    }
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == taskWidget && event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        if (taskWidget->childAt(mouseEvent->pos()) == nullptr) {
            // Wenn der Klick außerhalb der taskWidget war, Auswahl entfernen
            taskWidget->clearSelection();
            return true;  // Event wurde behandelt
        }
    }

    return QMainWindow::eventFilter(watched, event); // Standardverhalten beibehalten
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    // Überprüfe, ob der Klick außerhalb des taskWidget-Bereichs liegt.
    // event->pos() ist relativ zum Hauptfenster (MainWindow)
    if (!taskWidget->geometry().contains(event->pos())) {
        taskWidget->clearSelection();
    }

    // Rufe die Basisklassenimplementierung auf, damit andere Events weiterhin verarbeitet werden.
    QMainWindow::mousePressEvent(event);
}


