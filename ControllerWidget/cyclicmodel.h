#ifndef CYCLICMODEL_H
#define CYCLICMODEL_H

#include <vector>
//#include <deque>
#include <algorithm>
#include <iostream>
#include <memory>
#include <limits>
#include <Eigen/Dense>
#include <Eigen/StdDeque>
#include <QString>
#include <QDir>
#include <QFile>
#include <QDataStream>
#include <QTextStream>
#include <QDebug>
#include <QElapsedTimer>

#include "filtfilt.h"

#define N_HARMONICS 4              // NUM_HARM
#define N_STATES 10                // NUM_STATES
#define SAMPLE_DELTA_TIME 0.005992 // SAMPLE_TIME
#define N_SAMPLES 4000             // CYCLE_DATA_SIZE
#define EDGE_EFFECT 35             // EDGE_EFFECT
#define FILTER_ORDER 50            // FILTERORDER
#define BREATH_RATE 7.5            // BREATHRATE
#define PEAK_THRESHOLD 0.80        // For peak detection

typedef Eigen::Transform<double,3,Eigen::Affine> EigenAffineTransform3d;
// in order to use deque with Eigen Transform, we need this typedef
typedef std::deque< EigenAffineTransform3d, Eigen::aligned_allocator<EigenAffineTransform3d> > EigenDequeAffineTform3d;

class CyclicModel
{
public:
    CyclicModel();
    ~CyclicModel();

    void operator = (const CyclicModel &Other);

    void addObservation(const EigenAffineTransform3d &T_BB_CT_curTipPos,
                        const EigenAffineTransform3d &T_BB_targetPos,
                        const EigenAffineTransform3d &T_Box_BBmobile,
                        const EigenAffineTransform3d &T_BB_Box,
                        const EigenAffineTransform3d &T_Bird4,
                        const double sampleTime);
    void trainModel(const std::vector<double> data);
    void updatePeriod(const double shift);
    double getPrediction(const double timeShift,
                         const std::vector<double> &x_polar,
                         const std::vector<double> &x_rect);

    // Accessors
    const size_t getNumSamples() { return m_numSamples; }
    const bool isTrained() { return m_isTrained; }

    // testing function - external data load and save
    void loadData(QString filename, std::vector<double> &X);
    void saveFilteredData(QString filenam, const std::vector<double> &Y);
    void testLPF();

private:
    void retrainModel();

    std::shared_ptr< std::vector<double> > cycle_recalculate(const std::vector<double> &inputs);

    // data members
    // in order to use deque with Eigen Transform, we need the typedef above
    EigenDequeAffineTform3d m_BBfixed_CT,    // this stores all of the incoming CT points
                            m_BBfixed_Instr, // this stores all of the incoming instr_x points
                            m_BBfixed_BB,    // this stores all of the incoming BB points
                            m_Bird4;         // this stores all of the incoming 4th EM sensor points

    std::deque<double> m_timeData, m_breathingSignal; // stores time, and data to transfer to period updater

    size_t m_numSamples;

    bool m_isTrained;
    double m_lastTrainingTimestamp;

public:
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

#endif // CYCLICMODEL_H
