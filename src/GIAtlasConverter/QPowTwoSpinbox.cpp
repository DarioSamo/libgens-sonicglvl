//===========================================================================
//	  Code modified from: http://www.walletfox.com/course/customqspinbox.php
//===========================================================================

#include "QPowTwoSpinbox.h"

QPowTwoSpinbox::QPowTwoSpinbox(QWidget* parent) : QSpinBox(parent) {
}

void QPowTwoSpinbox::stepBy(int steps) {
  if (steps == 1)
      setValue(value() * 2);
  else if (steps == -1)
      setValue(value() / 2);
  else
      QSpinBox::stepBy(steps);
}