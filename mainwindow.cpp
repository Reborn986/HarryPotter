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
    , ui(new Ui::MainWindow)//初始化UI
{
    ui->setupUi(this);
    loadAllBooks();
    connect(ui->searchButton, &QPushButton::clicked, this, &MainWindow::on_searchButton_clicked);//把信号和槽连接起来，有点类似AndroidStudio的intent跳转
    connect(ui->resultsList, &QListWidget::itemClicked, this, &MainWindow::on_resultsList_itemClicked);
}
MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::loadAllBooks(){
    // Load books from resource files instead of directory
    QStringList bookResources = {
        ":/lushuyun_hp0_prequel.txt",
        ":/lushuyun_hp2_chamber_secrets.txt",
        ":/lushuyun_hp3_prisoner_azkaban.txt",
        ":/lushuyun_hp4_goblet_fire.txt",
        ":/lushuyun_hp6_half_blood_prince.txt",
        ":/lushuyun_hp7_deathly_hallows.txt",
        ":/lushuyun_quidditch_ages.txt",
        ":/lushuyun_tales_beedle_bard.txt"
    };
    
    foreach (QString resourcePath, bookResources) {
        QFile file(resourcePath);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream stream(&file);
            stream.setEncoding(QStringConverter::Utf8);
            QString content = stream.readAll();
            QString bookName = resourcePath.mid(2); // Remove ":/" prefix
            bookMap[bookName] = content;
            file.close();
        }
    }
}

void MainWindow::on_searchButton_clicked()
{
    QString keyword = ui->searchInput->text().trimmed();
    if (keyword.isEmpty()) {
        return;
    }
    
    searchResults.clear();
    ui->resultsList->clear();
    
    // Search for the keyword in all books
    QRegularExpression regex("\\b" + QRegularExpression::escape(keyword) + "\\b", 
                            QRegularExpression::CaseInsensitiveOption);
    
    int resultIndex = 1;
    for (auto it = bookMap.begin(); it != bookMap.end(); ++it) {
        QString bookName = it.key();
        QString content = it.value();
        
        QRegularExpressionMatchIterator matches = regex.globalMatch(content);
        while (matches.hasNext()) {
            QRegularExpressionMatch match = matches.next();
            int position = match.capturedStart();
            
            SearchResult result;
            result.bookName = bookName;
            result.keyword = keyword;
            result.position = position;
            result.page = findPage(content, position);
            result.chapter = findChapter(content, position);
            result.context = getContext(content, position);
            
            searchResults.append(result);
            
            // Add to list widget
            QString displayText = QString("%1\t%2\t%3\t%4\t%5")
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

void MainWindow::on_resultsList_itemClicked(QListWidgetItem *item)
{
    int index = ui->resultsList->row(item);
    if (index >= 0 && index < searchResults.size()) {
        SearchResult result = searchResults.at(index);
        ui->contextDisplay->setText(result.context);
    }
}

int MainWindow::findChapter(const QString &content, int position)
{
    // Simple chapter detection - look for "Chapter" before the position
    QRegularExpression chapterRegex("Chapter\\s+(\\d+)", QRegularExpression::CaseInsensitiveOption);
    int chapter = 1; // Default chapter
    
    int searchPos = 0;
    QRegularExpressionMatchIterator matches = chapterRegex.globalMatch(content);
    while (matches.hasNext()) {
        QRegularExpressionMatch match = matches.next();
        int matchPos = match.capturedStart();
        
        if (matchPos > position) {
            break;
        }
        
        chapter = match.captured(1).toInt();
    }
    
    return chapter;
}

int MainWindow::findPage(const QString &content, int position)
{
    // Approximate page calculation (assuming ~3000 characters per page)
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
