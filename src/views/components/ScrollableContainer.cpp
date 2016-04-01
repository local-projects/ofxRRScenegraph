#include "ScrollableContainer.h"
#include "ofxTransitions.h"

ScrollableContainer::ScrollableContainer() {

  ePadding = 0.0;
  _activeElementNr = 0;
  horizontalMode = false;
  snapToElements = false;
  snapWhileScrolling = false;
  isSnappingToElement = false;
  _swiped = false;
  elementChangedWhileDragging = false;

  scrollBarDisappearTime = 300;
  defaultDisappearTime = 300;
  defaultAppearTime = 300;
  snapToElementsTime = 500;
  easingCurve = &ofxTransitions::easeOutQuint;

  scrollContainer.setName("scrollContainer");
  scrollContainer.isDraggable(true);
  scrollContainer.isSingleTouchDraggable(true);
  scrollContainer.isScaleable(false);
  scrollContainer.isRotateable(false);
  scrollContainer.setDragDamping(0.3);
  scrollContainer.setDragThreshold(10.0);
  scrollContainer.setColor(ofColor(0, 0, 0));
  scrollContainer.isFilled(false);
  addChild(&scrollContainer);

  scrollBarWidth = 2;
  enableScrollBar = true;
  scrollBar.setColor(ofColor::grey);
  scrollBar.setSize(scrollBarWidth, 10);
  addChild(&scrollBar);

  setMaskType(MASK_TYPE_CLIPPLANES);
  setMaskObject(&mask);

  scrollContainer.addPositioner("scrollStop", &scrollRestrictor);

  ofAddListener(scrollContainer.dragStartEvent, this, &ScrollableContainer::onManualScrollStart);
  ofAddListener(scrollContainer.dragStopEvent, this, &ScrollableContainer::onManualScrollStop);
  ofAddListener(scrollContainer.swipeUpEvent, this, &ScrollableContainer::onSwipeUp);
  ofAddListener(scrollContainer.swipeDownEvent, this, &ScrollableContainer::onSwipeDown);
  ofAddListener(scrollContainer.swipeRightEvent, this, &ScrollableContainer::onSwipeRight);
  ofAddListener(scrollContainer.swipeLeftEvent, this, &ScrollableContainer::onSwipeLeft);
  ofAddListener(scrollContainer.positionChangedEvent, this, &ScrollableContainer::onContentsMoved);
  ofAddListener(scrollContainer.tapEvent, this, &ScrollableContainer::onContentTap);
  ofAddListener(scrollContainer.firstTouchDownEvent, this, &ScrollableContainer::onContentTouchDown);
  ofAddListener(scrollContainer.lastTouchUpEvent, this, &ScrollableContainer::onContentTouchUp);
  ofAddListener(scrollContainer.moveToCompleteEvent, this, &ScrollableContainer::onSnapFinished);
}

/***********
 *
 *  PUBLIC
 *
 ***********/

void ScrollableContainer::update() {

  if (_showScrollBar) {

    if (horizontalMode) {
      float scrollBarPos = ofMap(scrollContainer.getX(), 0, contentSize - width, 0, width - scrollBarSize, false);
      scrollBar.setX(-scrollBarPos);
    } else {
      float scrollBarPos = ofMap(scrollContainer.getY(), 0, contentSize - height, 0, height - scrollBarSize, false);
      scrollBar.setY(-scrollBarPos);
    }

    if (!scrollContainer.isDragging()) {
      if (!scrollContainer.isDragging()) {
        if (scrollBar.getAlpha() != 0.0) {
          if (scrollContainer.getSpeed().length() <= .1) {
            if (!scrollBar.isFadeTweenActive()) {
              scrollBar.fadeToInvisible(scrollBarDisappearTime, &ofxTransitions::linear, scrollBarDisappearTime * 2);
            }
          }
        }
      }
    }
  }
}

void ScrollableContainer::flashScrollbar() {
  if (_showScrollBar) {
    scrollBar.isVisible(true);
    scrollBar.fadeTo(255.0, scrollBarAppearTime);
  }
}

void ScrollableContainer::scrollToElement(int _elementNr, int _speed, int offset) {

  if (_elementNr >= elements.size())
    _elementNr = elements.size() - 1;
  if (_elementNr < 0)
    _elementNr = 0;

  float pos = 0;

  for (int i = 0; i < _elementNr; i++) {
    if (horizontalMode) {
      pos += elements[i]->getWidth();
    } else {
      pos += elements[i]->getHeight();
    }
    pos += ePadding;
  }

  // Allow passing in an offset in case you want to do something like scroll
  // to the 3rd element, but have it be 30px from the top of the screen/scroller
  pos -= offset;

  // If speed is 0 then we want to jump directly to the new element
  // Set the active element here *before* calling moveTo() so that no callbacks
  // related to the active element changing get triggered
  if (_speed == 0) {
    _activeElementNr = _elementNr;
  }

  if (horizontalMode) {
    scrollContainer.moveTo(-pos, scrollContainer.getY(), scrollContainer.getZ(), _speed, easingCurve);
  } else {
    scrollContainer.moveTo(scrollContainer.getX(), -pos, scrollContainer.getZ(), _speed, easingCurve);
  }
}

BasicScreenObject* ScrollableContainer::getElementBefore(BasicScreenObject *_element) {
  for (int i = 0; i < elements.size(); i++) {
    if (elements[i] == _element) {
      // if the first element was passed in, there is no element before it!
      if (i == 0) return nullptr;
      // otherwise return the previous one
      else return elements[i-1];
    }
  }
}
BasicScreenObject* ScrollableContainer::getElementAfter(BasicScreenObject *_element) {
  for (int i = 0; i < elements.size(); i++) {
    if (elements[i] == _element) {
      // if the last element was passed in, there is no element after it!
      if (i == elements.size()-1) return nullptr;
      // otherwise return the next one one
      else return elements[i+1];
    }
  }
}

void ScrollableContainer::addElement(BasicScreenObject *_element) {
  if (std::find(elements.begin(), elements.end(), _element) == elements.end()) {
    elements.push_back(_element);
    ofAddListener(_element->sizeChangedEvent, this, &ScrollableContainer::onElementChangedSize);
    scrollContainer.addChild(_element);
    scrollBar.moveMeToTop();
    _updateLayout();
  }
}

void ScrollableContainer::removeElement(BasicScreenObject *_element) {
  for (int i = 0; i < elements.size(); i++) {
    if (elements[i] == _element) {
      ofAddListener(_element->fadeToCompleteEvent, this, &ScrollableContainer::onElementRemoved);
      _element->fadeTo(0.0, defaultDisappearTime);
      break;
    }
  }
}

void ScrollableContainer::clearElements() {

  for (int i = 0; i < elements.size(); i++) {
    ofRemoveListener(elements[i]->sizeChangedEvent, this, &ScrollableContainer::onElementChangedSize);
    scrollContainer.removeChild(elements[i]);
  }
  elements.clear();
  _activeElementNr = 0;
  scrollToElement(0, 1);
  _updateLayout();
}

/***********
 *
 *  INTERNAL
 *
 ***********/

void ScrollableContainer::setSize(float _width, float _height) {
  BasicScreenObject::setSize(_width, _height);
  _updateLayout();
}

void ScrollableContainer::_updateLayout(bool tweened) {

  // find width & hight of all elements in the final layout
  // set position for every element
  float contentPos = 0;
  snapIsFullPage = true;

  for (int i = 0; i < elements.size(); i++) {
    BasicScreenObject *el = elements[i];

    if (horizontalMode) {
      if (!tweened) {
        el->setPosition(contentPos, 0);
      } else {
        el->moveTo(contentPos, 0, defaultAppearTime);
      }
      contentPos += el->getWidth();

      // if elements are smaller than than the scrollable container, then
      // we need to change the snapping behavior
      if (el->getWidth() < width) {
        snapIsFullPage = false;
      }
    } else {
      if (!tweened) {
        el->setPosition(0, contentPos);
      } else {
        el->moveTo(0, contentPos, defaultAppearTime);
      }
      contentPos += el->getHeight();

      // if elements are smaller than than the scrollable container, then
      // we need to change the snapping behavior
      if (el->getHeight() < height) {
        snapIsFullPage = false;
      }
    }

    if (i != elements.size() - 1) {
      contentPos += ePadding;
    }
  }

  contentSize = contentPos;

  // upate scrollContainer-, mask-, and scrollbar-size
  // update the restrictor
  mask.setSize(width, height);
  if (horizontalMode) {
    scrollContainer.setSize(contentPos, height);
    scrollBarSize = width / contentPos * width;
    scrollBar.setSize(scrollBarSize, scrollBarWidth);
    scrollBar.setPosition(0, height - scrollBar.getHeight());

    _showScrollBar = ((enableScrollBar) && (contentSize > width));
    if (!_showScrollBar)
      scrollBar.isVisible(false); // hide when false, handle in update() if true

    float resctrictSize = contentSize - width;
    scrollRestrictor.setRestrictionValue(ofRectangle(-resctrictSize, 0, resctrictSize, 0), ofRectangle(0, 0, 0, 0));
  } else {
    scrollContainer.setSize(width, contentPos);
    scrollBarSize = height / contentPos * height;
    scrollBar.setSize(scrollBarWidth, scrollBarSize);
    scrollBar.setPosition(width - scrollBar.getWidth(), 0);

    _showScrollBar = ((enableScrollBar) && (contentSize > height));
    if (!_showScrollBar)
      scrollBar.isVisible(false); // hide when false, handle in update() if true

    float resctrictSize = contentSize - height;
    scrollRestrictor.setRestrictionValue(ofRectangle(0, -resctrictSize, 0, resctrictSize), ofRectangle(0, 0, 0, 0));
  }

  if (_showScrollBar)
    flashScrollbar();
}

void ScrollableContainer::_updateActiveElement() {
  int newActiveElementNr = _findActiveElement();
  if (newActiveElementNr != _activeElementNr) {
    if (newActiveElementNr == -1) {
      ofLogWarning("ScrollableContainer") << "can't update to element -1 " << getElementCount();
      _activeElementNr = 0;
      return;
    }
    // active element changed
    _activeElementNr = newActiveElementNr;

    if (scrollContainer.isDragging()) {
      elementChangedWhileDragging = true;
    }
    ofLogVerbose("ScrollableContainer") << "active element changed: " << _activeElementNr;
    _handleActiveElementChange();
  }
}

int ScrollableContainer::_findActiveElement() {

  // if all elements are at least as big as the container, then the active
  // element is the one that takes up the most screen space (ie it's at least
  // 50% of the scroll view
  // if the elements are smaller, then the one closest to the top without having
  // been scrolled offscreen yet is the active element

  int active = -1;
  float start = 0;

  if (snapIsFullPage) {
    if (horizontalMode) {
      start = scrollContainer.getX();
    } else {
      start = scrollContainer.getY();
    }

    for (int i = 0; i < elements.size(); i++) {
      if (horizontalMode) {
        start += elements[i]->getWidth();
        start += ePadding;
        if (start >= width * .5) {
          active = i;
          break;
        }
      } else {
        start += elements[i]->getHeight();
        start += ePadding;
        if (start >= height * .5) {
          active = i;
          break;
        }
      }
    }
  }

  else {
    for (int i = 0; i < elements.size(); i++) {
      if (horizontalMode) {
        start += elements[i]->getWidth();
        start += ePadding;
        if (start >= -scrollContainer.getX()) {
          // Commented out since I was getting weird values for a horizontal scroller
          // Just return active + 1
//          if (scrollContainer.getDragSpeed().x < 0)
//            active = i + 1;
//          else
//            active = i;
          active = i + 1;
          break;
        }
      } else {
        start += elements[i]->getHeight();
        start += ePadding;
        // check if we're dragging up (speed > 0) or down (speed < 0)
        if (start >= -scrollContainer.getY()) {
          if (scrollContainer.getDragSpeed().y < 0)
            active = i + 1;
          else
            active = i;
          break;
        }
      }
    }
  }

  if (active > elements.size()-1) {
    active = elements.size()-1;
  }

  if (active == -1 && !snapIsFullPage) {
    active = elements.size()-1;
  }

  if (active == -1) {
    ofLogWarning("ScrollableContainer") << "_findActiveElement: active is -1! bad!";
  }

  return active;
}

void ScrollableContainer::_handleActiveElementChange() {
  BasicScreenObjectEvent event(elements[_activeElementNr]);
  ofNotifyEvent(activeElementChangedEvent, event, this);
  // ofLog(OF_LOG_NOTICE, ofToString(_activeElementNr));
}

void ScrollableContainer::onManualScrollStop(MultiTouchEvent &_event) {
  // ofLog(OF_LOG_NOTICE, "scrollstop");
  _handleRelease();
}

void ScrollableContainer::_handleRelease() {
  if (snapToElements) {
    if (snapWhileScrolling) {
      if (isSnappingToElement) return;

      scrollContainer.isDraggable(true);
      scrollContainer.isSingleTouchDraggable(true);

      // TODO need to snap back to current element
      // maybe just the code below?
    } else {
      if (_swiped) {
        scrollToElement(_activeElementNr, snapToElementsTime);
      } else {
        scrollToElement(_activeElementNr, snapToElementsTime);
      }
    }
  } else {
    scrollContainer.setSpeed(scrollContainer.getDragSpeed(), 0.9);
  }
}

void ScrollableContainer::onManualScrollStart(MultiTouchEvent &_event) {
  // ofLog(OF_LOG_NOTICE, "scrollstart");
  _swiped = false;
  elementChangedWhileDragging = false;

  dragStartPos = horizontalMode ? scrollContainer.getX() : scrollContainer.getY();

  flashScrollbar();
}

void ScrollableContainer::onSwipeUp(MultiTouchEvent &_event) {

  if (snapToElements == true) {
    if (snapWhileScrolling) return;
    if (horizontalMode == false && elementChangedWhileDragging == false && !scrollContainer.isMoveTweenActive()) {
      if (_activeElementNr < elements.size() - 1) {
        _swiped = true;
        _activeElementNr += 1;
      }
    }
  }
}

void ScrollableContainer::onSwipeDown(MultiTouchEvent &_event) {

  if (snapToElements == true) {
    if (snapWhileScrolling) return;
    if (horizontalMode == false && elementChangedWhileDragging == false && !scrollContainer.isMoveTweenActive()) {
      if (_activeElementNr > 0) {
        _swiped = true;
        _activeElementNr -= 1;
      }
    }
  }
}

void ScrollableContainer::onSwipeRight(MultiTouchEvent &_event) {

  if (snapToElements == true) {
    if (snapWhileScrolling) return;
    if (horizontalMode == true && elementChangedWhileDragging == false && !scrollContainer.isMoveTweenActive()) {
      if (_activeElementNr > 0) {
        _swiped = true;
        _activeElementNr -= 1;
      }
    }
  }
}

void ScrollableContainer::onSwipeLeft(MultiTouchEvent &_event) {

  if (snapToElements == true) {
    if (snapWhileScrolling) return;
    if (horizontalMode == true && elementChangedWhileDragging == false && !scrollContainer.isMoveTweenActive()) {
      if (_activeElementNr < elements.size() - 1) {
        _swiped = true;
        _activeElementNr += 1;
      }
    }
  }
}

void ScrollableContainer::onSnapFinished(BasicScreenObjectEvent &event) {
  ofLogVerbose("ScrollableContainer") << "snap finished";

  if (snapToElements && snapWhileScrolling) {
    isSnappingToElement = false;

    // Note: this line shouldn't be necessary, but without it sometiems the
    // container scrolls to a new element right as the snap finishes. Something
    // is probably off when checking states in onContentsMoved(), but leaving
    // this in for now
    dragStartPos = horizontalMode ? scrollContainer.getX() : scrollContainer.getY();

    if (scrollContainer.getActiveMultiTouches()->empty()) {
      scrollContainer.isDraggable(true);
      scrollContainer.isSingleTouchDraggable(true);
    }
  }
}

void ScrollableContainer::onContentsMoved(BasicScreenObjectEvent &event) {
  // if snapping while scrolling is on, check if we should snap to a new element
  if (snapToElements && snapWhileScrolling) {
    if (isSnappingToElement) return;
    if (!scrollContainer.isDraggable()) return;

    float pos = horizontalMode ? scrollContainer.getX() : scrollContainer.getY();
    float delta = pos - dragStartPos;
    int newActiveElementNr = _activeElementNr;

    if (delta > scrollContainer.getDragThreshold()) {
      newActiveElementNr--;
      newActiveElementNr = MAX(newActiveElementNr, 0);
    } else if (delta < -scrollContainer.getDragThreshold()) {
      newActiveElementNr++;
      newActiveElementNr = MIN(newActiveElementNr, elements.size() - 1);
    }

    // TODO if elements are smaller than the scroll container, than you won't be
    // able to scroll to the last several since that would result in blank space
    // after the elements. So, make sure we don't try to scroll to end elements,
    // or add some blank space in updateLayout(), etc.
    if (newActiveElementNr != _activeElementNr) {
      isSnappingToElement = true;
      _activeElementNr = newActiveElementNr;

      scrollContainer.isDraggable(false);
      scrollContainer.isSingleTouchDraggable(false);

      scrollToElement(_activeElementNr, snapToElementsTime);
      _handleActiveElementChange();
    }
  } else {
    _updateActiveElement();
  }
}

BasicScreenObject *ScrollableContainer::_elementByPosition(ofPoint _position) {
  BasicScreenObject *hitElement = NULL;

  for (int i = 0; i < elements.size(); i++) {
    if (horizontalMode) {

      if (_position.x > elements[i]->getX() && _position.x < elements[i]->getX() + elements[i]->getWidth()) {
        hitElement = elements[i];
        break;
      }

    } else {

      if (_position.y > elements[i]->getY() && _position.y < elements[i]->getY() + elements[i]->getHeight()) {
        hitElement = elements[i];
        break;
      }
    }
  }

  return hitElement;
}

void ScrollableContainer::onContentTap(MultiTouchEvent &event) {

  ofPoint hitPoint = event.touchPoint->localoffset;
  BasicScreenObject *hitElement = _elementByPosition(hitPoint);

  if (hitElement != NULL) {
    BasicScreenObjectEvent tapEvent(hitElement);
    ofNotifyEvent(elementTapEvent, tapEvent, this);
  }
}

void ScrollableContainer::onContentTouchDown(MultiTouchEvent &event) {

  ofPoint hitPoint = event.touchPoint->localoffset;
  BasicScreenObject *hitElement = _elementByPosition(hitPoint);

  flashScrollbar();

  if (hitElement != NULL) {
    BasicScreenObjectEvent tapEvent(hitElement);
    ofNotifyEvent(elementFirstTouchDownEvent, tapEvent, this);
  }

  if (scrollStabToStop)
    scrollContainer.setSpeed(0, 0, 0);
}

void ScrollableContainer::onContentTouchUp(MultiTouchEvent &event) {
  _handleRelease();
  ofNotifyEvent(elementLastTouchUpEvent, myEventArgs, this);
}

void ScrollableContainer::onElementChangedSize(BasicScreenObjectEvent &event) {
  _updateLayout();
  // ofLog(OF_LOG_NOTICE, "element changed size");
}

void ScrollableContainer::onElementRemoved(BasicScreenObjectEvent &event) {
  for (int i = 0; i < elements.size(); i++) {
    if (elements[i] == event.target) {
      ofRemoveListener(event.target->sizeChangedEvent, this, &ScrollableContainer::onElementChangedSize);
      ofRemoveListener(event.target->fadeToCompleteEvent, this, &ScrollableContainer::onElementRemoved);
      scrollContainer.removeChild(event.target);
      event.target->setAlpha(255.0);
      elements.erase(elements.begin() + i);
      _updateLayout(true);
      break;
    }
  }
}