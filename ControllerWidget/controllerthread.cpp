#include "controllerthread.h"

ControllerThread::ControllerThread(QObject *parent) :
    QObject(parent)
{
    m_isEpochSet = false;
    m_isReady = false;
    m_keepControlling = false;
    m_abort = false;
    m_latestReading.resize(4);

    m_numCycles = 0;

    m_cathKin = Kinematics_4DOF(0.05, 0.00135, 0.90*0.0254);

    m_mutex = new QMutex(QMutex::Recursive);
}

ControllerThread::~ControllerThread()
{
    std::cout << m_cathKin.forwardKinematics(1,0.1,0.1,0).matrix() << std::endl;

    //auto t1 = std::chrono::high_resolution_clock::now();

    std::cout << m_cathKin.JacobianNumeric(1,0.1,0.1,0).matrix() << std::endl;

    std::cout << m_cathKin.inverseKinematics3D(.005,.005,0.012,0.1) << std::endl;

    Eigen::Transform<double, 3, Eigen::Affine> T_in(Eigen::Matrix<double, 4, 4>::Identity());
    std::cout << T_in.matrix() << std::endl;
    Eigen::Vector4d configIn(0.0010, 0.0001, 0, 0);
    std::cout << m_cathKin.control_icra2016(T_in,configIn, 0.0) << std::endl;

    //this->thread()->msleep(1);
    //auto t2 = std::chrono::high_resolution_clock::now();
    //std::cout << t1.time_since_epoch().count() << std::endl;
    //std::cout << t2.time_since_epoch().count() << std::endl;
    //auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>( t2 - t1 ).count();
    //std::cout << "Took: " << duration << " ns" << std::endl;

    // stop controller

    m_mutex->lock();
    m_abort = true;
    m_mutex->unlock();

    qDebug() << "Ending ControllerThread - ID: " << QThread::currentThreadId() << ".";

    delete m_mutex;

    emit finished();
}

// ------------------------------
//     SLOTS IMPLEMENTATION
// ------------------------------
void ControllerThread::setEpoch(const QDateTime &epoch)
{
    QMutexLocker locker(m_mutex);

    if(!m_keepControlling)
    {
        m_epoch = epoch;
        m_isEpochSet = true;

        //emit logEventWithMessage(SRC_CONTROLLER, LOG_INFO, QTime::currentTime(), CONTROLLER_EPOCH_SET,
        //                         m_epoch.toString("dd/MM/yyyy - hh:mm:ss.zzz"));
    }
    else
        //emit logEvent(SRC_CONTROLLER, LOG_INFO, QTime::currentTime(), CONTROLLER_EPOCH_SET_FAILED);
        ;
}

void ControllerThread::printThreadID()
{
    qDebug() << QTime::currentTime() << "Worker Thread ID: " << QThread::currentThreadId();
}

void ControllerThread::receiveEMdata(QTime timeStamp, int sensorID, DOUBLE_POSITION_QUATERNION_TIME_Q_RECORD data)
{
    // Data Format
    // | Sensor ID | Time Stamp | x | y | z | r11 | r12 | .... | r33 |
    // |    int    |   double   |      ...      double      ...      |

    QMutexLocker locker(m_mutex);

    m_latestReading[sensorID] = data;

    // TODO: emit signal to log receiveEMdata event
    //emit logEvent(SRC_CONTROLLER, LOG_INFO, QTime::currentTime(), CONTROLLER_EM_RECEIVED);

    // process CT point
    Eigen::Transform<double,3,Eigen::Affine> curTipPos;
    Transform_From_EMreading(data, curTipPos);
    m_BB_CT_curTipPos = m_BB_Box*curTipPos*m_STm_BT*m_BT_CT; // convert to CT in terms of BBfixed
}

// ----------------
//    ACCESSORS
// ----------------




// ----------------
// HELPER FUNCTIONS
// ----------------
static void Transform_From_EMreading(DOUBLE_POSITION_QUATERNION_TIME_Q_RECORD &input, Eigen::Transform<double,3,Eigen::Affine> &output)
{
    QElapsedTimer elTimer;
    elTimer.start();

    Eigen::Translation3d trans(input.x, input.y, input.z);
    Eigen::Quaterniond quat(input.q[0], input.q[1], input.q[2], input.q[3]);
    output = trans * quat;

    qint64 elNsec = elTimer.nsecsElapsed();

    qDebug() << "Nsec elapsed:" << elNsec;
}

std::shared_ptr< std::vector<double> > ControllerThread::cycle_recalculate(std::vector<double> &inputs)
{
    //    std::ifstream is("inputs.txt");
    //    std::istream_iterator<double> start(is), end;
    //    std::vector<double> inputs(start, end);
    //    qDebug() << "Read " << inputs.size() << " numbers";

    //    std::shared_ptr< std::vector<double> > outputs = cycle_recalculate(inputs);

    //    std::ofstream output_file("outputs.txt");
    //    std::ostream_iterator<double> output_iterator(output_file, "\n");
    //    std::copy(outputs->begin(), outputs->end(), output_iterator);

//    QElapsedTimer elTimer;
//    elTimer.start();

    std::vector<double>::iterator iter = inputs.begin();

    // get all of the constants
    int m = static_cast<int>(*iter); ++iter; // m = number of sinusoid components
    double delta_t = *iter; ++iter; // Time step for each collected data point
    int N_initpts = static_cast<int>(*iter); ++iter;  // number of initialization points
    int edge_effect = static_cast<int>(*iter); ++iter;
    double omega_0 = *iter; ++iter;

    // calculate things from inputs
    int num_states = m * 2 + 2;
    int N_filtered = N_initpts - 2 * edge_effect;

    // parse the already low pass filtered data
    Eigen::Map<Eigen::VectorXd> z_init_x(iter._Ptr, N_filtered); std::advance(iter, N_filtered); // Initialize with the previously lowpass filtered data
    Eigen::Map<Eigen::VectorXd> z_init_y(iter._Ptr, N_filtered); std::advance(iter, N_filtered);
    Eigen::Map<Eigen::VectorXd> z_init_z(iter._Ptr, N_filtered); std::advance(iter, N_filtered);
    Eigen::Map<Eigen::VectorXd> z_init_xaxis(iter._Ptr, N_filtered); std::advance(iter, N_filtered);
    Eigen::Map<Eigen::VectorXd> z_init_yaxis(iter._Ptr, N_filtered); std::advance(iter, N_filtered);
    Eigen::Map<Eigen::VectorXd> z_init_zaxis(iter._Ptr, N_filtered); std::advance(iter, N_filtered);
    Eigen::Map<Eigen::VectorXd> z_init_angle(iter._Ptr, N_filtered); std::advance(iter, N_filtered);

    // allocate zero matrices
    Eigen::MatrixXd A_init_x(N_filtered, num_states - 1); A_init_x.setZero(); A_init_x.col(0).setOnes();
    Eigen::MatrixXd A_init_y(N_filtered, num_states - 1); A_init_y.setZero(); A_init_y.col(0).setOnes();
    Eigen::MatrixXd A_init_z(N_filtered, num_states - 1); A_init_z.setZero(); A_init_z.col(0).setOnes();
    Eigen::MatrixXd A_init_xaxis(N_filtered, num_states - 1); A_init_xaxis.setZero(); A_init_xaxis.col(0).setOnes();
    Eigen::MatrixXd A_init_yaxis(N_filtered, num_states - 1); A_init_yaxis.setZero(); A_init_yaxis.col(0).setOnes();
    Eigen::MatrixXd A_init_zaxis(N_filtered, num_states - 1); A_init_zaxis.setZero(); A_init_zaxis.col(0).setOnes();
    Eigen::MatrixXd A_init_angle(N_filtered, num_states - 1); A_init_angle.setZero(); A_init_angle.col(0).setOnes();

    for (int i = 0; i < N_filtered; i++)
    {
        for (int j = 1; j <= m; j++)
        {
            A_init_x(i, j)			= sin(j*omega_0*i*delta_t);
            A_init_y(i, j)			= sin(j*omega_0*i*delta_t);
            A_init_z(i, j)			= sin(j*omega_0*i*delta_t);
            A_init_xaxis(i, j)		= sin(j*omega_0*i*delta_t);
            A_init_yaxis(i, j)		= sin(j*omega_0*i*delta_t);
            A_init_zaxis(i, j)		= sin(j*omega_0*i*delta_t);
            A_init_angle(i, j)		= sin(j*omega_0*i*delta_t);
            A_init_x(i, j + m)		= cos(j*omega_0*i*delta_t);
            A_init_y(i, j + m)		= cos(j*omega_0*i*delta_t);
            A_init_z(i, j + m)		= cos(j*omega_0*i*delta_t);
            A_init_xaxis(i, j + m)	= cos(j*omega_0*i*delta_t);
            A_init_yaxis(i, j + m)	= cos(j*omega_0*i*delta_t);
            A_init_zaxis(i, j + m)	= cos(j*omega_0*i*delta_t);
            A_init_angle(i, j + m)	= cos(j*omega_0*i*delta_t);
        }
    }

    // Step 2. Use least squares estimate to solve for x.
    //VectorXd x_init_x = pseudoInverse(A_init_x) * z_init_x;
    Eigen::VectorXd x_init_x = A_init_x.jacobiSvd(Eigen::ComputeThinU | Eigen::ComputeThinV).solve(z_init_x);
    Eigen::VectorXd x_init_y = A_init_y.jacobiSvd(Eigen::ComputeThinU | Eigen::ComputeThinV).solve(z_init_y);
    Eigen::VectorXd x_init_z = A_init_z.jacobiSvd(Eigen::ComputeThinU | Eigen::ComputeThinV).solve(z_init_z);
    Eigen::VectorXd x_init_xaxis = A_init_xaxis.jacobiSvd(Eigen::ComputeThinU | Eigen::ComputeThinV).solve(z_init_xaxis);
    Eigen::VectorXd x_init_yaxis = A_init_yaxis.jacobiSvd(Eigen::ComputeThinU | Eigen::ComputeThinV).solve(z_init_yaxis);
    Eigen::VectorXd x_init_zaxis = A_init_zaxis.jacobiSvd(Eigen::ComputeThinU | Eigen::ComputeThinV).solve(z_init_zaxis);
    Eigen::VectorXd x_init_angle = A_init_angle.jacobiSvd(Eigen::ComputeThinU | Eigen::ComputeThinV).solve(z_init_angle);

    //VectorXd x_rect_x = x_init_x;
    //VectorXd x_rect_y = x_init_y;
    //VectorXd x_rect_z = x_init_z;
    //VectorXd x_rect_xaxis = x_init_xaxis;
    //VectorXd x_rect_yaxis = x_init_yaxis;
    //VectorXd x_rect_zaxis = x_init_zaxis;
    //VectorXd x_rect_angle = x_init_angle;

    // Step 3. Convert from rectangular into polar and assemble the state
    // vector, x, at k = 430 (which is the last trustworthy state we can know)
    // State vector, x
    // x = [c; r(1:4); omega; theta(1:4)];

    Eigen::VectorXd x_polar_x(num_states); x_polar_x.setZero();
    Eigen::VectorXd x_polar_y(num_states); x_polar_y.setZero();
    Eigen::VectorXd x_polar_z(num_states); x_polar_z.setZero();
    Eigen::VectorXd x_polar_xaxis(num_states); x_polar_xaxis.setZero();
    Eigen::VectorXd x_polar_yaxis(num_states); x_polar_yaxis.setZero();
    Eigen::VectorXd x_polar_zaxis(num_states); x_polar_zaxis.setZero();
    Eigen::VectorXd x_polar_angle(num_states); x_polar_angle.setZero();
    x_polar_x(0) = x_init_x(0); // c(dc offset)
    x_polar_y(0) = x_init_y(0); // c(dc offset)
    x_polar_z(0) = x_init_z(0); // c(dc offset)
    x_polar_xaxis(0) = x_init_xaxis(0); // c(dc offset)
    x_polar_yaxis(0) = x_init_yaxis(0); // c(dc offset)
    x_polar_zaxis(0) = x_init_zaxis(0); // c(dc offset)
    x_polar_angle(0) = x_init_angle(0); // c(dc offset)

    for (size_t i = 1; i <= m; i++)
    {
        x_polar_x(i) = std::sqrt(std::pow(x_init_x(i), 2) + std::pow(x_init_x(i + m), 2)); // r_i, convert rectangular coords back to polar
        x_polar_y(i) = std::sqrt(std::pow(x_init_y(i), 2) + std::pow(x_init_y(i + m), 2)); // r_i, convert rectangular coords back to polar
        x_polar_z(i) = std::sqrt(std::pow(x_init_z(i), 2) + std::pow(x_init_z(i + m), 2)); // r_i, convert rectangular coords back to polar
        x_polar_xaxis(i) = std::sqrt(std::pow(x_init_xaxis(i), 2) + std::pow(x_init_xaxis(i + m), 2)); // r_i, convert rectangular coords back to polar
        x_polar_yaxis(i) = std::sqrt(std::pow(x_init_yaxis(i), 2) + std::pow(x_init_yaxis(i + m), 2)); // r_i, convert rectangular coords back to polar
        x_polar_zaxis(i) = std::sqrt(std::pow(x_init_zaxis(i), 2) + std::pow(x_init_zaxis(i + m), 2)); // r_i, convert rectangular coords back to polar
        x_polar_angle(i) = std::sqrt(std::pow(x_init_angle(i), 2) + std::pow(x_init_angle(i + m), 2)); // r_i, convert rectangular coords back to polar
    }

    x_polar_x(m + 1) = omega_0; // omega_0, (rad / sec)
    x_polar_y(m + 1) = omega_0; // omega_0, (rad / sec)
    x_polar_z(m + 1) = omega_0; // omega_0, (rad / sec)
    x_polar_xaxis(m + 1) = omega_0; // omega_0, (rad / sec)
    x_polar_yaxis(m + 1) = omega_0; // omega_0, (rad / sec)
    x_polar_zaxis(m + 1) = omega_0; // omega_0, (rad / sec)
    x_polar_angle(m + 1) = omega_0; // omega_0, (rad / sec)

    for (int i = 0; i < m; i++)
    {
        x_polar_x(i + m + 2) = (i + 1.0)*omega_0*N_initpts*delta_t + atan2(x_init_x(i + m + 1), x_init_x(i + 1)); // theta = i*omega*T + phi, (rad)
        x_polar_y(i + m + 2) = (i + 1.0)*omega_0*N_initpts*delta_t + atan2(x_init_y(i + m + 1), x_init_y(i + 1)); // theta = i*omega*T + phi, (rad)
        x_polar_z(i + m + 2) = (i + 1.0)*omega_0*N_initpts*delta_t + atan2(x_init_z(i + m + 1), x_init_z(i + 1)); // theta = i*omega*T + phi, (rad)
        x_polar_xaxis(i + m + 2) = (i + 1.0)*omega_0*N_initpts*delta_t + atan2(x_init_xaxis(i + m + 1), x_init_xaxis(i + 1)); // theta = i*omega*T + phi, (rad)
        x_polar_yaxis(i + m + 2) = (i + 1.0)*omega_0*N_initpts*delta_t + atan2(x_init_yaxis(i + m + 1), x_init_yaxis(i + 1)); // theta = i*omega*T + phi, (rad)
        x_polar_zaxis(i + m + 2) = (i + 1.0)*omega_0*N_initpts*delta_t + atan2(x_init_zaxis(i + m + 1), x_init_zaxis(i + 1)); // theta = i*omega*T + phi, (rad)
        x_polar_angle(i + m + 2) = (i + 1.0)*omega_0*N_initpts*delta_t + atan2(x_init_angle(i + m + 1), x_init_angle(i + 1)); // theta = i*omega*T + phi, (rad)
    }


    // return output
    std::shared_ptr< std::vector<double> > outputs(std::make_shared< std::vector<double> >());

    //outputs->insert(outputs->end(), z_init_x.data(), z_init_x.data() + z_init_x.size());
    //outputs->insert(outputs->end(), z_init_y.data(), z_init_y.data() + z_init_y.size());
    //outputs->insert(outputs->end(), z_init_z.data(), z_init_z.data() + z_init_z.size());
    //outputs->insert(outputs->end(), z_init_xaxis.data(), z_init_xaxis.data() + z_init_xaxis.size());
    //outputs->insert(outputs->end(), z_init_yaxis.data(), z_init_yaxis.data() + z_init_yaxis.size());
    //outputs->insert(outputs->end(), z_init_zaxis.data(), z_init_zaxis.data() + z_init_zaxis.size());
    //outputs->insert(outputs->end(), z_init_angle.data(), z_init_angle.data() + z_init_angle.size());

    outputs->insert(outputs->end(), x_init_x.data(), x_init_x.data() + x_init_x.size());
    outputs->insert(outputs->end(), x_init_y.data(), x_init_y.data() + x_init_y.size());
    outputs->insert(outputs->end(), x_init_z.data(), x_init_z.data() + x_init_z.size());
    outputs->insert(outputs->end(), x_init_xaxis.data(), x_init_xaxis.data() + x_init_xaxis.size());
    outputs->insert(outputs->end(), x_init_yaxis.data(), x_init_yaxis.data() + x_init_yaxis.size());
    outputs->insert(outputs->end(), x_init_zaxis.data(), x_init_zaxis.data() + x_init_zaxis.size());
    outputs->insert(outputs->end(), x_init_angle.data(), x_init_angle.data() + x_init_angle.size());
    outputs->insert(outputs->end(), x_polar_x.data(), x_polar_x.data() + x_polar_x.size());
    outputs->insert(outputs->end(), x_polar_y.data(), x_polar_y.data() + x_polar_y.size());
    outputs->insert(outputs->end(), x_polar_z.data(), x_polar_z.data() + x_polar_z.size());
    outputs->insert(outputs->end(), x_polar_xaxis.data(), x_polar_xaxis.data() + x_polar_xaxis.size());
    outputs->insert(outputs->end(), x_polar_yaxis.data(), x_polar_yaxis.data() + x_polar_yaxis.size());
    outputs->insert(outputs->end(), x_polar_zaxis.data(), x_polar_zaxis.data() + x_polar_zaxis.size());
    outputs->insert(outputs->end(), x_polar_angle.data(), x_polar_angle.data() + x_polar_angle.size());

//    qint64 elNsec = elTimer.nsecsElapsed();

//    qDebug() << "Nsec elapsed:" << elNsec;

    return outputs;
}


inline const QString getCurrTimeStr()
{
    return QTime::currentTime().toString("HH.mm.ss.zzz");
}

inline const QString getCurrDateTimeStr()
{
    return QDateTime::currentDateTime().toString("dd/MM/yyyy - hh:mm:ss.zzz");
}