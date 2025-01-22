#ifndef LOADINGLABEL_H
#define LOADINGLABEL_H

#include <QLabel>

class QPropertyAnimation;
class LoadingLabel : public QLabel
{
    Q_OBJECT
    Q_PROPERTY(int loadingIndex READ loadingIndex WRITE changeIndex)
public:
    LoadingLabel(QWidget *parent = nullptr);

protected:
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;

private:
    void changeIndex(const int index);
    int loadingIndex() const;

    void initPixmap();

private:
    int m_loadingIndex;
    QPropertyAnimation *m_animation;
    QVector<QPixmap> m_pixmaps;
};

#endif  // LOADINGLABEL_H
