#include "browsehelper.h"
#include "channelbrowser.h"
#include "http.h"
#include "protobuf/simpleprotobuf.h"
#include "ui/forms/mainwindow.h"
#include "ui/widgets/renderers/browsenotificationrenderer.h"
#include "utils/uiutils.h"
#include <QApplication>

BrowseHelper* BrowseHelper::instance()
{
    std::call_once(m_onceFlag, [] { m_instance = new BrowseHelper; });
    return m_instance;
}

void BrowseHelper::browseChannel(QListWidget* widget, int index, const InnertubeEndpoints::ChannelResponse& resp)
{
    QJsonValue tabRenderer = resp.contents["twoColumnBrowseResultsRenderer"]["tabs"][index]["tabRenderer"];
    if (!tabRenderer["selected"].toBool())
    {
        QString params = tabRenderer["endpoint"]["browseEndpoint"]["params"].toString();
        auto bc = InnerTube::instance().getBlocking<InnertubeEndpoints::BrowseChannel>(resp.metadata.externalId, "", params);
        tabRenderer = bc.response.contents["twoColumnBrowseResultsRenderer"]["tabs"][index]["tabRenderer"];
    }

    try
    {
        QString title = tabRenderer["title"].toString();
        if (title == "Home")
            ChannelBrowser::setupHome(widget, tabRenderer, resp);
        else if (title == "Videos")
            ChannelBrowser::setupVideos(widget, tabRenderer, resp);
        else if (title == "Shorts")
            ChannelBrowser::setupShorts(widget, tabRenderer, resp);
        else if (title == "Live")
            ChannelBrowser::setupLive(widget, tabRenderer, resp);
        else if (title == "Membership")
            ChannelBrowser::setupMembership(widget, tabRenderer);
        else if (title == "Channels")
            ChannelBrowser::setupChannels(widget, tabRenderer);
        else if (title == "About")
            ChannelBrowser::setupAbout(widget, tabRenderer);
        else
            ChannelBrowser::setupUnimplemented(widget);
    }
    catch (const InnertubeException& ie)
    {
        browseFailed(ie, "Failed to get channel tab data");
    }
}

void BrowseHelper::browseHistory(ContinuableListWidget* widget, const QString& query)
{
    if (!InnerTube::instance().hasAuthenticated())
    {
        widget->addItem("Local history has not been implemented yet. You will need to log in.");
        return;
    }

    InnertubeReply* reply = InnerTube::instance().get<InnertubeEndpoints::BrowseHistory>(query);
    connect(reply, &InnertubeReply::exception, this, std::bind(&BrowseHelper::browseFailed, this, std::placeholders::_1, "Failed to get history data"));
    connect(reply, qOverload<const InnertubeEndpoints::BrowseHistory&>(&InnertubeReply::finished), this, [this, widget](const InnertubeEndpoints::BrowseHistory& endpoint)
    {
        setupVideoList(endpoint.response.videos, widget);
        widget->continuationToken = endpoint.continuationToken;
    });
}

void BrowseHelper::browseHome(ContinuableListWidget* widget)
{
    InnertubeReply* reply = InnerTube::instance().get<InnertubeEndpoints::BrowseHome>();
    connect(reply, &InnertubeReply::exception, this, std::bind(&BrowseHelper::browseFailed, this, std::placeholders::_1, "Failed to get home data"));
    connect(reply, qOverload<const InnertubeEndpoints::BrowseHome&>(&InnertubeReply::finished), this, [this, widget](const InnertubeEndpoints::BrowseHome& endpoint)
    {
        setupVideoList(endpoint.response.videos, widget);
        widget->continuationToken = endpoint.continuationToken;
    });
}

void BrowseHelper::browseNotificationMenu(ContinuableListWidget* widget)
{
    InnertubeReply* reply = InnerTube::instance().get<InnertubeEndpoints::GetNotificationMenu>("NOTIFICATIONS_MENU_REQUEST_TYPE_INBOX");
    connect(reply, &InnertubeReply::exception, this, std::bind(&BrowseHelper::browseFailed, this, std::placeholders::_1, "Failed to get notification data"));
    connect(reply, qOverload<const InnertubeEndpoints::GetNotificationMenu&>(&InnertubeReply::finished), this, [this, widget](const InnertubeEndpoints::GetNotificationMenu& endpoint)
    {
        setupNotificationList(endpoint.response.notifications, widget);
        widget->continuationToken = endpoint.continuationToken;
        MainWindow::topbar()->updateNotificationCount();
    });
}

void BrowseHelper::browseSubscriptions(ContinuableListWidget* widget)
{
    if (!InnerTube::instance().hasAuthenticated())
    {
        widget->addItem("You need to log in to view subscriptions.");
        return;
    }

    InnertubeReply* reply = InnerTube::instance().get<InnertubeEndpoints::BrowseSubscriptions>();
    connect(reply, &InnertubeReply::exception, this, std::bind(&BrowseHelper::browseFailed, this, std::placeholders::_1, "Failed to get subscriptions data"));
    connect(reply, qOverload<const InnertubeEndpoints::BrowseSubscriptions&>(&InnertubeReply::finished), this, [this, widget](const InnertubeEndpoints::BrowseSubscriptions& endpoint)
    {
        setupVideoList(endpoint.response.videos, widget);
        widget->continuationToken = endpoint.continuationToken;
    });
}

void BrowseHelper::browseTrending(QListWidget* widget)
{
    InnertubeReply* reply = InnerTube::instance().get<InnertubeEndpoints::BrowseTrending>();
    connect(reply, &InnertubeReply::exception, this, std::bind(&BrowseHelper::browseFailed, this, std::placeholders::_1, "Failed to get trending data"));
    connect(reply, qOverload<const InnertubeEndpoints::BrowseTrending&>(&InnertubeReply::finished), this, [this, widget](const InnertubeEndpoints::BrowseTrending& endpoint)
    {
        setupVideoList(endpoint.response.videos, widget);
    });
}

void BrowseHelper::search(ContinuableListWidget* searchWidget, const QString& query,
                          int dateF, int typeF, int durF, int featF, int sort)
{
    QByteArray compiledParams;
    QVariantMap filter, params;
    if (sort != -1) params.insert("sort", sort);
    if (dateF != -1) filter.insert("uploadDate", dateF + 1);
    if (typeF != -1) filter.insert("type", typeF + 1);
    if (durF != -1) filter.insert("duration", durF + 1);
    if (featF != -1) filter.insert(featureMap[featF], true);
    if (!filter.isEmpty()) params.insert("filter", filter);

    if (!params.isEmpty())
        compiledParams = QByteArray::fromHex(SimpleProtobuf::compile(params, searchMsgFields)).toBase64().toPercentEncoding();

    InnertubeReply* reply = InnerTube::instance().get<InnertubeEndpoints::Search>(query, "", compiledParams);
    connect(reply, &InnertubeReply::exception, this, std::bind(&BrowseHelper::browseFailed, this, std::placeholders::_1, "Failed to get search data"));
    connect(reply, qOverload<const InnertubeEndpoints::Search&>(&InnertubeReply::finished), this, [this, searchWidget](const InnertubeEndpoints::Search& endpoint)
    {
        searchWidget->addItem(QStringLiteral("About %1 results").arg(QLocale::system().toString(endpoint.response.estimatedResults)));
        setupChannelList(endpoint.response.channels, searchWidget);
        setupVideoList(endpoint.response.videos, searchWidget);
        searchWidget->continuationToken = endpoint.continuationToken;
    });
}

void BrowseHelper::browseFailed(const InnertubeException& ie, const QString& title)
{
    if (ie.severity() == InnertubeException::Severity::Normal)
        QMessageBox::critical(nullptr, title, ie.message());
    else
        qWarning().nospace() << title << ": " << ie.message();
}

void BrowseHelper::setupChannelList(const QList<InnertubeObjects::Channel>& channels, QListWidget* widget)
{
    for (const InnertubeObjects::Channel& channel : channels)
    {
        UIUtils::addChannelRendererToList(widget, channel);
        QCoreApplication::processEvents();
    }
}

void BrowseHelper::setupNotificationList(const QList<InnertubeObjects::Notification>& notifications, QListWidget* widget)
{
    for (const InnertubeObjects::Notification& n : notifications)
    {
        BrowseNotificationRenderer* renderer = new BrowseNotificationRenderer(widget);
        renderer->setData(n);
        UIUtils::addWidgetToList(widget, renderer);

        HttpReply* iconReply = Http::instance().get(n.channelIcon.url);
        connect(iconReply, &HttpReply::finished, renderer, &BrowseNotificationRenderer::setChannelIcon);

        HttpReply* thumbReply = Http::instance().get("https://i.ytimg.com/vi/" + n.videoId + "/mqdefault.jpg");
        connect(thumbReply, &HttpReply::finished, renderer, &BrowseNotificationRenderer::setThumbnail);

        QCoreApplication::processEvents();
    }
}

void BrowseHelper::setupVideoList(const QList<InnertubeObjects::Video>& videos, QListWidget* widget)
{
    QString lastShelf;
    for (const InnertubeObjects::Video& video : videos)
    {
        if (!video.shelf.text.isEmpty() && video.shelf.text != lastShelf)
        {
            UIUtils::addShelfTitleToList(widget, video.shelf.text);
            lastShelf = video.shelf.text;
        }

        UIUtils::addVideoRendererToList(widget, video);
        QCoreApplication::processEvents();
    }
}
