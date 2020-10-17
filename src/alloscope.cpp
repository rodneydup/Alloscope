// X/Y Oscilloscope app by Rodney DuPlessis

#include "Gamma/Oscillator.h"
#include "al/app/al_App.hpp"
#include "al/types/al_SingleRWRingBuffer.hpp"
#include "al/ui/al_ParameterGUI.hpp"
#include "shadercode.hpp"
#include "utility.hpp"

using namespace al;

struct Alloscope : public App {
  unsigned int BufferSize = 32768;
  bool drawGUI = false;

  Parameter freq1{"Base Frequency", 2.0f, 0.1f, 500.0f};  // parameter for setting oscillator 1
  Parameter ratio{"Ratio", 1.5f, 1.0f, 10.0f};  // parameter for setting osc 2 relative to osc 1
  Parameter outputVolume{"Output Volume", 0.0f, 0.0f, 1.0f};

  ParameterInt tailLength{"Tail Length", "", 2048, "", 1024, 8192};
  Parameter thickness{"Thickness", 0.8f, 0.05f, 1.5f};
  Parameter scale{"scale", 0.9f, 0.01f, 1.0f};

  ParameterMenu source{"Sound Source", "", 0,
                       ""};  // parameter for switching between internal/external sound source

  ParameterColor color{"Color", "", Color(0.0, 1.0, 0.0)};  // color picker

  gam::Sine<> oscillatorL;       // oscillator 1
  gam::Sine<> oscillatorR;       // oscillator 2
  Mesh scope{Mesh::LINE_STRIP};  // mesh for scope drawing

  RingBuffer BufferX{BufferSize};
  RingBuffer BufferY{BufferSize};

  ShaderProgram lineShader;
  Texture lineTexture;
  Texture pointTexture;

  osc::Recv server;

  void onInit() override {  // Called on app start
    std::cout << "onInit()" << std::endl;
  }

  void onCreate() override {  // Called when graphics context is available
    std::cout << "onCreate()" << std::endl;

    server.open(12000, "127.0.0.1", 0.05);

    server.handler(oscDomain()->handler());
    server.start();

    imguiInit();

    oscillatorL.freq(2);
    oscillatorR.freq(3);

    source.setElements({"External", "Internal"});

    lineShader.compile(shader::vertexCode, shader::fragmentCode, shader::geometryCode);
    // trail.filter(Texture::LINEAR);

    pointTexture.create2D(256, 256, Texture::R8, Texture::RED, Texture::SHORT);
    int Nx = pointTexture.width();
    int Ny = pointTexture.height();
    std::vector<short> alpha;
    alpha.resize(Nx * Ny);
    for (int j = 0; j < Ny; ++j) {
      float y = float(j) / (Ny - 1) * 2 - 1;
      for (int i = 0; i < Nx; ++i) {
        float x = float(i) / (Nx - 1) * 2 - 1;
        float m = exp(-13 * (x * x + y * y));
        m *= pow(2, 15) - 1;  // scale by the largest positive short int
        alpha[j * Nx + i] = m;
      }
    }
    pointTexture.submit(&alpha[0]);
    lineTexture.create1D(256, Texture::R8, Texture::RED, Texture::SHORT);
    std::vector<short> beta;
    beta.resize(lineTexture.width());
    for (int i = 0; i < beta.size(); ++i) {
      beta[i] = alpha[128 * beta.size() + i];
    }
    lineTexture.submit(&beta[0]);
  }

  void onAnimate(double dt) override {  // Called once before drawing
    oscillatorL.freq(freq1);            // update frequency of oscillator 1
    oscillatorR.freq(freq1 * ratio);    // Frequency of oscillator 2
                                        // is determined by ratio to oscillator 1

    scope.reset();  // Remove old vertices from mesh

    float aspect = ((float)height() /
                    (float)width());  // get aspect ratio of window to use to avoid stretched image

    int start = BufferX.getTail() - tailLength;
    if (start < 0) start = BufferSize + start;
    for (unsigned i = 0; i < tailLength;
         i++) {  // this loop updates all the points on the scope trail
      scope.vertex(BufferX[(start + i) % BufferSize] * aspect, BufferY[(start + i) % BufferSize]);
      scope.color(color.get().r, color.get().g, color.get().b,
                  (float(i) / tailLength));  // add color for each vertex
      scope.texCoord(thickness, 0.0);
    }
  }

  void onDraw(Graphics &g) override {  // Draw function
    g.clear(0);
    g.camera(Viewpoint::UNIT_ORTHO_INCLUSIVE);  // set camera position (x: -1 to 1, y: -1 to 1)

    gl::blending(true);  // needed for transparency
    gl::blendTrans();    // needed for transparency

    g.meshColor();         // set scope color (according to color set by color picker)
    lineTexture.bind();    // texture binding
    g.shader(lineShader);  // run shader
    g.draw(scope);         // draw scope
    lineTexture.unbind();

    if (drawGUI) {  // draw the GUI
      imguiBeginFrame();

      ParameterGUI::beginPanel("Configure");
      ImGui::Text("AUDIO");
      ParameterGUI::drawMenu(&source);
      if (source == 0) {
      } else {
        ParameterGUI::drawParameter(&freq1);
        ParameterGUI::drawParameter(&ratio);
      }
      ParameterGUI::drawParameter(&outputVolume);
      ParameterGUI::drawAudioIO(&audioIO());
      ImGui::Separator();
      ImGui::Text("SCOPE");
      ParameterGUI::drawParameterInt(&tailLength, "");
      ParameterGUI::drawParameterColor(&color);
      ParameterGUI::drawParameter(&thickness);
      ParameterGUI::drawParameter(&scale);
      ImGui::Separator();
      ImGui::Separator();
      ImGui::Text("Press g to show/hide gui");
      ImGui::Text("fps: %f", ImGui::GetIO().Framerate);

      ParameterGUI::endPanel();

      imguiEndFrame();
      imguiDraw();
    }
  }

  void onSound(AudioIOData &io) override {  // Audio callback
    while (io()) {
      if (source == 0) {  // if source is external (audio from another program or mic)
        float left = io.in(0);
        float right = io.in(1);
        io.out(0) = left * outputVolume;
        io.out(1) = right * outputVolume;
        BufferX.push_back(left * scale);
        BufferY.push_back(right * scale);
      } else {  // if source is internal (using oscillators generated in this program)
        float left = oscillatorL();
        float right = oscillatorR();
        io.out(0) = left * outputVolume;
        io.out(1) = right * outputVolume;
        BufferX.push_back(left * scale);
        BufferY.push_back(right * scale);
      }
    }
  }

  void onMessage(osc::Message &m) override {
    m.print();
    // Check that the address and tags match what we expect
    if (m.addressPattern() == "/freq") {
      // Extract the data out of the packet
      std::string str;
      float val;

      m >> val;

      // Print out the extracted packet data
      std::cout << " got here: " << val << std::endl;
      val = (val - 200) / 600;
      if (val < 0.2) {
        color = Color(1, val * 5, 0);
      } else if (val < 0.4) {
        val = (val - 0.2) * 5;
        color = Color(1 - val, 1, 0);
      } else if (val < 0.6) {
        val = (val - 0.4) * 5;
        color = Color(0, 1, val);
      } else if (val < 0.8) {
        val = (val - 0.6) * 5;
        color = Color(0, 1 - val, 1);
      } else if (val < 1) {
        val = (val - 0.8) * 5;
        color = Color(val, 0, 1);
      }
    }
  }

  bool onKeyDown(Keyboard const &k) override {
    switch (k.key()) {
      case 'p':
        break;
      case 'g':  // if G is pressed, hide/show the gui
        drawGUI = 1 - drawGUI;
        break;
      case 'r':
        break;
      default:
        break;
    }
    return true;
  }
};

int main() {
  Alloscope app;
  app.title("Alloscope");

  // for (int i = 0; i < AudioDevice::numDevices(); i++) {
  //   printf(" --- [%2d] ", i);
  //   AudioDevice dev(i);
  //   std::string s1 = dev.name();
  //   std::string s2 = "Soundflower";
  //   AudioDevice thing = dev(i);
  // }

  // find soundflower exact name and make that default audio device
  app.configureAudio(48000, 1024, 2, 2);
  app.start();
  return 0;
}
