
struct Controller
  : Runnable
{

  enum class State {

    /*
      Check if PS and PL are present and usable.
      Make sure they are turned off.
    */
    init,

    /*
      Waiting for commands from raspberry on uart #0
    */
    idle,

    /*
      test running
    */
    active,
  };
  State state = State::init;

  void on_ps_timeout()
  {
    LOG("");
  }

  void on_el_timeout()
  {
    LOG("");
  }

  void on_loop(uint32_t now_ms) override
  {
    //LOG("");
  }
};
