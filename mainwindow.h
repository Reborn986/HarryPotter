#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMap>
#include <QString>
#include <QListWidgetItem>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

struct SearchResult {
    QString bookName;
    QString keyword;
    int page;
    int chapter;
    int position;
    QString context;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void loadAllBooks();

private slots:
    void on_searchButton_clicked();
    void on_resultsList_itemClicked(QListWidgetItem *item);

private:
    Ui::MainWindow *ui;
    QMap<QString, QString> bookMap;
    QList<SearchResult> searchResults;
    
    int findChapter(const QString &content, int position);
    int findPage(const QString &content, int position);
    QString getContext(const QString &content, int position, int contextSize = 200);
};
#endif // MAINWINDOW_H
