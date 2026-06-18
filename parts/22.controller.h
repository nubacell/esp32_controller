
struct Controller {

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

};
