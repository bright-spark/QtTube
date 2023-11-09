#include "grayjayimportwizard.h"
#include "innertube.h"
#include "shared/choosesubspage.h"
#include "shared/choosewatchhistorypage.h"
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>

constexpr const char* subsSubtitle = "Select the Subscriptions file inside of the \"stores\" folder in the export folder.";
constexpr const char* watchHistorySubtitle = "Select the History file inside of the \"stores\" folder in the export folder.";

GrayjayImportWizard::GrayjayImportWizard(QWidget* parent)
    : DataWizard(Page_Conclusion, "Grayjay Import Wizard", parent)
{
    setPage(Page_Intro, new GrayjayImportIntroPage(this));
    setPage(Page_Subs, new GrayjayImportSubsPage(this));
    setPage(Page_WatchHistory, new GrayjayImportWatchHistoryPage(this));
    setStartId(Page_Intro);
}

GrayjayImportSubsPage::GrayjayImportSubsPage(QWidget* parent)
    : ImportFileSelectPage("Subscriptions", subsSubtitle, "Subscriptions",
                           GrayjayImportWizard::Page_ChooseSubs, parent),
      progressDialog(new QProgressDialog(this))
{
    progressDialog->cancel(); // just prevents from showing automatically
    progressDialog->setLabelText("Getting channel data...");
    progressDialog->setWindowModality(Qt::WindowModal);

    connect(this, &ImportFileSelectPage::fileSelected, this, &GrayjayImportSubsPage::verifyFile);
    connect(this, &GrayjayImportSubsPage::progress, this, &GrayjayImportSubsPage::tickProgress);
}

void GrayjayImportSubsPage::tickProgress()
{
    int newValue = progressDialog->value() + 1;
    progressDialog->setValue(newValue);
    if (newValue != progressDialog->maximum())
        return;

    emit completeChanged();
    wizard()->setPage(GrayjayImportWizard::Page_ChooseSubs, new ChooseSubsPage(
        subs,
         GrayjayImportWizard::Page_Conclusion,
         "grayjay.import.watch_history", GrayjayImportWizard::Page_WatchHistory,
         wizard()
    ));
}

void GrayjayImportSubsPage::verifyFile(const QString& fileName)
{
    QFile json(fileName);
    json.open(QIODevice::ReadOnly | QIODevice::Text);

    QJsonParseError parseError;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(json.readAll(), &parseError);

    if (parseError.error != QJsonParseError::NoError || !jsonDoc.isArray())
    {
        pathEdit->setText("Invalid file selected");
        return;
    }

    pathEdit->setText(fileName);

    const QJsonArray subsJson = jsonDoc.array();

    progressDialog->reset();
    progressDialog->setMaximum(subsJson.size());
    progressDialog->setValue(0);

    QThreadPool* threadPool = new QThreadPool(this);
    threadPool->setMaxThreadCount(QThread::idealThreadCount() / 2);

    for (const QJsonValue& entry : subsJson)
    {
        QString id = entry.toString().remove("https://www.youtube.com/channel/");
        threadPool->start([this, id] {
            if (!progressDialog->wasCanceled())
            {
                try
                {
                    auto endpoint = InnerTube::instance().getBlocking<InnertubeEndpoints::BrowseChannel>(id);
                    subs.append(Entity(id, endpoint.response.header.title));
                    // prevent rate limit (apparently it exists but i didn't hit it.. better safe than sorry)
                    QThread::sleep(1);
                }
                catch (const InnertubeException& ie)
                {
                    qWarning() << ie.message();
                }
            }

            emit progress();
        });
    }
}

GrayjayImportWatchHistoryPage::GrayjayImportWatchHistoryPage(QWidget* parent)
    : ImportFileSelectPage("Watch History", watchHistorySubtitle, "History",
                           GrayjayImportWizard::Page_ChooseWatchHistory, parent)
{
    connect(this, &ImportFileSelectPage::fileSelected, this, &GrayjayImportWatchHistoryPage::verifyFile);
}

void GrayjayImportWatchHistoryPage::verifyFile(const QString& fileName)
{
    QFile json(fileName);
    json.open(QIODevice::ReadOnly | QIODevice::Text);

    QJsonParseError parseError;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(json.readAll(), &parseError);

    if (parseError.error != QJsonParseError::NoError || !jsonDoc.isArray())
    {
        pathEdit->setText("Invalid file selected");
        return;
    }

    QList<Entity> outVideos;
    const QJsonArray videos = jsonDoc.array();
    for (const QJsonValue& entry : videos)
    {
        QStringList split = entry.toString().split("|||");
        if (split.size() != 4)
            continue;

        QString id = split[0].remove("https://www.youtube.com/watch?v=");
        QString name = QStringLiteral("<a href=\"%1\">%2</a>").arg(split[0], split[3]);
        outVideos.append(Entity(id, name));
    }

    pathEdit->setText(fileName);
    emit completeChanged();

    wizard()->setPage(GrayjayImportWizard::Page_ChooseWatchHistory, new ChooseWatchHistoryPage(
        outVideos, GrayjayImportWizard::Page_Conclusion, wizard()
    ));
}