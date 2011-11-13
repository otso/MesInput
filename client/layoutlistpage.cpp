#include <MButton>
#include <MButtonGroup>
#include <MLabel>
#include <MSceneManager>
#include <MSeparator>
#include <MContainer>
#include <QGraphicsLinearLayout>
#include <MProgressIndicator>

#include <QScriptEngine>
#include <QScriptValueIterator>

#include <QSignalMapper>

#include "layoutlistpage.h"
#include "mesfetcher.h"

LayoutListPage::LayoutListPage()
{
    setTitle("Popular keyboards");
    this->signalMapper = new QSignalMapper(this);
}

void LayoutListPage::init(MLabel *currentStatus)
{
    this->currentStatus = currentStatus;
}

void LayoutListPage::createContent()
{
    QGraphicsLinearLayout *layout = new QGraphicsLinearLayout(Qt::Vertical);
    centralWidget()->setLayout(layout);

    MLabel* popularLabel = new MLabel("<big>Popular keyboards</big>");
    layout->addItem(popularLabel);
    popularLabel->setAlignment(Qt::AlignHCenter | Qt::AlignTop);

    spinner = new MProgressIndicator(centralWidget(), MProgressIndicator::spinnerType);
    spinner->setObjectName("spinner");
    spinner->setUnknownDuration(true);

    layout->addItem(spinner);
    layout->setAlignment(spinner, Qt::AlignHCenter | Qt::AlignVCenter);
    layout->addStretch();
}

void LayoutListPage::receivedLayoutList(const QString& fetchText)
{
    qDebug("Received layout list.");
    qDebug() << fetchText;

    QScriptValue sc;
    QScriptEngine engine;
    sc = engine.evaluate(fetchText);

    if(this->checkValidity(sc)) {

        QScriptValueIterator it(sc);

        QGraphicsLinearLayout* central = (QGraphicsLinearLayout*)(centralWidget()->layout());

        central->removeItem(spinner);
        delete spinner;
        spinner = NULL;

        qDebug("Parsed items:");

        int containerCount = 1;
        while (it.hasNext()) {
            it.next();

            // last one is length, hence check
            if (it.value().isObject()) {

                // Statuscontainer
                MContainer *statusContainer = new MContainer();
                statusContainer->setTitle(QString::number(containerCount));
                containerCount++;

                // Status label
                statusContainer->setCentralWidget(new MLabel("Name: " + it.value().property("name").toString()));
                central->addItem(statusContainer);

                central->addItem(new MLabel("About: " + it.value().property("about").toString()));

                MButton *layoutButton = new MButton;
                layoutButton->setText("Install keyboard");
                central->addItem(layoutButton);
                central->setAlignment(layoutButton, Qt::AlignCenter);

                connect(layoutButton, SIGNAL(clicked()),
                        this->signalMapper, SLOT(map()));

                this->signalMapper->setMapping(layoutButton, it.value().property("id").toString());

                qDebug("Name %s ID %s about %s", qPrintable(it.value().property("name").toString()),
                        qPrintable(it.value().property("id").toString()),
                        qPrintable(it.value().property("about").toString()));
            }
        }

        connect(this->signalMapper, SIGNAL(mapped(const QString &)),
                this, SLOT(selectedLayout(const QString &)));

    } else {
        this->dismiss();
    }
}

void LayoutListPage::selectedLayout(const QString &layoutId)
{
    qDebug()<< "Layout selected - start download: " + layoutId;
    emit layoutRequested(layoutId);
    this->dismiss();
}

bool LayoutListPage::checkValidity(const QScriptValue& value)
{
    // TODO checks only whether we receive array type, could be improved

    qDebug() << "Checking validity of layout list";
    if (value.isArray()) {
        qDebug() << "Array inside, should be ok";
        return true;
    }
    qDebug() << "No array found, fail in validation";
    this->reportStatus("Unable to fetch the list of keyboards");
    return false;
}

void LayoutListPage::reportStatus(const QString& status)
{
    this->currentStatus->setText(status);
}
