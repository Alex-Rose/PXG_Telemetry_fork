#include "CustomTheme.h"

CustomTheme::CustomTheme() {}

bool CustomTheme::isValid() const
{
	return backgroundColor.isValid() && textColor.isValid() && gridColor.isValid() && !seriesColors.isEmpty();
}

void CustomTheme::apply(QtCharts::QChart *chart) const
{
	chart->setPlotAreaBackgroundBrush(QBrush(backgroundColor));
	chart->setBackgroundBrush(QBrush(backgroundColor));
	chart->setTitleBrush(QBrush(textColor));

	const auto allAxes = chart->axes();
	for(const auto &axis : allAxes) {
		axis->setTitleBrush(QBrush(textColor));
		axis->setLabelsColor(textColor);
		axis->setLinePenColor(textColor);
		axis->setGridLineColor(gridColor);
		axis->setLabelsBrush(QBrush(textColor));
		axis->setGridLinePen(QPen(gridColor));
	}
}

bool CustomTheme::operator==(const CustomTheme &other) const
{
	return backgroundColor == other.backgroundColor && textColor == other.textColor && gridColor == other.gridColor &&
		   seriesColors == other.seriesColors;
}

bool CustomTheme::operator!=(const CustomTheme &other) const { return !(other == *this); }

CustomTheme CustomTheme::defaultTheme()
{
	CustomTheme theme;

	theme.backgroundColor = Qt::white;
	theme.gridColor = Qt::black;
	theme.textColor = Qt::black;
	theme.seriesColors = {Qt::red, Qt::green, Qt::blue, Qt::yellow, Qt::cyan};

	return theme;
}
