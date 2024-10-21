#ifndef SHELLD_H
#define SHELLD_H

#include <QObject>

class KDirWatch;
class QTimer;

class Shelld : public QObject
{
    Q_OBJECT
public:
    Shelld();
    ~Shelld() override;

private:
    void update(const QString &path);

signals:

private:
    KDirWatch *m_dirWatch = nullptr;
    QTimer *m_timer;
};

#endif  // SHELLD_H
