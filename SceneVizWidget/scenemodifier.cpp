#include "scenemodifier.h"

SceneModifier::SceneModifier(Qt3DCore::QEntity *rootEntity)
    : m_rootEntity(rootEntity)
{
    // Y axis points up in OpenGL renderer
    m_entityList.append(new TriadEntity(m_rootEntity));
    m_entityList.append(new TriadEntity(m_rootEntity));
    m_entityList.append(new TriadEntity(m_rootEntity));
    m_entityList.append(new TriadEntity(m_rootEntity));
    m_entityList.append(new TriadEntity(m_rootEntity));
    //m_entityList.append(new usEntity(m_rootEntity));
    m_entityList.append(new usEntity(m_entityList[1])); // US plane attached to the BT

    static_cast<TriadEntity*>(m_entityList[0])->translate(QVector3D(0.0f, 0.0f, 0.0f));
    // EM_SENSOR_BB
    m_entityList[0]->setEnabled(false);
    m_entityList[1]->setEnabled(false);
    m_entityList[2]->setEnabled(false);
    m_entityList[3]->setEnabled(false);
    m_entityList[4]->setEnabled(true); // EM Box
    m_entityList[5]->setEnabled(true); // US plane

    Qt3DCore::QTransform tf;
    tf.setTranslation(QVector3D(0,0,0));
    static_cast<TriadEntity*>(m_entityList[4])->setTransformation(tf);
    static_cast<usEntity*>(m_entityList[5])->setTransformation(tf);


    //static_cast<usEntity*>(m_entityList[5])->setProperty();

    // dummy
    m_baseEMpose = tf.matrix();

    Qt3DCore::QTransform calib;
    calib.setRotation(QQuaternion(0.0098f, -0.0530f, -0.9873f, -0.1492f));

    m_calibMat = calib.matrix();

    m_tformOption = 0;

//    QMatrix4x4 T_Box_EM(-1,  0,  0, 0,
//                         0,  0, -1, 0,
//                         0, -1,  0, 0,
//                         0,  0,  0, 1);

    m_T_Box_EM = QMatrix4x4(0,  0,  1, 0,
                            0, -1,  0, 0,
                            1,  0,  0, 0,
                            0,  0,  0, 1);

    m_T_EM_CT = QMatrix4x4(1, 0, 0, 0,
                       0, 1, 0, 0,
                       0, 0, 1, 14.5,
                       0, 0, 0, 1);

    m_CT_US.setRotationZ(0);

    m_Box_US.setMatrix(m_T_Box_EM * m_T_EM_CT * m_CT_US.matrix()); //T_Box_EM

}

SceneModifier::~SceneModifier()
{
    //m_entityList.clear();
}

QVector3D SceneModifier::getTriadPosition(EM_SENSOR_IDS sensorID)
{
    return static_cast<TriadEntity*>(m_entityList[sensorID])->getTranslation();
}

void SceneModifier::enableObject(bool enabled, int objID)
{
//        // Look at this example for creating object classes
//        // http://doc.qt.io/qt-5/qt3drenderer-materials-cpp-barrel-cpp.html
//        // arrow class can be useful to create triads, and then a triad class
    Q_ASSERT(objID < m_entityList.size());

    m_entityList[objID]->setEnabled(enabled);
}

void SceneModifier::receiveEMreading(QTime timeStamp, int sensorID, DOUBLE_POSITION_MATRIX_TIME_Q_RECORD data)
{
    QMatrix4x4 tmp(data.s[0][0], data.s[0][1], data.s[0][2], data.x,
                   data.s[1][0], data.s[1][1], data.s[1][2], data.y,
                   data.s[2][0], data.s[2][1], data.s[2][2], data.z,
                            0.0,          0.0,          0.0, 1.0);

    Qt3DCore::QTransform tf;
    tf.setMatrix(tmp);
//    tf.setRotation(QQuaternion(data.q[0],data.q[1],data.q[2],data.q[3]));
//    tf.setTranslation(QVector3D(data.x,data.y,data.z));

    if(m_tformOption == 0)
        tf.setMatrix(tf.matrix());
    else if(m_tformOption == 1)
        tf.setMatrix(m_calibMat*tf.matrix()*m_Box_US.matrix());
    else if(m_tformOption == 2)
        tf.setMatrix(m_baseEMpose.inverted()*m_T_Box_EM.inverted() * tf.matrix()*m_Box_US.matrix());
    else
        tf.setMatrix(tf.matrix());

    static_cast<TriadEntity*>(m_entityList[sensorID])->setTransformation(tf);

//    static_cast<TriadEntity*>(m_entityList[sensorID])->rotate(QQuaternion(data.q[0],data.q[1],data.q[2],data.q[3]));
    //    static_cast<TriadEntity*>(m_entityList[sensorID])->translate(QVector3D(data.x,data.y,data.z));
}

void SceneModifier::receiveEMreading(std::vector<DOUBLE_POSITION_MATRIX_TIME_Q_RECORD> data)
{
    Q_ASSERT(data.size() == 4);

    for(size_t i = 0; i < data.size(); i++)
    {
        QMatrix4x4 tmp(data[i].s[0][0], data[i].s[0][1], data[i].s[0][2], data[i].x,
                       data[i].s[1][0], data[i].s[1][1], data[i].s[1][2], data[i].y,
                       data[i].s[2][0], data[i].s[2][1], data[i].s[2][2], data[i].z,
                             0.0,      0.0,        0.0, 1.0);

        Qt3DCore::QTransform tf;
        tf.setMatrix(tmp);

        if(m_tformOption == 0)
            tf.setMatrix(tf.matrix());
        else if(m_tformOption == 1)
            tf.setMatrix(m_calibMat*tf.matrix()*m_Box_US.matrix());
        else if(m_tformOption == 2)
            tf.setMatrix(m_baseEMpose.inverted()*m_T_Box_EM.inverted() * tf.matrix()*m_Box_US.matrix());
        else
            tf.setMatrix(tf.matrix());

        static_cast<TriadEntity*>(m_entityList[i])->setTransformation(tf);
    }
}

void SceneModifier::resetBase()
{
    Qt3DCore::QTransform tf;
    tf.setRotation(static_cast<TriadEntity*>(m_entityList[EM_SENSOR_BB])->getRotation());
    tf.setTranslation(static_cast<TriadEntity*>(m_entityList[EM_SENSOR_BB])->getTranslation());
    m_baseEMpose = tf.matrix();
}

void SceneModifier::changeTFormOption(int opt)
{
    std::cout << "m_tformOption changed to " << opt << std::endl;
    m_tformOption = opt;
}

void SceneModifier::setUSangle(int ang)
{
    std::cout << "US angle changed to " << ang << std::endl;
    m_usAngle = ang;
    m_CT_US.setRotationZ(m_usAngle);
    m_Box_US.setMatrix(m_T_Box_EM * m_T_EM_CT * m_CT_US.matrix()); //T_Box_EM
}


