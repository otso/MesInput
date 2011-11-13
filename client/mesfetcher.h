#ifndef MESFETCHER_H
#define MESFETCHER_H

#include <QtCore>
#include <QtNetwork>

class MLabel;

class MesFetcher : public QObject
{
Q_OBJECT
public:  
    MesFetcher();
    ~MesFetcher();
    void fetch(const QString& layoutUrl);
    bool init(MLabel *currentStatus);
    void deInit();
signals:
    void fetchCompleted(const QString& fetchText);
    void fetchError(const QString& fetchError);
private slots:
    void replyFinished(QNetworkReply* reply);
    void slotError(QNetworkReply::NetworkError error);
    void slotSslErrors(QList<QSslError> sslError);
    void slotProxyAuthenticationRequired(const QNetworkProxy&, QAuthenticator*);
    void reportStatus(const QString& status);
private:
    QNetworkAccessManager *networkManager;
    MLabel *currentStatus;
};

#endif // MESFETCHER_H

