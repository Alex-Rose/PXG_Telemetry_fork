#ifndef FILEDOWNLOADER_H
#define FILEDOWNLOADER_H

#include <QNetworkAccessManager>
#include <QObject>

class FileDownloader : public QObject
{
	Q_OBJECT

  signals:
	void fileDownloaded(int type, const QByteArray &data);

  public:
	explicit FileDownloader(QObject *parent);

	void downloadFile(const QUrl &url, int type = 0);

  private:
	QNetworkAccessManager *_network;
	QHash<QNetworkReply *, int> _replies;

  private slots:
	void requestFinished();
};

#endif // FILEDOWNLOADER_H
