#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow> //在qt里面开发时，为了和一般c++库区分，方便编译或者调用信号与槽的功能，所以要加Q
#include <QMap>//在存储书籍的时候用到了这个，实现存储和管理的键对映射
#include <QString>
#include <QListWidgetItem>//和信号槽有关的，点击链表的某一选项时，知道是啥被点击了
#include <QHash>
#include <QSet>
#include <QVector>
#include <algorithm>
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
    MainWindow(QWidget *parent = nullptr);//构造函数，初始化UI，加载书籍，连接信号与槽
    ~MainWindow();
    void loadAllBooks();
private slots:
    void on_searchButton_clicked();
    void on_resultsList_itemClicked(QListWidgetItem *item);
private:
    Ui::MainWindow *ui;
    QMap<QString, QString> bookMap;
    QList<SearchResult> searchResults;
    QList<int> boyerMooreSearch(const QString &text, const QString &pattern, bool case_lsy = true); // Boyer-Moore搜索函数
    int findChapter(const QString &content, int position);
    int findPage(const QString &content, int position);
    QString getContext(const QString &content, int position, int contextSize = 200);
};
#endif
