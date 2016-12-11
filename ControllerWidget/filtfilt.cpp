#include "filtfilt.h"

filtfilt::filtfilt()
{
    m_A = {1};

    // From MATLAB : m_B = fir1(50,(100/60/2)/(150/2),'low'); // order 50, 100 bpm, 150 Hz sampling
//    m_B = {0.00264726249703924,           0.00279642405272151,           0.00319013679531822,           0.00382941384359819,
//           0.00471069197708524,           0.00582582662708399,           0.00716217671948781,           0.00870277868575645,
//           0.0104266071117473,            0.0123089176854006,            0.0143216663725929,            0.0164339971333124,
//           0.0186127890229487,            0.0208232522381201,            0.0230295615915197,            0.0251955150597567,
//           0.0272852044611377,            0.0292636850004373,            0.0310976303728649,            0.0327559603516814,
//           0.0342104282892947,            0.0354361567303369,            0.0364121103516330,            0.0371214966872074,
//           0.0375520865406890,            0.0376964476024575,            0.0375520865406890,            0.0371214966872074,
//           0.0364121103516330,            0.0354361567303369,            0.0342104282892947,            0.0327559603516814,
//           0.0310976303728649,            0.0292636850004373,            0.0272852044611377,            0.0251955150597567,
//           0.0230295615915197,            0.0208232522381201,            0.0186127890229487,            0.0164339971333124,
//           0.0143216663725929,            0.0123089176854006,            0.0104266071117473,            0.00870277868575645,
//           0.00716217671948781,           0.00582582662708399,           0.00471069197708524,           0.00382941384359819,
//           0.00319013679531822,           0.00279642405272151,           0.00264726249703924};

    // From MATLAB : m_B = fir1(50,(120/60/2)/(42.7350/2),'low'); // order 50, 100 bpm, 42.7350 Hz sampling
    m_B = {-0.000569727095790538,       -0.000460005562187752,        -0.000341437553206024,        -0.000165785763108122,
           0.000123143497281698,         0.000586219883270480,         0.00128516073718559,          0.00227837404637298,
           0.00361672040102945,          0.00533945551423732,          0.00747061259853524,          0.0100160653579639,
           0.0129614783838126,           0.0162713041615132,           0.0198889274205894,           0.0237379916329247,
           0.0277248730484729,           0.0317421989856768,           0.0356732434065773,           0.0393969780822734,
           0.0427935153555936,           0.0457496513584374,           0.0481642083490973,           0.0499528823717932,
           0.0510523273876564,           0.0514232479879954,           0.0510523273876564,           0.0499528823717932,
           0.0481642083490973,           0.0457496513584374,           0.0427935153555936,           0.0393969780822734,
           0.0356732434065773,           0.0317421989856768,           0.0277248730484729,           0.0237379916329247,
           0.0198889274205894,           0.0162713041615132,           0.0129614783838126,           0.0100160653579639,
           0.00747061259853524,          0.00533945551423732,          0.00361672040102945,          0.00227837404637298,
           0.00128516073718559,          0.000586219883270480,         0.000123143497281698,        -0.000165785763108122,
           -0.000341437553206024,       -0.000460005562187752,        -0.000569727095790538};

    updateFilterParameters();
}

filtfilt::~filtfilt()
{

}


void filtfilt::add_index_range(std::vector<int> &indices, int beg, int end, int inc = 1)
{
    for (int i = beg; i <= end; i += inc)
    {
       indices.push_back(i);
    }
}

void filtfilt::add_index_const(std::vector<int> &indices, int value, size_t numel)
{
    while (numel--)
    {
        indices.push_back(value);
    }
}

void filtfilt::append_vector(std::vector<double> &vec, const std::vector<double> &tail)
{
    vec.insert(vec.end(), tail.begin(), tail.end());
}

std::vector<double> filtfilt::subvector_reverse(const std::vector<double> &vec, int idx_end, int idx_start)
{
    std::vector<double> result(&vec[idx_start], &vec[idx_end+1]);
    std::reverse(result.begin(), result.end());
    return result;
}

void filtfilt::filter(const std::vector<double> &X, std::vector<double> &Y, std::vector<double> &Zi)
{
    size_t input_size = X.size();
    //size_t filter_order = std::max(m_A.size(), m_B.size());
    size_t filter_order = m_nfilt;
//    m_B.resize(filter_order, 0);
//    m_A.resize(filter_order, 0);
    Zi.resize(filter_order, 0);
    Y.resize(input_size);

    const double *x = &X[0];
    const double *b = &m_B[0];
    const double *a = &m_A[0];
    double *z = &Zi[0];
    double *y = &Y[0];

    for (size_t i = 0; i < input_size; ++i)
    {
        size_t order = filter_order - 1;
        while (order)
        {
            if (i >= order)
            {
                z[order - 1] = b[order] * x[i - order] - a[order] * y[i - order] + z[order];
            }
            --order;
        }
        y[i] = b[0] * x[i] + z[0];
    }
    Zi.resize(filter_order - 1);
}

void filtfilt::run(const std::vector<double> &X, std::vector<double> &Y)
{
    using namespace Eigen;

    int len = X.size();     // length of input

    if (len <= m_nfact)
    {
        std::cerr << "Input data too short! Data must have length more than 3 times filter order." << std::endl;
    }

    std::vector<double> leftpad = subvector_reverse(X, m_nfact, 1);
    double _2x0 = 2 * X[0];
    std::transform(leftpad.begin(), leftpad.end(), leftpad.begin(), [_2x0](double val) {return _2x0 - val; });

    std::vector<double> rightpad = subvector_reverse(X, len - 2, len - m_nfact - 1);
    double _2xl = 2 * X[len-1];
    std::transform(rightpad.begin(), rightpad.end(), rightpad.begin(), [_2xl](double val) {return _2xl - val; });

    double y0;
    std::vector<double> signal1, signal2, zi;

    signal1.reserve(leftpad.size() + X.size() + rightpad.size());
    append_vector(signal1, leftpad);
    append_vector(signal1, X);
    append_vector(signal1, rightpad);

    zi.resize(m_zzi.size());

    // Do the forward and backward filtering
    y0 = signal1[0];
    std::transform(m_zzi.data(), m_zzi.data() + m_zzi.size(), zi.begin(), [y0](double val){ return val*y0; });
    filter(signal1, signal2, zi);
    std::reverse(signal2.begin(), signal2.end());
    y0 = signal2[0];
    std::transform(m_zzi.data(), m_zzi.data() + m_zzi.size(), zi.begin(), [y0](double val){ return val*y0; });
    filter(signal2, signal1, zi);
    Y = subvector_reverse(signal1, signal1.size() - m_nfact - 1, m_nfact);
}

void filtfilt::setFilterCoefficients(const std::vector<double> &B, const std::vector<double> &A)
{
    m_A = A;
    m_B = B;

    updateFilterParameters();
}

void filtfilt::updateFilterParameters()
{
    m_na = m_A.size();
    m_nb = m_B.size();
    m_nfilt = (m_nb > m_na) ? m_nb : m_na;
    m_nfact = 3 * (m_nfilt - 1); // length of edge transients

    // set up filter's initial conditions to remove DC offset problems at the
    // beginning and end of the sequence
    m_B.resize(m_nfilt, 0);
    m_A.resize(m_nfilt, 0);

    m_rows.clear(); m_cols.clear();

    //rows = [1:nfilt-1           2:nfilt-1             1:nfilt-2];
    add_index_range(m_rows, 0, m_nfilt - 2);
    if (m_nfilt > 2)
    {
        add_index_range(m_rows, 1, m_nfilt - 2);
        add_index_range(m_rows, 0, m_nfilt - 3);
    }
    //cols = [ones(1,nfilt-1)         2:nfilt-1          2:nfilt-1];
    add_index_const(m_cols, 0, m_nfilt - 1);
    if (m_nfilt > 2)
    {
        add_index_range(m_cols, 1, m_nfilt - 2);
        add_index_range(m_cols, 1, m_nfilt - 2);
    }
    // data = [1+a(2)         a(3:nfilt)        ones(1,nfilt-2)    -ones(1,nfilt-2)];

    auto klen = m_rows.size();
    m_data.clear();
    m_data.resize(klen);
    m_data[0] = 1 + m_A[1];  int j = 1;
    if (m_nfilt > 2)
    {
        for (int i = 2; i < m_nfilt; i++)
            m_data[j++] = m_A[i];
        for (int i = 0; i < m_nfilt - 2; i++)
            m_data[j++] = 1.0;
        for (int i = 0; i < m_nfilt - 2; i++)
            m_data[j++] = -1.0;
    }

    // Calculate initial conditions
    Eigen::MatrixXd sp = Eigen::MatrixXd::Zero(max_val(m_rows) + 1, max_val(m_cols) + 1);
    for (size_t k = 0; k < klen; ++k)
    {
        sp(m_rows[k], m_cols[k]) = m_data[k];
    }
    auto bb = Eigen::VectorXd::Map(m_B.data(), m_B.size());
    auto aa = Eigen::VectorXd::Map(m_A.data(), m_A.size());
    m_zzi = (sp.inverse() * (bb.segment(1, m_nfilt - 1) - (bb(0) * aa.segment(1, m_nfilt - 1))));



    if (m_A.empty())
    {
        std::cerr << "The feedback filter coefficients are empty." << std::endl;
    }
    if (std::all_of(m_A.begin(), m_A.end(), [](double coef){ return coef == 0; }))
    {
        std::cerr << "At least one of the feedback filter coefficients has to be non-zero." << std::endl;
    }
    if (m_A[0] == 0)
    {
        std::cerr << "First feedback coefficient has to be non-zero." << std::endl;
    }

    // Normalize feedback coefficients if a[0] != 1;
    auto a0 = m_A[0];
    if (a0 != 1.0)
    {
        std::transform(m_A.begin(), m_A.end(), m_A.begin(), [a0](double v) { return v / a0; });
        std::transform(m_B.begin(), m_B.end(), m_B.begin(), [a0](double v) { return v / a0; });
    }
}