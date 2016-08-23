/*
 BasicScreenObject.h

 Base Element for all Elements in the scenegraph, includes almost everything you will ever need.

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

#include "ofMain.h"
#include "ofxTimer.h"
#include "BasicScreenObjectEvent.h"
#include "IPositioner.h"
#include "ofxTweener.h"

#define MASK_TYPE_STENCIL 0
#define MASK_TYPE_SCISSOR 1
#define MASK_TYPE_CLIPPLANES 2



class BasicScreenObject : public ofNode {

public:
  BasicScreenObject();
  virtual ~BasicScreenObject();

  virtual void setup(){};
  virtual void firstUpdate(){};
  virtual void update(){};

  // called by parentobject, draws all children in childlist
  virtual void draw();
  virtual void drawChildren();

  virtual void drawForPicking();
  virtual void drawChildrenForPicking();

  // abstract method, must be overwritten
  virtual void _draw(){}; // the elements private draw method

  virtual void _drawForPicking(){};
  virtual void _drawAfterChildren(){};

  virtual void exit(){};
  virtual void onKill(BasicScreenObjectEvent &event);
  virtual void _onKill();

  // functions called by ofEvents
  void _draw(ofEventArgs &e);
  void _setup(ofEventArgs &e);
  void _update();
//  void _update(ofEventArgs &e);
  void _exit(ofEventArgs &e);

  // Basic Setters and Getters
    void setName(string _name);
//    {
//    myname = _name;
//  };
  string getName() {
    return myname;
  };
  GLint getPickingName() {
    return pickingName;
  }

  void setId(int _id) {
    myid = _id;
  };
  int getId() {
    return myid;
  };

  void isUpdating(bool _isupdating);
  bool isUpdating();

  bool isVisible();
  void isVisible(bool _visible);
  void isParentTreeVisible(bool _visible);
  bool isParentTreeVisible() {
    return _isParentTreeVisible;
  };

  float getCombinedAlpha();

  // Child Parent Root Relations:

  using ofNode::setParent;
  void setParent(BasicScreenObject *_parent);

  using ofNode::getParent;
  BasicScreenObject *getParent();
  BasicScreenObject *getRoot();

  virtual void addChild(BasicScreenObject *_child);
  virtual void addChildAt(BasicScreenObject *_child, int _index);
  virtual void removeChild(BasicScreenObject *_child);
  virtual void removeChildAt(int _index);
  virtual void removeChildren();
  void clearParent();

  virtual void moveMeToTop();
  virtual void moveChildToTop(BasicScreenObject *_child);

  virtual void moveMeToBottom();
  virtual void moveChildToBottom(BasicScreenObject *_child);

  virtual vector<BasicScreenObject *> *getChildren();
  virtual void killMeSoftly(float _time);

  bool isAddedToRenderer();
  void isAddedToRenderer(bool _added);
  // void	setChildrenParentTreeVisibility();
    int getNumChildren();

  void isOrderChildrenByZ(bool _isorderbyz);
  bool isOrderChildrenByZ();
  void doOrderChildrenByZ();

  ofVec3f globalToLocalDir(ofVec3f _global);

  ofVec3f localToGlobal(ofVec3f _local);
  ofVec3f localToGlobal(float _x, float _y, float _z = 0);

  ofVec3f globalToLocal(ofVec3f _global);
  ofVec3f globalToLocal(float _x, float _y, float _z = 0);

  ofVec3f foreignToLocal(BasicScreenObject *_foreign, ofVec3f _foreignpos);
  ofVec3f foreignToLocal(BasicScreenObject *_foreign, float _x, float _y, float _z);

  ofVec3f localToForeign(BasicScreenObject *_foreign, ofVec3f _local);
  ofVec3f localToForeign(BasicScreenObject *_foreign, float _x, float _y, float _z);

  ofRectangle getBoundingBox(BasicScreenObject *ref);

  ofRectangle getScreenBoundingBox();
  ofVec3f getScreenPosition();

  // Show Hide with Calbacks
  void hide();
  void hide(float _time);
  void show();
  void show(float _time);
  virtual void onHide(BasicScreenObjectEvent &event){};
  virtual void onShow(BasicScreenObjectEvent &event){};

  void enableDepthTest();
  void disableDepthTest();
  bool isDepthTestEnabled() {
    return depthtestenabled;
  };

  void enableLighting();
  void disableLighting();
  bool isLightingEnabled() {
    return lightingenabled;
  };

  // Masking
  virtual void setMaskObject(BasicScreenObject *_maskobject);
  void isMask(bool _ismask);
  bool isMask();
  void hasMask(bool _hasmask);
  bool hasMask();
  BasicScreenObject *getMaskObject();
  void setMaskType(int _masktype) {
    masktype = _masktype;
  };

  // GL Functions for Masking;
  void setupMask();
  void restoreMask();

  // Position, Offset and Anchorpoint

  using ofNode::setPosition;
  virtual void setPosition(float _x, float _y);

  virtual void setX(float _x);
  virtual void setY(float _y);
  virtual void setZ(float _z);

  // virtual void setOffset(float _x, float _y, float _z=0);
  // virtual void setOffset(ofVec3f _newoffset);

  // virtual void setAnchorPoint(float _x, float _y, float _z=0);
  // virtual void setAnchorPoint(ofVec3f _newoffset);

  // Attraction Point
  void setMoveAttractionPoint(float _endx, float _endy, float _endz, float _strength = 0.1, float _drag = 0.6);
  void stopMoveAttraction();
  void setRotationAttractionAngles(float _xrot, float _yrot, float _zrot, float _force, bool _inverse = false);
  void stopRotationAttraction();

  void setSpeed(ofVec3f _dir, float _drag = -1) {
    setSpeed(_dir.x, _dir.y, _dir.z, _drag);
  };
  void setSpeed(float _x, float _y, float _z, float _drag = -1);

  void addSpeed(ofVec3f _dir, float _drag = -1) {
    addSpeed(_dir.x, _dir.y, _dir.z, _drag);
  };
  void addSpeed(float _x, float _y, float _z, float _drag = -1);

  // Size and Scale
  virtual void setSize(float _width, float _height);
  virtual void setSize(ofVec2f _size);

  virtual void setSizeToWidth(float _width);
  virtual void setSizeToHeight(float _height);
  virtual void setSizeToObject(BasicScreenObject *_object, ofScaleMode mode = OF_SCALEMODE_FIT);
  virtual void setSizeToRect(float _width, float _height, ofScaleMode mode = OF_SCALEMODE_FIT);

  virtual void centerInObject(BasicScreenObject *_object);
  virtual void centerInRect(ofRectangle targetRect);

  // Color and Alpha and BlendFactors
  virtual void setAlpha(float _a);
  virtual void setColor(float _r, float _g, float _b);
  virtual void setColor(ofColor _c);
  virtual void setNormalizedColor(ofColor _c);
  virtual void setRandomColor();
  void setBlendFactors(float _sfactor, float _dfactor);

  // Rotation

  using ofNode::setOrientation;
  void setOrientation(const ofQuaternion &q, bool _inverse);
  void setOrientation(float _xangle, float _yangle, float _zangle);

  ofVec3f getOrientationAxisAngles();

  void addRotation(float _xrot, float _yrot, float _zrot);
  void setRotationSpeed(float _xrot, float _yrot, float _zrot, float _drag = -1);
  void addRotationSpeed(float _xrot, float _yrot, float _zrot, float _drag = -1);
  void setRotationDrag(float _drag);

  // getters

  // ofVec3f	getOffset();
  // ofVec3f	getAnchorPoint();

  ofVec3f getOrientationAngles() const;

  float getAlpha();
  float getWidth();
  float getHeight();
  ofVec2f getSize();

  ofColor getColor();

  ofVec3f getSpeed() {
    return speed;
  };

  // tweening functions. lots of convenience stuff, and transparent for most ofxTweener properties (see Tweener.addTween())
  void moveTo(float _endx, float _endy, float _endz, float _movetime);
  void moveTo(float _endx, float _endy, float _endz, float _movetime, float (ofxTransitions::*ease)(float, float, float, float));
  void moveTo(float _endx, float _endy, float _endz, float _movetime, float (ofxTransitions::*ease)(float, float, float, float), float delay);

  void moveTo(float _endx, float _endy, float _movetime);
  void moveTo(float _endx, float _endy, float _movetime, float (ofxTransitions::*ease)(float, float, float, float));
  void moveTo(float _endx, float _endy, float _movetime, float (ofxTransitions::*ease)(float, float, float, float), float delay);

  void fadeTo(float _endalpha, float _fadetime);
  void fadeTo(float _endalpha, float _fadetime, float (ofxTransitions::*ease)(float, float, float, float));
  void fadeTo(float _endalpha, float _fadetime, float (ofxTransitions::*ease)(float, float, float, float), float delay);

  void fadeToInvisible(float _fadetime);
  void fadeToInvisible(float _fadetime, float (ofxTransitions::*ease)(float, float, float, float));
  void fadeToInvisible(float _fadetime, float (ofxTransitions::*ease)(float, float, float, float), float delay);

  void scaleTo(float _endxscale, float _endyscale, float _endzscale, float _scaletime);
  void scaleTo(float _endxscale, float _endyscale, float _endzscale, float _scaletime, float (ofxTransitions::*ease)(float, float, float, float));
  void scaleTo(float _endxscale, float _endyscale, float _endzscale, float _scaletime, float (ofxTransitions::*ease)(float, float, float, float), float delay);

  void scaleTo(float _endscale, float _scaletime);
  void scaleTo(float _endscale, float _scaletime, float (ofxTransitions::*ease)(float, float, float, float));
  void scaleTo(float _endscale, float _scaletime, float (ofxTransitions::*ease)(float, float, float, float), float delay);

  void colorTo(float _endr, float _endg, float _endb, float _colortime);
  void colorTo(float _endr, float _endg, float _endb, float _colortime, float (ofxTransitions::*ease)(float, float, float, float));
  void colorTo(float _endr, float _endg, float _endb, float _colortime, float (ofxTransitions::*ease)(float, float, float, float), float delay);

  void colorTo(ofColor _color, float _colortime);
  void colorTo(ofColor _color, float _colortime, float (ofxTransitions::*ease)(float, float, float, float));
  void colorTo(ofColor _color, float _colortime, float (ofxTransitions::*ease)(float, float, float, float), float delay);

  void rotateTo(float _x, float _y, float _z, float _slerptime);
  void rotateTo(float _x, float _y, float _z, float _slerptime, float (ofxTransitions::*ease)(float, float, float, float));
  void rotateTo(float _x, float _y, float _z, float _slerptime, float (ofxTransitions::*ease)(float, float, float, float), float delay);

  void rotateTo(ofQuaternion _quat, float _slerptime);
  void rotateTo(ofQuaternion _quat, float _slerptime, float (ofxTransitions::*ease)(float, float, float, float));
  void rotateTo(ofQuaternion _quat, float _slerptime, float (ofxTransitions::*ease)(float, float, float, float), float delay);

  void sizeTo(float _width, float _height, float _time);
  void sizeTo(float _width, float _height, float _time, float (ofxTransitions::*ease)(float, float, float, float));
  void sizeTo(float _width, float _height, float _time, float (ofxTransitions::*ease)(float, float, float, float), float delay);

  void sizeToWidth(float _width, float _time);
  void sizeToWidth(float _width, float _time, float (ofxTransitions::*ease)(float, float, float, float));
  void sizeToWidth(float _width, float _time, float (ofxTransitions::*ease)(float, float, float, float), float delay);

  void sizeToHeight(float _height, float _time);
  void sizeToHeight(float _height, float _time, float (ofxTransitions::*ease)(float, float, float, float));
  void sizeToHeight(float _height, float _time, float (ofxTransitions::*ease)(float, float, float, float), float delay);

  virtual void onTweenComplete(float &param);

  // Events:
  BasicScreenObjectEvent myEventArgs;

  ofEvent<BasicScreenObjectEvent> moveToCompleteEvent;
  ofEvent<BasicScreenObjectEvent> rotateToCompleteEvent;
  ofEvent<BasicScreenObjectEvent> scaleToCompleteEvent;
  ofEvent<BasicScreenObjectEvent> fadeToCompleteEvent;
  ofEvent<BasicScreenObjectEvent> colorToCompleteEvent;
  ofEvent<BasicScreenObjectEvent> sizeToCompleteEvent;

  ofEvent<BasicScreenObjectEvent> killEvent;
  ofEvent<BasicScreenObjectEvent> hideEvent;
  ofEvent<BasicScreenObjectEvent> showEvent;

  ofEvent<BasicScreenObjectEvent> positionChangedEvent;
  ofEvent<BasicScreenObjectEvent> scaleChangedEvent;
  ofEvent<BasicScreenObjectEvent> rotationChangedEvent;
  ofEvent<BasicScreenObjectEvent> sizeChangedEvent;

  // Positioners
  void addPositioner(string _name, IPositioner *_positioner);
  bool removePositioner(string _name);
  IPositioner *getPositioner(string _name);
  bool hasPositioner(string _name);

  virtual void setRoot(BasicScreenObject *_root);

  bool isMoveTweenActive() {
    return isMoveTweening;
  };
  bool isFadeTweenActive() {
    return isFadeTweening;
  };
  bool isFadeToInvisibleTweenActive() {
    return isFadeTweeningToInvisible;
  };
  bool isSizeTweenActive() {
    return isSizeTweening;
  };
  bool isScaleTweenActive() {
    return isScaleTweening;
  };
  bool isColorTweenActive() {
    return isColorTweening;
  };
  bool isRotateTweenActive() {
    return isRotationTweening;
  };

protected:
  /********************************************************
   *
   *	GENERAL
   *
   ********************************************************/

  long age;
  int myid;
  string myname;

  bool issetup; // TODO: issetup needed??
  bool isupdating;
  bool _isAddedToRenderer;
  bool isorderbyz;
  bool isRenderer;

  /********************************************************
   *
   *	POSITIONING, SCALING, COLOR
   *
   ********************************************************/

  float tweenx, tweeny, tweenz;
  float tweenEndX, tweenEndY, tweenEndZ;
  bool isMoveTweening;

  float tweenscalex, tweenscaley, tweenscalez;
  float tweenEndScaleX, tweenEndScaleY, tweenEndScaleZ;
  bool isScaleTweening;

  float tweenHeight, tweenWidth;
  float tweenEndHeight, tweenEndWidth;
  bool isSizeTweening;

  float width, height;
  ofRectangle bounding;

  // Move
  ofVec3f speed;
  ofVec3f accel;
  float movedrag;
  float moveattractionforce;
  ofVec3f endposition;
  void doMove();

  // Rotation

  ofVec3f around;

  ofQuaternion qx;
  ofQuaternion qy;
  ofQuaternion qz;

  float tweenrotslerp;
  ofQuaternion startquat;
  ofQuaternion endquat;
  bool isRotationTweening;

  ofVec3f rotationspeed;
  ofVec3f rotationaccel;
  float rotationdrag;

  // Rotation Attraction
  ofQuaternion rotationattractionquat;
  ofVec3f rotationattractionangles;
  float rotationattractionforce;
  void doRotate();

  // Color
  ofColor color;
  float tweenr, tweeng, tweenb;
  float tweenEndR, tweenEndG, tweenEndB;
  float tweenEndAlpha;
  bool isColorTweening;
  bool isFadeTweening;
  bool isFadeTweeningToInvisible;

  // Screenobjects:
  BasicScreenObject *root;
  BasicScreenObject *parent_so;
  BasicScreenObject *maskobject;
  vector<BasicScreenObject *> childlist;
  vector<BasicScreenObject *>::iterator childit;

  // ofNode callbacks
  virtual void onPositionChanged();
  virtual void onOrientationChanged();
  virtual void onScaleChanged();

  // map of active positioners
  map<string, IPositioner *> positioners;
  map<string, IPositioner *>::iterator positioner;

  // killing
  ofxTimer killingtimer;
  void killingAfterTimer(ofEventArgs &event);

  // visbility
  ofxTimer visibletimer;
  void hideAfterTimer(ofEventArgs &event);

  bool hasmask;
  bool ismask;
  int masktype;

  bool depthtestenabled;
  bool depthtestbefore;

  bool lightingenabled;
  bool lightingbefore;

  float sfactor;
  float dfactor;

  GLint pickingName;

  void setChildrenParentTreeVisibility();
  void setChildrenParentTreeAddedToRenderer();

private:
  float alpha;
  bool isvisible;
  bool _isParentTreeVisible;
};
