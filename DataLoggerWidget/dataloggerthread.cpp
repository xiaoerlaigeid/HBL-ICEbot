#include "dataloggerthread.h"

DataLoggerThread::DataLoggerThread(QObject *parent) : QObject(parent)
{
    qRegisterMetaType< std::vector<int> >("std::vector<int>");
    qRegisterMetaType< std::vector<QString> >("std::vector<QString>");

    m_isEpochSet = false;
    m_isReady = false;
    m_keepLogging = false;

    m_mutex = new QMutex(QMutex::Recursive);

//    for (int i = 0; i < DATALOG_NUM_FILES; i++)
//    {
//        m_files.push_back(std::shared_ptr<QFile>(new QFile()) );
//        m_TextStreams.push_back(std::shared_ptr<QTextStream>(new QTextStream()));
//        m_DataStreams.push_back(std::shared_ptr<QTextStream>(new QTextStream()));
//    }
    m_files.resize(DATALOG_NUM_FILES);
    m_TextStreams.resize(DATALOG_NUM_FILES);
    m_DataStreams.resize(DATALOG_NUM_FILES);

    //qRegisterMetaType<DOUBLE_POSITION_MATRIX_TIME_Q_RECORD>("DOUBLE_POSITION_MATRIX_TIME_Q_RECORD");

}

DataLoggerThread::~DataLoggerThread()
{
    closeLogFiles();

    qDebug() << "Ending DataLoggerThread - ID: " << reinterpret_cast<int>(QThread::currentThreadId()) << ".";

    delete m_mutex;

    emit finished();
}

// ------------------------------
//     SLOTS IMPLEMENTATION
// ------------------------------
void DataLoggerThread::setEpoch(const QDateTime &epoch)
{
    QMutexLocker locker(m_mutex);

    if(!m_keepLogging)
    {
        m_epoch = epoch;
        m_isEpochSet = true;

        // TODO: Implement generic logEventWithMessage slot
        //emit logEventWithMessage(SRC_DATALOGGER, LOG_INFO, QTime::currentTime(), DATALOG_EPOCH_SET,
        //                         m_epoch.toString("dd/MM/yyyy - hh:mm:ss.zzz"));
    }
    else
        emit logEvent(SRC_DATALOGGER, LOG_INFO, QTime::currentTime(), DATALOG_EPOCH_SET_FAILED);
}

void DataLoggerThread::setRootDirectory(QString rootDir)
{
    QMutexLocker locker(m_mutex);

    // check if the directory exists, if not, create it
    QDir dir;
    if( ! dir.exists(rootDir) )
    {
        if( dir.mkpath(rootDir) )
        {
            qDebug() << "Folder created: " << rootDir;
            QDir imgDir;
            if( imgDir.mkpath(rootDir + QString("/images")) )
                qDebug() << "Folder created: " << "/images";
        }
        else
        {
            qDebug() << "Folder creation failed: " << rootDir;

            emit statusChanged(DATALOG_FOLDER_ERROR);
        }
    }
    else
    {
        QDir imgDir;
        if(! dir.exists(rootDir+ QString("/images")) )
        {
            if( imgDir.mkpath(rootDir + QString("/images")) )
                qDebug() << "Folder created: " << "/images";
        }
    }
    qDebug() << "Root dir: " << rootDir;

    m_rootDirectory = rootDir;
}

void DataLoggerThread::initializeDataLogger(std::vector<int> enableMask, std::vector<QString> fileNames)
{
    QMutexLocker locker(m_mutex);

    emit statusChanged(DATALOG_INITIALIZE_BEGIN);

    if(enableMask.size() == DATALOG_NUM_FILES) // within bounds
    {
        for (size_t i = 0; i < enableMask.size(); i++) // iterate through items
        {
            if(enableMask[i]) // user wants to log this
            {
                bool status = true;

                m_files[i].reset(new QFile());
                m_files[i]->setFileName(fileNames[i]); // set filename of QFile

                if(m_files[i]->open(QIODevice::WriteOnly | QIODevice::Append)) // opened successfuly
                {
                    if( DATALOG_EM_ID == i)
                    {
                        m_DataStreams[i].reset(new QDataStream());
                        m_DataStreams[i]->setDevice(&(*m_files[i]));
                        (*m_DataStreams[i]) << QDateTime::currentMSecsSinceEpoch();
                    }
                    else
                    {
                        m_TextStreams[i].reset(new QTextStream());
                        m_TextStreams[i]->setDevice(&(*m_files[i]));
                        (*m_TextStreams[i]) << "File opened at: " << getCurrDateTimeStr() << '\n';
                    }

                    emit fileStatusChanged(i, DATALOG_FILE_OPENED);
                }
                else // can't open
                {
                    qDebug() << "File could not be opened: " << m_files[i]->fileName();

                    status = false;
                    emit fileStatusChanged(i, DATALOG_FILE_ERROR);
                }
            }
            else // user doesn't want
            {
                emit fileStatusChanged(i, DATALOG_FILE_CLOSED);
            }
        }
        emit statusChanged(DATALOG_INITIALIZED);
        m_isReady = true;
    }
    else
        emit statusChanged(DATALOG_INITIALIZE_FAILED); //error!
}

// This is for writing to EM file as text
//void DataLoggerThread::logEMdata(QTime timeStamp,
//                                 int sensorID,
//                                 DOUBLE_POSITION_MATRIX_TIME_STAMP_RECORD data)
//{
//    QDateTime ts;
//    ts.setMSecsSinceEpoch(data.time*1000);

//    // Data Format
//    // | Sensor ID | Time Stamp | x | y | z | r11 | r12 | ... | r33 |

//    QString output;
//    output.append(QString("%1\t%2\t%3\t%4\t%5\t%6\t%7\t%8\t%9\t%10\t%11\t%12\t%13\t%14\n")
//                     .arg(sensorID)
//                     .arg(QString::number(data.x,'f',m_prec))
//                     .arg(QString::number(data.y,'f',m_prec))
//                     .arg(QString::number(data.z,'f',m_prec))
//                     .arg(QString::number(data.s[0][0],'f',m_prec))
//                     .arg(QString::number(data.s[0][1],'f',m_prec))
//                     .arg(QString::number(data.s[0][2],'f',m_prec))
//                     .arg(QString::number(data.s[1][0],'f',m_prec))
//                     .arg(QString::number(data.s[1][1],'f',m_prec))
//                     .arg(QString::number(data.s[1][2],'f',m_prec))
//                     .arg(QString::number(data.s[2][0],'f',m_prec))
//                     .arg(QString::number(data.s[2][1],'f',m_prec))
//                     .arg(QString::number(data.s[2][2],'f',m_prec))
//                     .arg(QString::number(data.time*1000,'f',m_prec)));
//                     //.arg(ts.toString("hh:mm:ss:zzz")));

//    QMutexLocker locker(m_mutex);

//    if(m_files[DATALOG_EM_ID]->isOpen())
//    {
//        (*m_TextStreams[DATALOG_EM_ID]) << output;
//
//     // emit fileStatusChanged(DATALOG_EM_ID, DATALOG_FILE_DATA_LOGGED);
//    }
//    else
//        qDebug() << "File is closed.";
//}

void DataLoggerThread::logEMdata(QTime timeStamp,
                                 int sensorID,
                                 DOUBLE_POSITION_MATRIX_TIME_Q_RECORD data)
{
    // Data Format
    // | Sensor ID | Time Stamp | x | y | z | q1 | q2 | q3 | q4 | quality |
    // |    int    |   double   |        ...      double      ...         |

    QMutexLocker locker(m_mutex);

    if(m_isReady && m_files[DATALOG_EM_ID]->isOpen())
    {
        (*m_DataStreams[DATALOG_EM_ID]) << sensorID
                                        << data.time*1000
                                        << data.x
                                        << data.y
                                        << data.z
                                        << data.s[0][0]
                                        << data.s[0][1]
                                        << data.s[0][2]
                                        << data.s[1][0]
                                        << data.s[1][1]
                                        << data.s[1][2]
                                        << data.s[2][0]
                                        << data.s[2][1]
                                        << data.s[2][2]
                                        << data.quality;
//        << data.q[0]
//        << data.q[1]
//        << data.q[2]
//        << data.q[3]

        // emit fileStatusChanged(DATALOG_EM_ID, DATALOG_FILE_DATA_LOGGED);
    }
    else
        qDebug() << "File is closed.";
}

// save image and write details to text file
void DataLoggerThread::logFrmGrabImage(std::shared_ptr<Frame> frm)
{
    QDateTime imgTime;
    imgTime.setMSecsSinceEpoch(frm->timestamp_);

    // file name of frame
    QString m_imgFname = imgTime.toString("ddMMyyyy_hhmmsszzz");
    // populate m_imgFname with index
    m_imgFname.append( QString("_%1.jpg").arg(frm->index_) );

    QString m_DirImgFname = m_rootDirectory;
    m_DirImgFname.append("/images/");
    m_DirImgFname.append(m_imgFname);
    // save frame
    //state = frame->image_.save(m_imgFname, "JPG", 100);

    std::vector<int> compression_params;
    compression_params.push_back(CV_IMWRITE_JPEG_QUALITY);
    compression_params.push_back(100);

    cv::imwrite(m_DirImgFname.toStdString().c_str(), frm->image_, compression_params); // write frame

    QMutexLocker locker(m_mutex);

    // output to text
    if(m_isReady && m_files[DATALOG_FrmGrab_ID]->isOpen())
    {
        (*m_TextStreams[DATALOG_FrmGrab_ID]) << m_imgFname << "\t"
                     << QString::number(frm->timestamp_, 'f', 1) << '\n';

        // emit fileStatusChanged(DATALOG_FrmGrab_ID, DATALOG_FILE_DATA_LOGGED);
    }
    else
        qDebug() << "FrmGrab text file closed.";

}

void DataLoggerThread::logLabJackData(qint64 timeStamp, std::vector<double> data)
{
    QString output = QString("%1").arg(timeStamp);
    for(const double &d : data)
    {
        output.append("\t");
        output.append(QString::number(d, 'f', 6));
    }

    QMutexLocker locker(m_mutex);

    if(m_isReady && m_files[DATALOG_ECG_ID]->isOpen())
    {
//        (*m_TextStreams[DATALOG_ECG_ID]) << timeStamp.toString("HH:mm:ss.zzz") << "\t" << QString::number(data, 'f', 6) << '\n';
        (*m_TextStreams[DATALOG_ECG_ID]) << output << '\n';

        // emit fileStatusChanged(DATALOG_ECG_ID, DATALOG_FILE_DATA_LOGGED);
    }
    else
        ;//qDebug() << "File is closed.";
}

void DataLoggerThread::logEPOSdata(QTime timeStamp, int dataType, const int motID, long data)
{
    QString output = QString("%1\t").arg(timeStamp.msecsSinceStartOfDay());
    switch(dataType)
    {
    case EPOS_COMMANDED:
        output.append("CMND\t");
        break;
    case EPOS_READ:
        output.append("READ\t");
        break;
    }

    output.append(QString("%1\t%2\n").arg(motID).arg(data));


    QMutexLocker locker(m_mutex);

    if(m_isReady && m_files[DATALOG_EPOS_ID]->isOpen())
    {
        (*m_TextStreams[DATALOG_EPOS_ID]) << output;

        // emit fileStatusChanged(DATALOG_EPOS_ID, DATALOG_FILE_DATA_LOGGED);
    }
    else
        ;//qDebug() << "File is closed.";
}

void DataLoggerThread::logEPOSdata(QTime timeStamp, int dataType, std::vector<long> data)
{
    QString output;

    for(size_t i = 0; i < data.size(); i++)
    {
        output.append(QString("%1\t").arg(timeStamp.msecsSinceStartOfDay()));
        switch(dataType)
        {
        case EPOS_COMMANDED:
            output.append("CMND\t");
            break;
        case EPOS_READ:
            output.append("READ\t");
            break;
        }

        output.append(QString("%1\t%2\n").arg(i).arg(data[i]));
    }

    QMutexLocker locker(m_mutex);

    if(m_isReady && m_files[DATALOG_EPOS_ID]->isOpen())
    {
        (*m_TextStreams[DATALOG_EPOS_ID]) << output << '\n';

        // emit fileStatusChanged(DATALOG_EPOS_ID, DATALOG_FILE_DATA_LOGGED);
    }
    else
        ;//qDebug() << "File is closed.";
}

void DataLoggerThread::logControllerData(QTime timeStamp, int loopIdx, int dataType, std::vector<double> data)
{
//    CONTROLLER_DXYZPSI = 0, // 4 values
//    CONTROLLER_USER_XYZDXYZPSI, // 7 values
//    CONTROLLER_CURR_PSY_GAMMA, // 2 values
//    CONTROLLER_T_BB_CT_curTipPos, // 16 values
//    CONTROLLER_CURR_TASK, // 4 values
//    CONTROLLER_PERIOD, // 1 value
//    CONTROLLER_BIRD4_MODEL_PARAMS, // 19 = 10 polar + 9 rect
//    CONTROLLER_RESETBB, // 16 values
//    CONTROLLER_MODES, // 6 values
//    CONTROLLER_USANGLE // 1 value
    QString output;
    output.append(QString("%1\t%2\t").arg(loopIdx).arg(timeStamp.msecsSinceStartOfDay()));

    switch(dataType)
    {
    case CONTROLLER_DXYZPSI: // 4 values
        output.append(QString("DXYZPSI\t%1\t%2\t%3\t%4")
                             .arg(QString::number(data[0],'f',m_prec2))
                             .arg(QString::number(data[1],'f',m_prec2))
                             .arg(QString::number(data[2],'f',m_prec2))
                             .arg(QString::number(data[3],'f',m_prec2)));
        break;
    case CONTROLLER_USER_XYZDXYZPSI: // 7 values
        output.append(QString("USERXYZDXYZPSI\t%1\t%2\t%3\t%4\t%5\t%6\t%7")
                             .arg(QString::number(data[0],'f',2))
                             .arg(QString::number(data[1],'f',2))
                             .arg(QString::number(data[2],'f',2))
                             .arg(QString::number(data[3],'f',2))
                             .arg(QString::number(data[4],'f',2))
                             .arg(QString::number(data[5],'f',2))
                             .arg(QString::number(data[6],'f',2)));
        break;
    case CONTROLLER_CURR_PSY_GAMMA: // 2 values
        output.append(QString("CURRPSYGAMMA\t%1\t%2")
                             .arg(QString::number(data[0],'f',m_prec2))
                             .arg(QString::number(data[1],'f',m_prec2)));
        break;
    case CONTROLLER_T_BB_CT_curTipPos: // 16 values
        output.append(QString("T_BB_CT\t%1\t%2\t%3\t%4\t%5\t%6\t%7\t%8\t%9\t%10\t%11\t%12\t%13\t%14\t%15\t%16")
                      .arg(QString::number(data[0],'f',m_prec2))
                      .arg(QString::number(data[1],'f',m_prec2))
                      .arg(QString::number(data[2],'f',m_prec2))
                      .arg(QString::number(data[3],'f',m_prec2))
                      .arg(QString::number(data[4],'f',m_prec2))
                      .arg(QString::number(data[5],'f',m_prec2))
                      .arg(QString::number(data[6],'f',m_prec2))
                      .arg(QString::number(data[7],'f',m_prec2))
                      .arg(QString::number(data[8],'f',m_prec2))
                      .arg(QString::number(data[9],'f',m_prec2))
                      .arg(QString::number(data[10],'f',m_prec2))
                      .arg(QString::number(data[11],'f',m_prec2))
                      .arg(QString::number(data[12],'f',m_prec2))
                      .arg(QString::number(data[13],'f',m_prec2))
                      .arg(QString::number(data[14],'f',m_prec2))
                      .arg(QString::number(data[15],'f',m_prec2)));
        break;
    case CONTROLLER_CURR_TASK: // 4 values
        output.append(QString("CURRTASK\t%1\t%2\t%3\t%4")
                      .arg(QString::number(data[0],'f',m_prec2))
                      .arg(QString::number(data[1],'f',m_prec2))
                      .arg(QString::number(data[2],'f',m_prec2))
                      .arg(QString::number(data[3],'f',m_prec2)));
        break;
    case CONTROLLER_PERIOD: // 1 value
        output.append(QString("PERIOD\t%1")
                             .arg(QString::number(data[0],'f',m_prec)));
        break;
    case CONTROLLER_BIRD4_MODEL_PARAMS: // 19 = 10 polar + 9 rect
        output.append(QString("BIRD4MODELPARAMS\t%1\t%2\t%3\t%4\t%5\t%6\t%7\t%8\t%9\t%10\t%11\t%12\t%13\t%14\t%15\t%16\t%17\t%18\t%19")
                             .arg(QString::number(data[0],'f',m_prec2))
                             .arg(QString::number(data[1],'f',m_prec2))
                             .arg(QString::number(data[2],'f',m_prec2))
                             .arg(QString::number(data[3],'f',m_prec2))
                             .arg(QString::number(data[4],'f',m_prec2))
                             .arg(QString::number(data[5],'f',m_prec2))
                             .arg(QString::number(data[6],'f',m_prec2))
                             .arg(QString::number(data[7],'f',m_prec2))
                             .arg(QString::number(data[8],'f',m_prec2))
                             .arg(QString::number(data[9],'f',m_prec2))
                             .arg(QString::number(data[10],'f',m_prec2))
                             .arg(QString::number(data[11],'f',m_prec2))
                             .arg(QString::number(data[12],'f',m_prec2))
                             .arg(QString::number(data[13],'f',m_prec2))
                             .arg(QString::number(data[14],'f',m_prec2))
                             .arg(QString::number(data[15],'f',m_prec2))
                             .arg(QString::number(data[16],'f',m_prec2))
                             .arg(QString::number(data[17],'f',m_prec2))
                             .arg(QString::number(data[18],'f',m_prec2)));
        break;
    case CONTROLLER_RESETBB: // 16 values
        output.append(QString("RESETBB\t%1\t%2\t%3\t%4\t%5\t%6\t%7\t%8\t%9\t%10\t%11\t%12\t%13\t%14\t%15\t%16")
                             .arg(QString::number(data[0],'f',m_prec2))
                             .arg(QString::number(data[1],'f',m_prec2))
                             .arg(QString::number(data[2],'f',m_prec2))
                             .arg(QString::number(data[3],'f',m_prec2))
                             .arg(QString::number(data[4],'f',m_prec2))
                             .arg(QString::number(data[5],'f',m_prec2))
                             .arg(QString::number(data[6],'f',m_prec2))
                             .arg(QString::number(data[7],'f',m_prec2))
                             .arg(QString::number(data[8],'f',m_prec2))
                             .arg(QString::number(data[9],'f',m_prec2))
                             .arg(QString::number(data[10],'f',m_prec2))
                             .arg(QString::number(data[11],'f',m_prec2))
                             .arg(QString::number(data[12],'f',m_prec2))
                             .arg(QString::number(data[13],'f',m_prec2))
                             .arg(QString::number(data[14],'f',m_prec2))
                             .arg(QString::number(data[15],'f',m_prec2)));
        break;
    case CONTROLLER_MODES: // 6 values
        output.append(QString("MODES\t%1\t%2\t%3\t%4\t%5\t%6")
                             .arg(QString::number(data[0],'f',1))
                             .arg(QString::number(data[1],'f',1))
                             .arg(QString::number(data[2],'f',1))
                             .arg(QString::number(data[3],'f',1))
                             .arg(QString::number(data[4],'f',1))
                             .arg(QString::number(data[5],'f',1)));
        break;
    case CONTROLLER_USANGLE: // 1 value
        output.append(QString("USANGLE\t%1")
                             .arg(QString::number(data[0],'f',m_prec)));
        break;
    case CONTROLLER_SWEEP_START: // 4 values
        output.append(QString("SWEEP\t%1\t%2\t%3\t%4")
                             .arg(QString::number(data[0],'f',1)) // nSteps_
                             .arg(QString::number(data[1],'f',m_prec)) // stepAngle_
                             .arg(QString::number(data[2],'f',m_prec)) // convLimit_
                             .arg(QString::number(data[3],'f',1))); // imgDuration_
        break;
    case CONTROLLER_NEXT_SWEEP: // 1 value
        output.append(QString("SWEEPNEXT\t%1")
                             .arg(QString::number(data[0],'f',m_prec)));
        break;
    case CONTROLLER_SWEEP_CONVERGED: // 1 value
        output.append(QString("SWEEPCONVD\t%1")
                             .arg(QString::number(data[0],'f',m_prec)));
        break;
    default:
        output.append(QString("UNKNOWN"));
        break;
    }

    QMutexLocker locker(m_mutex);

    // output to text
    if(m_isReady && m_files[DATALOG_Control_ID]->isOpen())
    {
        (*m_TextStreams[DATALOG_Control_ID]) << output << '\n';

        // emit fileStatusChanged(DATALOG_Control_ID, DATALOG_FILE_DATA_LOGGED);
    }
    else
        ;//qDebug() << "Controller text file closed.";
}

void DataLoggerThread::logEvent(int source, int logType, QTime timeStamp, int eventID)
{
    // Data Format
    // | Time Stamp | Log Type | Source | eventID |

    QString output = timeStamp.toString("HH:mm:ss.zzz");

    switch(logType)
    {
    case LOG_INFO:
        output.append(" INFO ");
        break;
    case LOG_WARNING:
        output.append(" WARNING ");
        break;
    case LOG_ERROR:
        output.append(" ERROR ");
        break;
    case LOG_FATAL:
        output.append(" FATAL ");
        break;
    default:
        output.append(" UNKNOWN ");
        break;
    }

    switch(source)
    {
    case SRC_CONTROLLER:
        output.append("CONTROLLER ");
        break;
    case SRC_EM:
        output.append("EM ");
        break;
    case SRC_EPOS:
        output.append("EPOS ");
        break;
    case SRC_FRMGRAB:
        output.append("FRMGRAB ");
        break;
    case SRC_EPIPHAN:
        output.append("EPIPHAN ");
        break;
    case SRC_LABJACK:
        output.append("LABJACK ");
        break;
    case SRC_OMNI:
        output.append("OMNI ");
        break;
    case SRC_GUI:
        output.append("GUI ");
        break;
    case SRC_UNKNOWN:
        output.append("UNKNOWN ");
        break;
    default:
        output.append("UNKNOWN ");
        break;
    }

    output.append(QString("%1\n").arg(eventID));

    QMutexLocker locker(m_mutex);

    if(m_isReady && m_files[DATALOG_Log_ID]->isOpen())
    {
        (*m_TextStreams[DATALOG_Log_ID]) << output;

        emit fileStatusChanged(DATALOG_Log_ID, DATALOG_FILE_DATA_LOGGED);
    }
    else
        qDebug() << "Event logger: File is closed!";
}

void DataLoggerThread::logError(int source, int logType,
                                QTime timeStamp, int errCode,
                                QString message)
{
    // Data Format
    // | Time Stamp | Log Type | Source | eventID |

    QString output = timeStamp.toString("HH:mm:ss.zzz");

    switch(logType)
    {
    case LOG_INFO:
        output.append(" INFO ");
        break;
    case LOG_WARNING:
        output.append(" WARNING ");
        break;
    case LOG_ERROR:
        output.append(" ERROR ");
        break;
    case LOG_FATAL:
        output.append(" FATAL ");
        break;
    default:
        output.append(" UNKNOWN ");
        break;
    }

    switch(source)
    {
    case SRC_CONTROLLER:
        output.append("CONTROLLER ");
        break;
    case SRC_EM:
        output.append("EM ");
        break;
    case SRC_EPOS:
        output.append("EPOS ");
        break;
    case SRC_FRMGRAB:
        output.append("FRMGRAB ");
        break;
    case SRC_EPIPHAN:
        output.append("EPIPHAN ");
        break;
    case SRC_LABJACK:
        output.append("LABJACK ");
        break;
    case SRC_OMNI:
        output.append("OMNI ");
        break;
    case SRC_GUI:
        output.append("GUI ");
        break;
    case SRC_UNKNOWN:
        output.append("UNKNOWN ");
        break;
    default:
        output.append("UNKNOWN ");
        break;
    }

    output.append(QString("%1 ").arg(errCode));

    output.append(message);

    QMutexLocker locker(m_mutex);

    if(m_isReady && m_files[DATALOG_Error_ID]->isOpen())
    {
        (*m_TextStreams[DATALOG_Error_ID]) << output << '\n';

        emit fileStatusChanged(DATALOG_Error_ID, DATALOG_FILE_DATA_LOGGED);
    }
    else
        qDebug() << "Error logger: File is closed!";
}

void DataLoggerThread::startLogging()
{
    QMutexLocker locker(m_mutex);

    // make connections

    emit statusChanged(DATALOG_LOGGING_STARTED);

}

void DataLoggerThread::stopLogging()
{
    QMutexLocker locker(m_mutex);

    // sever connections

    emit statusChanged(DATALOG_LOGGING_STOPPED);
}

void DataLoggerThread::logNote(QTime timeStamp, QString note)
{
    if(m_files[DATALOG_Note_ID]->isOpen())
    {
        (*m_TextStreams[DATALOG_Note_ID]) << "[" << timeStamp.toString("HH:mm:ss.zzz") << "] " << note << endl;

        emit fileStatusChanged(DATALOG_Note_ID, DATALOG_FILE_DATA_LOGGED);
    }
}

void DataLoggerThread::closeLogFile(const unsigned short fileID)
{
    QMutexLocker locker(m_mutex);

    // pointer not null and file is open
    if(m_files[fileID] && m_files[fileID]->isOpen())
    {
        if( DATALOG_EM_ID != fileID )
        {
            (*m_TextStreams[fileID]) << "File closed at: " << getCurrDateTimeStr() << '\n';
            m_TextStreams[fileID]->flush();
            //m_DataStreams[fileID]->flush(); // no need to flush, data is not buffered
        }
        m_files[fileID]->close();

        emit fileStatusChanged(fileID, DATALOG_FILE_CLOSED);
    }
}

void DataLoggerThread::closeLogFiles()
{
    QMutexLocker locker(m_mutex);

    for (unsigned short i = 0; i < DATALOG_NUM_FILES; i++) // iterate through items
    {
        closeLogFile(i);
    }
    emit statusChanged(DATALOG_CLOSED);

    m_isReady = false;
}

// Helper functions

inline const QString getCurrDateTimeFileStr()
{
    return QDateTime::currentDateTime().toString("yyyyMMdd_hhmmsszzz");
}

inline const QString getCurrDateTimeStr()
{
    return QDateTime::currentDateTime().toString("yyyy/MM/dd - hh:mm:ss.zzz");
}
