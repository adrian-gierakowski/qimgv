#include "thumbnailvideoplayer.h"
#include "../../settings.h"
#include <stdexcept>
#include <QDebug>
#include <QThread>

static void wakeup(void *ctx) {
    QMetaObject::invokeMethod((ThumbnailVideoPlayer*)ctx, "on_mpv_events", Qt::QueuedConnection);
}

ThumbnailVideoPlayer::ThumbnailVideoPlayer(QObject *parent)
    : QObject(parent), m_isPlaying(false), mpv_gl(nullptr)
{
    mpv = mpv_create();
    if(!mpv)
        throw std::runtime_error("could not create mpv context");

    mpv_set_option_string(mpv, "vo", "libmpv");
    mpv_set_option_string(mpv, "audio", "no");
    mpv_set_option_string(mpv, "hwdec", "auto");
    mpv_set_option_string(mpv, "vid", "1");
    // Limit decoding fps for preview, maybe low res
    mpv_set_option_string(mpv, "vd-lavc-skiploopfilter", "all");
    mpv_set_option_string(mpv, "vd-lavc-skipframe", "nonref");

    if (mpv_initialize(mpv) < 0)
        throw std::runtime_error("could not initialize mpv context");

    mpv_observe_property(mpv, 0, "pause", MPV_FORMAT_FLAG);
    mpv_set_wakeup_callback(mpv, wakeup, this);

    // Setup render context (Software renderer)
    mpv_render_param params[] = {
        {MPV_RENDER_PARAM_API_TYPE, const_cast<char *>(MPV_RENDER_API_TYPE_SW)},
        {MPV_RENDER_PARAM_INVALID, nullptr}
    };

    if(mpv_render_context_create(&mpv_gl, mpv, params) < 0) {
        qDebug() << "failed to initialize mpv SW context";
    }

    renderTimer = new QTimer(this);
    durationTimer = new QTimer(this);
    durationTimer->setSingleShot(true);
    connect(durationTimer, &QTimer::timeout, this, &ThumbnailVideoPlayer::onDurationLimitReached);
    connect(renderTimer, &QTimer::timeout, this, &ThumbnailVideoPlayer::updateFrame);
}

ThumbnailVideoPlayer::~ThumbnailVideoPlayer() {
    stop();
    if (mpv_gl) {
        mpv_render_context_free(mpv_gl);
    }
    if (mpv) {
        mpv_terminate_destroy(mpv);
    }
}

void ThumbnailVideoPlayer::onDurationLimitReached() {
    stop();
    emit finished();
}

void ThumbnailVideoPlayer::loadVideo(const QString& file) {
    if(file.isEmpty()) return;

    m_isPlaying = true;
    mpv::qt::command(mpv, QVariantList() << "loadfile" << file);
    mpv::qt::set_property(mpv, "pause", false);
    mpv::qt::set_property(mpv, "loop-file", "inf");

    // Start rendering timer (~15fps)
    if(!renderTimer->isActive())
        renderTimer->start(66);

    int limit = settings->previewDurationLimit();
    if (limit > 0) {
        durationTimer->start(limit * 1000);
    }

}

void ThumbnailVideoPlayer::play() {
    m_isPlaying = true;
    mpv::qt::set_property(mpv, "pause", false);
    if(!renderTimer->isActive())
        renderTimer->start(66);

    int limit = settings->previewDurationLimit();
    if (limit > 0) {
        durationTimer->start(limit * 1000);
    }

}

void ThumbnailVideoPlayer::stop() {
    m_isPlaying = false;
    mpv::qt::set_property(mpv, "pause", true);
    renderTimer->stop();
    durationTimer->stop();
    mpv::qt::command(mpv, QVariantList() << "stop");
}

void ThumbnailVideoPlayer::setMuted(bool muted) {
    if(!muted) {
        mpv_set_option_string(mpv, "audio", "auto");
    } else {
        mpv_set_option_string(mpv, "audio", "no");
    }
    mpv::qt::set_property(mpv, "mute", muted ? "yes" : "no");
}

QPixmap ThumbnailVideoPlayer::currentPixmap() {
    QMutexLocker lock(&m_mutex);
    return m_currentPixmap;
}

void ThumbnailVideoPlayer::on_mpv_events() {
    while (mpv) {
        mpv_event *event = mpv_wait_event(mpv, 0);
        if (event->event_id == MPV_EVENT_NONE) {
            break;
        }
        handle_mpv_event(event);
    }
}

void ThumbnailVideoPlayer::handle_mpv_event(mpv_event *event) {
    switch (event->event_id) {
    case MPV_EVENT_END_FILE: {
        emit finished();
        break;
    }
    default:
        break;
    }
}

void ThumbnailVideoPlayer::updateFrame() {
    if(!mpv_gl || !m_isPlaying) return;

    uint64_t flags = mpv_render_context_update(mpv_gl);
    if(flags & MPV_RENDER_UPDATE_FRAME) {
        int w = 320;
        int h = 240;

        int video_w = mpv::qt::get_property_variant(mpv, "width").toInt();
        int video_h = mpv::qt::get_property_variant(mpv, "height").toInt();
        if(video_w > 0 && video_h > 0) {
            float aspect = (float)video_w / video_h;
            w = 320;
            h = w / aspect;
        }

        if(m_currentImage.width() != w || m_currentImage.height() != h) {
            m_currentImage = QImage(w, h, QImage::Format_RGB32);
        }

        int pitch = m_currentImage.bytesPerLine();
        void* pixels = m_currentImage.bits();

        int size[2] = {w, h};
        char format[] = "rgb0";
        size_t stride = pitch;

        mpv_render_param params[] = {
            {MPV_RENDER_PARAM_SW_SIZE, size},
            {MPV_RENDER_PARAM_SW_FORMAT, format},
            {MPV_RENDER_PARAM_SW_STRIDE, &stride},
            {MPV_RENDER_PARAM_SW_POINTER, pixels},
            {MPV_RENDER_PARAM_INVALID, nullptr}
        };

        mpv_render_context_render(mpv_gl, params);

        QPixmap pm = QPixmap::fromImage(m_currentImage);
        {
            QMutexLocker lock(&m_mutex);
            m_currentPixmap = pm;
        }
        emit frameReady();
    }
}
