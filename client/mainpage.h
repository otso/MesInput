#ifndef MAINPAGE_H
#define MAINPAGE_H

#include <QGraphicsLinearLayout>
#include <MApplicationPage>
#include <MTextEdit>

#include "mesfetcher.h"

class QGraphicsLinearLayout;
class MesFetcher;
class MTextEdit;
class MLabel;

class MainPage : public MApplicationPage {
    Q_OBJECT
public:
    MainPage(QGraphicsItem *parent = 0);
    virtual ~MainPage();

protected:
    void createContent();

private slots:
    void fetchLayout();
    void fetchLayout(const QString& layoutId);
    void displayListPage();
    void depositLayout(const QString& fetchText);
    void about();
    void deleteLayouts();
    void processDeleteQuery();

private:
    bool checkValidity(const QString& textToValidate);

private:
    // TODO d-pointer + better way to access layout added items
    QGraphicsLinearLayout* layout;
    MesFetcher* fetcher;
    MTextEdit* textEdit;
    MLabel* status;
    QString layoutId;
};

#endif
