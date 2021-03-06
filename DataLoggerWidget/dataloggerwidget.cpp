#include "dataloggerwidget.h"
#include "ui_dataloggerwidget.h"

DataLoggerWidget::DataLoggerWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DataLoggerWidget)
{
    ui->setupUi(this);

    m_worker = new DataLoggerThread;
    m_worker->moveToThread(&m_thread);

    connect(&m_thread, &QThread::finished, m_worker, &QObject::deleteLater);
    m_thread.start();

    // connect widget signals to worker slots
    connect(this, SIGNAL(initializeDataLogger(std::vector<int>,std::vector<QString>)),
            m_worker, SLOT(initializeDataLogger(std::vector<int>,std::vector<QString>)));
    connect(this, SIGNAL(setRootDir(QString)), m_worker, SLOT(setRootDirectory(QString)));
    connect(this, SIGNAL(closeLogFiles()), m_worker, SLOT(closeLogFiles()));
    connect(this, SIGNAL(startLogging()), m_worker, SLOT(startLogging()));
    connect(this, SIGNAL(stopLogging()), m_worker, SLOT(stopLogging()));
    connect(this, SIGNAL(logNote(QTime,QString)), m_worker, SLOT(logNote(QTime,QString)));

    // connect worker signals to widget slots
    connect(m_worker, SIGNAL(statusChanged(int)), this, SLOT(workerStatusChanged(int)));
    connect(m_worker, SIGNAL(fileStatusChanged(unsigned short,int)),
                this, SLOT(workerFileStatusChanged(unsigned short,int)));

    m_statusList.push_back(ui->EMstatusLabel);
    m_statusList.push_back(ui->ECGstatusLabel);
    m_statusList.push_back(ui->EPOSstatusLabel);
    m_statusList.push_back(ui->FrmGrabStatusLabel);
    m_statusList.push_back(ui->LogStatusLabel);
    m_statusList.push_back(ui->ErrorStatusLabel);
    m_statusList.push_back(ui->NotesStatusLabel);
    m_statusList.push_back(ui->ControlStatusLabel);

    m_fileNameList.push_back(ui->EMpathLineEdit);
    m_fileNameList.push_back(ui->ECGpathLineEdit);
    m_fileNameList.push_back(ui->EPOSpathLineEdit);
    m_fileNameList.push_back(ui->FrmGrabPathLineEdit);
    m_fileNameList.push_back(ui->LogPathLineEdit);
    m_fileNameList.push_back(ui->ErrorPathLineEdit);
    m_fileNameList.push_back(ui->NotesPathLineEdit);
    m_fileNameList.push_back(ui->ControlPathLineEdit);

    m_checkBoxList.push_back(ui->EMcheckBox);
    m_checkBoxList.push_back(ui->ECGcheckBox);
    m_checkBoxList.push_back(ui->EPOScheckBox);
    m_checkBoxList.push_back(ui->FrmGrabCheckBox);
    m_checkBoxList.push_back(ui->LogCheckBox);
    m_checkBoxList.push_back(ui->ErrorCheckBox);
    m_checkBoxList.push_back(ui->NotesCheckBox);
    m_checkBoxList.push_back(ui->ControlCheckBox);

    m_writeCount.resize(DATALOG_NUM_FILES, 0);
}

DataLoggerWidget::~DataLoggerWidget()
{
    m_thread.quit();
    m_thread.wait();

    qDebug() << "DataLogger thread quit.";

    delete ui;
}

void DataLoggerWidget::workerStatusChanged(int status)
{
    switch(status)
    {
    case DATALOG_INITIALIZE_BEGIN:
        ui->widgetStatusLineEdit->setText("Initializing");
        ui->initFilesButton->setEnabled(false);
        ui->dataDirLineEdit->setReadOnly(true);
        ui->dataDirPushButton->setEnabled(false);
        ui->startLoggingButton->setEnabled(true);
        ui->stopLoggingButton->setEnabled(false);
        ui->closeFilesButton->setEnabled(false);
        ui->logNoteButton->setEnabled(false);
        for(size_t i = 0; i < m_checkBoxList.size(); i++)
        {
            m_fileNameList[i]->setReadOnly(true);
            m_checkBoxList[i]->setEnabled(false);
        }
        break;
    case DATALOG_INITIALIZE_FAILED:
        ui->widgetStatusLineEdit->setText("Failed!");
        ui->initFilesButton->setEnabled(true);
        ui->dataDirLineEdit->setReadOnly(false);
        ui->dataDirPushButton->setEnabled(true);
        ui->startLoggingButton->setEnabled(false);
        ui->stopLoggingButton->setEnabled(false);
        ui->closeFilesButton->setEnabled(false);
        ui->logNoteButton->setEnabled(false);
        for(size_t i = 0; i < m_checkBoxList.size(); i++)
        {
            m_fileNameList[i]->setReadOnly(false);
            m_checkBoxList[i]->setEnabled(true);
        }
        break;
    case DATALOG_INITIALIZED:
        ui->widgetStatusLineEdit->setText("Initialized");
        ui->initFilesButton->setEnabled(false);
        ui->dataDirLineEdit->setReadOnly(true);
        ui->dataDirPushButton->setEnabled(false);
        ui->startLoggingButton->setEnabled(true);
        ui->stopLoggingButton->setEnabled(false);
        ui->closeFilesButton->setEnabled(true);
        ui->logNoteButton->setEnabled(true);
        for(size_t i = 0; i < m_checkBoxList.size(); i++)
        {
            m_fileNameList[i]->setReadOnly(true);
            m_checkBoxList[i]->setEnabled(false);
        }
        break;
    case DATALOG_LOGGING_STARTED:
        ui->widgetStatusLineEdit->setText("Logging");
        ui->initFilesButton->setEnabled(false);
        ui->dataDirLineEdit->setReadOnly(true);
        ui->dataDirPushButton->setEnabled(false);
        ui->startLoggingButton->setEnabled(false);
        ui->stopLoggingButton->setEnabled(true);
        ui->closeFilesButton->setEnabled(false);
        ui->logNoteButton->setEnabled(true);
        break;
    case DATALOG_LOGGING_STOPPED:
        ui->widgetStatusLineEdit->setText("Stopped");
        ui->initFilesButton->setEnabled(false);
        ui->dataDirLineEdit->setReadOnly(true);
        ui->dataDirPushButton->setEnabled(false);
        ui->startLoggingButton->setEnabled(true);
        ui->stopLoggingButton->setEnabled(false);
        ui->closeFilesButton->setEnabled(true);
        ui->logNoteButton->setEnabled(true);
        break;
    case DATALOG_CLOSED:
        ui->widgetStatusLineEdit->setText("Closed");
        ui->initFilesButton->setEnabled(true);
        ui->dataDirLineEdit->setReadOnly(false);
        ui->dataDirPushButton->setEnabled(true);
        ui->startLoggingButton->setEnabled(false);
        ui->stopLoggingButton->setEnabled(false);
        ui->closeFilesButton->setEnabled(false);
        ui->logNoteButton->setEnabled(false);
        for(size_t i = 0; i < m_checkBoxList.size(); i++)
        {
            m_fileNameList[i]->setReadOnly(false);
            m_checkBoxList[i]->setEnabled(true);
        }
        break;
    default:
        qDebug() << "Unrecognized status.";
        break;
    }
}

void DataLoggerWidget::workerFileStatusChanged(const unsigned short fileID, int status)
{
    switch(status)
    {
    case DATALOG_FILE_OPENED:
        m_statusList[fileID]->setText("Opened");
        m_statusList[fileID]->setStyleSheet("QLabel { background-color : green;}");
        break;
    case DATALOG_FILE_CLOSED:
        m_statusList[fileID]->setText("Closed");
        m_statusList[fileID]->setStyleSheet("QLabel { background-color : yellow;}");
        break;
    case DATALOG_FILE_ERROR:
        m_statusList[fileID]->setText("ERROR!");
        m_statusList[fileID]->setStyleSheet("QLabel { background-color : red;}");
        break;
    case DATALOG_FILE_DATA_LOGGED:
        m_writeCount[fileID] += 1; // data written
        break;
    default:
        qDebug() << "Worker status not recognized!";
        break;
    }
}

void DataLoggerWidget::on_dataDirPushButton_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                                                    "../ICEbot_QT_v1/LoggedData",
                                                    QFileDialog::ShowDirsOnly
                                                    | QFileDialog::DontResolveSymlinks);

    ui->dataDirLineEdit->setText(dir);

    ui->initFilesButton->setEnabled(true); // enable initialize button
}

void DataLoggerWidget::on_initFilesButton_clicked()
{
    std::vector<int> enableMask;
    std::vector<QString> fileNames;

    QString rootDir = ui->dataDirLineEdit->text();
    QString datetime = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmsszzz");
    rootDir.append("/");
    rootDir.append(datetime);

    ui->subdirLineEdit->setText(tr("/")+datetime);

    emit setRootDir(rootDir);

    for(int i = 0; i < DATALOG_NUM_FILES; i++)
    {
        enableMask.push_back(m_checkBoxList[i]->isChecked());

        QString fname = rootDir;
        fname.append("/");
        fname.append(datetime);
        fname.append("_");
        fname.append(m_fileNameList[i]->text());
        fileNames.push_back(fname);
    }

    emit initializeDataLogger(enableMask, fileNames);
}

void DataLoggerWidget::on_closeFilesButton_clicked()
{
    for(size_t i = 0; i < DATALOG_NUM_FILES; i++)
        qDebug() << m_writeCount[i] << "records written to" << m_fileNameList[i]->text();

    // reset counts
    m_writeCount.clear();
    m_writeCount.resize(DATALOG_NUM_FILES, 0);

    emit closeLogFiles();
}

void DataLoggerWidget::on_startLoggingButton_clicked()
{
    emit startLogging();
}

void DataLoggerWidget::on_stopLoggingButton_clicked()
{
    emit stopLogging();
}

void DataLoggerWidget::on_logNoteButton_clicked()
{
    emit logNote(QTime::currentTime(), ui->noteTextEdit->toPlainText());

    ui->noteTextEdit->clear();
}
