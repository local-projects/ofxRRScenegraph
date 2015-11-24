//
//  Video.cpp
//
//  Created by Matt Felsen on 11/20/15.
//
//

#include "Video.h"

Video::Video() {
  video = new ofVideoPlayer();
  mode = OF_RECTMODE_CORNER;
  tmpFilepath = "";
}

Video::~Video() {
  video->close();
}

void Video::update() {
  video->update();
}

void Video::_draw() {
  if (video->getWidth() > 0) {
    ofPushStyle();

    ofSetRectMode(mode);
    video->draw(0, 0, width, height);
    ofPopStyle();
  }
}

void Video::load(string _filename) {
  video->close();
  delete video;
  video = new ofVideoPlayer();

  video->load(_filename);
  video->play();
  setSize(video->getWidth(), video->getHeight());
  ofNotifyEvent(videoLoadedEvent, myEventArgs, this);
  tmpFilepath = _filename;
}

ofVideoPlayer *Video::getVideoPointer() {
  return video;
}

bool Video::isLoaded() {
  return video->isLoaded();
}

void Video::close() {
  if (isLoaded()) {
    video->close();
  }
}

void Video::setVideoPointer(ofVideoPlayer *_video) {
  video = _video;
}

void Video::setRectMode(ofRectMode _mode) {
  mode = _mode;
}