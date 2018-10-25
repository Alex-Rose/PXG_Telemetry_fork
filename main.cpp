#include "F1Telemetry.h"
#include <QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	F1Telemetry w;
	w.show();

	return a.exec();
}
