#include <QSizePolicy>

#include "line-frame.h"
#include "ui_line-frame.h"

namespace Kiran
{
LineFrame::LineFrame(QWidget *parent)
    : KiranColorBlock(parent),
      ui(new Ui::LineFrame)
{
    ui->setupUi(this);
    setRadius(0);
}

LineFrame::~LineFrame()
{
    delete ui;
}

void LineFrame::setFrameShape(QFrame::Shape type)
{
    ui->line->setFrameShape(type);
    if (QFrame::HLine == type)
    {
        ui->gridLayout->setContentsMargins(10, 0, 10, 0);
    }
    else if (QFrame::VLine == type)
    {
        ui->gridLayout->setContentsMargins(0, 10, 0, 10);
    }
}
}  // namespace Kiran
