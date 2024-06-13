﻿#include "mainwindow.h"
#include "mysettings.h"

#include <QApplication>
#include <QDir>
#include <QTranslator>
#include <QFileInfo>
#include <QStandardPaths>
#include <QCommandLineParser>

int main(int argc, char *argv[])
{
#ifndef Q_OS_ANDROID
    // A trick to handle non-ascii path
    // The application cannot find the plugins when the path contains non ascii characters.
    // However, the plugins will be load after creating MainWindow(or QApplication?).
    // QDir will handle the path correctly.
    QDir* pluginDir = new QDir;
    if(pluginDir->cd("plugins")) // has plugins folder
    {
        qputenv("QT_PLUGIN_PATH", pluginDir->absolutePath().toLocal8Bit());
    }
    delete pluginDir;
#endif

    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication a(argc, argv);

#ifdef Q_OS_ANDROID
    // on Android, use default.
    MySettings::init(QSettings::NativeFormat);
#else

    // translator is not loaded there.
    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addOption({"config-path",
                      "Use specified file as config file",
                      "file path"});
    parser.process(a);

    // on PC, store preferences in files for portable use
    if(parser.isSet("config-path"))
    {
        qDebug() << "Config file path:" << parser.value("config-path");
        MySettings::init(QSettings::IniFormat, parser.value("config-path"));
    }
    else
    {
        // Firstly, find it in current working directory
        QString configPath = "preference.ini";
        if(!QFileInfo::exists(configPath))
        {
            // Then, find it in AppConfigLocation
            configPath = QStandardPaths::locate(QStandardPaths::AppConfigLocation, "preference.ini");
            if(configPath.isEmpty() || !QFileInfo::exists(configPath))
            {
                // If no config file is found, create one in current working directory
                configPath = "preference.ini";
            }
        }
        MySettings::init(QSettings::IniFormat, configPath);
    }

#endif

    MySettings* m_settings = MySettings::defaultSettings();
    // set language by config file
    QTranslator translator;
    m_settings->beginGroup("SerialTest");
    QString language = m_settings->value("Lang_Name").toString();
    QString languageFile = m_settings->value("Lang_Path").toString();
    m_settings->endGroup();

    bool languageSet = false;
    if(language == "(sys)")
        languageSet = false;
    else if(language == "zh_CN")
        languageSet = translator.load(":/i18n/SerialTest_zh_CN.qm");
    else if(language == "zh_TW")
        languageSet = translator.load(":/i18n/SerialTest_zh_TW.qm");
    else if(language == "nb_NO")
        languageSet = translator.load(":/i18n/SerialTest_nb_NO.qm");
    else if(language == "en")
        languageSet = true;
    else if(language == "(ext)")
        languageSet = translator.load(languageFile);

    if(!languageSet)
    {
        // set language by system locale
        QLocale locale = QLocale::system();
        QLocale::Country countryOrRegion = locale.country();
        if(locale.language() == QLocale::Chinese || countryOrRegion == QLocale::China)
            languageSet = translator.load(":/i18n/SerialTest_zh_CN.qm");
        else if(locale.language() == QLocale::Chinese || countryOrRegion == QLocale::Taiwan)
            languageSet = translator.load(":/i18n/SerialTest_zh_TW.qm");
        else if(countryOrRegion == QLocale::Norway)
            languageSet = translator.load(":/i18n/SerialTest_nb_NO.qm");
    }
    if(languageSet)
        a.installTranslator(&translator);

    m_settings = nullptr;

    MainWindow w;
    w.show();
    return a.exec();
}
