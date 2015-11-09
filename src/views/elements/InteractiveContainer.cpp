#include "InteractiveContainer.h"

InteractiveContainer::InteractiveContainer() {
  _isFilled = true;
}

void InteractiveContainer::_draw() {
  if (_isFilled == true) {
    ofDrawRectangle(0, 0, width, height);
  }
}

void InteractiveContainer::_drawForPicking() {
  ofDrawRectangle(0, 0, width, height);
}
