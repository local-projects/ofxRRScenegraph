#include "BasicScreenObject.h"
#include "Renderer.h"

int totalObjects = 0;
int namedObjectCount = 0;


BasicScreenObject::BasicScreenObject() {
  masktype = MASK_TYPE_CLIPPLANES;
  ismask = false;
  hasmask = false;

  setPosition(0, 0, 0);
  speed.set(0, 0, 0);
  accel.set(0, 0, 0);
  moveattractionforce = 0;
  movedrag = 1;

  setScale(1, 1, 1);
  width = height = 10;

  alpha = 255;
  setColor(255, 255, 255);
  tweenr = 255;
  tweeng = 255;
  tweenb = 255;
  isColorTweening = false;
  isFadeTweening = false;
  isFadeTweeningToInvisible = false;

  // Rotation
  rotationdrag = 0.99;
  rotationspeed.set(0, 0, 0);

  setRotationSpeed(0, 0, 0);
  setRotationDrag(0.99);
  rotationattractionforce = 0;
  rotationattractionquat.set(0, 0, 0, 1);
  rotationattractionangles.set(0, 0, 0);

  isRotationTweening = false;

  lightingbefore = false;
  lightingenabled = false;
  depthtestbefore = false;
  depthtestenabled = false;

  age = 0;
  _isAddedToRenderer = false;
  issetup = false; // TODO: issetup needed??

  parent_so = NULL;
  root = NULL;

  myEventArgs.target = this;

  sfactor = GL_SRC_ALPHA;
  dfactor = GL_ONE_MINUS_SRC_ALPHA;

  tweenx = 0;
  tweeny = 0;
  tweenz = 0;
  isMoveTweening = false;
  isSizeTweening = false;

  tweenscalex = 0;
  tweenscaley = 0;
  tweenscalez = 0;
  tweenrotslerp = 0.0;
  isScaleTweening = false;

  isvisible = true;
  _isParentTreeVisible = true;
  isorderbyz = false;
  isupdating = true;

  isRenderer = false;

//  ofAddListener(ofEvents().setup, this, &BasicScreenObject::_setup);
//  ofAddListener(ofEvents().update, this, &BasicScreenObject::_update);
//  ofAddListener(ofEvents().exit, this, &BasicScreenObject::_exit);
//  ofAddListener(killEvent, this, &BasicScreenObject::onKill);
//  ofAddListener(hideEvent, this, &BasicScreenObject::onHide);
//  ofAddListener(showEvent, this, &BasicScreenObject::onShow);
//  ofAddListener(Tweener.onTweenCompleteEvent, this, &BasicScreenObject::onTweenComplete);
//    cout << "--- " << ++totalObjects << " objects ---" << endl;
}

BasicScreenObject::~BasicScreenObject() {
  childlist.clear();
//  ofRemoveListener(ofEvents().setup, this, &BasicScreenObject::_setup);
//  ofRemoveListener(ofEvents().update, this, &BasicScreenObject::_update);
//  ofRemoveListener(ofEvents().exit, this, &BasicScreenObject::_exit);
//  ofRemoveListener(killEvent, this, &BasicScreenObject::onKill);
//  ofRemoveListener(showEvent, this, &BasicScreenObject::onShow);
//  ofRemoveListener(hideEvent, this, &BasicScreenObject::onHide);
//  ofRemoveListener(Tweener.onTweenCompleteEvent, this, &BasicScreenObject::onTweenComplete);
//    cout << "--- " << --totalObjects << " objects ---" << endl;
}

void BasicScreenObject::setName(string _name) {
    myname = _name;
};

int BasicScreenObject::getNumChildren() {
    int numChildren = childlist.size();
    for (auto child : childlist) {
        numChildren += child->getNumChildren();
    }
    return numChildren;
}

/********************************************************
 *
 *	RENDERING
 *
 ********************************************************/

void BasicScreenObject::enableDepthTest() {
  depthtestenabled = true;
}
void BasicScreenObject::disableDepthTest() {
  depthtestenabled = false;
}

void BasicScreenObject::enableLighting() {
  lightingenabled = true;
}
void BasicScreenObject::disableLighting() {
  lightingenabled = false;
}

void BasicScreenObject::setBlendFactors(float _sfactor, float _dfactor) {
  sfactor = _sfactor;
  dfactor = _dfactor;
}

bool BasicScreenObject::isUpdating() {
  return isupdating;
}
void BasicScreenObject::isUpdating(bool _isupdating) {
  isupdating = _isupdating;
}

void BasicScreenObject::killMeSoftly(float _time) {
  killingtimer.setup(_time / 1000, false);
  killingtimer.stopTimer();
  ofAddListener(killingtimer.TIMER_REACHED, this, &BasicScreenObject::killingAfterTimer);
  killingtimer.startTimer();
}

void BasicScreenObject::killingAfterTimer(ofEventArgs &event) {
  ofNotifyEvent(killEvent, myEventArgs, this);
  _onKill();
}

void BasicScreenObject::onKill(BasicScreenObjectEvent &event) {
  _onKill();
}
void BasicScreenObject::_onKill() {
}

/********************************************************
 *
 *	EVENTS TRIGGERED BY OF
 *
 ********************************************************/

void BasicScreenObject::_setup(ofEventArgs &e) {
  issetup = true;
  setup();
}

void BasicScreenObject::_update() {
  if (!isupdating)
    return;
  if (age == 1)
    firstUpdate();

  // TODO: use Animation elements equal to Positioners Animator.h

  // Update Animations based on Tweening
  if (isMoveTweening)
    setPosition(tweenx, tweeny, tweenz);
  if (isScaleTweening)
    setScale(tweenscalex, tweenscaley, tweenscalez);
  if (isColorTweening)
    setColor(tweenr, tweeng, tweenb);
  if (isSizeTweening)
    setSize(tweenWidth, tweenHeight);
  if (isRotationTweening) {
    ofQuaternion nowquat = getOrientationQuat();
    nowquat.slerp(tweenrotslerp, startquat, endquat);
    setOrientation(nowquat);
  }

  // Animations based on Forces and Attractionpoints
  doRotate();
  doMove();

  if (isorderbyz)
    doOrderChildrenByZ();

  update();

  
    
  if (positioners.size() > 0) {
    for (positioner = positioners.begin(); positioner != positioners.end(); positioner++) {
      IPositioner *p = positioner->second;
      p->restrict(this);
    }
  }

  age++;
    for (auto child : childlist) child->_update();
}

void BasicScreenObject::_exit(ofEventArgs &e) {
  exit();
}

/********************************************************
 *
 *	SCENEGRAPH
 *
 ********************************************************/

BasicScreenObject *BasicScreenObject::getParent() {
  if (parent_so)
    return parent_so;
  return this;
}

void BasicScreenObject::setParent(BasicScreenObject *_parent) {
  _isAddedToRenderer = _parent->isAddedToRenderer();
  parent_so = _parent;
  _isParentTreeVisible = _parent->isParentTreeVisible();
  setChildrenParentTreeVisibility();
  setChildrenParentTreeAddedToRenderer();
  setParent(*dynamic_cast<ofNode *>(_parent));
}

BasicScreenObject *BasicScreenObject::getRoot() {
  if (parent_so == NULL) {
    root = this;
    return root;
  }
  return parent_so->getRoot();
}

void BasicScreenObject::setRoot(BasicScreenObject *_root) {
  root = _root;

  for (int i = 0; i < childlist.size(); i++) {
    childlist[i]->setRoot(_root);
  }
}

void BasicScreenObject::addChild(BasicScreenObject *_child) {
  childlist.push_back(_child);
  _child->setParent(this);
  _child->setRoot(getRoot());
}

void BasicScreenObject::addChildAt(BasicScreenObject *_child, int _index) {
  childit = childlist.begin();
  childlist.insert(childit + _index, _child);
  _child->setParent(this);
  _child->setRoot(getRoot());
}

void BasicScreenObject::removeChild(BasicScreenObject *_child) {
  for (int i = 0; i < childlist.size(); i++) {
    if (childlist[i] == _child) {
    _child->clearParent();
      childlist.erase(childlist.begin() + i);
      break;
    }
  }

}

void BasicScreenObject::removeChildren() {
  childlist.resize(0);
}

void BasicScreenObject::removeChildAt(int _index) {
  childlist.erase(childlist.begin() + _index);
}

void BasicScreenObject::clearParent() {
  _isAddedToRenderer = false;
  setChildrenParentTreeAddedToRenderer();
  ofNode::clearParent();
};

void BasicScreenObject::setChildrenParentTreeAddedToRenderer() {
  for (int i = 0; i < childlist.size(); i++) {
    childlist[i]->isAddedToRenderer(_isAddedToRenderer);
  }
}

void BasicScreenObject::moveMeToTop() {
  if (parent_so != NULL) {
    parent_so->moveChildToTop(this);
  }
}

void BasicScreenObject::moveChildToTop(BasicScreenObject *_child) {
  removeChild(_child);
  addChild(_child);
}

void BasicScreenObject::moveMeToBottom() {
  if (parent_so != NULL) {
    parent_so->moveChildToBottom(this);
  }
}
void BasicScreenObject::moveChildToBottom(BasicScreenObject *_child) {
  removeChild(_child);
  addChildAt(_child, 0);
}

vector<BasicScreenObject *> *BasicScreenObject::getChildren() {
  return &childlist;
}

bool BasicScreenObject::isAddedToRenderer() {
  return _isAddedToRenderer;
}

void BasicScreenObject::isAddedToRenderer(bool _added) {
  _isAddedToRenderer = _added;
  setChildrenParentTreeAddedToRenderer();
};

void BasicScreenObject::isOrderChildrenByZ(bool _isorderbyz) {
  isorderbyz = _isorderbyz;
}
bool BasicScreenObject::isOrderChildrenByZ() {
  return isorderbyz;
}

// TODO: Cameraabhängig machen
void BasicScreenObject::doOrderChildrenByZ() {
  // std::sort(childlist.begin(), childlist.end(), compareZ());
}

/********************************************************
 *
 *	DRAWING FUNCTIONS
 *
 *	invoked by the parent or the root/renderer
 *
 ********************************************************/

void BasicScreenObject::draw() {

  // int elapsed = 0;

  if ((isvisible && _isParentTreeVisible && _isAddedToRenderer) || ismask) {

    glPushMatrix();
    glMultMatrixf(getLocalTransformMatrix().getPtr());

    if (hasmask)
      setupMask();

    glBlendFunc(sfactor, dfactor);

    lightingbefore = glIsEnabled(GL_LIGHTING);
    depthtestbefore = glIsEnabled(GL_DEPTH_TEST);

    if (depthtestenabled && !depthtestbefore)
      glEnable(GL_DEPTH_TEST);
    if (!depthtestenabled && depthtestbefore)
      glDisable(GL_DEPTH_TEST);

    if (lightingenabled && !lightingbefore)
      glEnable(GL_LIGHTING);
    if (!lightingenabled && lightingbefore)
      glDisable(GL_LIGHTING);

    ofPushMatrix();
    ofPushStyle();

    ofSetColor(color.r, color.g, color.b, getCombinedAlpha());
    _draw();

    if (((Renderer*) getRoot())->isDrawDebug()) {
      ofNoFill();
      ofSetColor(ofColor(255, 0.25f * 255));
      ofSetLineWidth(1);
      ofDrawRectangle(0, 0, width, height);

      ofSetColor(255);
      ofDrawBitmapString(myname, 0, 20);
    }

    ofPopStyle();
    ofPopMatrix();

    ofPushMatrix();
    drawChildren();
    ofPopMatrix();
    ofPushMatrix();
    _drawAfterChildren();
    ofPopMatrix();

    // lighting out
    if (lightingenabled && !lightingbefore)
      glDisable(GL_LIGHTING);
    if (!lightingenabled && lightingbefore)
      glEnable(GL_LIGHTING);

    // Depthtest out
    if (depthtestenabled && !depthtestbefore)
      glDisable(GL_DEPTH_TEST);
    if (!depthtestenabled && depthtestbefore)
      glEnable(GL_DEPTH_TEST);

    if (hasmask)
      restoreMask();

    glPopMatrix();
  }
}

void BasicScreenObject::_draw(ofEventArgs &e) {
  _draw();
}

void BasicScreenObject::drawChildren() {
  for (int i = 0; i < childlist.size(); i++) {
    childlist[i]->draw();
  }
}

void BasicScreenObject::drawForPicking() {
  if (isvisible && _isParentTreeVisible && _isAddedToRenderer) {

    glPushMatrix();
    glMultMatrixf(getLocalTransformMatrix().getPtr());

    if (hasmask)
      setupMask();

    depthtestbefore = glIsEnabled(GL_DEPTH_TEST);
    if (depthtestenabled && !depthtestbefore)
      glEnable(GL_DEPTH_TEST);
    if (!depthtestenabled && depthtestbefore)
      glDisable(GL_DEPTH_TEST);

    drawChildrenForPicking();

    if (depthtestenabled && !depthtestbefore) {
      glDisable(GL_DEPTH_TEST);
    }
    if (!depthtestenabled && depthtestbefore) {
      glEnable(GL_DEPTH_TEST);
    }
    if (hasmask)
      restoreMask();
    glPopMatrix();
  }
}

void BasicScreenObject::drawChildrenForPicking() {
  for (int i = 0; i < childlist.size(); i++) {
    childlist[i]->drawForPicking();
  }
}

/********************************************************
 *
 *	COORDINATE SYSTEM HELPERS
 *
 ********************************************************/

ofVec3f BasicScreenObject::localToGlobal(ofVec3f _local) {
  ofVec3f origpoint(_local);
  ofMatrix4x4 global = getGlobalTransformMatrix();
  origpoint = origpoint * global;
  return origpoint;
}

ofVec3f BasicScreenObject::localToGlobal(float _x, float _y, float _z) {
  ofVec3f local(_x, _y, _z);
  return localToGlobal(local);
}

ofVec3f BasicScreenObject::globalToLocal(ofVec3f _global) {
  ofVec3f origpoint(_global);
  ofMatrix4x4 local = getParent()->getGlobalTransformMatrix().getInverse();
  origpoint = origpoint * local;
  return origpoint;
}

ofVec3f BasicScreenObject::globalToLocal(float _x, float _y, float _z) {
  ofVec3f globalpoint(_x, _y, _z);
  return globalToLocal(globalpoint);
}

ofVec3f BasicScreenObject::globalToLocalDir(ofVec3f _globaldir) {
  ofMatrix4x4 local = getParent()->getGlobalTransformMatrix().getInverse();
  local.setTranslation(0, 0, 0);
  return _globaldir * local;
}

ofVec3f BasicScreenObject::foreignToLocal(BasicScreenObject *_foreign, ofVec3f _foreignpos) {
  ofMatrix4x4 inv = getGlobalTransformMatrix().getInverse();
  ofMatrix4x4 trans = _foreign->getGlobalTransformMatrix();
  trans.postMult(inv);
  return _foreignpos * trans;
}

ofVec3f BasicScreenObject::foreignToLocal(BasicScreenObject *_foreign, float _x, float _y, float _z) {
  return foreignToLocal(_foreign, ofVec3f(_x, _y, _z));
}

ofVec3f BasicScreenObject::localToForeign(BasicScreenObject *_foreign, ofVec3f _local) {
  ofMatrix4x4 inv = _foreign->getGlobalTransformMatrix().getInverse();
  ofMatrix4x4 trans = getGlobalTransformMatrix();
  trans.postMult(inv);
  return _local * trans;
}

ofVec3f BasicScreenObject::localToForeign(BasicScreenObject *_foreign, float _x, float _y, float _z) {
  return localToForeign(_foreign, ofVec3f(_x, _y, _z));
}

ofRectangle BasicScreenObject::getBoundingBox(BasicScreenObject *ref) {

  ofVec2f screenpos[4];

  screenpos[0] = localToForeign(ref, 0, 0, 0);
  screenpos[1] = localToForeign(ref, width, 0, 0);
  screenpos[2] = localToForeign(ref, width, height, 0);
  screenpos[3] = localToForeign(ref, 0, height, 0);

  float smallestx = FLT_MAX;
  float smallesty = FLT_MAX;
  float biggestx = -FLT_MAX;
  float biggesty = -FLT_MAX;

  for (int i = 0; i < 4; i++) {
    ofVec2f spos = screenpos[i];

    if (spos.x > biggestx) {
      biggestx = spos.x;
    }
    if (spos.x < smallestx) {
      smallestx = spos.x;
    }
    if (spos.y > biggesty) {
      biggesty = spos.y;
    }
    if (spos.y < smallesty) {
      smallesty = spos.y;
    }
  }

  ofRectangle boundingbox;
  boundingbox.x = smallestx;
  boundingbox.y = smallesty;

  boundingbox.width = biggestx - smallestx;
  boundingbox.height = biggesty - smallesty;

  return boundingbox;
}

ofVec3f BasicScreenObject::getScreenPosition() {
  Renderer *r = (Renderer *)getRoot();
  ofVec3f screenpos = r->getCamera()->worldToScreen(getGlobalPosition());
  screenpos.y = ofGetHeight() - screenpos.y;
  return screenpos;
}

ofRectangle BasicScreenObject::getScreenBoundingBox() {
  Renderer *r = (Renderer *)getRoot();
  ofVec2f screenpos[4];

  screenpos[0] = r->getCamera()->worldToScreen(localToGlobal(0, 0, 0));
  screenpos[1] = r->getCamera()->worldToScreen(localToGlobal(width, 0, 0));
  screenpos[2] = r->getCamera()->worldToScreen(localToGlobal(width, height, 0));
  screenpos[3] = r->getCamera()->worldToScreen(localToGlobal(0, height, 0));

  screenpos[0].y = ofGetHeight() - screenpos[0].y;
  screenpos[1].y = ofGetHeight() - screenpos[1].y;
  screenpos[2].y = ofGetHeight() - screenpos[2].y;
  screenpos[3].y = ofGetHeight() - screenpos[3].y;

  float smallestx = FLT_MAX;
  float smallesty = FLT_MAX;
  float biggestx = -FLT_MAX;
  float biggesty = -FLT_MAX;

  for (int i = 0; i < 4; i++) {
    ofVec2f spos = screenpos[i];

    if (spos.x > biggestx) {
      biggestx = spos.x;
    }
    if (spos.x < smallestx) {
      smallestx = spos.x;
    }
    if (spos.y > biggesty) {
      biggesty = spos.y;
    }
    if (spos.y < smallesty) {
      smallesty = spos.y;
    }
  }

  ofRectangle boundingbox;
  boundingbox.x = smallestx;
  boundingbox.y = smallesty;

  boundingbox.width = biggestx - smallestx;
  boundingbox.height = biggesty - smallesty;

  return boundingbox;
}

/********************************************************
 *
 *	VISIBILITY
 *
 ********************************************************/

bool BasicScreenObject::isVisible() {
  return isvisible;
}
void BasicScreenObject::isVisible(bool _visible) {

  isvisible = _visible;
  setChildrenParentTreeVisibility();
}

void BasicScreenObject::isParentTreeVisible(bool _visible) {
  _isParentTreeVisible = _visible;
  setChildrenParentTreeVisibility();
}

void BasicScreenObject::setChildrenParentTreeVisibility() {
  for (int i = 0; i < childlist.size(); i++) {
    childlist[i]->isParentTreeVisible(_isParentTreeVisible && isvisible);
  }
}

void BasicScreenObject::hide() {
  isVisible(false);
  setAlpha(0);
  ofNotifyEvent(hideEvent, myEventArgs, this);
}

void BasicScreenObject::hide(float _time) {
  fadeTo(0, _time);
  visibletimer.setup(_time / 1000, false);
  visibletimer.stopTimer();
  ofAddListener(visibletimer.TIMER_REACHED, this, &BasicScreenObject::hideAfterTimer);
  visibletimer.startTimer();
}

void BasicScreenObject::hideAfterTimer(ofEventArgs &event) {
  ofRemoveListener(visibletimer.TIMER_REACHED, this, &BasicScreenObject::hideAfterTimer);
  hide();
}

void BasicScreenObject::show() {
  ofNotifyEvent(showEvent, myEventArgs, this);
  ofRemoveListener(visibletimer.TIMER_REACHED, this, &BasicScreenObject::hideAfterTimer);
  isVisible(true);
  setAlpha(255);
}

void BasicScreenObject::show(float _time) {
  ofNotifyEvent(showEvent, myEventArgs, this);
  ofRemoveListener(visibletimer.TIMER_REACHED, this, &BasicScreenObject::hideAfterTimer);
  isVisible(true);
  fadeTo(255, _time);
}

/********************************************************
 *
 *	COLOR
 *
 ********************************************************/

void BasicScreenObject::setColor(float _r, float _g, float _b) {
  color.set(_r, _g, _b);
}
void BasicScreenObject::setColor(ofColor _c) {
  setColor(_c.r, _c.g, _c.b);
}
ofColor BasicScreenObject::getColor() {
  return color;
}

void BasicScreenObject::setNormalizedColor(ofColor _c) {
  setColor(_c.r * 255.0, _c.g * 255.0, _c.b * 255.0);
}

void BasicScreenObject::setRandomColor() {
  setColor(ofRandom(0, 255), ofRandom(0, 255), ofRandom(0, 255));
}

void BasicScreenObject::setAlpha(float _a) {
  alpha = _a;
}
float BasicScreenObject::getAlpha() {
  return alpha;
}

float BasicScreenObject::getCombinedAlpha() {
  float combinedbalpha;
  if (parent != NULL) {
    combinedbalpha = (alpha / 255.0) * parent_so->getCombinedAlpha();
  } else {
    combinedbalpha = alpha;
  }
  return combinedbalpha;
}

/********************************************************
 *
 *	MASK
 *
 ********************************************************/

// TODO: using STENCIL_MASK, drawForPicking() is not masked

void BasicScreenObject::isMask(bool _ismask) {
  ismask = _ismask;
}
bool BasicScreenObject::isMask() {
  return ismask;
}

void BasicScreenObject::hasMask(bool _hasmask) {
  hasmask = _hasmask;
}
bool BasicScreenObject::hasMask() {
  return hasmask;
}

void BasicScreenObject::setMaskObject(BasicScreenObject *_maskobject) {
  maskobject = _maskobject;
  maskobject->isMask(true);
  maskobject->disableDepthTest();
  hasMask(true);
}

BasicScreenObject *BasicScreenObject::getMaskObject() {
  return maskobject;
}

void BasicScreenObject::setupMask() {
  if (masktype == MASK_TYPE_STENCIL) {
    glClear(GL_STENCIL_BUFFER_BIT);
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_ALWAYS, 0x1, 0x1);
    glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
    glColorMask(0, 0, 0, 0);
    glDisable(GL_DEPTH_TEST);
    maskobject->disableDepthTest();
    maskobject->draw();
    glColorMask(1, 1, 1, 1);
    glStencilFunc(GL_EQUAL, 0x1, 0x1);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

  } else if (masktype == MASK_TYPE_SCISSOR) {
    glEnable(GL_SCISSOR_TEST);
    ofVec3f maskpos = maskobject->localToGlobal(0, 0, 0);
    maskpos.y = ofGetHeight() - maskpos.y - maskobject->getHeight();
    glScissor(maskpos.x, maskpos.y, maskobject->getWidth(), maskobject->getHeight());

  } else if (masktype == MASK_TYPE_CLIPPLANES) {
    GLdouble eq0[] = {1, 0, 0, -maskobject->getPosition().x};
    GLdouble eq1[] = {0, 1, 0, -maskobject->getPosition().y};
    GLdouble eq2[] = {-1, 0, 0, maskobject->getWidth() + maskobject->getPosition().x};
    GLdouble eq3[] = {0, -1, 0, maskobject->getHeight() + maskobject->getPosition().y};

    glClipPlane(GL_CLIP_PLANE0, eq0);
    glClipPlane(GL_CLIP_PLANE1, eq1);
    glClipPlane(GL_CLIP_PLANE2, eq2);
    glClipPlane(GL_CLIP_PLANE3, eq3);
    glEnable(GL_CLIP_PLANE0);
    glEnable(GL_CLIP_PLANE1);
    glEnable(GL_CLIP_PLANE2);
    glEnable(GL_CLIP_PLANE3);
  }
}

void BasicScreenObject::restoreMask() {
  if (masktype == MASK_TYPE_STENCIL) {
    glDisable(GL_STENCIL_TEST);
  } else if (masktype == MASK_TYPE_SCISSOR) {
    glDisable(GL_SCISSOR_TEST);
  } else if (masktype == MASK_TYPE_CLIPPLANES) {
    glDisable(GL_CLIP_PLANE0);
    glDisable(GL_CLIP_PLANE1);
    glDisable(GL_CLIP_PLANE2);
    glDisable(GL_CLIP_PLANE3);
  }
}

/********************************************************
 *
 *	POSITION
 *
 ********************************************************/

void BasicScreenObject::setPosition(float _x, float _y) {
  setPosition(_x, _y, getZ());
}
void BasicScreenObject::setX(float _x) {
  setPosition(_x, getY(), getZ());
}
void BasicScreenObject::setY(float _y) {
  setPosition(getX(), _y, getZ());
}
void BasicScreenObject::setZ(float _z) {
  setPosition(getX(), getY(), _z);
}

/********************************************************
 *
 *	SIZE
 *
 ********************************************************/

float BasicScreenObject::getWidth() {
  return width;
}
float BasicScreenObject::getHeight() {
  return height;
}

void BasicScreenObject::setSize(float _width, float _height) {
  int oldWidth = width;
  int oldHeight = height;
  width = _width;
  height = _height;

  if (oldWidth != width || oldHeight != height) {
    ofNotifyEvent(sizeChangedEvent, myEventArgs, this);
  }
}

void BasicScreenObject::setSize(ofVec2f _size) {
  setSize(_size.x, _size.y);
}

void BasicScreenObject::setSizeToWidth(float _width) {
    float scale = _width / getWidth();
    float _height = scale * getHeight();
    setSize(_width, _height);
}

void BasicScreenObject::setSizeToHeight(float _height) {
    float scale = _height / getHeight();
    float _width = scale * getWidth();
    setSize(_width, _height);
}

void BasicScreenObject::setSizeToObject(BasicScreenObject *_object, ofScaleMode mode) {
  setSizeToRect(_object->getWidth(), _object->getHeight(), mode);
}

void BasicScreenObject::setSizeToRect(float _width, float _height, ofScaleMode mode) {
  ofRectangle myRect = ofRectangle(0, 0, getWidth(), getHeight());
  ofRectangle scaleTo = ofRectangle(0, 0, _width, _height);
  myRect.scaleTo(scaleTo, mode);

  setSize(myRect.width, myRect.height);
}

void BasicScreenObject::centerInObject(BasicScreenObject *_object) {
  ofRectangle rect = ofRectangle(_object->getPosition(), _object->getWidth(), _object->getHeight());
  centerInRect(rect);
}

void BasicScreenObject::centerInRect(ofRectangle targetRect) {
  ofRectangle myRect = ofRectangle(getX(), getY(), getWidth(), getHeight());
  myRect.alignTo(targetRect);
  setPosition(myRect.x, myRect.y);
}

ofVec2f BasicScreenObject::getSize() {
  return ofVec2f(width, height);
}

/********************************************************
 *
 *	ROTATION
 *
 ********************************************************/

ofVec3f BasicScreenObject::getOrientationAxisAngles() {

  ofVec3f result;
  ofQuaternion q = getOrientationQuat();

  double x = q.x();
  double y = q.y();
  double z = q.z();
  double w = q.w();

  double sqw = w * w;
  double sqx = x * x;
  double sqy = y * y;
  double sqz = z * z;
  double unit = sqx + sqy + sqz + sqw; // if normalised is one, otherwise is correction factor
  double test = x * y + z * w;

  if (test > 0.499 * unit) { // singularity at north pole
    result.y = 2 * atan2(x, w);
    result.z = PI / 2;
    result.x = 0;

  } else if (test < -0.499 * unit) { // singularity at south pole
    result.y = -2 * atan2(x, w);
    result.z = -PI / 2;
    result.x = 0;

  } else {
    result.y = atan2(2 * y * w - 2 * x * z, sqx - sqy - sqz + sqw);
    result.z = asin(2 * test / unit);
    result.x = atan2(2 * x * w - 2 * y * z, -sqx + sqy - sqz + sqw);
  }
  return result;
}

void BasicScreenObject::setOrientation(const ofQuaternion &q, bool _inverse) {
  ofNode::setOrientation(q);
}

void BasicScreenObject::setOrientation(float _xangle, float _yangle, float _zangle) {
  qx.makeRotate(_xangle, ofVec3f(1, 0, 0));
  qy.makeRotate(_yangle, ofVec3f(0, 1, 0));
  qz.makeRotate(_zangle, ofVec3f(0, 0, 1));

  ofQuaternion nowquat;
  nowquat = qx * qy * qz;
  nowquat.normalize();
  setOrientation(nowquat);
}

ofVec3f BasicScreenObject::getOrientationAngles() const {
  ofVec3f euler = getOrientationEuler();
  return euler;
}

void BasicScreenObject::addRotation(float _xangle, float _yangle, float _zangle) {

  qx.makeRotate(_xangle, ofVec3f(1, 0, 0));
  qy.makeRotate(_yangle, ofVec3f(0, 1, 0));
  qz.makeRotate(_zangle, ofVec3f(0, 0, 1));

  ofQuaternion nowquat = getOrientationQuat();
  nowquat *= qx * qy * qz;
  nowquat.normalize();
  setOrientation(nowquat);
}

void BasicScreenObject::setRotationSpeed(float _xrot, float _yrot, float _zrot, float _drag) {
  if (_drag >= 0) {
    setRotationDrag(_drag);
  }
  rotationspeed.set(_xrot, _yrot, _zrot);
}

void BasicScreenObject::addRotationSpeed(float _xrot, float _yrot, float _zrot, float _drag) {
  if (_drag >= 0) {
    setRotationDrag(_drag);
  }
  rotationaccel.set(_xrot, _yrot, _zrot);
  rotationspeed = rotationspeed + rotationaccel;
}

void BasicScreenObject::setRotationAttractionAngles(float _x, float _y, float _z, float _force, bool _inverse) {

  rotationattractionangles.set(_x, _y, _z);
  rotationattractionforce = _force;

  qx.makeRotate(_x, ofVec3f(1, 0, 0));
  qy.makeRotate(_y, ofVec3f(0, 1, 0));
  qz.makeRotate(_z, ofVec3f(0, 0, 1));
  rotationattractionquat = qx * qy * qz;
  rotationattractionquat.normalize();

  if (_inverse) {
    rotationattractionquat = rotationattractionquat.inverse();
  }
}

void BasicScreenObject::stopRotationAttraction() {
  rotationattractionforce = 0;
}

void BasicScreenObject::setRotationDrag(float _drag) {
  rotationdrag = _drag;
}

void BasicScreenObject::setSpeed(float _x, float _y, float _z, float _drag) {
  speed.set(_x, _y, _z);
  if (_drag >= 0) {
    movedrag = _drag;
  }
}

void BasicScreenObject::addSpeed(float _x, float _y, float _z, float _drag) {
  setSpeed(speed.x + _x, speed.y + _y, speed.z + _z, _drag);
}

/********************************************************
 *
 *	TRANSITIONING
 *
 ********************************************************/

void BasicScreenObject::moveTo(float _endx, float _endy, float _movetime) {
  moveTo(_endx, _endy, getZ(), _movetime, &ofxTransitions::easeInOutCubic, 0);
}
void BasicScreenObject::moveTo(float _endx, float _endy, float _movetime, float (ofxTransitions::*ease)(float, float, float, float)) {
  moveTo(_endx, _endy, getZ(), _movetime, ease, 0);
}
void BasicScreenObject::moveTo(float _endx, float _endy, float _movetime, float (ofxTransitions::*ease)(float, float, float, float), float delay) {
  moveTo(_endx, _endy, getZ(), _movetime, ease, delay);
}
void BasicScreenObject::moveTo(float _endx, float _endy, float _endz, float _movetime) {
  moveTo(_endx, _endy, _endz, _movetime, &ofxTransitions::easeInOutCubic, 0);
}
void BasicScreenObject::moveTo(float _endx, float _endy, float _endz, float _movetime, float (ofxTransitions::*ease)(float, float, float, float)) {
  moveTo(_endx, _endy, _endz, _movetime, ease, 0);
}
void BasicScreenObject::moveTo(float _endx, float _endy, float _endz, float _movetime, float (ofxTransitions::*ease)(float, float, float, float), float delay) {
  setSpeed(0, 0, 0);
  tweenx = getX();
  tweeny = getY();
  tweenz = getZ();
  tweenEndX = _endx;
  tweenEndY = _endy;
  tweenEndZ = _endz;
  Tweener.addTween(this, tweenx, _endx, _movetime / 1000.0, ease, delay / 1000.0);
  Tweener.addTween(this, tweeny, _endy, _movetime / 1000.0, ease, delay / 1000.0);
  Tweener.addTween(this, tweenz, _endz, _movetime / 1000.0, ease, delay / 1000.0);
  isMoveTweening = true;
}

void BasicScreenObject::fadeTo(float _endalpha, float _fadetime) {
  fadeTo(_endalpha, _fadetime, &ofxTransitions::easeInOutCubic, 0);
}
void BasicScreenObject::fadeTo(float _endalpha, float _fadetime, float (ofxTransitions::*ease)(float, float, float, float)) {
  fadeTo(_endalpha, _fadetime, ease, 0);
}
void BasicScreenObject::fadeTo(float _endalpha, float _fadetime, float (ofxTransitions::*ease)(float, float, float, float), float delay) {
  visibletimer.stopTimer();
  tweenEndAlpha = _endalpha;
  Tweener.addTween(this, alpha, _endalpha, _fadetime / 1000.0, ease, delay / 1000.0);
  isFadeTweening = true;
  isFadeTweeningToInvisible = false;
}
void BasicScreenObject::fadeToInvisible(float _fadetime) {
  fadeToInvisible(_fadetime, &ofxTransitions::easeInOutCubic, 0);
}
void BasicScreenObject::fadeToInvisible(float _fadetime, float (ofxTransitions::*ease)(float, float, float, float)) {
  fadeToInvisible(_fadetime, ease, 0);
}
void BasicScreenObject::fadeToInvisible(float _fadetime, float (ofxTransitions::*ease)(float, float, float, float), float delay) {
  visibletimer.stopTimer();
  tweenEndAlpha = 0.0;
  Tweener.addTween(this, alpha, tweenEndAlpha, _fadetime / 1000.0, ease, delay / 1000.0);
  isFadeTweening = true;
  isFadeTweeningToInvisible = true;
}

void BasicScreenObject::scaleTo(float _endscale, float _scaletime) {
  scaleTo(_endscale, _endscale, _endscale, _scaletime, &ofxTransitions::easeInOutCubic, 0);
}
void BasicScreenObject::scaleTo(float _endscale, float _scaletime, float (ofxTransitions::*ease)(float, float, float, float)) {
  scaleTo(_endscale, _endscale, _endscale, _scaletime, ease, 0);
}
void BasicScreenObject::scaleTo(float _endscale, float _scaletime, float (ofxTransitions::*ease)(float, float, float, float), float delay) {
  scaleTo(_endscale, _endscale, _endscale, _scaletime, ease, delay);
}
void BasicScreenObject::scaleTo(float _endxscale, float _endyscale, float _endzscale, float _scaletime) {
  scaleTo(_endxscale, _endyscale, _endzscale, _scaletime, &ofxTransitions::easeInOutCubic, 0);
}
void BasicScreenObject::scaleTo(float _endxscale, float _endyscale, float _endzscale, float _scaletime,
                                float (ofxTransitions::*ease)(float, float, float, float)) {
  scaleTo(_endxscale, _endyscale, _endzscale, _scaletime, ease, 0);
}
void BasicScreenObject::scaleTo(float _endxscale, float _endyscale, float _endzscale, float _scaletime,
                                float (ofxTransitions::*ease)(float, float, float, float), float delay) {
  tweenscalex = getScale().x;
  tweenscaley = getScale().y;
  tweenscalez = getScale().z;
  tweenEndScaleX = _endxscale;
  tweenEndScaleY = _endyscale;
  tweenEndScaleZ = _endzscale;
  Tweener.addTween(this, tweenscalex, _endxscale, _scaletime / 1000.0, ease, delay / 1000.0);
  Tweener.addTween(this, tweenscaley, _endyscale, _scaletime / 1000.0, ease, delay / 1000.0);
  Tweener.addTween(this, tweenscalez, _endzscale, _scaletime / 1000.0, ease, delay / 1000.0);
  isScaleTweening = true;
}

void BasicScreenObject::colorTo(ofColor _color, float _colortime) {
  colorTo(_color.r, _color.g, _color.b, _colortime, &ofxTransitions::easeInOutCubic, 0);
}
void BasicScreenObject::colorTo(ofColor _color, float _colortime, float (ofxTransitions::*ease)(float, float, float, float)) {
  colorTo(_color.r, _color.g, _color.b, _colortime, ease, 0);
}
void BasicScreenObject::colorTo(ofColor _color, float _colortime, float (ofxTransitions::*ease)(float, float, float, float), float delay) {
  colorTo(_color.r, _color.g, _color.b, _colortime, ease, delay);
}
void BasicScreenObject::colorTo(float _endr, float _endg, float _endb, float _colortime) {
  colorTo(_endr, _endg, _endb, _colortime, &ofxTransitions::easeInOutCubic, 0);
}
void BasicScreenObject::colorTo(float _endr, float _endg, float _endb, float _colortime, float (ofxTransitions::*ease)(float, float, float, float)) {
  colorTo(_endr, _endg, _endb, _colortime, ease, 0);
}
void BasicScreenObject::colorTo(float _endr, float _endg, float _endb, float _colortime, float (ofxTransitions::*ease)(float, float, float, float),
                                float delay) {
  tweenr = color.r;
  tweeng = color.g;
  tweenb = color.b;
  tweenEndR = _endr;
  tweenEndG = _endg;
  tweenEndB = _endb;
  Tweener.addTween(this, tweenr, _endr, _colortime / 1000.0, ease, delay / 1000.0);
  Tweener.addTween(this, tweeng, _endg, _colortime / 1000.0, ease, delay / 1000.0);
  Tweener.addTween(this, tweenb, _endb, _colortime / 1000.0, ease, delay / 1000.0);
  isColorTweening = true;
}

void BasicScreenObject::rotateTo(float _x, float _y, float _z, float _slerptime) {
  rotateTo(_x, _y, _z, _slerptime, &ofxTransitions::easeInOutSine, 0);
}
void BasicScreenObject::rotateTo(float _x, float _y, float _z, float _slerptime, float (ofxTransitions::*ease)(float, float, float, float)) {
  rotateTo(_x, _y, _z, _slerptime, ease, 0);
}
void BasicScreenObject::rotateTo(float _x, float _y, float _z, float _slerptime, float (ofxTransitions::*ease)(float, float, float, float), float delay) {
  qx.makeRotate(_x, ofVec3f(1, 0, 0));
  qy.makeRotate(_y, ofVec3f(0, 1, 0));
  qz.makeRotate(_z, ofVec3f(0, 0, 1));
  endquat = qx * qy * qz;
  endquat.normalize();
  rotateTo(endquat, _slerptime, ease, delay);
}
void BasicScreenObject::rotateTo(ofQuaternion _quat, float _slerptime) {
  rotateTo(_quat, _slerptime, &ofxTransitions::easeInOutSine, 0);
}
void BasicScreenObject::rotateTo(ofQuaternion _quat, float _slerptime, float (ofxTransitions::*ease)(float, float, float, float)) {
  rotateTo(_quat, _slerptime, ease, 0);
}
void BasicScreenObject::rotateTo(ofQuaternion _quat, float _slerptime, float (ofxTransitions::*ease)(float, float, float, float), float delay) {
  rotationspeed.set(0, 0, 0);
  stopRotationAttraction();
  tweenrotslerp = 0.0;
  endquat = _quat;
  startquat = getOrientationQuat();
  Tweener.addTween(this, tweenrotslerp, 1.0, _slerptime / 1000.0, ease, delay / 1000.0);
  isRotationTweening = true;
}

void BasicScreenObject::sizeTo(float _width, float _height, float _time) {
  sizeTo(_width, _height, _time, &ofxTransitions::easeInOutCubic, 0);
}
void BasicScreenObject::sizeTo(float _width, float _height, float _time, float (ofxTransitions::*ease)(float, float, float, float)) {
  sizeTo(_width, _height, _time, ease, 0);
}
void BasicScreenObject::sizeTo(float _width, float _height, float _time, float (ofxTransitions::*ease)(float, float, float, float), float delay) {
  tweenWidth = width;
  tweenHeight = height;
  tweenEndWidth = _width;
  tweenEndHeight = _height;
  Tweener.addTween(this, tweenWidth, _width, _time / 1000.0, ease, delay / 1000.0);
  Tweener.addTween(this, tweenHeight, _height, _time / 1000.0, ease, delay / 1000.0);
  isSizeTweening = true;
}

void BasicScreenObject::sizeToWidth(float _width, float _time) {
  sizeToWidth(_width, _time, &ofxTransitions::easeInOutCubic, 0);
}
void BasicScreenObject::sizeToWidth(float _width, float _time, float (ofxTransitions::*ease)(float, float, float, float)) {
  sizeToWidth(_width, _time, ease, 0);
}
void BasicScreenObject::sizeToWidth(float _width, float _time, float (ofxTransitions::*ease)(float, float, float, float), float delay) {
  float scale = _width / getWidth();
  float _height = scale * getHeight();
  sizeTo(_width, _height, _time, ease, delay);
}

void BasicScreenObject::sizeToHeight(float _height, float _time) {
  sizeToHeight(_height, _time, &ofxTransitions::easeInOutCubic, 0);
}
void BasicScreenObject::sizeToHeight(float _height, float _time, float (ofxTransitions::*ease)(float, float, float, float)) {
  sizeToHeight(_height, _time, ease, 0);
}
void BasicScreenObject::sizeToHeight(float _height, float _time, float (ofxTransitions::*ease)(float, float, float, float), float delay) {
  float scale = _height / getHeight();
  float _width = scale * getWidth();
  sizeTo(_width, _height, _time, ease, delay);
}

void BasicScreenObject::setMoveAttractionPoint(float _endx, float _endy, float _endz, float _force, float _drag) {
  endposition.set(_endx, _endy, _endz);
  moveattractionforce = _force;
  movedrag = _drag;
}

void BasicScreenObject::stopMoveAttraction() {
  moveattractionforce = 0;
}

void BasicScreenObject::doRotate() {

  rotationspeed *= rotationdrag;

  if (rotationspeed.length() > 0) {
    addRotation(rotationspeed.x, rotationspeed.y, rotationspeed.z);
  }

  if (rotationattractionforce > 0) {
    ofQuaternion betweenquat = rotationattractionquat - getOrientationQuat();
    float betweenangle;
    ofVec3f dirvec(1, 0, 0);
    betweenquat.getRotate(betweenangle, dirvec);
    ofQuaternion nowquat = getOrientationQuat();
    nowquat.slerp(rotationattractionforce, nowquat, rotationattractionquat);
    setOrientation(nowquat);
  }
}

void BasicScreenObject::doMove() {
  speed *= movedrag;

  if (speed.length() > 0) {
    move(speed.x, speed.y, speed.z);
  }

  if (moveattractionforce > 0) {
    ofVec3f dist;
    dist.set(endposition);
    dist -= getPosition();
    dist *= moveattractionforce;
    addSpeed(dist.x, dist.y, dist.z, movedrag);
  }
}

/********************************************************
 *
 *	POSITIONER SYSTEM
 *
 ********************************************************/

// ofNode callbacks
void BasicScreenObject::onPositionChanged() {
  ofNotifyEvent(positionChangedEvent, myEventArgs, this);
};
void BasicScreenObject::onOrientationChanged() {
  ofNotifyEvent(rotationChangedEvent, myEventArgs, this);
};
void BasicScreenObject::onScaleChanged() {
  ofNotifyEvent(scaleChangedEvent, myEventArgs, this);
};

void BasicScreenObject::addPositioner(string _name, IPositioner *_positioner) {
  positioners[_name] = _positioner;
}

bool BasicScreenObject::removePositioner(string _name) {
  return true;
}

IPositioner *BasicScreenObject::getPositioner(string _name) {
  map<string, IPositioner *>::iterator iter = positioners.find(_name);
  if (iter != positioners.end())
    return iter->second;
  return NULL;
}

bool BasicScreenObject::hasPositioner(string _name) {
  map<string, IPositioner *>::iterator iter = positioners.find(_name);
  if (iter != positioners.end())
    return true;
  return false;
}

/********************************************************
 *
 *	TWEENER EVENTS
 *
 ********************************************************/

void BasicScreenObject::onTweenComplete(float &param) {
  // ofLog(OF_LOG_NOTICE, "TWEEN COMPLETE");

  if (&param == &tweenx || &param == &tweeny || &param == &tweenz) {
    // ofLog(OF_LOG_NOTICE, "move tween complete");
    setPosition(tweenEndX, tweenEndY, tweenEndZ);
    if (isMoveTweening) {
      isMoveTweening = false;
      ofNotifyEvent(moveToCompleteEvent, myEventArgs, this);
    }

  } else if (&param == &tweenscalex || &param == &tweenscaley || &param == &tweenscalez) {
    // ofLog(OF_LOG_NOTICE, "scale tween complete");
    setScale(tweenEndScaleX, tweenEndScaleY, tweenEndScaleZ);
    if (isScaleTweening) {
      isScaleTweening = false;
      ofNotifyEvent(scaleToCompleteEvent, myEventArgs, this);
    }

  } else if (&param == &tweenrotslerp) {
    // ofLog(OF_LOG_NOTICE, "rotation tween complete");
    setOrientation(endquat);
    isRotationTweening = false;
    ofNotifyEvent(rotateToCompleteEvent, myEventArgs, this);

  } else if (&param == &tweenr || &param == &tweeng || &param == &tweenb) {
    // ofLog(OF_LOG_NOTICE, "color tween complete");
    setColor(tweenEndR, tweenEndG, tweenEndB);
    if (isColorTweening) {
      isColorTweening = false;
      ofNotifyEvent(colorToCompleteEvent, myEventArgs, this);
    }

  } else if (&param == &alpha) {
    // ofLog(OF_LOG_NOTICE, "fade tween complete");
    setAlpha(tweenEndAlpha);
    isFadeTweening = false;
    if (isFadeTweeningToInvisible) {
      isVisible(false);
      isFadeTweeningToInvisible = false;
    }
    ofNotifyEvent(fadeToCompleteEvent, myEventArgs, this);

  } else if (&param == &tweenWidth || &param == &tweenHeight) {
    // ofLog(OF_LOG_NOTICE, "size tween complete");
    setSize(tweenEndWidth, tweenEndHeight);
    if (isSizeTweening) {
      isSizeTweening = false;
      ofNotifyEvent(sizeToCompleteEvent, myEventArgs, this);
    }
  }
}