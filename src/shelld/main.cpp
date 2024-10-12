#include <QCoreApplication>
#include <memory>

#include "shelld.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    Shelld shelld;

    return app.exec();  // keep running
}
