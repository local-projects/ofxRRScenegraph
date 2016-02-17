//
//  ShapeLine.h
//
//  Created by Matt Felsen on 2/17/16.
//
//

#pragma once
#include "BasicShape.h"

class ShapeLine : public BasicShape {

public:
  void _draw();
  void setEndPoint(ofVec2f end);
  void setEndPoint(float x, float y);

protected:
  ofVec2f endPoint;
};
