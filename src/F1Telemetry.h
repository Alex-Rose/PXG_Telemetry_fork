#ifndef F1TELEMETRY_H
#define F1TELEMETRY_H

#include "F1Listener.h"

#include <QCheckBox>
#include <QMainWindow>
#include <QMessageBox>
#include <QNetworkAccessManager>

namespace Ui
{
class F1Telemetry;
}

class Tracker;
class FileDownloader;
class CheckUpdatesDialog;
struct TrackedCars;

class F1Telemetry : public QMainWindow
{
	Q_OBJECT

  public:
	explicit F1Telemetry(QWidget *parent = nullptr);
	~F1Telemetry();

  private:
	Ui::F1Telemetry *ui;
	F1Listener *_listener = nullptr;
	Tracker *_tracker;
	FileDownloader *_downloader;
	CheckUpdatesDialog *_updateDialog;
	bool _isAutoCheckUpdates = false;

	enum DownloadingFileTypes { VersionFile, ChangelogFile };

	void loadSettings();
	void saveSetings();
	void initDefaultSettings();

	void initMenu();

	bool isGreaterVersion(const QString &version);

  protected:
	void closeEvent(QCloseEvent *event);

  private slots:
	void buildListener();
	void startTracking(const TrackedCars &cars);

	void checkUpdates();
	void fileDownloaded(int type, const QByteArray &data);
	void showChangeLog();
	void changelogAutoDisplay();
	void contact();
	void editTheme();
	void updateTheme();
	void editPreferences();
};

#endif // F1TELEMETRY_H
