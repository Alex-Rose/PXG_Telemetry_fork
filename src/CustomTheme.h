#ifndef CUSTOMTHEME_H
#define CUSTOMTHEME_H

#include <QChart>
#include <QColor>


class CustomTheme
{
  public:
	CustomTheme();

	bool isValid() const;

	QColor backgroundColor;
	QColor gridColor;
	QColor textColor;
	QList<QColor> seriesColors;

	void apply(QtCharts::QChart *chart) const;

	bool operator==(const CustomTheme &other) const;
	bool operator!=(const CustomTheme &other) const;

	static CustomTheme defaultTheme();
};

#endif // CUSTOMTHEME_H
