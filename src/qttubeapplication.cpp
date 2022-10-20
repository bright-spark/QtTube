#include "innertube.h"
#include "qttubeapplication.h"
#include "ui/mainwindow.h"
#include <array>
#include <QMouseEvent>

constexpr std::array<Qt::Key, 5> sneed = { Qt::Key_S, Qt::Key_N, Qt::Key_E, Qt::Key_E, Qt::Key_D };

bool QtTubeApplication::notify(QObject* receiver, QEvent* event)
{
    if (receiver->objectName() == "MainWindowWindow")
    {
        if (event->type() == QEvent::MouseMove)
        {
            QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
            TopBar* topbar = MainWindow::instance()->topbar;
            if (!topbar->alwaysShow)
            {
                if (mouseEvent->pos().y() < 35)
                {
                    if (topbar->isHidden() && topbar->animation->state() != topbar->animation->Running)
                    {
                        topbar->animation->setStartValue(QRect(0, 0, topbar->width(), 0));
                        topbar->animation->setEndValue(QRect(0, 0, topbar->width(), 35));
                        disconnect(topbar->animation, &QPropertyAnimation::finished, topbar, &TopBar::hide);
                        topbar->animation->start();
                        topbar->show();
                    }
                }
                else if (topbar->animation->state() != topbar->animation->Running && !topbar->isHidden())
                {
                    topbar->animation->setEndValue(QRect(0, 0, topbar->width(), 0));
                    topbar->animation->setStartValue(QRect(0, 0, topbar->width(), 35));
                    connect(topbar->animation, &QPropertyAnimation::finished, topbar, &TopBar::hide);
                    topbar->animation->start();
                }
            }
        }
        else if (event->type() == QEvent::KeyPress)
        {
            QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
            Qt::Key key = Qt::Key(keyEvent->key());

            if (key == sneed[m_sneedProgress])
                m_sneedProgress++;
            else
                m_sneedProgress = 0;

            if (m_sneedProgress == (int)sneed.size())
            {
                m_sneedProgress = 0;
                WatchView::instance()->loadVideo(InnerTube::instance().get<InnertubeEndpoints::Next>("T3ElIvNBYj0"),
                                                 InnerTube::instance().get<InnertubeEndpoints::Player>("T3ElIvNBYj0"));
            }
        }
    }

    return QApplication::notify(receiver, event);
}
