#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidget>
#include <QJsonArray>
#include <QJsonObject>
#include <QTimer>
#include <QMenu>
#include <QSystemTrayIcon>
#include <QLabel>
#include <QStringList>
#include <QVector>
#include <QEvent>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

struct Task {
    QString title;
    QString description;
    int priority;
    QString category;
    QStringList tags;
    bool completed;
    QVector<Task> subtasks;  // Verwendung von QVector statt QList für Effizienz
    QDateTime dueDate;  // Fälligkeitsdatum hinzufügen
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void loadTasks();
    void saveTasks();

private slots:
    void addTask();
    void removeTask();
    void editTask();
    void updateTaskOrder();
    void showNotifications();
    void taskContextMenu(const QPoint &pos);
    void markTaskCompleted();
    void deleteTask();
    void taskDoubleClicked(QListWidgetItem *item);

private:
    Ui::MainWindow *ui;
    QVector<Task> taskList;  // Auch hier QVector für bessere Performance
    QListWidget *taskWidget;

    QTimer *notificationTimer;
    QSystemTrayIcon *trayIcon;
    int totalPoints;
    QLabel *pointsLabel;

    void setupUI();
    void checkAchievements();
    QSet<QString> unlockedAchievements;
    void loadStyleSheet();
    void refreshTaskList();
    QWidget* createTaskWidget(const Task &task);
    QJsonArray tasksToJson(const QVector<Task> &tasks);
    void jsonToTasks(const QJsonArray &jsonArray, QVector<Task> &tasks);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

};

#endif // MAINWINDOW_H
