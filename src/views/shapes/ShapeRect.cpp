#include "ShapeRect.h"

ShapeRect::ShapeRect() {
  mode = OF_RECTMODE_CORNER;
}

void ShapeRect::_draw() {
  ofPushStyle();
  if (_isFilled) {
    ofFill();
  } else {
    ofNoFill();
  }
  ofSetRectMode(mode);
  ofFill();

  ofDrawRectangle(0, 0, width, height);

  if (_isStroked) {
    ofSetLineWidth(_strokeWidth);
    ofNoFill();
    ofSetColor(_strokeColor, getCombinedAlpha());
    ofDrawRectangle(0, 0, width, height);
  }

  ofPopStyle();
}