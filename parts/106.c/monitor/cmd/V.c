
using namespace nc;

void monitor_V(const char * args)
{
  monitor_.disable_.V_ = !monitor_.disable_.V_;
  LOG("disable_={raw_=%i I_=%i V_=%i T_=%i}",
      monitor_.disable_.raw_,
      monitor_.disable_.I_,
      monitor_.disable_.V_,
      monitor_.disable_.T_);
}
