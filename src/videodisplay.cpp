#include "videodisplay.h"
#include <QPainter>
#include <QPaintEvent>
#include <QSvgRenderer>
#include <QPainterPath>
#include <QFontMetricsF>
#include <algorithm>

VideoDisplay::VideoDisplay(QWidget *parent) : QWidget(parent)
{
    setAttribute(Qt::WA_OpaquePaintEvent, true);
    setAttribute(Qt::WA_NoSystemBackground, true);
    auto palette = this->palette();
    palette.setColor(QPalette::Window, Qt::black);
    setPalette(palette);
    setMouseTracking(true);
}


void VideoDisplay::setBackground(const QPixmap &pixmap)
{
    m_useDefaultBg = false;
    m_currentBg = pixmap;
    update();
}

void VideoDisplay::useDefaultBackground()
{
    m_useDefaultBg = true;
    update();
}

void VideoDisplay::setHasActiveVideo(const bool &value)
{
    if (m_hasActiveVideo != value)
    {
        if (value)
        {
            // Switching to having video.
            // Make sure the background is painted black initially to prevent
            // any old buffer from "bleeding though" when in HW mode.
            m_repaintBackgroundOnce = true;
        }
        m_hasActiveVideo = value;
        update();
    }
}

void VideoDisplay::setOverlay(const QList<VideoOverlayLine> &lines, const QColor &text, const QColor &border)
{
    m_overlay = lines;
    m_overlayText = text;
    m_overlayBorder = border;
    update();
}

void VideoDisplay::clearOverlay()
{
    if (m_overlay.isEmpty())
        return;
    m_overlay.clear();
    update();
}

void VideoDisplay::paintOverlay(QPainter &painter)
{
    if (m_overlay.isEmpty())
        return;
    const qreal maxWidth = width() * 0.92;
    const qreal maxHeight = height() * 0.92;

    // Shrink everything together if the block is too tall or a line too wide.
    auto measure = [&](qreal scale, qreal &totalH, qreal &widest) {
        totalH = 0;
        widest = 0;
        for (const auto &line : m_overlay) {
            QFont f = line.font;
            if (f.pointSizeF() > 0)
                f.setPointSizeF(f.pointSizeF() * scale);
            else if (f.pixelSize() > 0)
                f.setPixelSize(std::max(1, static_cast<int>(f.pixelSize() * scale)));
            QFontMetricsF fm(f, this);
            totalH += fm.height();
            widest = std::max(widest, fm.horizontalAdvance(line.text));
        }
    };
    qreal scale = 1.0;
    qreal totalH, widest;
    measure(scale, totalH, widest);
    if (totalH > maxHeight || widest > maxWidth) {
        scale = std::min(maxHeight / std::max(totalH, 1.0), maxWidth / std::max(widest, 1.0));
        measure(scale, totalH, widest);
    }

    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::TextAntialiasing, true);
    qreal y = (height() - totalH) / 2.0;
    for (const auto &line : m_overlay) {
        QFont f = line.font;
        if (f.pointSizeF() > 0)
            f.setPointSizeF(f.pointSizeF() * scale);
        else if (f.pixelSize() > 0)
            f.setPixelSize(std::max(1, static_cast<int>(f.pixelSize() * scale)));
        QFontMetricsF fm(f, this);
        if (!line.text.isEmpty()) {
            const qreal x = (width() - fm.horizontalAdvance(line.text)) / 2.0;
            QPainterPath path;
            path.addText(x, y + fm.ascent(), f, line.text);
            const qreal borderWidth = std::max(2.0, fm.height() / 9.0);
            QPen pen(m_overlayBorder, borderWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
            painter.strokePath(path, pen);
            painter.fillPath(path, m_overlayText);
        }
        y += fm.height();
    }
    painter.restore();
}

void VideoDisplay::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);

    if (m_hasActiveVideo)
    {
        // playing - fix black bars in hw mode (event is not called in sw mode when playing).
        // (perhaps simplify all this by always using fillOnPaint. Performance should be measured though.)
        if (m_fillOnPaint || m_repaintBackgroundOnce)
        {
            painter.fillRect(event->rect(), Qt::black);
            m_repaintBackgroundOnce = false;
        }
    }
    else
    {
        // stopped - draw background image
        painter.fillRect(event->rect(), Qt::black);
        if (m_useDefaultBg)
        {
            QSvgRenderer renderer(QString(":icons/Icons/okjlogo.svg"));
#if (QT_VERSION >= QT_VERSION_CHECK(5,15,0))
            renderer.setAspectRatioMode(Qt::KeepAspectRatio);
#endif
            renderer.render(&painter);
        }
        else
        {
            painter.drawPixmap(rect(), m_currentBg, m_currentBg.rect());
        }
        paintOverlay(painter);
    }
}
