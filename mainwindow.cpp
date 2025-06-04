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
    QMap<QString, QString> new_lsy_name;
    new_lsy_name.insert(":/lushuyun_hp0_prequel.txt", "Harry Potter Prequel.txt");
    new_lsy_name.insert(":/lushuyun_hp2_chamber_secrets.txt", "Harry_Potter_and_the_Chamber_of_Secrets_Book_2");
    new_lsy_name.insert(":/lushuyun_hp7_deathly_hallows.txt","Harry_Potter_and_the_Deathly_Hallows_Book_7");
    new_lsy_name.insert(":/lushuyun_hp3_prisoner_azkaban.txt","Harry Potter and the Prisoner of Azkaban");
    new_lsy_name.insert(":/lushuyun_hp4_goblet_fire.txt","Harry Potter and the Goblet of Fire");
    new_lsy_name.insert(":/lushuyun_hp6_half_blood_prince.txt","Harry Potter and the Half-Blood Prince");
    new_lsy_name.insert(":/lushuyun_quidditch_ages.txt","Quidditch Through the Ages");
    new_lsy_name.insert(":/lushuyun_tales_beedle_bard.txt","The Tales of Beedle the Bard");
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
            QString bookName = new_lsy_name.value(resourcePath);
            bookMap[bookName] = content;//把书名和内容作为键-对的形式存在哈希表中
            file.close();//关闭文件
        }
    }
}
QList<int> MainWindow::boyerMooreSearch(const QString &text, const QString &pattern, bool case_lsy)// 实现高效的Boyer-Moore算法进行字符串搜索
{
    QList<int> matches;//首先，存储匹配位置的列表
    if (pattern.isEmpty() || text.isEmpty()) {//为了防止崩溃，先处理空字符串情况
        return matches;//如果模式或文本为空，直接返回空列表
    }
    QString searchText = case_lsy ? text : text.toLower();//根据是否区分大小写处理文本和模式
    QString searchPattern = case_lsy ? pattern : pattern.toLower();
    int textLen = searchText.length();//文本长度
    int patternLen = searchPattern.length();//模式长度
    if (patternLen > textLen) {//如果模式比文本长，不可能有匹配
        return matches;
    }
    QHash<QChar, int> bad_Table;//构建坏字符表，这个表记录每个字符在模式中最后出现的位置
    for (int i = 0; i < patternLen; i++) {
        bad_Table[searchPattern[i]] = i;
    }
    int shift = 0;//当前在文本中的对齐位置
    while (shift <= textLen - patternLen) {//主搜索循环
        int j = patternLen - 1;//从模式末尾开始比较
        while (j >= 0 && shift + j < textLen && searchPattern[j] == searchText[shift + j]) {//从右向左匹配
            j--;//字符匹配，继续比较前一个字符
        }
        if (j < 0) {
            matches.append(shift);//找到完全匹配
            shift += 1;//简单移动一位继续搜索
        } else {//不匹配时使用坏字符启发式，计算移动距离
            int bad_shift = 1;//默认移动1位
            if (shift + j < textLen) {//确保不匹配字符在文本范围内
                QChar mismatchChar = searchText[shift + j];//不匹配的字符
                int lastOccurrence = bad_Table.value(mismatchChar, -1);
                bad_shift = qMax(1, j - lastOccurrence);//计算移动距离，将模式中该字符的最后出现位置对齐到当前不匹配位置，确保至少移动1位
            }
            shift += bad_shift;//应用移动距离
        }
    }
    return matches;//返回所有匹配位置
}
void MainWindow::on_searchButton_clicked()
{
    QString keyword = ui->searchInput->text().trimmed();
    if (keyword.isEmpty()) {
        return;
    }
    searchResults.clear();
    ui->resultsList->clear();
    //创建一个有序的书籍列表，按照哈利波特系列的正确顺序
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
        QList<int> exactMatches = boyerMooreSearch(content, keyword, false);//搜索精确匹配
        QString possessiveKeyword = keyword + "'s";//搜索带所有格的匹配 (如 Harry's)
        QList<int> possessiveMatches = boyerMooreSearch(content, possessiveKeyword, false);
        QString smartPossessiveKeyword = keyword + "'s";//搜索带右单引号的所有格 (如 Harry's - 智能引号)
        QList<int> smartpossessiveMatches = boyerMooreSearch(content, smartPossessiveKeyword, false);
        QSet<int> allMatches;//合并所有匹配结果
        for (int pos : exactMatches) {
            allMatches.insert(pos);
        }
        for (int pos : possessiveMatches) {
            allMatches.insert(pos);
        }
        for (int pos : smartpossessiveMatches) {
            allMatches.insert(pos);
        }
        QList<int> sortedMatches = allMatches.values();//将匹配位置转换为有序列表
        std::sort(sortedMatches.begin(), sortedMatches.end());
        for (int position : sortedMatches) {//为每个匹配创建搜索结果
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
    if (index > 0 && (index - 1) < searchResults.size()) {//跳过标题行
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
QString MainWindow::getContext(const QString &content, int position, int contextSize)//lsy是路舒匀的缩写，代码防伪标识
{
    int lsy_paragraphStart = content.lastIndexOf("\n", position);//找到包含关键词的段落起始位置
    if (lsy_paragraphStart == -1)
        lsy_paragraphStart = 0;
    else
        lsy_paragraphStart += 1;//跳过换行符
    int lsy_paragraphEnd = content.indexOf("\n", position);//找到段落结束位置
    if (lsy_paragraphEnd == -1)
        lsy_paragraphEnd = content.length();
    QString lsy_context = content.mid(lsy_paragraphStart, lsy_paragraphEnd - lsy_paragraphStart);//提取包含关键词的段落
    return lsy_context;
}
