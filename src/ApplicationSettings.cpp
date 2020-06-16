#include "ApplicationSettings.h"

#include <QApplication>
#include <QFile>
#include <QVector>

ApplicationSettings::ApplicationSettings() {}

void ApplicationSettings::init()
{
	if(m_settings.allKeys().isEmpty() || !QFile::exists(m_settings.fileName())) {
        reset();
    }

    m_settings.sync();
}

void ApplicationSettings::clear() { m_settings.clear(); }

bool ApplicationSettings::isEmpty() const { return m_settings.allKeys().isEmpty(); }

void ApplicationSettings::importFromFile(const QString &settingFile)
{
    QSettings importedSettings(settingFile, QSettings::IniFormat);
    const auto &allKeys = m_settings.allKeys();
	for(const auto &key : allKeys) {
        m_settings.setValue(key, importedSettings.value(key, m_settings.value(key)));
    }
}

void ApplicationSettings::importFromSettings(const ApplicationSettings &settings)
{
    const auto &allKeys = m_settings.allKeys();
	for(const auto &key : allKeys) {
        m_settings.setValue(key, settings.value(key, m_settings.value(key)));
    }
}

void ApplicationSettings::exportToFile(const QString &settingFile) const
{
    QSettings exportedSettings(settingFile, QSettings::IniFormat);
    const auto &allKeys = m_settings.allKeys();
	for(const auto &key : allKeys) {
        exportedSettings.setValue(key, m_settings.value(key));
    }
}

void ApplicationSettings::setValue(const QString &key, const QVariant &value) { m_settings.setValue(key, value); }

void ApplicationSettings::setColorValue(const QString &key, const QColor &color)
{
    m_settings.setValue(key, color.name());
}

void ApplicationSettings::setColorListValue(const QString &key, const QList<QColor> &colors)
{
    QVariantList values;
	for(const auto &color : colors) {
        values << color.name();
    }

    setValue(key, values);
}

QVariant ApplicationSettings::value(const QString &key, const QVariant &defaultValue) const
{
    return m_settings.value(key, defaultValue);
}

QColor ApplicationSettings::colorValue(const QString &key, const QColor &defaultColor) const
{
    auto colorName = value(key, defaultColor).toString();
	if(colorName.isEmpty()) {
        return defaultColor;
    }

    return {colorName};
}

QList<QColor> ApplicationSettings::colorListValue(const QString &key, const QList<QColor> &defaultColorList) const
{
	if(!m_settings.contains(key)) {
		return defaultColorList;
	}

    const auto list = value(key).toList();
	QList<QColor> colors;
	for(const auto &value : list) {
        colors << QColor(value.toString());
    }

    return colors;
}

void ApplicationSettings::beginGroup(const QString &groupName) { m_settings.beginGroup(groupName); }

void ApplicationSettings::endGroup() { m_settings.endGroup(); }
