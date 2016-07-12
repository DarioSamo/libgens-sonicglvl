//===========================================================================
//	  Code modified from: http://www.walletfox.com/course/customqspinbox.php
//===========================================================================

#pragma once

#include <QSpinBox>

class QPowTwoSpinbox : public QSpinBox {
    Q_OBJECT
public:
    QPowTwoSpinbox(QWidget* parent = 0);
public slots:
    void stepBy(int steps);
};
