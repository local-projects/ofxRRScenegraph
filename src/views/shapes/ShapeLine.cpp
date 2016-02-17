//
//  ShapeLine.h
//
//  Created by Matt Felsen on 2/17/16.
//
//

#include "ShapeLine.h"

void ShapeLine::_draw() {
  ofPushStyle();

  ofSetLineWidth(_strokeWidth);
  ofDrawLine(0, 0, endPoint.x, endPoint.y);

  ofPopStyle();
}

void ShapeLine::setEndPoint(ofVec2f end) {
  setEndPoint(end.x, end.y);
}

void ShapeLine::setEndPoint(float x, float y) {
  endPoint.set(x, y);
  setSize(x, y);
}
