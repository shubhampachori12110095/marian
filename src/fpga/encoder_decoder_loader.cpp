#include "encoder_decoder_loader.h"
#include "encoder_decoder.h"
#include "encoder.h"
#include "decoder.h"
#include "best_hyps.h"
#include "model.h"
#include "common/god.h"
#include "kernel.h"
#include "hello_world.h"

using namespace std;

namespace amunmt {
namespace FPGA {

EncoderDecoderLoader::EncoderDecoderLoader(const std::string name,
                     const YAML::Node& config)
:Loader(name, config)
{
  cerr << "opencl start" << endl;
  openCLInfo_.context = CreateContext(100, openCLInfo_.devices, openCLInfo_.numDevices);
  openCLInfo_.device = openCLInfo_.devices[0];

  cerr << "EncoderDecoderLoader1:" << endl;

  cl_command_queue commands = CreateCommandQueue(openCLInfo_.context, openCLInfo_.devices[0]);
  cl_kernel kernel = CreateKernel("kernels/square.cl", "square", openCLInfo_.context, openCLInfo_.devices[0]);

  cerr << "EncoderDecoderLoader2:" << endl;
  HelloWorld(kernel, openCLInfo_.context, openCLInfo_.devices[0], commands, 1024);
  cerr << "EncoderDecoderLoader3:" << endl;
  HelloWorld(kernel, openCLInfo_.context, openCLInfo_.devices[0], commands, 2048);
  cerr << "EncoderDecoderLoader4:" << endl;


  clReleaseCommandQueue(commands);
  cerr << "EncoderDecoderLoader5:" << endl;
  clReleaseKernel(kernel);

  cerr << "opencl end" << endl;

}



EncoderDecoderLoader::~EncoderDecoderLoader()
{
  clReleaseContext(openCLInfo_.context);
}

void EncoderDecoderLoader::Load(const God &god)
{
  std::string path = Get<std::string>("path");
  //cerr << "path=" << path << endl;

  Weights *weights = new Weights(openCLInfo_.context, openCLInfo_.devices[0], path);
  weights_.reset(weights);
}

ScorerPtr EncoderDecoderLoader::NewScorer(const God &god, const DeviceInfo &deviceInfo) const
{
  size_t d = deviceInfo.deviceId;
  size_t tab = Has("tab") ? Get<size_t>("tab") : 0;

  EncoderDecoder *ed = new EncoderDecoder(god, name_, config_, tab, *weights_, openCLInfo_);
  return ScorerPtr(ed);
}

BestHypsBasePtr EncoderDecoderLoader::GetBestHyps(const God &god) const
{
  BestHyps *obj = new BestHyps();
  return BestHypsBasePtr(obj);
}


}
}

