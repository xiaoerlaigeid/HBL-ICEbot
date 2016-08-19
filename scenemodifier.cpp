#include "scenemodifier.h"

SceneModifier::SceneModifier(Qt3DCore::QEntity *rootEntity)
    : m_rootEntity(rootEntity)
{
    m_entityList.append(new TriadEntity(m_rootEntity));
    m_entityList.append(new TriadEntity(m_rootEntity));
    m_entityList.append(new TriadEntity(m_rootEntity));
    m_entityList.append(new TriadEntity(m_rootEntity));

    static_cast<TriadEntity*>(m_entityList[0])->translate(QVector3D(5.0f, 0.0f, 0.0f));
    // EM_SENSOR_BB
    m_entityList[0]->setEnabled(false);
    m_entityList[1]->setEnabled(false);
    m_entityList[2]->setEnabled(false);
    m_entityList[3]->setEnabled(false);
}

SceneModifier::~SceneModifier()
{

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

void SceneModifier::receiveEMreading(QTime timeStamp, int sensorID, DOUBLE_POSITION_QUATERNION_TIME_Q_RECORD data)
{
//    Qt3DCore::QTransform tf;
//    tf.setRotation(QQuaternion(data.q[0],data.q[1],data.q[2],data.q[3]));
//    tf.setTranslation(QVector3D(data.x,data.y,data.z));
//    static_cast<TriadEntity*>(m_entityList[sensorID])->setTransformation(tf);

    static_cast<TriadEntity*>(m_entityList[sensorID])->rotate(QQuaternion(data.q[0],data.q[1],data.q[2],data.q[3]));
    static_cast<TriadEntity*>(m_entityList[sensorID])->translate(QVector3D(data.x,data.y,data.z));
}


