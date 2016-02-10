/*
 BasicButton

 Button Object, with classic button functionality, and possiblity to set images for all kinds of states

 Copyright (C) 2014 Rob&Rose

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 and associated documentation files (the "Software"), to deal in the Software without restriction,
 including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so,
 subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or substantial
 portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
 TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 IN THE SOFTWARE.
 */

#pragma once
#include "BasicInteractiveObject.h"
#include "Image.h"

class BasicButton : public BasicInteractiveObject {

public:
  BasicButton();
  virtual ~BasicButton(){}; // destructor

  ofEvent<BasicScreenObjectEvent> pressEvent;
  ofEvent<BasicScreenObjectEvent> releaseEvent;

  virtual void onTap(MultiTouchEvent &_event){};
  virtual void onDoubleTap(MultiTouchEvent &_event){};
  virtual void onFirstTouchDown(MultiTouchEvent &_event);
  virtual void onLastTouchUp(MultiTouchEvent &_event);
  virtual void onStartMultiTouchScope(MultiTouchEvent &_event){};

  virtual void setImage(ofImage *_normal, ofImage *_selected = NULL, ofImage *_active = NULL, ofImage *_disabled = NULL);
  virtual void setImage(ofTexture *_normal, ofTexture *_selected = NULL, ofTexture *_active = NULL, ofTexture *_disabled = NULL);
  virtual void toggle();
  virtual void select();
  virtual void deselect();
  virtual void enable();
  virtual void disable();
  virtual void isScalingImage(bool _scale) {
    _isScalingImage = _scale;
  };
  virtual bool isScalingImage() {
    return _isScalingImage;
  };
	virtual void isCircle(bool circle) {
		_isCircle = circle;
	};
	virtual bool isCircle() {
		return _isCircle;
	};

  bool isSelected();

protected:
  virtual void _draw();
  virtual void _drawForPicking();

  string mytxt;
  ofTexture *currentImage;
  ofTexture *active;
  ofTexture *normal;
  ofTexture *selected;
  ofTexture *disabled;
  ofTexture *temp;

  bool isEnabled;
  bool _isSelected;
  bool hasActiveimage;
  bool _isScalingImage;
  bool _isCircle;
};