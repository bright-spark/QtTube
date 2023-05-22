#ifndef UIUTILITIES_H
#define UIUTILITIES_H
#include "innertube/objects/channel/channel.h"
#include "innertube/objects/video/reel.h"
#include "innertube/objects/video/video.h"
#include <initializer_list>
#include <QLabel>
#include <QLayout>
#include <QListWidget>
#include <QTabWidget>

class UIUtilities
{
public:
    static inline QString defaultStyle;

    static void addBoldLabelToList(QListWidget* list, const QString& text);
    static void addChannelRendererToList(QListWidget* list, const InnertubeObjects::Channel& channel);
    static void addSeparatorToList(QListWidget* list);
    static void addShelfTitleToList(QListWidget* list, const QJsonValue& shelf);
    static void addShelfTitleToList(QListWidget* list, const QString& title);
    static void addVideoRendererToList(QListWidget* list, const InnertubeObjects::Reel& reel);
    static void addVideoRendererToList(QListWidget* list, const InnertubeObjects::Video& video);
    static QListWidgetItem* addWidgetToList(QListWidget* list, QWidget* widget);
    static void addWrappedLabelToList(QListWidget* list, const QString& text);
    static void clearLayout(QLayout* layout);
    static void copyToClipboard(const QString& text);
    static void elide(QLabel* label, int targetWidth);
    static QPixmap icon(const QString& name, bool fromQIcon = false, const QSize& size = QSize(), const QPalette& pal = QPalette());
    static bool preferDark(const QPalette& pal = QPalette());
    static void setMaximumLines(QWidget* widget, int lines);
    static void setTabsEnabled(QTabWidget* widget, bool enabled, std::initializer_list<int> indexes);
    static void setThumbnail(QLabel* label, const QJsonArray& thumbsArr, bool getBest = false);

#ifdef Q_OS_WIN
    static void setAppStyle(const QString& styleName, bool dark);
#else
    static void setAppStyle(const QString& styleName);
#endif

    template<typename T>
    static T findParent(QWidget* widget);
private:
#ifdef Q_OS_WIN
    static inline QString darkStylesheet = R"(
        QLineEdit {
            background: rgb(42,42,42);
            border: 1px solid rgb(30,30,30);
        }
        QListView::item {
            background: rgb(49,49,49);
        }
        QComboBox, QMessageBox, QPushButton, QScrollBar::vertical, QSpinBox, QTabBar::tab {
            background: rgb(42,42,42);
        }
        QPushButton::hover, QTabBar::tab::hover, QTabBar::tab::selected {
            background: rgb(30,30,30);
        }
        QTabWidget::pane {
            border-color: rgb(30,30,30);
        }
    )";
#endif
};

#include "uiutilities.tpp"

#endif // UIUTILITIES_H
