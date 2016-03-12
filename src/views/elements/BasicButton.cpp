#include "BasicButton.h"

BasicButton::BasicButton() {
  isEnabled = true;
  _isSelected = false;
  hasActiveimage = false;
  _isScalingImage = false;
  _isCircle = false;
  _isFilled = true;

  temp = NULL;
  normal = NULL;
  selected = NULL;
  disabled = NULL;
  active = NULL;
  currentImage = NULL;
}

void BasicButton::onFirstTouchDown(MultiTouchEvent &_event) {
  if (isEnabled) {
    if (hasActiveimage) {
      currentImage = active;
    }
    ofNotifyEvent(pressEvent, myEventArgs, this);
  }
}

void BasicButton::onLastTouchUp(MultiTouchEvent &_event) {
  if (isEnabled) {
    currentImage = temp;
    ofNotifyEvent(releaseEvent, myEventArgs, this);
  }
}

void BasicButton::_draw() {
  if (normal == NULL && _isFilled) {
    if (_isCircle)
      ofDrawCircle(0, 0, width, height);
    else
      ofDrawRectangle(0, 0, width, height);
  }

  if (currentImage != NULL) {
    // ofSetColor(color.r, color.g, color.b, 0);	// TODO: strange stuff with alpha
    if (_isScalingImage) {
      currentImage->draw(0, 0, width, height);
    } else {
      currentImage->draw((width - currentImage->getWidth()) * .5, (height - currentImage->getHeight()) * .5);
    }
  }
}

void BasicButton::_drawForPicking() {
  if (!isEnabled) return;

  if (_isCircle)
    ofDrawCircle(0, 0, width, height);
  else
    ofDrawRectangle(0, 0, width, height);
}

void BasicButton::setImage(ofImage *_normal, ofImage *_selected, ofImage *_active, ofImage *_disabled) {

  ofTexture *normalTex = NULL;
  ofTexture *selectedTex = NULL;
  ofTexture *activeTex = NULL;
  ofTexture *disabledTex = NULL;

  if (_normal) normalTex = &_normal->getTexture();
  if (_selected) selectedTex = &_selected->getTexture();
  if (_active) disabledTex = &_active->getTexture();
  if (_disabled) disabledTex = &_disabled->getTexture();

  setImage(normalTex, selectedTex, activeTex, disabledTex);
}

void BasicButton::setImage(ofTexture *_normal, ofTexture *_selected, ofTexture *_active, ofTexture *_disabled) {

  normal = _normal;
  selected = _selected;

  if (selected == NULL) {
    selected = normal;
  }

  active = _active;

  hasActiveimage = true;
  if (active == NULL) {
    active = normal;
    hasActiveimage = false;
  }

  disabled = _disabled;
  if (disabled == NULL) {
    disabled = normal;
  }
  temp = normal;
  currentImage = normal;
}

void BasicButton::toggle() {
  if (_isSelected) {
    deselect();
  } else {
    select();
  }
}

bool BasicButton::isSelected() {
  return _isSelected;
}

void BasicButton::select() {
  if (_isSelected || !isEnabled)
    return;
  _isSelected = true;
  temp = selected;
  currentImage = selected;
}

void BasicButton::deselect() {
  if (!_isSelected || !isEnabled)
    return;
  _isSelected = false;
  temp = normal;
  currentImage = normal;
}

void BasicButton::disable() {
  if (!isEnabled)
    return;
  isEnabled = false;
  _isSelected = false;
  temp = disabled;
  currentImage = disabled;
}

void BasicButton::enable() {
  if (isEnabled)
    return;
  isEnabled = true;
  temp = normal;
  currentImage = normal;
}