#include "F1Telemetry.h"
#include <QApplication>
#include <QFile>

QString readVersion()
{
	QFile versionFile(":/version");
	if(versionFile.open(QIODevice::ReadOnly)) {
		return versionFile.readAll();
	}

	return QString();
}

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	a.setOrganizationName("Pixelgames");
	a.setApplicationName("PXG F1 Telemetry");
	a.setApplicationVersion(readVersion());

	F1Telemetry w;
	w.show();

	return a.exec();
}
