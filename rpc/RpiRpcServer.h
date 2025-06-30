//
// Created by wenyiyu on 2025/6/22.
//

#ifndef RPIRPCSERVER_H
#define RPIRPCSERVER_H
#include "rpi.grpc.pb.h"
#include "rpi.pb.h"

class RpiRpcServer final: RPI_PRC::RpiControllerService::Service {
public:
    grpc::Status GetINA226(grpc::ServerContext* context, const RPI_PRC::EMPTY* request,
        RPI_PRC::INA226Response* response) override;
    grpc::Status SetPinMode(grpc::ServerContext* context, const RPI_PRC::PinModeRequest* request,
        RPI_PRC::EMPTY* response) override;
    grpc::Status SetPinValue(grpc::ServerContext* context, const RPI_PRC::PinValueRequest* request,
        RPI_PRC::EMPTY* response) override;
    grpc::Status GetPinValue(grpc::ServerContext* context, const RPI_PRC::PinNumber* request,
        RPI_PRC::PinValueResponse* response) override;
    grpc::Status GetSGP30(grpc::ServerContext* context, const RPI_PRC::EMPTY* request,
        RPI_PRC::SGP30Response* response) override;
    grpc::Status GetSHT3X(grpc::ServerContext* context, const RPI_PRC::EMPTY* request,
        RPI_PRC::SHT3XResponse* response) override;
};



#endif //RPIRPCSERVER_H
