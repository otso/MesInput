#include "mesfetcher.h"
#include <QtCore>
#include <QtNetwork>
#include <QByteArray>
#include <QFile>
#include <MLabel>

MesFetcher::MesFetcher() : networkManager(new QNetworkAccessManager)
{
}

MesFetcher::~MesFetcher()
{
    if(networkManager)
    {
        delete networkManager;
        networkManager = 0;
    }
}

bool MesFetcher::init(MLabel *currentStatus)
{
    this->currentStatus = currentStatus;

    bool bRet = false;
    // Initialize proxy settings
    QUrl proxyUrl(getenv("http_proxy"));
    if(proxyUrl.isValid())
    {
	qDebug() << "Setting proxy::" << proxyUrl.host() << proxyUrl.port();
        QNetworkProxy proxy(QNetworkProxy::HttpProxy, proxyUrl.host(), proxyUrl.port(), proxyUrl.userName(), proxyUrl.password());
        //QNetworkProxy::setApplicationProxy(proxy);
        networkManager->setProxy(proxy);
    }
    
    // Connect manager signals
    bRet = QObject::connect(networkManager, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(replyFinished(QNetworkReply*)));
    bRet = QObject::connect(networkManager, SIGNAL(proxyAuthenticationRequired(const QNetworkProxy&, QAuthenticator*)),
            this, SLOT(slotProxyAuthenticationRequired(const QNetworkProxy&, QAuthenticator*)));
    return bRet;
}

void MesFetcher::deInit()
{
    // TODO
}

void MesFetcher::fetch(const QString &layoutUrl)
{
    qDebug("Sending request.");
    this->reportStatus("Fetching keyboard...");

    QUrl url(layoutUrl);
    QNetworkRequest request(url);
    request.setRawHeader("Host", "www.mesinput.com"); // TODO remove

    QNetworkReply *reply = networkManager->get(request);
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)),
	                this, SLOT(slotError(QNetworkReply::NetworkError)));
    connect(reply, SIGNAL(sslErrors(QList<QSslError>)),
		         this, SLOT(slotSslErrors(QList<QSslError>)));
}

void MesFetcher::replyFinished(QNetworkReply* reply)
{
    qDebug() << "Layout received.";
    this->reportStatus("Keyboard received");

    QByteArray data = reply->readAll();
    QString layoutData = QString::fromUtf8(data);
    reply->deleteLater();
    emit fetchCompleted(layoutData);
}

void MesFetcher::slotError(QNetworkReply::NetworkError error)
{
    qDebug() << "Error!!" << error;
    this->reportStatus("Problems with the network, unable to fetch the keyboard.");

    emit fetchError("Network Error");
}

void MesFetcher::slotSslErrors(QList<QSslError> /*errorList*/)
{
    qDebug() << "SSL errors!!";
    emit fetchError("SSL Errors");
}

void MesFetcher::slotProxyAuthenticationRequired(const QNetworkProxy& /*proxy*/, QAuthenticator* /*auth*/)
{
    qDebug() << "Proxy authentication needed, error!!";
    emit fetchError("Proxy Authentication Needed");
}

void MesFetcher::reportStatus(const QString& status)
{
    this->currentStatus->setText(status);
}
