#pragma once
#define WIN32_LEAN_AND_MEAN 1
#define WDL_NO_SUPPORT_UTF8

#include "IPlug_include_in_plug_hdr.h"
#include <elements.hpp>
using namespace cycfi::elements;
const int kNumPresets = 1;

enum EParams
{
  kGain = 0,
  kNumParams
};

using namespace iplug;
using namespace igraphics;

class MyPlugEffect : public Plugin
{
public:
  MyPlugEffect(const InstanceInfo& info);

    // IEditorDelegate
  void* OpenWindow(void* pParent) override;
  void CloseWindow() override;
  void OnUIOpen() override;
  
#if IPLUG_DSP // http://bit.ly/2S64BDd
  void ProcessBlock(sample** inputs, sample** outputs, int nFrames) override;
#endif

  
private:
  std::unique_ptr<view> mView;
};
