#include "include/mainwindow.h"
#include "ui_mainwindow.h"
#include <QPushButton>
#include <QFrame>
#include <QPlainTextEdit>
#include <QFileDialog>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QLabel>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
  ui->setupUi(this);

  this->setWindowTitle("Matrix");

  tabsWidget->setMovable(true);
  tabsWidget->setTabsClosable(true);
  createTab();

  treeView->setMaximumWidth(0);
  treeView->setMidLineWidth(0);

  this->setCentralWidget(splitter);
  splitter->addWidget(treeView);
  splitter->addWidget(tabsWidget);

  this->connect(tabsWidget,SIGNAL(tabCloseRequested(int)),this,SLOT(closeTab(int)));
  this->connect(treeView,SIGNAL(clicked(QModelIndex)),this,SLOT(openTreeViewFile(QModelIndex)));

  QFontDatabase::addApplicationFont(":/res/fonts/heartland-script-maisfontes.f796/heartland-script.otf");
  QFontDatabase::addApplicationFont(":/res/fonts/voire-regular-maisfontes.9114/voire-regular.ttf");


  tabsWidget->show();
}

MainWindow::~MainWindow()
{
    delete ui;
}



void MainWindow::on_actionNew_File_triggered()
{
    createTab();
}

void MainWindow::createTab(){
    QFrame *tabFrame = new QFrame(this);
    QVBoxLayout *tabLayout = new QVBoxLayout(tabFrame);
    QPlainTextEdit *textEdit = new QPlainTextEdit();

    QFont font = textEdit->document()->defaultFont();
//    font.setFamily("Voire");
    textEdit->setFont(font);

    textEdit->setTabStopDistance(QFontMetrics(textEdit->font()).horizontalAdvance(' ')*4);

    textEdit->setObjectName("Text Edit");

    tabsWidget->setTabToolTip(tabsWidget->currentIndex(),"Untitled");

    this->connect(textEdit,SIGNAL(textChanged()),this,SLOT(textEditChanged()));

    tabLayout->addWidget(textEdit);

    QLabel *status = new QLabel(this);

    status->setText("Line 1, Column 1");
    status->setObjectName("status");
    tabLayout->addWidget(status);

    this->connect(textEdit,SIGNAL(cursorPositionChanged()),this,SLOT(updateStatus()));

    int index = tabsWidget->addTab(tabFrame,"Untitled");
    tabsWidget->setCurrentIndex(index);



}


void MainWindow::on_actionClose_File_triggered()
{
    closeTab(tabsWidget->currentIndex());
}

void MainWindow::closeTab(int index)
{
    tabsWidget->removeTab(index);
}

void MainWindow::on_actionOpen_File_triggered()
{
    QString filePath = QFileDialog::getOpenFileName(this,"Open file");

    openTabFile(filePath);

}

void MainWindow::openTabFile(QString filepath)
{
    QFile file(filepath);
    QFileInfo fileInfo(filepath);

    if(!file.open(QIODevice::ReadOnly|QFile::Text)){
        QMessageBox::warning(this,"Warning","Cannot open file : " + file.errorString());
        return;
    }
    createTab();
    tabsWidget->setTabToolTip(tabsWidget->currentIndex(),filepath);

    QTextStream in(&file);
    QString text = in.readAll();

    currentTextEdit()->setPlainText(text);
    file.close();
    tabsWidget->setTabText(tabsWidget->currentIndex(),fileInfo.fileName());
}

QPlainTextEdit* MainWindow::currentTextEdit()
{
    QList<QPlainTextEdit *> fileEditList = tabsWidget->findChildren<QPlainTextEdit *>("Text Edit");

    for(int i=0; i<fileEditList.count();i++)
    {
        if(tabsWidget->indexOf(fileEditList.at(i)->parentWidget()) == tabsWidget->currentIndex())
        {
            return fileEditList.at(i);
        }
    }

    return new QPlainTextEdit;
}

void MainWindow::textEditChanged()
{
    QString tabname = tabsWidget->tabText(tabsWidget->currentIndex());

    if(tabname.at(0)!='*')
    {
        tabsWidget->setTabText(tabsWidget->currentIndex(),"*"+tabname);
    }
}

void MainWindow::on_actionOpen_Folder_triggered()
{
    QString dirPath = QFileDialog::getExistingDirectory(this,"Open Folder");
    fileSystemModel->setRootPath(dirPath);
    treeView->setModel(fileSystemModel);
    treeView->setRootIndex(fileSystemModel->index(dirPath));
    treeView->hideColumn(1);
    treeView->hideColumn(2);
    treeView->hideColumn(3);

    treeView->setMinimumWidth(width() * 20 / 100);
    treeView->setMaximumWidth(width() * 30 / 100);
}

void MainWindow::openTreeViewFile(QModelIndex index)
{
    QString filePath = fileSystemModel->fileInfo(index).absolutePath();
    openTabFile(filePath);
}

void MainWindow::on_actionSave_triggered()
{
    QString filepath = tabsWidget->tabToolTip(tabsWidget->currentIndex());

    if(filepath=="Untitled")
    {
        on_actionSave_As_triggered();
        return;
    }

    QFile file(filepath);

    if(!file.open(QFile::WriteOnly|QFile::Text))
    {
        QMessageBox::warning(this,"Warning","Cannot save file" + file.errorString());
    }

    QTextStream out(&file);

    QString text = currentTextEdit()->toPlainText();

    out << text;

    file.close();

    QString newTabText = tabsWidget->tabText(tabsWidget->currentIndex()).remove(0,1);

    tabsWidget->setTabText(tabsWidget->currentIndex(),newTabText);

}


void MainWindow::on_actionSave_As_triggered()
{
    if(tabsWidget->count()==0){
        QMessageBox::warning(this,"Warning","No file opened");
        return;
    }

    QString filepath = QFileDialog::getSaveFileName(this,"Save As ...");

    QFile file(filepath);

    if(!file.open(QFile::WriteOnly|QFile::Text))
    {
        QMessageBox::warning(this,"Warning","Cannot save file + " + file.errorString());
        return;
    }

    QTextStream out(&file);

    QString text = currentTextEdit()->toPlainText();

    out << text;

    file.close();

    closeTab(tabsWidget->currentIndex());
    openTabFile(filepath);
}

void MainWindow::updateStatus()
{
    QString line = QString::number(currentTextEdit()->textCursor().blockNumber()+1);
    QString column = QString::number(currentTextEdit()->textCursor().columnNumber()+1);

    QString new_status_str = "Line " + line + ", Column "+ column;
    currentStatus()->setText(new_status_str);
}

QLabel* MainWindow::currentStatus()
{
    QList<QLabel *> statusList = tabsWidget->findChildren<QLabel *>("status");

    for(int i=0; i<statusList.count();i++)
    {
        if(tabsWidget->indexOf(statusList.at(i)->parentWidget()) == tabsWidget->currentIndex())
        {
            return statusList.at(i);
        }
    }

    return new QLabel;
}

void MainWindow::on_actionQuit_triggered()
{
    QApplication::quit();
}

void MainWindow::on_actionAbout_triggered()
{
    QString about = "Name : Matrix Editor\n"
                    "Version : 1.0.0\n"
                    "Author : Mourad Latoundji\n";
    QMessageBox::about(this,"About", about);
}



