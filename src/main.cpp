#include "F1Telemetry.h"
#include <QApplication>

const QString& VERSION = "1.0";

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	a.setOrganizationName("Pixelgames");
	a.setApplicationName("PXG F1 Telemetry");
	a.setApplicationVersion(VERSION);

	F1Telemetry w;
	w.show();

	return a.exec();
}
