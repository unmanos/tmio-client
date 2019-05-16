#ifndef CHARTSETTINGSPOPUP_H
#define CHARTSETTINGSPOPUP_H

#include <QDialog>

namespace Ui {
class ChartSettingsPopup;
}

class ChartSettingsPopup : public QDialog
{
    Q_OBJECT

public:
    explicit ChartSettingsPopup(QWidget *parent = 0);
    ~ChartSettingsPopup();

    struct              Settings
    {
        int             bollingerHistory;
        double          bollingerMultiplier;
        bool            bollingerEnable;
        bool            smaShortEnable;
        int             smaShortHistory;
        bool            smaMiddleEnable;
        int             smaMiddleHistory;
        bool            smaLongEnable;
        int             smaLongHistory;

        bool            vmaShortEnable;
        int             vmaShortHistory;
        bool            vmaLongEnable;
        int             vmaLongHistory;

        int             macdShortHistory;
        int             macdLongHistory;
        int             macdSignalHistory;
        double          macdVolumeWeight;
        Settings()
        {
            bollingerEnable     =true;
            bollingerHistory    =20;
            bollingerMultiplier =2;
            smaShortEnable      =true;
            smaShortHistory     =7;
            smaMiddleEnable     =true;
            smaMiddleHistory    =30;
            smaLongEnable       =true;
            smaLongHistory      =100;
            macdShortHistory    =12;
            macdLongHistory     =26;
            macdSignalHistory   =9;
            macdVolumeWeight    =0;
            vmaShortEnable      =true;
            vmaShortHistory     =10;
            vmaLongEnable       =true;
            vmaLongHistory      =100;
        }
    };

    const Settings&     settings();

public slots:
    void                onChanged();

signals:
    void                changed(const ChartSettingsPopup::Settings&);

private:
    Ui::ChartSettingsPopup *ui;
    Settings                _settings;
};

#endif // CHARTSETTINGSPOPUP_H
