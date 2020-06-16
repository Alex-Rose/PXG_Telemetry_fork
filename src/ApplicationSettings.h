#ifndef APPLICATIONSETTINGS_H
#define APPLICATIONSETTINGS_H


#include <QColor>
#include <QSettings>


class ApplicationSettings
{
  public:
	ApplicationSettings();

    virtual ~ApplicationSettings() = default;

    virtual void init();

    virtual void reset() {}
    void clear();

	bool isEmpty() const;

    void importFromFile(const QString &settingFile);
    void importFromSettings(const ApplicationSettings &settings);
    void exportToFile(const QString &settingFile) const;

    void setValue(const QString &key, const QVariant &value);
    void setColorValue(const QString &key, const QColor &color);
	void setColorListValue(const QString &key, const QList<QColor> &colors);

    QVariant value(const QString &key, const QVariant &defaultValue = QVariant()) const;
    QColor colorValue(const QString &key, const QColor &defaultColor = QColor()) const;
	QList<QColor> colorListValue(const QString &key, const QList<QColor> &defaultColorList = QList<QColor>()) const;

	void beginGroup(const QString &groupName);
	void endGroup();

  protected:
    QSettings m_settings;
};

#endif // APPLICATIONSETTINGS_H
