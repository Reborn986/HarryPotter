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

// 实现简化版Boyer-Moore算法进行字符串搜索
QList<int> MainWindow::boyerMooreSearch(const QString &text, const QString &pattern, bool caseSensitive)
{
    QList<int> matches;
    if (pattern.isEmpty() || text.isEmpty()) {
        return matches;
    }

    QString searchText = caseSensitive ? text : text.toLower();
    QString searchPattern = caseSensitive ? pattern : pattern.toLower();

    int textLen = searchText.length();
    int patternLen = searchPattern.length();

    if (patternLen > textLen) {
        return matches;
    }

    // 使用简单的暴力搜索算法（更可靠）
    for (int i = 0; i <= textLen - patternLen; i++) {
        bool found = true;
        for (int j = 0; j < patternLen; j++) {
            if (searchText[i + j] != searchPattern[j]) {
                found = false;
                break;
            }
        }
        if (found) {
            matches.append(i);
        }
    }

    return matches;
}

void MainWindow::on_searchButton_clicked()
{
    QString keyword = ui->searchInput->text().trimmed();
    if (keyword.isEmpty()) {
        return;
    }

    searchResults.clear();
    ui->resultsList->clear();

    // 创建一个有序的书籍列表，按照哈利波特系列的正确顺序
    QStringList orderedBooks = {
        "Harry Potter Prequel.txt",                          // 前传 (HP0)
        "Harry_Potter_and_the_Chamber_of_Secrets_Book_2",    // 密室 (HP2)
        "Harry Potter and the Prisoner of Azkaban",          // 阿兹卡班 (HP3)
        "Harry Potter and the Goblet of Fire",               // 火焰杯 (HP4)
        "Harry Potter and the Half-Blood Prince",            // 混血王子 (HP6)
        "Harry_Potter_and_the_Deathly_Hallows_Book_7",       // 死亡圣器 (HP7)
        "Quidditch Through the Ages",                        // 衍生作品
        "The Tales of Beedle the Bard"                       // 衍生作品
    };

    ui->resultsList->addItem(QString("序号    人名/地名              页码         章节         书名"));  // 添加标题行

    int resultIndex = 1;

    for (const QString &bookName : orderedBooks) {
        if (!bookMap.contains(bookName)) continue;
        QString content = bookMap[bookName];

        // 搜索精确匹配
        QList<int> exactMatches = boyerMooreSearch(content, keyword, false);

        // 搜索带所有格的匹配 (如 Harry's)
        QString possessiveKeyword = keyword + "'s";
        QList<int> possessiveMatches = boyerMooreSearch(content, possessiveKeyword, false);

        // 搜索带右单引号的所有格 (如 Harry's - 智能引号)
        QString smartPossessiveKeyword = keyword + "'s";
        QList<int> smartPossessiveMatches = boyerMooreSearch(content, smartPossessiveKeyword, false);

        // 合并所有匹配结果
        QSet<int> allMatches;
        for (int pos : exactMatches) {
            allMatches.insert(pos);
        }
        for (int pos : possessiveMatches) {
            allMatches.insert(pos);
        }
        for (int pos : smartPossessiveMatches) {
            allMatches.insert(pos);
        }

        // 将匹配位置转换为有序列表
        QList<int> sortedMatches = allMatches.values();
        std::sort(sortedMatches.begin(), sortedMatches.end());

        // 为每个匹配创建搜索结果
        for (int position : sortedMatches) {
            SearchResult result;
            result.bookName = bookName;
            result.keyword = keyword;
            result.position = position;
            result.page = findPage(content, position);
            result.chapter = findChapter(content, position);
            result.context = getContext(content, position);
            searchResults.append(result);

            QString displayText = QString("%1    %2              %3         %4         %5")
                                      .arg(resultIndex)
                                      .arg(keyword)
                                      .arg(result.page)
                                      .arg(result.chapter)
                                      .arg(bookName);
            ui->resultsList->addItem(displayText);
            resultIndex++;
        }
    }
}

void MainWindow::on_resultsList_itemClicked(QListWidgetItem *item)//显示结果
{
    int index = ui->resultsList->row(item);
    if (index > 0 && (index - 1) < searchResults.size()) { // 跳过标题行
        SearchResult result = searchResults.at(index - 1);
        ui->contextDisplay->setText(result.context);
    }
}

int MainWindow::findChapter(const QString &content, int position)
{
    QRegularExpression chapterRegex("Chapter\\s+(\\d+)", QRegularExpression::CaseInsensitiveOption);//初始化正则表达式，和上面统一；匹配模式：Chapter + 1个或多个空白字符（\\s+） + 数字（(\\d+)），并且case这个方法是忽略大小写的意思
    int chapter = 1; //默认值（没匹配到任何章节的时候就返回这个）
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
    // 找到包含关键词的段落起始位置
    int lsy_paragraphStart = content.lastIndexOf("\n", position);
    if (lsy_paragraphStart == -1)
        lsy_paragraphStart = 0;
    else
        lsy_paragraphStart += 1; // 跳过换行符

    // 找到段落结束位置
    int lsy_paragraphEnd = content.indexOf("\n", position);
    if (lsy_paragraphEnd == -1)
        lsy_paragraphEnd = content.length();

    // 提取包含关键词的段落
    QString lsy_context = content.mid(lsy_paragraphStart, lsy_paragraphEnd - lsy_paragraphStart);

    return lsy_context;
}
