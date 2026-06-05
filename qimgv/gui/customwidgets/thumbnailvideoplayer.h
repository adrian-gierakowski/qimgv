#pragma once

#include <QtCore/QMetaObject>
#include <QObject>
#include <QImage>
#include <QPixmap>
#include <QTimer>
#include <QMutex>

#include <mpv/client.h>
#include <mpv/render.h>
#include <mpv/render_gl.h>


#include "../../../plugins/player_mpv/src/qthelper.hpp"

class ThumbnailVideoPlayer : public QObject {
    Q_OBJECT
public:
    explicit ThumbnailVideoPlayer(QObject *parent = nullptr);
    ~ThumbnailVideoPlayer() override;

    void loadVideo(const QString& file);
    void play();
    void stop();
    void setMuted(bool muted);

    QPixmap currentPixmap();

signals:
    void frameReady();
    void finished();

private slots:
    void on_mpv_events();
    void updateFrame();

private:
    void handle_mpv_event(mpv_event *event);

    mpv_handle *mpv;
    mpv_render_context *mpv_gl;

    QTimer *renderTimer;
    QTimer *durationTimer;
    void onDurationLimitReached();
    QImage m_currentImage;
    QPixmap m_currentPixmap;
    QMutex m_mutex;
    bool m_isPlaying;
};
