#include "MyPlugEffect.h"
#include "IPlug_include_in_plug_src.h"



/* https://github.com/iPlug2/iPlug2/wiki/06_Load_a_resource */

MyPlugEffect::MyPlugEffect(const InstanceInfo& info)
: iplug::Plugin(info, MakeConfig(kNumParams, kNumPresets))
{
  GetParam(kGain)->InitDouble("Gain", 0., 0., 100.0, 0.01, "%");
  
}

#if IPLUG_DSP
void MyPlugEffect::ProcessBlock(sample** inputs, sample** outputs, int nFrames)
{
  const double gain = GetParam(kGain)->Value() / 100.;
  const int nChans = NOutChansConnected();
  
  for (int s = 0; s < nFrames; s++) {
    for (int c = 0; c < nChans; c++) {
      outputs[c][s] = inputs[c][s] * gain;
    }
  }
}
#endif

namespace fs = std::filesystem;


#include <algorithm>
#include <filesystem>
#include <string>

#if defined(_WIN32)
  #include <windows.h>
#elif defined(__APPLE__)
  #include <mach-o/dyld.h>
#else
  #include <unistd.h>
#endif

namespace fs = std::filesystem;

std::string get_executable_dir()
{
  char buf[1024];
  uint32_t size = sizeof(buf);
#if defined(_WIN32)
  GetModuleFileNameA(NULL, buf, size);
#elif defined(__APPLE__)
  if (_NSGetExecutablePath(buf, &size) != 0)
    return "";
#else
  ssize_t len = readlink("/proc/self/exe", buf, size - 1);
  if (len != -1)
    buf[len] = '\0';
#endif

  fs::path p(buf);
  return p.parent_path().generic_string(); // generic_string() ensures forward slashes
}



using namespace cycfi::elements;

// Main window background color
auto constexpr bkd_color = rgba(35, 35, 37, 255);
auto background = box(bkd_color);

using slider_ptr = std::shared_ptr<basic_slider_base>;
slider_ptr vsliders[3];

using dial_ptr = std::shared_ptr<basic_dial>;
dial_ptr dials[3];

template <bool is_vertical>
auto make_markers()
{
  auto track = basic_track<5, is_vertical>();
  return slider_labels<10>(slider_marks_lin<40>(track), // Track with marks
                           0.8,                         // Label font size (relative size)
                           "0", "1", "2", "3", "4",     // Labels
                           "5", "6", "7", "8", "9", "10");
}

auto make_vslider(int index)
{
  // 1. Get the directory and append your relative path
  std::string base_dir = get_executable_dir();
  std::string image_path = base_dir + "/resources/images/slider-white.png";
  image slider_knob = image{image_path, 1.0 / 4};

  vsliders[index] = share(slider(align_center(slider_knob), make_markers<true>(), (index + 1) * 0.25));
  return align_center(vmargin({20, 20}, hold(vsliders[index])));
}

auto make_vsliders() { return hmin_size(250, margin_right(10, htile(make_vslider(0), make_vslider(1), make_vslider(2)))); }

auto make_dial(int index)
{
  float const knob_scale = 1.0 / 3;
  // 1. Get the directory and append your relative path
  std::string base_dir = get_executable_dir();
  std::string image_path = base_dir + "/resources/images/knob_sprites_white_128x128.png";

  sprite knob = sprite{image_path.c_str(), 128 * knob_scale, knob_scale};

  dials[index] = share(dial(radial_marks<15>(knob), (index + 1) * 0.25));

  auto markers = radial_labels<15>(hold(dials[index]),
                                   0.7,                     // Label font size (relative size)
                                   "0", "1", "2", "3", "4", // Labels
                                   "5", "6", "7", "8", "9", "10");

  return align_center_middle(markers);
}

auto make_dials() { return hmargin(20, vtile(make_dial(0), make_dial(1), make_dial(2))); }

auto make_controls()
{
  return margin({20, 10, 20, 10},
    vmin_size(350,
      htile(margin({20, 20, 20, 20},
        pane("Sliders", make_vsliders(), 0.8f)), hstretch(0.5, margin({20, 20, 20, 20},
          pane("Knobs", make_dials(), 0.8f))))));
}

void link_control(int index, view& view_)
{
  vsliders[index]->on_change = [index, &view_](double val) {
    dials[index]->basic_dial::value(val);
    view_.refresh(*dials[index]);
  };
  
  dials[index]->on_change = [index, &view_](double val) {
    vsliders[index]->slider_base::value(val);
    view_.refresh(*vsliders[index]);
  };
  
}

void link_controls(view& view_)
{
  link_control(0, view_);
  link_control(1, view_);
  link_control(2, view_);
}




void* MyPlugEffect::OpenWindow(void* pParent) { // ... inside OpenWindow ...
  using namespace cycfi::elements;

  // 1. Force the theme into existence globally
  // We use a static local here to ensure it lives as long as the DLL is loaded
  static theme my_theme;

  auto handle = static_cast<host_view_handle>(pParent);
  
  try
  {
    // 2. Direct allocation
    // If this crashes, the issue is inside the Windows API calls Elements makes
    auto* raw_view = new view(handle);

    // 3. Now it is safe to reset the unique_ptr
    mView.reset(raw_view);

    //mView->content(layer(align_center(label("Elements is Live")), box(colors::black)));
    mView->content(background);

    // 1. Get the directory and append your relative path
    std::string base_dir = get_executable_dir();
    std::string image_path = base_dir + "/resources/images/space.jpg";
    // Inside OpenWindow
    //std::string path = "c:/Development/VSCode/VST/iPlug2Elements/Examples/MyPlugEffect/resources/images/space.jpg";

    if (!fs::exists(image_path))
    {
      // If you see this in your Debugger, Elements definitely won't find it
      OutputDebugStringA("!!! PATH NOT FOUND: ");
      OutputDebugStringA(image_path.c_str());
      OutputDebugStringA("\n");

      // Fallback: Use a simple box so it doesn't crash
      mView->content(box(colors::red));
    }
    else
    {

     
     /* mView->content(layer(align_center(make_controls()), // The slider on top
                           scroller(image{image_path}), // The image in the middle
                           box(colors::black)          // The base background
                           ));*/

      mView->content(make_controls(), hstretch(1.0f,background));
       // The slider on top))
      // File exists, now safe to call the constructor
      //mView->content(scroller(image{image_path}));
      min_size({700, 700}, mView->content());
    }


    link_controls(*mView);

    return pParent;

  }
  catch (...)
  {
    OutputDebugStringA("Elements Constructor Crashed\n");
  }

  return pParent;
}

void MyPlugEffect::CloseWindow() { mView = nullptr; }

void MyPlugEffect::OnUIOpen() {
 

    
}

