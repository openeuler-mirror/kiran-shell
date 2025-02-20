
#include <QIcon>
#include <QPainter>
#include <QPropertyAnimation>

#include "ks-i.h"
#include "loading-label.h"

static constexpr int ANIMATION_STEP = 10;

LoadingLabel::LoadingLabel(QWidget *parent)
    : QLabel(parent)
{
    setFixedSize(16, 16);

    m_animation = new QPropertyAnimation(this, "loadingIndex");
    m_animation->setDuration(1000);
    m_animation->setLoopCount(-1);

    for (int i = 0; i <= ANIMATION_STEP; ++i)
    {
        m_animation->setKeyValueAt(i / 10.0, i);
    }

    initPixmap();
}

void LoadingLabel::showEvent(QShowEvent *event)
{
    m_animation->start();
}

void LoadingLabel::hideEvent(QHideEvent *event)
{
    m_animation->stop();
}

int LoadingLabel::loadingIndex() const
{
    return m_loadingIndex;
}

void LoadingLabel::initPixmap()
{
    m_pixmaps.clear();
    auto originalPixmap = QIcon::fromTheme(KS_ICON_LOADING).pixmap(size()).scaled(size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);

    for (int i = 0; i < ANIMATION_STEP; i++)
    {
        // 计算旋转角度
        double angle = (360.0 / ANIMATION_STEP) * i;

        // 创建一个固定尺寸的画布
        int originalPixW = originalPixmap.width();
        int originalPixH = originalPixmap.height();
        QImage canvas(originalPixW, originalPixH, QImage::Format_ARGB32_Premultiplied);
        canvas.fill(Qt::transparent);

        // 使用 QPainter 绘制旋转后的图片
        QPainter painter(&canvas);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setRenderHint(QPainter::SmoothPixmapTransform);

        // 将坐标中心移动到画布中心，再进行旋转
        painter.translate(originalPixW / 2.0, originalPixH / 2.0);
        painter.rotate(angle);
        painter.translate(-originalPixW / 2.0, -originalPixH / 2.0);

        // 绘制原始图片
        painter.drawPixmap(0, 0, originalPixmap);
        painter.end();

        m_pixmaps.append(QPixmap::fromImage(canvas));
    }
}

void LoadingLabel::changeIndex(const int &index)
{
    m_loadingIndex = index;
    setPixmap(m_pixmaps[m_loadingIndex]);
}
