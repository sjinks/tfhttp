#include "clienthandler.h"
#include "clienthandler_p.h"

ClientHandler::ClientHandler(
    tls* ctx, const ev::loop_ref& loop, const std::shared_ptr<Server>& server, const std::shared_ptr<Database>& database
)
    : d_ptr(std::make_unique<ClientHandlerPrivate>(this, ctx, loop, server, database))
{}

ClientHandler::~ClientHandler() = default;

int ClientHandler::accept(int fd)
{
    return this->d_func()->accept(fd);
}
