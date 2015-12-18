//
//  Video.h
//
//  Created by Matt Felsen on 11/20/15.
//
//

#pragma once
#include "BasicScreenObject.h"

class Video : public BasicScreenObject {
public:
  Video();
  virtual ~Video();

  void setup(){};
  void update();

  void load(string _filename);

  void setVideoPointer(ofVideoPlayer *_video);
  ofVideoPlayer *getVideoPointer();

  void setRectMode(ofRectMode _mode);

  bool isLoaded();
  void close();

  ofEvent<BasicScreenObjectEvent> videoLoadedEvent;

protected:
  void _draw();

private:
  ofVideoPlayer *video;

  ofRectMode mode;
  bool pointerSetExternally;

  string tmpFilepath;
};