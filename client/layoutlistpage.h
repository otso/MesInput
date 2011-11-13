#ifndef LAYOUTLISTPAGE_H
#define LAYOUTLISTPAGE_H

#include <MApplicationPage>

class QSignalMapper;
class MesFetcher;
class QScriptValue;
class MLabel;
class MProgressIndicator;

class LayoutListPage : public MApplicationPage {
    Q_OBJECT
public:
    LayoutListPage();
    void init(MLabel *currentStatus);

protected:
    virtual void createContent();

public slots:
    void receivedLayoutList(const QString& fetchText);

private slots:
    void selectedLayout(const QString& layoutId);

signals:
    void layoutRequested(const QString& layoutId);

private:
    bool checkValidity(const QScriptValue& value);
    void reportStatus(const QString& status);

private:
    QSignalMapper *signalMapper;
    MLabel *currentStatus;
    MProgressIndicator* spinner;
};

#endif
