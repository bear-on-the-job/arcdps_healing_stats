// Generated by the gRPC C++ plugin.
// If you make any local change, they will be lost.
// source: evtc_rpc.proto

#include "evtc_rpc.pb.h"
#include "evtc_rpc.grpc.pb.h"

#include <functional>
#include <grpcpp/impl/codegen/async_stream.h>
#include <grpcpp/impl/codegen/async_unary_call.h>
#include <grpcpp/impl/codegen/channel_interface.h>
#include <grpcpp/impl/codegen/client_unary_call.h>
#include <grpcpp/impl/codegen/client_callback.h>
#include <grpcpp/impl/codegen/message_allocator.h>
#include <grpcpp/impl/codegen/method_handler.h>
#include <grpcpp/impl/codegen/rpc_service_method.h>
#include <grpcpp/impl/codegen/server_callback.h>
#include <grpcpp/impl/codegen/server_callback_handlers.h>
#include <grpcpp/impl/codegen/server_context.h>
#include <grpcpp/impl/codegen/service_type.h>
#include <grpcpp/impl/codegen/sync_stream.h>
namespace evtc_rpc {

static const char* evtc_rpc_method_names[] = {
  "/evtc_rpc.evtc_rpc/Connect",
};

std::unique_ptr< evtc_rpc::Stub> evtc_rpc::NewStub(const std::shared_ptr< ::grpc::ChannelInterface>& channel, const ::grpc::StubOptions& options) {
  (void)options;
  std::unique_ptr< evtc_rpc::Stub> stub(new evtc_rpc::Stub(channel));
  return stub;
}

evtc_rpc::Stub::Stub(const std::shared_ptr< ::grpc::ChannelInterface>& channel)
  : channel_(channel), rpcmethod_Connect_(evtc_rpc_method_names[0], ::grpc::internal::RpcMethod::BIDI_STREAMING, channel)
  {}

::grpc::ClientReaderWriter< ::evtc_rpc::Message, ::evtc_rpc::Message>* evtc_rpc::Stub::ConnectRaw(::grpc::ClientContext* context) {
  return ::grpc::internal::ClientReaderWriterFactory< ::evtc_rpc::Message, ::evtc_rpc::Message>::Create(channel_.get(), rpcmethod_Connect_, context);
}

void evtc_rpc::Stub::experimental_async::Connect(::grpc::ClientContext* context, ::grpc::experimental::ClientBidiReactor< ::evtc_rpc::Message,::evtc_rpc::Message>* reactor) {
  ::grpc::internal::ClientCallbackReaderWriterFactory< ::evtc_rpc::Message,::evtc_rpc::Message>::Create(stub_->channel_.get(), stub_->rpcmethod_Connect_, context, reactor);
}

::grpc::ClientAsyncReaderWriter< ::evtc_rpc::Message, ::evtc_rpc::Message>* evtc_rpc::Stub::AsyncConnectRaw(::grpc::ClientContext* context, ::grpc::CompletionQueue* cq, void* tag) {
  return ::grpc::internal::ClientAsyncReaderWriterFactory< ::evtc_rpc::Message, ::evtc_rpc::Message>::Create(channel_.get(), cq, rpcmethod_Connect_, context, true, tag);
}

::grpc::ClientAsyncReaderWriter< ::evtc_rpc::Message, ::evtc_rpc::Message>* evtc_rpc::Stub::PrepareAsyncConnectRaw(::grpc::ClientContext* context, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncReaderWriterFactory< ::evtc_rpc::Message, ::evtc_rpc::Message>::Create(channel_.get(), cq, rpcmethod_Connect_, context, false, nullptr);
}

evtc_rpc::Service::Service() {
  AddMethod(new ::grpc::internal::RpcServiceMethod(
      evtc_rpc_method_names[0],
      ::grpc::internal::RpcMethod::BIDI_STREAMING,
      new ::grpc::internal::BidiStreamingHandler< evtc_rpc::Service, ::evtc_rpc::Message, ::evtc_rpc::Message>(
          [](evtc_rpc::Service* service,
             ::grpc::ServerContext* ctx,
             ::grpc::ServerReaderWriter<::evtc_rpc::Message,
             ::evtc_rpc::Message>* stream) {
               return service->Connect(ctx, stream);
             }, this)));
}

evtc_rpc::Service::~Service() {
}

::grpc::Status evtc_rpc::Service::Connect(::grpc::ServerContext* context, ::grpc::ServerReaderWriter< ::evtc_rpc::Message, ::evtc_rpc::Message>* stream) {
  (void) context;
  (void) stream;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}


}  // namespace evtc_rpc
