#include "gainswidget.h"
#include "ui_gainswidget.h"

gainsWidget::gainsWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::gainsWidget)
{
    ui->setupUi(this);
    this->setWindowTitle("Adjust Gains");

    qRegisterMetaType<GainsPYRT>("GainsPYRT");
    qRegisterMetaType<ConvergenceLimits>("ConvergenceLimits");

}

gainsWidget::~gainsWidget()
{
    delete ui;
}

void gainsWidget::on_closeButton_clicked()
{
    emit closeGainsWindow();
}

void gainsWidget::closeEvent(QCloseEvent *event)
{
    emit closeGainsWindow();
    event->accept();
}

void gainsWidget::on_setGainsButton_clicked()
{
    GainsPYRT gains;

    gains.kPitchMin = ui->gainPitchMinSpinbox->value();
    gains.kPitch = gains.kPitchMin;
    gains.kPitchMax = ui->gainPitchMaxSpinbox->value();

    gains.kYawMin = ui->gainYawMinSpinbox->value();
    gains.kYaw = gains.kYawMin;
    gains.kYawMax = ui->gainYawMaxSpinbox->value();

    gains.kRollMin = ui->gainRollMinSpinbox->value();
    gains.kRoll = gains.kRollMin;
    gains.kRollMax = ui->gainRollMaxSpinbox->value();

    gains.kTransMin = ui->gainTransMinSpinbox->value();
    gains.kTrans = gains.kTransMin;
    gains.kTransMax = ui->gainTransMaxSpinbox->value();

    emit setGains(gains);
}

void gainsWidget::on_setLimitsButton_clicked()
{
    ConvergenceLimits limits;

    limits.posMin = ui->posConvMinSpinbox->value();
    limits.posMax = ui->posConvMaxSpinbox->value();
    limits.angleMin = ui->angConvMinSpinbox->value();
    limits.angleMax = ui->angConvMaxSpinbox->value();

    emit setLimits(limits);
}
