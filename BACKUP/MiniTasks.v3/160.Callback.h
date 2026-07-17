
typedef void * cb_ctx_t;
typedef void * cb_arg_t;
typedef void (*cb_t)(cb_ctx_t cb_ctx, cb_arg_t cb_arg);

struct Callback
{
  cb_t cb_;
  cb_ctx_t cb_ctx_;

  void operator ()(cb_arg_t cb_arg)
  {
    if (cb_) {
      cb_(cb_ctx_, cb_arg);
    }
  }
};
