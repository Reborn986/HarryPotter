#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QStringConverter>
#include <QCoreApplication>
#include <QRegularExpression>
#include <QDebug>
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    loadAllBooks();
    this->setWindowTitle("C++程序设计实践&路舒匀&哈利波特书籍搜索");// 设置窗口标题
    this->setStyleSheet("MainWindow { background-image: url(:/background.jpg); background-position: center; background-repeat: no-repeat; background-size: cover; }");//设置背景图片

    QString transparentStyle = "background-color: rgba(255, 255, 255, 150); border-radius: 10px; padding: 5px;";// 设置半透明效果的搜索区域
    ui->searchInput->setStyleSheet("QLineEdit { " + transparentStyle + " font-size: 14px; color: #333; }");
    ui->searchButton->setStyleSheet("QPushButton { background-color: rgba(142, 68, 173, 200); color: white; border-radius: 15px; font-weight: bold; padding: 8px; }"
                                   "QPushButton:hover { background-color: rgba(155, 89, 182, 200); }"
                                   "QPushButton:pressed { background-color: rgba(125, 60, 152, 200); }");//把按钮的颜色设一下
    ui->resultsList->setStyleSheet("QListWidget { " + transparentStyle + " }");
    ui->contextDisplay->setStyleSheet("QTextEdit { " + transparentStyle + " }");
    ui->searchButton->setText("搜索"); // 修改按钮文本
    connect(ui->searchButton, &QPushButton::clicked, this, &MainWindow::on_searchButton_clicked);
    connect(ui->resultsList, &QListWidget::itemClicked, this, &MainWindow::on_resultsList_itemClicked);//信号槽的连接
}
MainWindow::~MainWindow()
{
    delete ui;
}
void MainWindow::loadAllBooks(){
    QMap<QString, QString> aliasToOriginalFilenameMap;
    aliasToOriginalFilenameMap.insert(":/lushuyun_hp0_prequel.txt", "Harry Potter Prequel.txt");
    aliasToOriginalFilenameMap.insert(":/lushuyun_hp2_chamber_secrets.txt", "Harry_Potter_and_the_Chamber_of_Secrets_Book_2");
    aliasToOriginalFilenameMap.insert(":/lushuyun_hp7_deathly_hallows.txt","Harry_Potter_and_the_Deathly_Hallows_Book_7");
    aliasToOriginalFilenameMap.insert(":/lushuyun_hp3_prisoner_azkaban.txt","Harry Potter and the Prisoner of Azkaban");
    aliasToOriginalFilenameMap.insert(":/lushuyun_hp4_goblet_fire.txt","Harry Potter and the Goblet of Fire");
    aliasToOriginalFilenameMap.insert(":/lushuyun_hp6_half_blood_prince.txt","Harry Potter and the Half-Blood Prince");
    aliasToOriginalFilenameMap.insert(":/lushuyun_quidditch_ages.txt","Quidditch Through the Ages");
    aliasToOriginalFilenameMap.insert(":/lushuyun_tales_beedle_bard.txt","The Tales of Beedle the Bard");

    QStringList bookResources = {//路径以:/开头是Qt资源的写法，这里用的就是我在res里面写的别名了
        ":/lushuyun_hp0_prequel.txt",
        ":/lushuyun_hp2_chamber_secrets.txt",
        ":/lushuyun_hp3_prisoner_azkaban.txt",
        ":/lushuyun_hp4_goblet_fire.txt",
        ":/lushuyun_hp6_half_blood_prince.txt",
        ":/lushuyun_hp7_deathly_hallows.txt",
        ":/lushuyun_quidditch_ages.txt",
        ":/lushuyun_tales_beedle_bard.txt"
    };//创建一个字符串列表，和python的列表类似但是写法不一样
    for (QString resourcePath: bookResources) {//for遍历资源路径
        QFile file(resourcePath);//为每一个路径创建一个QFile对象
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {//文件用只读的方式打开
            QTextStream stream(&file);//创建QTextStream对象，和文件关联
            stream.setEncoding(QStringConverter::Utf8);//为了防止乱码，设置编码为UTF-8，正确处理国际字符
            QString content = stream.readAll();//用readALL()读取整个文件内容到字符串
            QString bookName = aliasToOriginalFilenameMap.value(resourcePath);
            bookMap[bookName] = content;//把书名和内容作为键-对的形式存在哈希表中
            file.close();//关闭文件
        }
    }
}
void MainWindow::on_searchButton_clicked()
{
    QString keyword = ui->searchInput->text().trimmed();
    if (keyword.isEmpty()) {
        return;
    }//获取、然后清理用户输入的关键词
    searchResults.clear();
    ui->resultsList->clear();//清空之前的搜索结果
    QRegularExpression regex("\\b" + QRegularExpression::escape(keyword) + "\\b", //创建正则表达式，这来用于搜索完整单词匹配
                            QRegularExpression::CaseInsensitiveOption);
    int resultIndex = 1;
    for (auto it = bookMap.begin(); it != bookMap.end(); ++it) {//遍历加载所有的书籍
        QString bookName = it.key();
        QString content = it.value();
        QRegularExpressionMatchIterator matches = regex.globalMatch(content);//在当前这一本书籍下查找所有的匹配项
        while (matches.hasNext()) {
            QRegularExpressionMatch match = matches.next();
            int position = match.capturedStart(); 
            SearchResult result;//为每一个匹配都创建一个结果对象
            result.bookName = bookName;//书的名称
            result.keyword = keyword;//关键词
            result.position = position;//位置
            result.page = findPage(content, position);//哪一页
            result.chapter = findChapter(content, position);//哪一章
            result.context = getContext(content, position);//内容是啥
            searchResults.append(result);//把这些结果添加到列表里面
            QString displayText = QString("%1\t%2\t%3\t%4\t%5")//在UI里面显示结果
                                .arg(resultIndex)
                                .arg(keyword)
                                .arg(result.page)
                                .arg(result.chapter)
                                .arg(bookName);
            ui->resultsList->addItem(displayText);
            resultIndex++;//开始下一本书的搜索了
        }
    }
}
void MainWindow::on_resultsList_itemClicked(QListWidgetItem *item)//显示结果
{
    int index = ui->resultsList->row(item);
    if (index >= 0 && index < searchResults.size()) {
        SearchResult result = searchResults.at(index);
        ui->contextDisplay->setText(result.context);
    }
}
int MainWindow::findChapter(const QString &content, int position)
{
    QRegularExpression chapterRegex("Chapter\\s+(\\d+)", QRegularExpression::CaseInsensitiveOption);//初始化正则表达式，和上面统一；匹配模式：Chapter + 1个或多个空白字符（\\s+） + 数字（(\\d+)），并且case这个方法是忽略大小写的意思
    int chapter = 1; //默认值（没匹配到任何章节的时候就返回这个）
    int searchPos = 0;
    QRegularExpressionMatchIterator matches = chapterRegex.globalMatch(content);//globalMatch是遍历文本正则表达式用的
    while (matches.hasNext()) {
        QRegularExpressionMatch match = matches.next();
        int matchPos = match.capturedStart();//匹配起始的位置
        if (matchPos > position) {
            break;//要是匹配的位置超过目标位置匹配就终止
        }
        chapter = match.captured(1).toInt();//更新匹配的数字
    }
    return chapter;
}
int MainWindow::findPage(const QString &content, int position)
{
    const int CHARS_PER_PAGE = 3000;
    return (position / CHARS_PER_PAGE) + 1;
}
QString MainWindow::getContext(const QString &content, int position, int contextSize)
{
    // Get text surrounding the keyword
    int startPos = qMax(0, position - contextSize);
    int endPos = qMin(content.length(), position + contextSize);
    
    // Find paragraph boundaries
    int paragraphStart = content.lastIndexOf("\n\n", position);
    if (paragraphStart == -1) paragraphStart = content.lastIndexOf("\n", position);
    if (paragraphStart == -1) paragraphStart = qMax(0, position - contextSize);
    
    int paragraphEnd = content.indexOf("\n\n", position);
    if (paragraphEnd == -1) paragraphEnd = content.indexOf("\n", position);
    if (paragraphEnd == -1) paragraphEnd = qMin(content.length(), position + contextSize);
    
    // Include previous and next paragraphs
    int prevParagraphStart = content.lastIndexOf("\n\n", paragraphStart - 1);
    if (prevParagraphStart == -1) prevParagraphStart = qMax(0, paragraphStart - contextSize);
    
    int nextParagraphEnd = content.indexOf("\n\n", paragraphEnd + 1);
    if (nextParagraphEnd == -1) nextParagraphEnd = qMin(content.length(), paragraphEnd + contextSize);
    
    return content.mid(prevParagraphStart, nextParagraphEnd - prevParagraphStart);
}
