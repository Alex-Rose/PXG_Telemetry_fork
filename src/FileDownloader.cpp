#include "FileDownloader.h"

#include <QNetworkReply>

FileDownloader::FileDownloader(QObject *parent) : QObject(parent) { _network = new QNetworkAccessManager(this); }

void FileDownloader::downloadFile(const QUrl &url, int type)
{
	auto networkReply = _network->get(QNetworkRequest(url));
	connect(networkReply, &QNetworkReply::finished, this, &FileDownloader::requestFinished);

	_replies[networkReply] = type;
}

void FileDownloader::requestFinished()
{
	auto reply = qobject_cast<QNetworkReply *>(sender());

	if(reply) {
		auto type = _replies.value(reply);
		auto data = reply->readAll();
		emit fileDownloaded(type, data);
		reply->deleteLater();
	} else {
		qCritical("Error downloading file");
	}
}
