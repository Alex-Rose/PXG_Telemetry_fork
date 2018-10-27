#include "F1Telemetry.h"
#include <QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	a.setOrganizationName("Pixelgames");
	a.setApplicationName("F1Telemetry");

	F1Telemetry w;
	w.show();

	return a.exec();
}
