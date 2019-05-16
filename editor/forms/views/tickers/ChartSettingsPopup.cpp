#include <QDebug>
#include "ChartSettingsPopup.h"
#include "ui_ChartSettingsPopup.h"

//
ChartSettingsPopup::ChartSettingsPopup(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ChartSettingsPopup)
{
    ui->setupUi(this);
    // bollinger
    ui->bollingerHistory->setValue(_settings.bollingerHistory);
    ui->bollingerMultiplier->setValue(_settings.bollingerMultiplier);
    ui->bollingerMultiplier->setSingleStep(0.1);
    ui->bollingerEnable->setChecked(_settings.bollingerEnable);
    connect(ui->bollingerHistory,SIGNAL(valueChanged(int)),this,SLOT(onChanged()));
    connect(ui->bollingerMultiplier,SIGNAL(valueChanged(double)),this,SLOT(onChanged()));
    connect(ui->bollingerEnable,SIGNAL(clicked(bool)),this,SLOT(onChanged()));
    // sma
    ui->smaShortEnable->setChecked(_settings.smaShortEnable);
    ui->smaShortHistory->setValue(_settings.smaShortHistory);
    connect(ui->smaShortEnable,SIGNAL(clicked(bool)),this,SLOT(onChanged()));
    connect(ui->smaShortHistory,SIGNAL(valueChanged(int)),this,SLOT(onChanged()));
    ui->smaMiddleEnable->setChecked(_settings.smaMiddleEnable);
    ui->smaMiddleHistory->setValue(_settings.smaMiddleHistory);
    connect(ui->smaMiddleEnable,SIGNAL(clicked(bool)),this,SLOT(onChanged()));
    connect(ui->smaMiddleHistory,SIGNAL(valueChanged(int)),this,SLOT(onChanged()));
    ui->smaLongEnable->setChecked(_settings.smaLongEnable);
    ui->smaLongHistory->setValue(_settings.smaLongHistory);
    connect(ui->smaLongEnable,SIGNAL(clicked(bool)),this,SLOT(onChanged()));
    connect(ui->smaLongHistory,SIGNAL(valueChanged(int)),this,SLOT(onChanged()));
    // vma
    ui->vmaShortEnable->setChecked(_settings.vmaShortEnable);
    ui->vmaShortHistory->setValue(_settings.vmaShortHistory);
    connect(ui->vmaShortEnable,SIGNAL(clicked(bool)),this,SLOT(onChanged()));
    connect(ui->vmaShortHistory,SIGNAL(valueChanged(int)),this,SLOT(onChanged()));
    ui->vmaLongEnable->setChecked(_settings.vmaLongEnable);
    ui->vmaLongHistory->setValue(_settings.vmaLongHistory);
    connect(ui->vmaLongEnable,SIGNAL(clicked(bool)),this,SLOT(onChanged()));
    connect(ui->vmaLongHistory,SIGNAL(valueChanged(int)),this,SLOT(onChanged()));
    // macd
    ui->macdShortHistory->setValue(_settings.macdShortHistory);
    connect(ui->macdShortHistory,SIGNAL(valueChanged(int)),this,SLOT(onChanged()));
    ui->macdLongHistory->setValue(_settings.macdLongHistory);
    connect(ui->macdLongHistory,SIGNAL(valueChanged(int)),this,SLOT(onChanged()));
    ui->macdSignalHistory->setValue(_settings.macdSignalHistory);
    connect(ui->macdSignalHistory,SIGNAL(valueChanged(int)),this,SLOT(onChanged()));
    ui->macdVolumeWeight->setValue((int)(_settings.macdVolumeWeight*100));
    connect(ui->macdVolumeWeight,SIGNAL(valueChanged(int)),this,SLOT(onChanged()));


    //setAttribute(Qt::WA_TranslucentBackground);
    setWindowOpacity(0.9);

    onChanged();
}

//
const ChartSettingsPopup::Settings&     ChartSettingsPopup::settings()
{
    return(_settings);
}

//
ChartSettingsPopup::~ChartSettingsPopup()
{
    delete ui;
}

//
void                ChartSettingsPopup::onChanged()
{
    qDebug() << "Changed values";
    // bollinger
    _settings.bollingerEnable=(ui->bollingerEnable->checkState()==Qt::Checked);
    _settings.bollingerHistory=ui->bollingerHistory->value();
    _settings.bollingerMultiplier=ui->bollingerMultiplier->value();
    ui->bollingerHistory->setEnabled(_settings.bollingerEnable);
    ui->bollingerMultiplier->setEnabled(_settings.bollingerEnable);
    // sma
    _settings.smaShortEnable=(ui->smaShortEnable->checkState()==Qt::Checked);
    _settings.smaShortHistory=ui->smaShortHistory->value();
    ui->smaShortHistory->setEnabled(_settings.smaShortEnable);
    _settings.smaMiddleEnable=(ui->smaMiddleEnable->checkState()==Qt::Checked);
    _settings.smaMiddleHistory=ui->smaMiddleHistory->value();
    ui->smaMiddleHistory->setEnabled(_settings.smaMiddleEnable);
    _settings.smaLongEnable=(ui->smaLongEnable->checkState()==Qt::Checked);
    _settings.smaLongHistory=ui->smaLongHistory->value();
    ui->smaLongHistory->setEnabled(_settings.smaLongEnable);
    // vma
    _settings.vmaShortEnable=(ui->vmaShortEnable->checkState()==Qt::Checked);
    _settings.vmaShortHistory=ui->vmaShortHistory->value();
    ui->vmaShortHistory->setEnabled(_settings.vmaShortEnable);
    _settings.vmaLongEnable=(ui->vmaLongEnable->checkState()==Qt::Checked);
    _settings.vmaLongHistory=ui->vmaLongHistory->value();
    ui->vmaLongHistory->setEnabled(_settings.vmaLongEnable);
    // macd
    _settings.macdShortHistory=ui->macdShortHistory->value();
    ui->macdLongHistory->setMinimum(ui->macdShortHistory->value()+1);
    _settings.macdLongHistory=ui->macdLongHistory->value();
    _settings.macdSignalHistory=ui->macdSignalHistory->value();
    _settings.macdVolumeWeight=((double)ui->macdVolumeWeight->value())/100.0;

    emit changed(_settings);
    //EG_TRACE();
}
