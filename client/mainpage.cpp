#include "mainpage.h"
#include "layoutlistpage.h"

#include <MButton>
#include <MButtonGroup>
#include <MLabel>
#include <MContainer>
#include <MSceneManager>
#include <MTextEdit>
#include <QFile>
#include <MGConfItem>
#include <MSeparator>
#include <MDialog>

#include <MImageWidget>

#include <QDomDocument>
#include <QDomElement>

namespace
{
    const char * const VKBUserLayoutPath = ".config/meego-keyboard/layouts/";
}

MainPage::MainPage(QGraphicsItem *parent)
    : MApplicationPage(parent)
{
    setTitle("MesInput");
}

MainPage::~MainPage()
{
    // TODO
}

void MainPage::createContent()
{
    this->layout = new QGraphicsLinearLayout(Qt::Vertical);
    this->layout->setSpacing(10);
    centralWidget()->setLayout(this->layout);

    // Image loading:
    QString image_name = QString("/usr/share/mesinput/mesinput_small.png");
    QImage img(image_name);
    MImageWidget* image = new MImageWidget(&img);
    image->setVisible(true);
    this->layout->addItem(image);

    // Statuscontainer
    MContainer *statusContainer = new MContainer();
    statusContainer->setTitle("Status");

    // Status label
    this->status = new MLabel("Ready to install keyboards");
    statusContainer->setCentralWidget(this->status, false);
    this->layout->addItem(statusContainer);

    // List container
    MContainer *listContainer = new MContainer();
    listContainer->setTitle("List of keyboards");

    MLabel* popularLabel = new MLabel("Select from a list of popular keyboards:");
    popularLabel->setWordWrap(true);
    popularLabel->setWrapMode(QTextOption::WordWrap);
    listContainer->setCentralWidget(popularLabel);

    MButton *listButton;
    listButton = new MButton;
    listButton->setText("Popular keyboards");

    this->layout->addItem(listContainer);
    this->layout->addItem(listButton);
    this->layout->setAlignment(listButton, Qt::AlignCenter);

    // Some space
    this->layout->addItem(new MLabel("<br>"));

    // Install container
    MContainer *installContainer = new MContainer();
    installContainer->setTitle("Install");

    // Layout ID query field
    MLabel* keyboardLabel = new MLabel("Keyboard ID to install, see www.mesinput.com for a full list of keyboards:");
    keyboardLabel->setWordWrap(true);
    keyboardLabel->setWrapMode(QTextOption::WordWrap);
    installContainer->setCentralWidget(keyboardLabel);
    this->layout->addItem(installContainer);

    this->textEdit = new MTextEdit(MTextEditModel::SingleLine);
    this->textEdit->setContentType(M::NumberContentType);
    this->textEdit->setPrompt(QString("Keyboard ID:"));
    this->layout->addItem(this->textEdit);
    this->textEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    // Individual layouts
    MButton *fetchButton;
    fetchButton = new MButton;
    fetchButton->setText("Install keyboard");

    this->layout->addItem(fetchButton);
    this->layout->setAlignment(fetchButton, Qt::AlignCenter);

    // Some space
    this->layout->addItem(new MLabel("<br>"));

    // container
    MContainer *miscContainer = new MContainer();
    miscContainer->setTitle("Miscellaneous");

    // About button
    MButton *aboutButton;
    aboutButton = new MButton;
    aboutButton->setText("About");

    this->layout->addItem(miscContainer);

    this->layout->addItem(aboutButton);
    this->layout->setAlignment(aboutButton, Qt::AlignCenter);

    // Delete all
    MButton *deleteButton;
    deleteButton = new MButton;
    deleteButton->setText("Delete keyboards");

    this->layout->addItem(deleteButton);
    this->layout->setAlignment(deleteButton, Qt::AlignCenter);

    // TODO check networkAccessible : NetworkAccessibility -> errors
    // TODO check return - is this the right place for init
    this->fetcher = new MesFetcher;
    this->fetcher->init(this->status);

    connect(fetchButton, SIGNAL(clicked()),
            this, SLOT(fetchLayout()));

    connect(listButton, SIGNAL(clicked()),
            this, SLOT(displayListPage()));

    connect(aboutButton, SIGNAL(clicked()),
            this, SLOT(about()));

    connect(deleteButton, SIGNAL(clicked()),
            this, SLOT(deleteLayouts()));

}

void MainPage::fetchLayout()
{
    this->status->setText("Fetching keyboard: " + this->textEdit->text());

    this->fetchLayout(this->textEdit->text());
}

void MainPage::fetchLayout(const QString& layoutId)
{
    // Remove previous individual layout fetch slot connection
    disconnect(this->fetcher, 0, 0, 0);

    this->layoutId = layoutId;

    connect(this->fetcher, SIGNAL(fetchCompleted(const QString&)),
            this, SLOT(depositLayout(const QString&)));

    this->fetcher->fetch("http://www.mesinput.com/api/vkblayouts/xml/" + layoutId);
}

void MainPage::depositLayout(const QString& fetchText)
{
    qDebug("Writing to layout file.");
    qDebug() << "Layout received:" << fetchText;

    if (this->checkValidity(fetchText)) {
        // Layout name needs to be unique, otherwise we need to update the gconf key
        QString layoutFilename(QString("mesinput-layout_") + this->layoutId + QString(".xml"));
        QString pathToCreate = QDir::homePath() + QDir::separator() + QString(VKBUserLayoutPath);

        if (!(QDir(pathToCreate).exists())) {
            QDir().mkpath(pathToCreate);
            qDebug() << "Created dir: " << pathToCreate; // Check the result
        }

        qDebug() << "Depositing: " << pathToCreate + layoutFilename;
        QFile file(pathToCreate + layoutFilename);

        qDebug() << "File name:" << file.fileName();
        file.open(QIODevice::WriteOnly | QIODevice::Text);
        QTextStream out(&file);
        out << fetchText;
        file.close();

        QString binFilename(QString("libmeego-keyboard.so"));
        MGConfItem configuredLayoutsActive("/meegotouch/inputmethods/onscreen/active");
        QVariant layouts = configuredLayoutsActive.value();
        if (layouts.type() == QVariant::StringList) {
            QStringList availableLayouts = layouts.toStringList();
            if (!availableLayouts.contains(layoutFilename, Qt::CaseInsensitive)) {
                availableLayouts.append(binFilename);
                availableLayouts.append(layoutFilename);
            }
            configuredLayoutsActive.set(availableLayouts);
        }

        MGConfItem configuredLayoutsEnabled("/meegotouch/inputmethods/onscreen/enabled");
        layouts = configuredLayoutsEnabled.value();
        if (layouts.type() == QVariant::StringList) {
            QStringList availableLayouts = layouts.toStringList();
            if (!availableLayouts.contains(layoutFilename, Qt::CaseInsensitive)) {
                availableLayouts.append(binFilename);
                availableLayouts.append(layoutFilename);
            }
            configuredLayoutsEnabled.set(availableLayouts);
        }

        this->status->setText("Keyboard ready to use");
    }
}

bool MainPage::checkValidity(const QString& textToValidate)
{
    qDebug("Checking validity");

    QDomDocument document;
    document.setContent(textToValidate);

    QDomNodeList elements = document.elementsByTagName(QString("keyboard"));
    if(elements.size() != 1) {
        qDebug("Not valid");
        this->status->setText("Keyboard not valid, is the ID right?");
        return false;
    }
    qDebug("Valid keyboard");
    return true;
}

void MainPage::displayListPage()
{
    qDebug("Showing layout list.");

    LayoutListPage *layoutListPage = new LayoutListPage();
    layoutListPage->init(this->status);

    // Remove previous individual layout fetch slot connection
    disconnect(this->fetcher, 0, 0, 0);

    connect(this->fetcher, SIGNAL(fetchCompleted(const QString&)),
            layoutListPage, SLOT(receivedLayoutList(const QString&)));

    connect(layoutListPage, SIGNAL(layoutRequested(const QString&)),
            this, SLOT(fetchLayout(const QString&)));

    this->fetcher->fetch("http://www.mesinput.com/api/layouts/");

    layoutListPage->appear(scene(), MSceneWindow::DestroyWhenDismissed);
}

void MainPage::about()
{
    qDebug("In about dialog");
    MDialog* dialog = new MDialog("About MesInput",
                           M::OkButton);

    dialog->setCentralWidget(new MLabel("<br><p style=\"color: white\">MesInput v0.5<br> \
                                        (c) Otso Virtanen<br><br>www.mesinput.com</p><br>"));

    dialog->appear();
}

void MainPage::deleteLayouts()
{
    qDebug("In delete layouts dialog");
    MDialog* dialog = new MDialog("Confirmation",
                           M::OkButton | M::CancelButton);

    dialog->setCentralWidget(new MLabel("<br><p style=\"color: white\">Delete all the loaded keyboards?<br><br>"));

    connect(dialog, SIGNAL(accepted()), SLOT(processDeleteQuery()));
    dialog->appear();
}

void MainPage::processDeleteQuery()
{
    qDebug() << "Delete the keyboards";

    QDir pathToDelete(QDir::homePath() + QDir::separator() + QString(VKBUserLayoutPath));
    pathToDelete.setFilter(QDir::Files);
    QStringList entries = pathToDelete.entryList();

    // Notifying meego-im-uiserver.
    MGConfItem configuredLayouts("/meegotouch/inputmethods/onscreen/enabled");
    QVariant layouts = configuredLayouts.value();
    if (layouts.type() == QVariant::StringList) {
        QStringList availableLayouts = layouts.toStringList();

        for(QStringList::ConstIterator entry=entries.begin(); entry!=entries.end(); ++entry) {
           qDebug() << *entry;
           if (availableLayouts.contains(*entry, Qt::CaseInsensitive)) {
               int tbRemoved = availableLayouts.indexOf(*entry);
               availableLayouts.removeAt(tbRemoved-1); // the .so file
               qDebug() << "Removing from Gconf: " << *entry;
               availableLayouts.removeOne(*entry); // assuming only one layout name
           }
        }
        configuredLayouts.set(availableLayouts);
    }

    // Notifying meego-im-uiserver.
    MGConfItem configuredLayoutsActive("/meegotouch/inputmethods/onscreen/active");
    QVariant layoutsActive = configuredLayoutsActive.value();
    if (layoutsActive.type() == QVariant::StringList) {
        QStringList availableLayouts = layoutsActive.toStringList();

        for(QStringList::ConstIterator entry=entries.begin(); entry!=entries.end(); ++entry) {
           qDebug() << *entry;
           if (availableLayouts.contains(*entry, Qt::CaseInsensitive)) {
               int tbRemoved = availableLayouts.indexOf(*entry);
               availableLayouts.removeAt(tbRemoved-1); // the .so file
               qDebug() << "Removing from Gconf: " << *entry;
               availableLayouts.removeOne(*entry); // assuming only one layout name
           }
           pathToDelete.remove(*entry);
        }
        configuredLayoutsActive.set(availableLayouts);
    }

    this->status->setText("Deleted loaded keyboards.");
}
