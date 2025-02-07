#pragma once
#include "genericstore.h"
#include <QObject>

namespace InnertubeObjects { struct Reel; struct Video; }

class QSettings;

class SettingsStore : public GenericStore
{
    Q_OBJECT

    Q_PROPERTY(bool blockAds MEMBER blockAds NOTIFY blockAdsChanged)
    Q_PROPERTY(bool disablePlayerInfoPanels MEMBER disablePlayerInfoPanels NOTIFY disablePlayerInfoPanelsChanged)
    Q_PROPERTY(SettingsStore::PlayerQuality preferredQuality MEMBER preferredQuality NOTIFY preferredQualityChanged)
    Q_PROPERTY(int preferredVolume MEMBER preferredVolume NOTIFY preferredVolumeChanged)
    Q_PROPERTY(bool qualityFromPlayer MEMBER qualityFromPlayer NOTIFY qualityFromPlayerChanged)
    Q_PROPERTY(bool restoreAnnotations MEMBER restoreAnnotations NOTIFY restoreAnnotationsChanged)
    Q_PROPERTY(QStringList sponsorBlockCategories MEMBER sponsorBlockCategories NOTIFY sponsorBlockCategoriesChanged)
    Q_PROPERTY(bool volumeFromPlayer MEMBER volumeFromPlayer NOTIFY volumeFromPlayerChanged)
public:
    enum class PlayerQuality { Auto, HighRes, HD2160, HD1440, HD1080, HD720, Large, Medium, Small, Tiny };
    Q_ENUM(PlayerQuality)

    QString appStyle;
    bool blockAds;
    bool condensedCounts;
    bool darkTheme;
    bool deArrow;
    bool deArrowThumbs;
    bool deArrowTitles;
    bool disable60Fps;
    bool disablePlayerInfoPanels;
    int filterLength;
    bool filterLengthEnabled;
    QStringList filteredChannels;
    QStringList filteredTerms;
    bool fullSubs;
    bool h264Only;
    bool hideShorts;
    bool hideStreams;
    bool imageCaching;
    bool playbackTracking;
    bool preferLists;
    PlayerQuality preferredQuality;
    int preferredVolume;
    bool qualityFromPlayer;
    bool restoreAnnotations;
    bool returnDislikes;
    bool showSBToasts;
    QStringList sponsorBlockCategories;
    bool vaapi;
    bool volumeFromPlayer;
    bool watchtimeTracking;

    explicit SettingsStore(QObject* parent = nullptr) : GenericStore("settings.ini") {}

    bool channelIsFiltered(const QString& channelId) const;
    bool strHasFilteredTerm(const QString& str) const;
    bool videoIsFiltered(const InnertubeObjects::Reel& reel) const;
    bool videoIsFiltered(const InnertubeObjects::Video& video) const;

    void initialize() override;
    void save() override;
private:
    void readIntoStringList(QSettings& settings, QStringList& list, const QString& prefix, const QString& key);
    void writeStringList(QSettings& settings, const QStringList& list, const QString& prefix, const QString& key);
signals:
    void blockAdsChanged(bool);
    void disablePlayerInfoPanelsChanged(bool);
    void preferredQualityChanged(SettingsStore::PlayerQuality);
    void preferredVolumeChanged(int);
    void qualityFromPlayerChanged(bool);
    void restoreAnnotationsChanged(bool);
    void sponsorBlockCategoriesChanged(const QStringList&);
    void volumeFromPlayerChanged(bool);
};
